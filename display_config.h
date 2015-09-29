#ifndef _DISPLAY_CONFIG_H
#define _DISPLAY_CONFIG_H

#if DISPLAY_BUS != DISPLAY_BUS_DISABLED
  #if DISPLAY_BUS == DISPLAY_BUS_I2CTWI && DISPLAY_TYPE == DISPLAY_TYPE_SSD1306
    #include "i2c_bus.h"
    #include "display_ssd1306_i2c.h"
  #endif
#endif /* DISPLAY_BUS */

#endif /* _DISPLAY_CONFIG_H */
