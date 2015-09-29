#ifndef _DISPLAY_CONFIG_H
#define _DISPLAY_CONFIG_H

#if DISPLAY_BUS == DISPLAY_BUS_DISABLED
  #warning "DISPLAY BUS is disabled"
#else
  #if DISPLAY_BUS == DISPLAY_BUS_I2CTWI && DISPLAY_TYPE == DISPLAY_TYPE_SSD1306
    #include "../i2c_bus.h"
    #include "ssd1306_i2c.h"
    #warning "DISPLAY BUS with SSD1306"
  #else
    #warning "DISPLAY BUS without SSD1306"
  #endif
  #warning "DISPLAY BUS is not disabled"
#endif /* DISPLAY_BUS */

#endif /* _DISPLAY_CONFIG_H */
