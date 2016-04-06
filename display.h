// display.h

#ifndef _DISPLAY_H
#define _DISPLAY_H

void display_init(void);
void display_flash(char* text, char* text2);
void display_show(void);
void display_clear(void);
//void display_show(char* text, char* text2, char* text3, char* text4);

typedef struct {
	char* desc_1;
	char* text_1;
	char* desc_2;
	char* text_2;
} DISPLAY_t;

extern DISPLAY_t display;
extern uint8_t display_page;

#endif /* _DISPLAY_H */