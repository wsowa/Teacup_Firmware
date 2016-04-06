// display.c

#include <stdio.h>

#include "display.h"
#include "pinio.h"
#include "delay.h"
#include "machine.h"
#include "u8g.h"

#define RST DIO3

u8g_t u8g;
DISPLAY_t display;
uint8_t display_page = 0;
DISPLAY_t old_display;

void display_init(void) {
	
	// Reset pin sequence
	SET_OUTPUT(RST);
	WRITE(RST, 1);
	delay_ms(2);
	WRITE(RST, 0);
	delay_ms(12);
	WRITE(RST, 1);

	// Init display with u8glib
	u8g_InitI2C(&u8g, &u8g_dev_ssd1306_128x32_i2c, U8G_I2C_OPT_NONE);

	display_page = 0;
}

void display_clear(void) {
	DISPLAY_t empty;
	empty.desc_1 = "";
	empty.desc_2 = "";
	empty.text_1 = "";
	empty.text_2 = "";
	display = empty;
}

void draw(char* text, const u8g_fntpgm_uint8_t* font, int x, int y) {
	u8g_SetFont(&u8g, font);
	u8g_DrawStr(&u8g, x, y, text);
}

void _display_4l(char* text, char* text2, char* text3, char* text4) {
	u8g_FirstPage(&u8g);
	do
	{
		draw(text, u8g_font_9x18, 0, 13);
		draw(text2, u8g_font_9x18, 38, 13);
		draw(text3, u8g_font_9x18, 0, 29);
		draw(text4, u8g_font_9x18, 38, 29);
	} while (u8g_NextPage(&u8g));
}

void display_4l(DISPLAY_t *display) {
	_display_4l(display->desc_1, display->text_1, display->desc_2, display->text_2);
}

void _display_2l(char* text, char* text2) {
	u8g_FirstPage(&u8g);
	do
	{
		draw(text, u8g_font_9x18, 0, 23);
		draw(text2, u8g_font_9x18, 38, 23);
	} while (u8g_NextPage(&u8g));
}

void display_2l(DISPLAY_t *display) {
	_display_2l(display->desc_1, display->text_1);
}

void display_show_2(void) {

}

void display_show(void) {
	char buf[16];
	TIME_t m_time;
	switch (display_page) {
		case 255:
			// display_clear();
			// sprintf(buf, "%6d", p1_machine.active);
			// display.desc_1 = "Status: ";
			// display.text_1 = buf;
			display_4l(&display);
			break;
		case 0:
			m_time = get_time();
			sprintf(buf, "%4d:%02d:%02d", m_time.hours, m_time.minutes, m_time.seconds);
			display.desc_1 = "Zeit";
			display.text_1 = buf;
			display_2l(&display);
			break;
		case 1:
			display_clear();
			sprintf(buf, "%14ld", p1_machine.counter);
			display.desc_1 = "Zyklen";
			display.desc_2 = buf;
			display_4l(&display);
			break;
		case 2:
		// actual disabled
			display_clear();
			sprintf(buf, "%11ld ms", p1_machine.m_time);
			display.desc_1 = "Zykluszeit";
			display.desc_2 = buf;
			display_4l(&display);
			break;
	}
}

void display_flash(char* text, char* text2) {
	int8_t i;
	
	for (i=-32; i < 1; i++ ) {
		u8g_FirstPage(&u8g);
		do
		{
			draw(text, u8g_font_9x18, 0, 13+i);
			draw(text2, u8g_font_9x18, 0, 32+i);
		} while (u8g_NextPage(&u8g));
	}
}