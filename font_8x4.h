#ifndef _FONT_8x4_H_
#define _FONT_8x4_H_

#include	<stdint.h>

#define FONT_ROWS 8
#define FONT_COLUMNS 4
#define FONT_SYMBOLS_SPACE 1

typedef struct {
  uint8_t columns;
  uint8_t data[FONT_COLUMNS];
} SYMBOL;

extern SYMBOL font_8x4[];

#endif /* _FONT_8x4_H_ */
