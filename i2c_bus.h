#ifndef _I2C_BUS_H
#define _I2C_BUS_H

#include <stdint.h>

// uncomment if we act as slave device
// #define I2C_SLAVE_MODE
// uncomment if we act as master device
#define I2C_MASTER_MODE
// uncomment if we use EEPROM chips
// #define I2C_EEPROM_SUPPORT

#define I2C_BITRATE              100000
#define I2C_PORT                 PORTC
#define I2C_DDR                  DDRC

#define I2C_SCL_PIN              0
#define I2C_SDA_PIN              1
#define I2C_ENABLE_PULLUPS       1

#define I2C_BUFFER_SIZE          4
#ifdef I2C_EEPROM_SUPPORT
#define I2C_PAGE_ADDRESS_SIZE    2 // depends on EEPROM type, usually it is 1 or 2 bytes
#endif /* I2C_EEPROM_SUPPORT */

#ifdef I2C_SLAVE_MODE
#define I2C_SLAVE_RX_BUFFER_SIZE 1
#define I2C_SLAVE_TX_BUFFER_SIZE 1
#endif /* I2C_SLAVE_MODE */

#define I2C_MODE_MASK           0b00001100
#define I2C_MODE_SARP           0b00000000 // Start-Addr_R-Read-Stop: just read mode
#define I2C_MODE_SAWP           0b00000100 // Start-Addr_W-Write-Stop: just write mode
#define I2C_MODE_ENHA           0b00001000 // Start-Addr_W-WrPageAdr-rStart-Addr_R-Read-Stop
#define I2C_MODE_BUSY           0b01000000 // Transponder is busy
#define I2C_MODE_FREE           0b10111111 // Transponder is free

#define I2C_INTERRUPTED         0b10000000  // Transmiting Interrupted
#define I2C_NOINTERRUPTED       0b01111111    // Transmiting No Interrupted

#define I2C_ERROR_BUS_FAIL      0b00000001
#define I2C_ERROR_NACK          0b00000010
#define I2C_ERROR_NO_ANSWER     0b00010000
#define I2C_ERROR_LOW_PRIO      0b00100000

#define I2C_STATE_BUS_FAIL              0x00
#define I2C_STATE_START                 0x08
#define I2C_STATE_RESTART               0x10
#define I2C_STATE_SLAWACK               0x18
#define I2C_STATE_SLAWNACK              0x20
#define I2C_STATE_BYTEACK               0x28
#define I2C_STATE_BYTENACK              0x30
#define I2C_STATE_COLLISION             0x38
#define I2C_STATE_SLARACK               0x40
#define I2C_STATE_SLARNACK              0x48
#define I2C_STATE_GOT_BYTE              0x50
#define I2C_STATE_GOT_BYTE_NACK         0x58
#define I2C_STATE_SLAW_LP               0x68
#define I2C_STATE_SLAW_LP_ANY           0x78

#ifdef I2C_SLAVE_MODE
#define I2C_STATE_SLAW                  0x60
#define I2C_STATE_SLAW_ANY              0x70
#define I2C_STATE_RCV_BYTE              0x80
#define I2C_STATE_RCV_BYTE_ANY          0x90
#define I2C_STATE_RCV_LAST_BYTE         0x88
#define I2C_STATE_RCV_LAST_BYTE_ANY     0x98
#define I2C_STATE_RCV_RESTART           0xA0
#define I2C_STATE_RCV_SLAR              0xA8
#define I2C_STATE_RCV_SLAR_LP           0xB0
#define I2C_STATE_SND_BYTE_ACK          0xB8
#define I2C_STATE_SND_LAST_BYTE_NACK    0xC0
#define I2C_STATE_SND_LAST_BYTE_ACK     0xC0
#endif /* I2C_SLAVE_MODE */


typedef void (*I2C_HANDLER)(void);

extern I2C_HANDLER i2c_error_func;
#ifdef I2C_MASTER_MODE
extern I2C_HANDLER i2c_master_func;
#endif /* I2C_MASTER_MODE */
#ifdef I2C_SLAVE_MODE
extern I2C_HANDLER i2c_slave_func;
#endif /* I2C_SLAVE_MODE */

#define MACRO_I2C_ERROR         (i2c_error_func)()
#ifdef I2C_MASTER_MODE
#define MACRO_I2C_MASTER        (i2c_master_func)()
#endif /* I2C_MASTER_MODE */
#ifdef I2C_SLAVE_MODE
#define MACRO_I2C_SLAVE         (i2c_slave_func)()
#endif /* I2C_SLAVE_MODE */

typedef enum {I2C_MASTER, I2C_SLACE} I2C_MODE_T;

typedef struct {
  uint8_t address; // an address of a slave device
  uint8_t* data; // a data to be sent
  uint8_t size; // no more 256 bytes in a packet
  uint8_t index; // current position
} I2C_MSG_T;


void i2c_bus_init(uint8_t address);
void i2c_mode_set(I2C_MODE_T mode);
void i2c_send_to(uint8_t address, uint8_t* block, size_t tx_len);
void i2c_send_handler(void);
void i2c_do_nothing(void);


#endif /* _I2C_BUS_H */
