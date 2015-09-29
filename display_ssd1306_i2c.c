#include "i2c_bus.h"
#include "display_ssd1306_i2c.h"


void display_init(void) {
  i2c_bus_init(DISPLAY_I2C_ADDRESS, i2c_do_nothing);
}
