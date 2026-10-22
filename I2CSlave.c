#include <util/twi.h>
#include <avr/interrupt.h>

#include "I2CSlave.h"

static void (*I2C_recv)(uint8_t);
static void (*I2C_req)();
static void (*I2C_transaction_end)();
static void (*I2C_transaction_abort)();

void I2C_setCallbacks(void (*recv)(uint8_t), void (*req)())
{
  I2C_recv = recv;
  I2C_req = req;
}

void I2C_setTransactionEndCallback(void (*end)())
{
  I2C_transaction_end = end;
}

void I2C_setTransactionAbortCallback(void (*abort)())
{
  I2C_transaction_abort = abort;
}

void I2C_init(uint8_t address)
{
  uint8_t saved_sreg = SREG;

  cli();
  // load address into TWI address register
  TWAR = address << 1;
  // set the TWCR to enable address matching and enable TWI, clear TWINT, enable TWI interrupt
  TWCR = (1<<TWIE) | (1<<TWEA) | (1<<TWINT) | (1<<TWEN);
  SREG = saved_sreg;
}

void I2C_stop(void)
{
  uint8_t saved_sreg = SREG;

  // clear acknowledge and enable bits
  cli();
  TWCR = 0;
  TWAR = 0;
  SREG = saved_sreg;
}

ISR(TWI_vect)
{
  switch(TW_STATUS)
  {
    case TW_SR_SLA_ACK:
      /* Addressed as a receiver; the following byte is the first payload. */
      break;

    case TW_SR_DATA_ACK:
      if( I2C_recv != 0 )
      {
        I2C_recv(TWDR);
      }
      break;

    case TW_ST_SLA_ACK:
    case TW_ST_DATA_ACK:
      if( I2C_req != 0 )
      {
        I2C_req();
      }
      else
      {
        TWDR = 0xff;
      }
      break;

    case TW_SR_STOP:
    case TW_SR_DATA_NACK:
    case TW_ST_DATA_NACK:
    case TW_ST_LAST_DATA:
      if( I2C_transaction_end != 0 )
      {
        I2C_transaction_end();
      }
      break;

    case TW_BUS_ERROR:
      /* Recover from an illegal START/STOP condition on the bus. */
      if( I2C_transaction_abort != 0 )
      {
        /* The current transaction is no longer trustworthy.  Give the
         * application a chance to discard any partial protocol state before
         * the TWI peripheral is re-enabled below. */
        I2C_transaction_abort();
      }
      TWCR = 0;
      break;

    default:
      break;
  }

  /* Clear TWINT and remain enabled as an addressed slave. */
  TWCR = (1<<TWIE) | (1<<TWINT) | (1<<TWEA) | (1<<TWEN);
}
