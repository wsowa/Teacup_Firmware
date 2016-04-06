// machine.c

#include "delay.h"
#include "clock.h"
#include "serial.h"
#include "sersendf.h"
#include "machine.h"
#include "pinio.h"
#include "display.h"
#include "config_wrapper.h"
#include "heater.h"
#include "memory_barrier.h"

TIME_t m_time;
TIME_t clr_time = {0,0,0};

BUTTON_t BSS button_start;
BUTTON_t BSS button_test;
BUTTON_t BSS button_sensor;

DISPLAY_t display_machine;

MACHINE_t BSS p1_machine;

uint32_t display_time;

uint8_t idle = 0;

void wait(uint16_t ms);
void machine_stop(void);

void button_init(void) {
	SET_INPUT(B_START);
	#ifdef B_START_PULLUP
		PULLUP_ON(B_START);
	#endif

	SET_INPUT(B_SENSOR);
	#ifdef B_SENSOR_PULLUP
		PULLUP_ON(B_SENSOR);
	#endif

	SET_INPUT(B_TEST);
	#ifdef B_TEST_PULLUP
		PULLUP_ON(B_TEST);
	#endif

	SET_OUTPUT(DIODE_1);
	SET_OUTPUT(DIODE_2);
	SET_OUTPUT(DIODE_3);

	button_test.value = 0;
}

void counter_add(BUTTON_t *button, uint8_t values) {
	if(button->triggered && !button->wait) {
		button->counter++;
		button->wait = 1;
		button->value++;
		if (! (button->value % values))
			button->value = 0;
		sersendf_P(PSTR("C: %d/%lu/%d/%su\n"), button->triggered, button->counter, button->wait, button->value);
	}
}

void restart_counter(BUTTON_t *button) {
	button->counter = 0;
}

uint8_t read_button_start(void) {
	return READ(B_START);
}

uint8_t read_button_sensor(void) {
	return !READ(B_SENSOR);
}

uint8_t read_button_test(void) {
	return READ(B_TEST);
}

uint8_t (*f)(void);

void read_button(BUTTON_t *button, uint8_t (*f)(void)) {
	if((*f)() == 0) {
		button->status_counter++;
		if(button->status_counter > 10)
			button->status_counter = 10;
		button->reset_counter = 0;
	} else {
		button->reset_counter++;
		if(button->reset_counter > 10)
			button->reset_counter = 10;
		button->status_counter = 0;
	}
}

void set_button_status(BUTTON_t *button, uint8_t values) {
	if(button->status_counter > 4 && button->wait == 0) {
		button->triggered = 1;
	} 
	else if(button->reset_counter > 4) {
		button->triggered = 0;
		button->wait = 0;
	}

	counter_add(button, values);
}

void set_status(void) {
	if (!idle) {
		WRITE(DIODE_1, button_start.triggered);
		WRITE(DIODE_2, button_test.triggered);
		WRITE(DIODE_3, button_sensor.triggered);
	}
}

void button_tick(void) {
	if (!idle) {
		read_button(&button_start, read_button_start);
		set_button_status(&button_start, 3);
		read_button(&button_test, read_button_test);
		set_button_status(&button_test, 3);
		read_button(&button_sensor, read_button_sensor);
		set_button_status(&button_sensor, 1);

		//set_status();
	}
}

void set_time(TIME_t _time) {
	m_time = _time;
}

TIME_t get_time(void) {
	return m_time;
}

void clear_time(void) {
	m_time = clr_time;
}

void wait(uint16_t ms) {
	// never put a wait in the button_tick!!!
	for (;ms > 0;ms--) {
		clock();
		delay_ms(1);
	}
}

void set_dioden(uint8_t status) {
	WRITE(DIODE_1, status);
	WRITE(DIODE_2, status);
	WRITE(DIODE_3, status);
}

void blink(uint16_t ms) {
	set_dioden(0);
	WRITE(DIODE_1, 1);
	wait(ms);
	WRITE(DIODE_1, 0);
	WRITE(DIODE_2, 1);
	wait(ms);
	WRITE(DIODE_2, 0);
	WRITE(DIODE_3, 1);
	wait(ms);
	WRITE(DIODE_3, 0);
}

void dioden_start(void) {
	idle = 1;
	set_dioden(0);
	blink(50);
	blink(50);
	blink(80);
	idle = 0;
}

void countdown(uint8_t counter) {
	wait(100);
	char buf[16];
	uint8_t i = counter;
	for(;i > 0; i--) {
		sprintf(buf, "%6d", i);
		display.text_1 = buf;
		display_show();
		switch(i) {
			case 0:
				set_dioden(0);
			case 1:
				set_dioden(0);
				WRITE(DIODE_3, 1);
			case 2:
				WRITE(DIODE_2, 1);
			case 3:
				WRITE(DIODE_1, 1);
		}
 		wait(500);
 	}
	set_dioden(0);
	display_clear();
	display.desc_1 = "START";
}

void heater_off(void) {
	uint8_t index;
	for (index = 0; index <= NUM_HEATERS; index++) {
		heater_set(index, 0);
	}
	set_dioden(0);
}

#define p1_start() if (m_clock_10ms < 10)
#define p1_timing(start, end) if (m_clock_10ms <= (end) && m_clock_10ms > (start))
#define p1_tick(time) p1_timing((time), ((time)+10))
#define p1_end(end) if (m_clock_10ms > (end))

static uint32_t last_counter = 0;

#define ventil_a(v) WRITE(DIODE_1, (v)); \
heater_set(0, ((v) == 0 ? 0 : 255))
#define ventil_b(v) WRITE(DIODE_2, (v)); \
heater_set(1, ((v) == 0 ? 0 : 255))

#define t_wait 100
#define t_start 30
#define t_pull 400

void machine_pull(void) {
	uint8_t m_active = 0;
	uint32_t m_clock_10ms;
	ATOMIC_START;
	m_active = p1_machine.active;
	m_clock_10ms = p1_machine.clock_10ms;
	if (!m_active) {
		p1_machine.clock_10ms = 0;
		p1_machine.active = 1;
		p1_machine.counter = 0;
		sersendf_P(PSTR("C: %d Start!\n"), last_counter);
		clear_time();
	}
	m_active = p1_machine.active;
	ATOMIC_END;

	if (m_active) {
		p1_start() {
			// display_time = p1_machine.m_time;
			// p1_machine.m_time = 0;
			set_dioden(0);
			heater_off();
		}

		p1_tick(t_wait) {
			// Zylinder B und A-2 drucklos
			sersendf_P(PSTR("valve_b on\n"));
			ventil_b(1);
		}

		p1_tick(t_start) {
			// Pull
			sersendf_P(PSTR("valve_a on\n"));
			ventil_a(1);
		}

		p1_tick(t_pull) {
			// Return
			sersendf_P(PSTR("valve_a off\n"));
			ventil_a(0);
			sersendf_P(PSTR("valve_b off\n"));
			ventil_b(0);
		}

		p1_end(300) {
			if (button_sensor.counter > last_counter) {
				p1_machine.counter++;
				sersendf_P(PSTR("Pull %lu done\n"), p1_machine.counter);
				ATOMIC_START;
				p1_machine.m_time = p1_machine.clock_10ms - 10;
				p1_machine.clock_10ms = 0;
				ATOMIC_END;
				last_counter = button_sensor.counter;
			}
			p1_end(10000)
				machine_stop();
			//p1_machine.active = 0;
		}
	}
}

void machine_start(void) {
	display_clear();
	display_page = 255;
	display.desc_1 = "PULL";
	countdown(3);
	wait(500);
	p1_machine.clock_10ms = 0;
	p1_machine.ready = 1;
}

static char stop_time[16];
void machine_stop(void) {
	display_clear();
	display_page = 255;
	display.desc_1 = "STOP";
	sprintf(stop_time, "%7d:%02d:%02d", m_time.hours, m_time.minutes, m_time.seconds);
	display.desc_2 = stop_time;
	heater_off();
	p1_machine.active = 0;
	p1_machine.ready = 0;
	button_start.value = 0;
}

void machine_prepare(void) {
	display_clear();
	display_page = 255;
	display.desc_1 = "PREPARE";
}

void machine_go(void) {
	if (button_start.wait) {
		switch(button_start.value) {
			case 0:
				machine_stop();
				break;
			case 1:
				machine_prepare();
				DEBUG_LED(1);
				break;
			case 2:
				machine_start();
				DEBUG_LED(0);
				break;
			default:
				machine_stop();
				break;
		}
	}

	if (button_test.wait) {
		display_page = button_test.value;
	}
}

void machine_tick(void) {
	p1_machine.clock_10ms += 10;
	if (p1_machine.ready)
		machine_pull();
}
