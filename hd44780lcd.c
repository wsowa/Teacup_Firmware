#include	"gcode_process.h"

/** \file
	\brief Work out what to do with received G-Code commands
*/

#include	<string.h>
#ifndef SIMULATOR
#include	<avr/interrupt.h>
#endif

#include	"gcode_parse.h"

#include	"dda.h"
#include	"dda_queue.h"
#include	"watchdog.h"
#include	"delay.h"
#include	"serial.h"
#include	"sermsg.h"
#include	"temp.h"
#include	"heater.h"
#include	"timer.h"
#include	"sersendf.h"
#include	"pinio.h"
#include	"debug.h"
#include	"clock.h"
#include	"config_wrapper.h"
#include	"home.h"


// the above is copied straight out of gcode_process.c , probably should be cleaned up





void init_lcd(){

	serial_writestr_P(PSTR("Debug lcd_init_start"));

	SET_OUTPUT(BEEPER);
	SET_OUTPUT(LCD_PINS_RS);
	SET_OUTPUT(LCD_PINS_ENABLE);
	SET_OUTPUT(LCD_PINS_D4);
	SET_OUTPUT(LCD_PINS_D5);
	SET_OUTPUT(LCD_PINS_D6);
	SET_OUTPUT(LCD_PINS_D7);
	SET_INPUT(BTN_EN1);
	SET_INPUT(BTN_EN2);
	SET_INPUT(BTN_ENC);

	WRITE(LCD_PINS_RS,0);
	WRITE(LCD_PINS_ENABLE,0);

	write8bits(0x30);
	delay_ms(4500);
	write8bits(0x30);
	delay_ms(4500);
	write8bits(0x30);
	delay_ms(150);
	write8bits(0x08);
	delay_ms(100);
	write8bits(0x01);

				





	serial_writestr_P(PSTR("Debug lcd_init_end"));


}


void write4bits(uint8_t value) {


WRITE(LCD_PINS_D4,((value >> 1)  & 0x01));
WRITE(LCD_PINS_D5,((value >> 2)  & 0x01));
WRITE(LCD_PINS_D6,((value >> 3)  & 0x01));
WRITE(LCD_PINS_D7,((value >> 4)  & 0x01));

WRITE(LCD_PINS_ENABLE,0);
delay_ms(1);
WRITE(LCD_PINS_ENABLE,1);
delay_ms(1);
WRITE(LCD_PINS_ENABLE,0);
delay_ms(100);

}
void write8bits(uint8_t value) {


WRITE(LCD_PINS_D4,((value >> 1)  & 0x01));
WRITE(LCD_PINS_D5,((value >> 2)  & 0x01));
WRITE(LCD_PINS_D6,((value >> 3)  & 0x01));
WRITE(LCD_PINS_D7,((value >> 4)  & 0x01));
WRITE(LCD_PINS_ENABLE,0);
delay_ms(1);
WRITE(LCD_PINS_ENABLE,1);
delay_ms(1);
WRITE(LCD_PINS_ENABLE,0);
delay_ms(100);
WRITE(LCD_PINS_D4,((value >> 5)  & 0x01));
WRITE(LCD_PINS_D5,((value >> 6)  & 0x01));
WRITE(LCD_PINS_D6,((value >> 7)  & 0x01));
WRITE(LCD_PINS_D7,((value >> 8)  & 0x01));
WRITE(LCD_PINS_ENABLE,0);
delay_ms(1);
WRITE(LCD_PINS_ENABLE,1);
delay_ms(1);
WRITE(LCD_PINS_ENABLE,0);
delay_ms(100);
}
