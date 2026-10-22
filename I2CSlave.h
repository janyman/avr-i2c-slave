#ifndef I2C_SLAVE_H
#define I2C_SLAVE_H

#include <avr/interrupt.h>
#include <stdint.h>

void I2C_init(uint8_t address);
void I2C_stop(void);
void I2C_setCallbacks(void (*recv)(uint8_t), void (*req)());

/*
 * Called from the TWI ISR when the peripheral reports normal transaction
 * completion: a STOP condition, a NACK, or the last byte of a transmission.
 * The transaction may still have left application-level state pending, so
 * the callback decides whether that state should be retained or cleared.
 */
void I2C_setTransactionEndCallback(void (*end)());

/*
 * Called from the TWI ISR after a TW_BUS_ERROR.  The transaction is invalid
 * and any partial application-level protocol state should be discarded.
 */
void I2C_setTransactionAbortCallback(void (*abort)());

inline void __attribute__((always_inline)) I2C_transmitByte(uint8_t data)
{
  TWDR = data;
}

ISR(TWI_vect);

#endif
