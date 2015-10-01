#include "i2c_bus.h"
#include "display_ssd1306_i2c.h"


void display_init(void) {
  uint8_t commands[] = {
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

  i2c_bus_init(DISPLAY_I2C_ADDRESS, i2c_do_nothing);
  i2c_send_to(DISPLAY_I2C_ADDRESS, commands, sizeof(commands));
}
