#include <avr/io.h>
#include <avr/interrupt.h>
#include "sersendf.h"
#include "i2c_bus.h"

/**
 * I2C bus implementation.
 * See section 22 of atmega328 datasheet.
 */

#define TWI_DEBUG

// the queue for a message to be sent
#define I2C_QUEUE_SIZE 8
// find the next message index after 'x', where 0 <= x < I2C_QUEUE_SIZE
#define I2CQ_NEXT(x) ((x) < I2C_QUEUE_SIZE-1 ? (x)+1 : 0)

I2C_MSG_T i2c_queue[I2C_QUEUE_SIZE];
uint8_t i2c_queue_head = 0;
uint8_t i2c_queue_tail = 0;

uint8_t i2c_current_mode = I2C_MASTER;
uint8_t i2c_address; // the address of a device that is used in communication process
uint8_t i2c_state; // the state if TWI component of MCU
uint8_t i2c_index; // an index inside the buffer
uint8_t i2c_byte_count; // the count of bytes it should send

#ifdef I2C_MASTER_MODE
uint8_t* i2c_buffer;
#endif /* I2C_MASTER_MODE */

#ifdef I2C_SLAVE_MODE
uint8_t i2c_in_buffer[I2C_SLAVE_RX_BUFFER_SIZE];
uint8_t i2c_out_buffer[I2C_SLAVE_TX_BUFFER_SIZE];
#endif /* I2C_SLAVE_MODE */

#ifdef I2C_EEPROM_SUPPORT
uint8_t i2c_page_address[I2C_PAGE_ADDRESS_SIZE]; // for SAWSARP mode (see ENHA in i2c_bus.h)
uint8_t i2c_page_index; // an index inside the page address buffer
uint8_t i2c_page_count; // the count of bytes in page address
#endif /* I2C_EEPROM_SUPPORT */


I2C_HANDLER i2c_error_func = &i2c_do_nothing;
#ifdef I2C_MASTER_MODE
I2C_HANDLER i2c_master_func = &i2c_do_nothing;
#endif /* I2C_MASTER_MODE */
#ifdef I2C_SLAVE_MODE
I2C_HANDLER i2c_slave_func = &i2c_do_nothing;
#endif /* I2C_SLAVE_MODE */


void i2c_bus_init(uint8_t address) {
  i2c_address = address;
#ifdef I2C_MASTER_MODE
 #ifdef I2C_ENABLE_PULLUPS
  I2C_PORT |= (1 << I2C_SCL_PIN) | (1 << I2C_SDA_PIN);
  I2C_DDR &= ~((1 << I2C_SCL_PIN) | (1 << I2C_SDA_PIN));
 #endif /* I2C_ENABLE_PULLUPS */

  /**
   * TWI Bit Rate Register
   * SCL_freq = CPU_freq / (16 + 2*TWBR)
   * See: page 235 of atmega328 datasheet.
   */
  TWBR = ((F_CPU / I2C_BITRATE) - 16) / 2;
  /**
   * TWI Status Register
   * Lower two bits set the prescaler value.
   * See: page 236 of atmega328 datasheet.
   */
  TWSR = 0x00;
#endif /* I2C_MASTER_MODE */

#ifdef I2C_SLAVE_MODE
  TWAR = i2c_address; // we listen to broadcasts if lowest bit is set
  TWCR = (0<<TWINT)|(0<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|(1<<TWEN)|(1<<TWIE);
#endif
}


void i2c_mode_set(I2C_MODE_T mode) {
  i2c_current_mode = mode;
}


/**
 * Function places a data for slave device in the queue.
 */
void i2c_send_to(uint8_t address, uint8_t* block, size_t tx_len) {
  I2C_MSG_T message = {address, block, tx_len, 0};
  uint8_t i2c_queue_head = I2CQ_NEXT(i2c_queue_head);
  i2c_queue[i2c_queue_head] = message;
#ifdef TWI_DEBUG
  sersendf_P(PSTR("\ni2c_send_to[%sx]: block %lx [%sx, %sx, %sx, %sx], count %su, index %su, head %su, tail %su"),
             address, block, block[0], block[1], block[2], block[3],
             (uint16_t) tx_len, (uint16_t) i2c_queue_head, (uint16_t) i2c_queue_head, (uint16_t) i2c_queue_tail);
#endif
}


/**
 * This function is used to start I2C transmission, also it is
 * involved into end and error event handling.
 * It is triggered from clock.c:clock_250ms.
 */
void i2c_send_handler(void) {
  if (i2c_state & I2C_MODE_BUSY) {
    // not now coz it's busy
    return;
  }

  uint8_t i2c_queue_tail = I2CQ_NEXT(i2c_queue_tail);
  I2C_MSG_T message = i2c_queue[i2c_queue_tail];

  /**
   * TODO: TWI interrupt handler should directly work with I2C_MST_T queue.
   */
  i2c_address = message.address;
  i2c_buffer = message.data;
  i2c_byte_count = message.size;
  i2c_index = message.index;

  i2c_state = I2C_MODE_SAWP; // just sent
  i2c_master_func = &i2c_send_handler;
  i2c_error_func = &i2c_send_handler;

#ifdef TWI_DEBUG
  sersendf_P(PSTR("\ni2c_send_hanlder[%sx]: block %lx [%sx, %sx, %sx, %sx], count %su, index %su, head %su, tail %su"),
             message.address, message.data, message.data[0], message.data[1], message.data[2], message.data[3],
             (uint16_t) message.size, (uint16_t) i2c_queue_tail, (uint16_t) i2c_queue_head, (uint16_t) i2c_queue_tail);
#endif
  // start I2C transmission
  TWCR = (1<<TWINT)|(0<<TWEA)|(1<<TWSTA)|(0<<TWSTO)|(1<<TWEN)|(1<<TWIE);
  i2c_state |= I2C_MODE_BUSY;
}


/**
 * Empty handler.
 */
void i2c_do_nothing(void) {
}


/**
 * This interrupt fo all the I2C things.
 *
 * Some words about TWCR bits:
 * Bit 7 (TWINT) is used to run TWI.
 * Bit 6 (TWEA) is used to send ACK (if set) in cases of
 *     a) device's own slave address has been received;
 *     b) general call has been received;
 *     c) a data byte has been received.
 * Bit 5 (TWSTA) is 1 if app wants to be a master, don't forget to clear this bit.
 * Bit 4 (TWSTO) is generated STOP in master mode if set (cleared
 * automaticly), recovered from error condition in slave mode if set.
 * Bit 3 (TWWC) is write collision flag. Sets on try to writeto TWDR when TWINT is low.
 * Bit 2 (TWEN) activates SDA/SCL pins if set. Set to 0 to disable TWI.
 * Bit 1 (Reserved).
 * Bit 0 (TWIE) enables TWI interrupt.
 */

ISR(TWI_vect) {
  // cut the prescaler bits out
  switch (TWSR & 0xF8) {
  case I2C_STATE_BUS_FAIL:
    // a hardware error was detected, for instance, there was START condition while data transmission
    i2c_state |= I2C_ERROR_BUS_FAIL;
    TWCR = (1<<TWINT)|(i2c_current_mode<<TWEA)|(0<<TWSTA)|(1<<TWSTO)|(1<<TWEN)|(1<<TWIE);
    MACRO_I2C_ERROR;
    break;
  case I2C_STATE_START:
    // START happens, send a target address
    if ((i2c_state & I2C_MODE_MASK) == I2C_MODE_SARP) {
      i2c_address |= 0x01;
    } else {
      i2c_address &= 0xFE;
    }
    TWDR = i2c_address;
    TWCR = (1<<TWINT)|(i2c_current_mode<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|(1<<TWEN)|(1<<TWIE);
    break;
  case I2C_STATE_RESTART:
    // RESTART has been happened, now we can change a mode between read/write
    if ((i2c_state & I2C_MODE_MASK) == I2C_MODE_ENHA) {
      i2c_address |= 0x01;
    } else {
      i2c_address &= 0xFE;
    }
    TWDR = i2c_address;
    TWCR = (1<<TWINT)|(i2c_current_mode<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|(1<<TWEN)|(1<<TWIE);
    break;
  case I2C_STATE_SLAWACK:
    // SLA+W was sent, ACK was received, then
    if ((i2c_state & I2C_MODE_MASK) == I2C_MODE_SAWP) {
      TWDR = i2c_buffer[i2c_index++];
      TWCR = (1<<TWINT)|(i2c_current_mode<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|(1<<TWEN)|(1<<TWIE);
    }
#ifdef I2C_EEPROM_SUPPORT
    if ((i2c_state & I2C_MODE_MASK) == I2C_MODE_ENHA) {
      TWDR = i2c_page_address[i2c_page_index++];
      TWCR = (1<<TWINT)|(i2c_current_mode<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|(1<<TWEN)|(1<<TWIE);
    }
#endif /* I2C_EEPROM_SUPPORT */
    break;
  case I2C_STATE_SLAWNACK:
    //  SLA+W was sent, got NACK, so slave is busy or out of bus, retry
    i2c_state |= I2C_ERROR_NO_ANSWER;
    TWCR = (1<<TWINT)|(i2c_current_mode<<TWEA)|(0<<TWSTA)|(1<<TWSTO)|(1<<TWEN)|(1<<TWIE);
    MACRO_I2C_ERROR;
    break;
  case I2C_STATE_BYTEACK:
    // a byte was sent, got ACK,
    if ((i2c_state & I2C_MODE_MASK) == I2C_MODE_SAWP) { // it was just a byte of data
      if (i2c_index == i2c_byte_count) { // last byte
        // send stop
        TWCR = (1<<TWINT)|(i2c_current_mode<<TWEA)|(0<<TWSTA)|(1<<TWSTO)|(1<<TWEN)|(1<<TWIE);
        MACRO_I2C_MASTER; // process stop state
      } else {
        TWDR = i2c_buffer[i2c_index++]; // send the next byte
        TWCR = (1<<TWINT)|(i2c_current_mode<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|(1<<TWEN)|(1<<TWIE);
      }
    }

#ifdef I2C_EEPROM_SUPPORT
    if( (i2c_state & I2C_MODE_MASK) == I2C_MODE_ENHA) { // it was page address byte
      if(i2c_page_index == i2c_page_count) { // it was last byte of page address
        TWCR = (1<<TWINT)|(i2c_current_mode<<TWEA)|(1<<TWSTA)|(0<<TWSTO)|(1<<TWEN)|(1<<TWIE);
      } else {
        TWDR = i2c_page_address[i2c_page_index++]; // send the next page address byte
        TWCR = (1<<TWINT)|(i2c_current_mode<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|(1<<TWEN)|(1<<TWIE);
      }
    }
#endif /* I2C_EEPROM_SUPPORT */
    break;
  case I2C_STATE_BYTENACK:
    // byte was sent but got NACK, there are two reasons:
    // first, a slave stops transmission and it is ok
    // second, a slave becames crazy
    i2c_state |= I2C_ERROR_NACK;
    TWCR = (1<<TWINT)|(i2c_current_mode<<TWEA)|(0<<TWSTA)|(1<<TWSTO)|(1<<TWEN)|(1<<TWIE);
    MACRO_I2C_MASTER; // process exit state
    break;
  case I2C_STATE_COLLISION:
    // it seems there is another master on the bus
    i2c_state |= I2C_ERROR_LOW_PRIO;
    // setup all again
    i2c_index = 0;
#ifdef I2C_EEPROM_SUPPORT
    i2c_page_index = 0;
#endif /* I2C_EEPROM_SUPPORT */
    // try to resend when the bus becames free
    TWCR = (1<<TWINT)|(i2c_current_mode<<TWEA)|(1<<TWSTA)|(0<<TWSTO)|(1<<TWEN)|(1<<TWIE);
    break;
  case I2C_STATE_SLARACK:
    // SLA+R was sent, got АСК, then receive bytes
    if (i2c_index + 1 == i2c_byte_count) { // if buffer ends on this byte
      // request a byte then send NACK to slave and it will release the bus
      TWCR = (1<<TWINT)|(0<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|(1<<TWEN)|(1<<TWIE);
    } else {
      // or just receive a byte and sent ACK
      TWCR = (1<<TWINT)|(1<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|(1<<TWEN)|(1<<TWIE);
    }
    break;
  case I2C_STATE_SLARNACK:
    // SLA+R was sent, got NАСК, it seems the slave is busy
    i2c_state |= I2C_ERROR_NO_ANSWER;
    TWCR = (1<<TWINT)|(i2c_current_mode<<TWEA)|(0<<TWSTA)|(1<<TWSTO)|(1<<TWEN)|(1<<TWIE);
    MACRO_I2C_ERROR;
    break;
  case I2C_STATE_GOT_BYTE:
    i2c_buffer[i2c_index++] = TWDR;
    // TODO: Add BUFFER OVERFLOW check
    if (i2c_index + 1 == i2c_byte_count) {
      // last byte wait the processing
      TWCR = (1<<TWINT)|(0<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|(1<<TWEN)|(1<<TWIE);
    } else {
      // request the next byte
      TWCR = (1<<TWINT)|(0<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|(1<<TWEN)|(1<<TWIE);
    }
    break;
  case I2C_STATE_GOT_BYTE_NACK:
    // the last byte received, send NACK to make the slave to release the bus
    i2c_buffer[i2c_index] = TWDR;
    TWCR = (1<<TWINT)|(i2c_current_mode<<TWEA)|(0<<TWSTA)|(1<<TWSTO)|(1<<TWEN)|(1<<TWIE);
    MACRO_I2C_MASTER;
    break;
  case I2C_STATE_SLAW_LP:
  case I2C_STATE_SLAW_LP_ANY:
    // another master on the bus send some bytes, receive them
    i2c_state |= I2C_ERROR_LOW_PRIO;
    // restore the transfer
    i2c_index = 0;
#ifdef I2C_EEPROM_SUPPORT
    i2c_page_index = 0;
#endif /* I2C_EEPROM_SUPPORT */

#ifdef I2C_SLAVE_MODE
  case I2C_STATE_SLAW:
  case I2C_STATE_SLAW_ANY:
    i2c_state |= I2C_MODE_BUSY; // lock the bus
    i2c_index = 0;
    if (I2C_SLAVE_RX_BUFFER_SIZE == 1) {
      // we should take alone byte and send NACK
      TWCR = (1<<TWINT)|(0<TWEA)|(0<<TWSTA)|(0<<TWSTO)|(1<<TWEN)|(1<<TWIE);
    } else {
      // get a byte and send ACK
      TWCR = (1<<TWINT)|(1<TWEA)|(0<<TWSTA)|(0<<TWSTO)|(1<<TWEN)|(1<<TWIE);
    }
    break;
  case I2C_STATE_RCV_BYTE:
  case I2C_STATE_RCV_BYTE_ANY:
    i2c_in_buffer[i2c_index++] = TWDR;
    if (i2c_index == I2C_SLAVE_RX_BUFFER_SIZE - 1) {
      // there is a space for on byte only, send NACK
      TWCR = (1<<TWINT)|(0<TWEA)|(0<<TWSTA)|(0<<TWSTO)|(1<<TWEN)|(1<<TWIE);
    } else {
      // there are a lot of space, send ACK
      TWCR = (1<<TWINT)|(1<TWEA)|(0<<TWSTA)|(0<<TWSTO)|(1<<TWEN)|(1<<TWIE);
    }
    break;
  case I2C_STATE_RCV_LAST_BYTE:
  case I2C_STATE_RCV_LAST_BYTE_ANY:
    i2c_in_buffer[i2c_index] = TWDR;
    if (i2c_state & I2C_INTERRUPTED) {
      // Если у нас был прерываный сеанс от имени мастера
      // Влепим в шину свой Start поскорей и сделаем еще одну попытку
      TWCR = (1<<TWINT)|(1<TWEA)|(1<<TWSTA)|(0<<TWSTO)|(1<<TWEN)|(1<<TWIE);
    } else {
      // Если не было такого факта, то просто отвалимся и будем ждать
      TWCR = (1<<TWINT)|(1<TWEA)|(0<<TWSTA)|(0<<TWSTO)|(1<<TWEN)|(1<<TWIE);
    }
    MACRO_I2C_SLAVE;
    break;
  case I2C_STATE_RCV_RESTART:
    // we have got a ReStart. What we will do?
    // Here we can do additional logic but we don't need it at this time.
    // Just ignore it now.
    TWCR = (1<<TWINT)|(1<TWEA)|(0<<TWSTA)|(0<<TWSTO)|(1<<TWEN)|(1<<TWIE);
    break;
  case I2C_STATE_RCV_SLAR_LP:
    // Got own address on read from another master
    i2c_state |= I2C_ERROR_LOW_PRIO | I2C_INTERRUPTED;

    // reinit
    i2c_index = 0;
#ifdef I2C_EEPROM_SUPPORT
    i2c_page_index = 0;
#endif /* I2C_EEPROM_SUPPORT */
  case I2C_STATE_RCV_SLAR:
    // We have got own address on read
    i2c_index = 0;
    TWDR = i2c_out_buffer[i2c_index];
    if (I2C_SLAVE_TX_BUFFER_SIZE == 1) {
      // if it is last byte, we hope to receive NACK
      TWCR = (1<<TWINT)|(0<TWEA)|(0<<TWSTA)|(0<<TWSTO)|(1<<TWEN)|(1<<TWIE);
    } else {
      // wait for ACK
      TWCR = (1<<TWINT)|(1<TWEA)|(0<<TWSTA)|(0<<TWSTO)|(1<<TWEN)|(1<<TWIE);
    }
    break;
  case I2C_STATE_SND_BYTE_ACK:
    // send byte and got ACK, then send next byte to master
    TWDR = i2c_out_buffer[++i2c_index];
    if (I2C_SLAVE_TX_BUFFER_SIZE - 1 == i2c_index) {
      // it was last byte, send it and wait NACK
      TWCR = (1<<TWINT)|(0<TWEA)|(0<<TWSTA)|(0<<TWSTO)|(1<<TWEN)|(1<<TWIE);
    } else {
      // send byte and wait ACK
      TWCR = (1<<TWINT)|(1<TWEA)|(0<<TWSTA)|(0<<TWSTO)|(0<<TWEN)|(1<<TWIE);
    }
    break;
  case I2C_STATE_SND_LAST_BYTE_NACK:
  case I2C_STATE_SND_LAST_BYTE_ACK:
    // we have sent the last byte and have received NACK or ACK (don't care at this case)
    if (i2c_state & I2C_INTERRUPTED) {
      // there was interrupted master transfer
      i2c_state &= I2C_NOINTERRUPTED;
      // generate Start as the bus becames free
      TWCR = (1<<TWINT)|(1<TWEA)|(1<<TWSTA)|(0<<TWSTO)|(1<<TWEN)|(1<<TWIE);
    } else {
      // if we alone then just release the bus
      TWCR = (1<<TWINT)|(1<TWEA)|(0<<TWSTA)|(0<<TWSTO)|(1<<TWEN)|(1<<TWIE);
    }
    MACRO_I2C_SLAVE;
    break;
#endif /* I2C_SLAVE_MODE */

  default:
    break;
  }
}
