// machine.c
#define TEST
#ifndef TEST
#include <stdio.h>

#include "pinio.h"
#include "delay.h"
#include "machine.h"

#include "u8g.h"

#define RST DIO3

u8g_t u8g;

void display_init(void) {
	
	// Reset pin sequence
	// SET_OUTPUT(RST);
	// WRITE(RST, 1);
	// delay_ms(1);
	// WRITE(RST, 0);
	// delay_ms(10);
	// WRITE(RST, 1);

	// Init display with u8glib
	u8g_InitI2C(&u8g, &u8g_dev_ssd1306_128x32_i2c, U8G_I2C_OPT_NONE);
}

void draw(void) {
	u8g_SetFont(&u8g, u8g_font_10x20);
	u8g_DrawStr(&u8g, 0, 20, "Hallo!");
}

void display_show(void) {
	u8g_FirstPage(&u8g);
	do
	{
		draw();
	} while (u8g_NextPage(&u8g));
	u8g_Delay(100);
}
#endif