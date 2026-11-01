# AVR I2C Slave Library

This library provides interrupt-based I2C slave functionality for Atmel 8-bit microcontrollers equipped with a TWI peripheral. 

A fork of https://github.com/thegouger/avr-i2c-slave

## Usage

**Provide callbacks for receiving and handling requests in your application code.**
eg:
```c
void I2C_received(uint8_t data);
void I2C_requested();

I2C_setCallbacks(I2C_received, I2C_requested);
```

The library calls the received callback *for each byte* the master transmits to the slave.
The library calls the requested callback *for each byte* the master attempts to read from the slave.

**Handle transaction completion and aborts**
```c
void I2C_transaction_end();
void I2C_transaction_abort();

I2C_setTransactionEndCallback(I2C_transaction_end);
I2C_setTransactionAbortCallback(I2C_transaction_abort);
```

The transaction-end callback is called from the TWI interrupt handler when a transaction ends normally, such as when the master sends STOP, sends a NACK,
or the final transmitted byte is acknowledged as complete. The transaction end callback may retain state if application protocol spans multiple I2C transactions.

The transaction abort callback is called from the TWI interrupt handler when the peripheral reports `TW_BUS_ERROR`, meaning that the bus sequence was invalid. The abort callback should reset the application protocol state.

Both callbacks run in interrupt context and should be short and non-blocking.

**Init the I2C slave with the slave address**
```c
I2C_init(I2C_ADDRESS);
```

**Transmitting data to the master when requested**
```c
I2C_transmitByte(data);
```

## Example


The example in `main.c` implements a sample application in which the slave can receive a byte, 
and then will echo the byte when requested.

To compile:
`make`

## More about I2C bus error

On the AVR, `TW_BUS_ERROR` means that the TWI peripheral detected an illegal
bus condition, such as an invalid START or STOP transition while the bus was
idle. This can be caused by electrical noise, ringing, poor rise times on a
long cable, a reset or disconnect of the master, or another malformed bus
sequence. It is different from a normal NACK: after a bus error, the slave
cannot assume that the address direction, byte count, or message boundaries
are valid. The abort callback is therefore the place to reset the
application's protocol parser to its idle state.

For example, suppose that bus error occurs when slave has received the low byte but not the high byte when it was expecting a 16-bit word from the master. Such state is no longer trustworthy after a bus
error; retaining it could cause bytes from a later transaction to be
interpreted as the missing bytes from the aborted one.
