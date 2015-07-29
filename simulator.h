#ifdef SIMULATOR

#undef READ
#undef WRITE
#undef TOGGLE
#undef SET_INPUT
#undef SET_OUTPUT
#undef GET_INPUT
#undef GET_OUTPUT

// Compiler appeasement
#undef disable_transmit
#undef enable_transmit
#define disable_transmit()
#define enable_transmit()
#undef USB_SERIAL

#undef BSS
#ifdef __MACH__  // Mac OS X
  #define BSS __attribute__ ((__section__ ("__DATA,.bss")))
#else
  #define BSS __attribute__ ((__section__ (".bss")))
#endif

#ifndef _SIMULATOR_H
#define _SIMULATOR_H

#include <stdint.h>
#include <stdbool.h>
#include "simulator/data_recorder.h"

#define PROGMEM
#define PGM_P const char *
#define PSTR(x) (x)
#define pgm_read_byte(x) (*((uint8_t *)(x)))
#define pgm_read_word(x) (*((uint16_t *)(x)))
#define pgm_read_dword(x) (*((uint32_t *)(x)))

#define MASK(PIN)   (1 << PIN)
#define ACD         7
#define OCIE1A      1

// TODO: Implement simulated EEMEM persistence
#define EEMEM
#define eeprom_read_dword(ptr32) (*(ptr32))
#define eeprom_read_word(ptr16) (*(ptr16))
#define eeprom_write_dword(ptr32, i32) (*(ptr32)=i32)
#define eeprom_write_word(ptr16, i16) (*(ptr16)=i16)

typedef enum {
  DIO0, DIO1, DIO2, DIO3, DIO4, DIO5, DIO6, DIO7, DIO8, DIO9,
  DIO10, DIO11, DIO12, DIO13, DIO14, DIO15, DIO16, DIO17, DIO18, DIO19,
  DIO20, DIO21, DIO22, DIO23, DIO24, DIO25, DIO26, DIO27, DIO28, DIO29,
  DIO30, DIO31, DIO32, DIO33, DIO34, DIO35, DIO36, DIO37, DIO38, DIO39,
  DIO40, DIO41, DIO42, DIO43, DIO44, DIO45, DIO46, DIO47, DIO48, DIO49,
  DIO50, DIO51, DIO52, DIO53, DIO54, DIO55, DIO56, DIO57, DIO58, DIO59,
  DIO60, DIO61, DIO62, DIO63, DIO64, DIO65, DIO66, DIO67, DIO68, DIO69,
  AIO0, AIO1, AIO2, AIO3, AIO4, AIO5, AIO6, AIO7, AIO8, AIO9,
  AIO10, AIO11, AIO12, AIO13, AIO14, AIO15,
  PIN_NB
} pin_t;

// AVR stand-ins
typedef enum {
  WGM00 = 0,
  WGM01,
  WGM20,
  WGM21,
  CS00 = 0,
  CS02,
  CS20,
  CS21,
  CS22,
} masks_t;

#undef TEMP_PIN_CHANNEL
#define TEMP_PIN_CHANNEL 0

extern uint8_t ACSR;
extern uint8_t TIMSK1;
extern volatile bool sim_interrupts;

bool READ(pin_t pin);
void WRITE(pin_t pin, bool on);
void SET_OUTPUT(pin_t pin);
void SET_INPUT(pin_t pin);

// Simulate AVR interrupts.
#define ISR(fn) void fn (void)
void TIMER1_COMPA_vect(void);
void TIMER1_COMPB_vect(void);

// Compare-timers for next interrupts.
extern uint16_t OCR1A, OCR1B;

// Interrupt control registers.
extern uint16_t TCCR1A, TCCR1B;
enum { CS10 = 1 , OCIE1B = 3 };

#define TCNT1 (sim_tick_counter())
void cli(void);
void sei(void);

#ifdef USE_WATCHDOG
#define wd_init()
#define wd_reset()
#endif

void sim_start(int argc, char ** argv);
void sim_info(const char fmt[], ...);
void sim_debug(const char fmt[], ...);
void sim_error(const char msg[]);
void sim_assert(bool cond, const char msg[]);
void sim_gcode_ch(char ch);
void sim_gcode(const char msg[]);

/**
 * Initialize simulator timer and set time scale.
 *
 * @param scale time slow-down factor; 0=warp-speed, 1=real-time, 2-half-time, etc.
 */
void sim_timer_init(uint8_t scale);

void sim_timer_stop(void);
void sim_timer_set(void);
uint16_t sim_tick_counter(void);
uint64_t sim_runtime_ns(void); ///< Simulated run-time in nanoseconds
void sim_time_warp(void); ///< skip ahead to next timer interrupt, when time_scale==0

#define DIO0_PIN "proof of life"

#endif /* _SIMULATOR_H */
#endif /* SIMULATOR */
