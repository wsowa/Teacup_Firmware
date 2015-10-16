#include <string.h>
#include "sersendf.h"
#include "i2c_bus.h"
#include "font_8x4.h"
#include "display_ssd1306_i2c.h"


/**
 * Initializes the display's controller configuring the way of
 * displaying data.
 */
void display_init(void) {
  uint8_t block[] = {
    0x00, // command marker
    0xAE, // display off
    0xD5, 0x80, // display clock divider (reset)
    0xA8, 0x1F, // 1/32 duty
    0x40 | 0x00, // start line (reset)
    0x20, 0x02, // page addressing mode (reset)
    0xA0 | 0x00, // no segment remap (reset)
    0xC0 | 0x00, // normal com pins mapping (reset)
    0xDA, 0x02, // sequental without remap com pins
    0x81, 0x7F, // contrast (reset)
    0xDB, 0x20, // Vcomh (reset)
    0xD9, 0xF1, // precharge period
    0x8D, 0x14, // charge pump
    0xA6, // positive display
    0xA4, // resume display
    0xAF //display on
  };

  i2c_bus_init(DISPLAY_I2C_ADDRESS);

  sersendf_P(PSTR("\nD: display init\n"));
  sersendf_P(PSTR("\nD: block %lx, tx_len %sd\n"), block, sizeof(block));

  i2c_send_to(DISPLAY_I2C_ADDRESS, block, sizeof(block));
}


/**
 * Prints the text at a given position.
 */
void display_text(uint8_t page, uint8_t column, char* message) {
  uint8_t block[128];
  uint8_t* pointer = block + 1;

  // setup cursor on display
  block[0] = 0x00;
  block[1] = 0xB0 | page;
  block[2] = 0x00 | (column & 0x0F);
  block[3] = 0x10 | ((column >> 4) & 0x0F);
  sersendf_P(PSTR("\nD: cursor set\n"));
  i2c_send_to(DISPLAY_I2C_ADDRESS, block, 4);

  //render text to bitmap
  while (*message) {
    SYMBOL symbol = font_8x4[((uint8_t) *message)-0x20];
    memcpy(pointer, symbol.data, symbol.columns);
    pointer += symbol.columns + FONT_SYMBOLS_SPACE;
    message++;
  }
  block[0] = 0x40; // data marker
  sersendf_P(PSTR("\nD: text print, %s\n"), message);
  i2c_send_to(DISPLAY_I2C_ADDRESS, block, pointer-block);
}
