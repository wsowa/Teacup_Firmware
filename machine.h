// machine.h

#ifndef _MACHINE_H
#define _MACHINE_H

#include	<stdint.h>
#include 	<stdio.h>

#define DEBUG_LED(v) WRITE(DIO13,(v))

void button_init(void);
void button_tick(void);
void set_status(void);

typedef struct {
  uint8_t   triggered     :1;
  uint8_t	wait		  :1;
  uint8_t   last_state	  :1;
  uint8_t   wait_gcode	  :1;
  uint8_t   start_done    :1;
  uint8_t   reset_counter;
  uint8_t	status_counter;
  uint8_t   value;
  uint32_t  counter;
} BUTTON_t;

typedef struct {
  uint8_t   active		:1;
  uint8_t 	ready		:1;
  uint32_t  counter;
  uint32_t  m_time;
  uint32_t 	clock_10ms;
} MACHINE_t;

typedef struct {
  uint16_t 	hours;
  uint8_t 	minutes;
  uint8_t 	seconds;
} TIME_t;

void set_time(TIME_t m_time);
TIME_t get_time(void);

extern BUTTON_t button_start;
extern BUTTON_t button_test;
extern BUTTON_t button_sensor;

void dioden_start(void);

void machine_tick(void);
void machine_go(void);
void machine_pull(void);
void queue_machine(void);

extern MACHINE_t p1_machine;

#endif /* _MACHINE_H */