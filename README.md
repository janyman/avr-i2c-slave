# AVR I2C Slave Library

This library provides interrupt-based I2C slave functionality for Atmel 8-bit microcontrollers equipped with a TWI peripheral. 

It is somewhat based on an existing library found here: https://github.com/devthrash/I2C-slave-lib

Files
-----
* `I2CSlave.c` -- Implements init/stop, and interrupt-based receive and request logic
* `I2CSlave.h` -- Function prototypes and a convience method for transmitting data

Usage
-----
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

The transaction-end callback is called from the TWI interrupt handler when a
transaction ends normally, such as when the master sends STOP, sends a NACK,
or the final transmitted byte is acknowledged as complete. The transaction
abort callback is called from the TWI interrupt handler when the peripheral
reports `TW_BUS_ERROR`, meaning that the bus sequence was invalid. The abort
callback should discard any partial protocol state, while the end callback may
retain state when the application protocol spans multiple I2C transactions.

Partial protocol state is application-level information describing a protocol
message that has started but is not yet complete. For example, an application
might have received a register address and be waiting for its low and high
value bytes, have received the low byte but not the high byte, or have sent a
register identifier in response to a polling request and be waiting for the
master's follow-up write. Such state is no longer trustworthy after a bus
error; retaining it could cause bytes from a later transaction to be
interpreted as the missing bytes from the aborted one.

On the AVR, `TW_BUS_ERROR` means that the TWI peripheral detected an illegal
bus condition, such as an invalid START or STOP transition while the bus was
idle. This can be caused by electrical noise, ringing, poor rise times on a
long cable, a reset or disconnect of the master, or another malformed bus
sequence. It is different from a normal NACK: after a bus error, the slave
cannot assume that the address direction, byte count, or message boundaries
are valid. The abort callback is therefore the place to reset the
application's protocol parser to its idle state.

Both callbacks run in interrupt context and should be short and non-blocking.

**Init the I2C slave with the slave address**
```c
I2C_init(I2C_ADDRESS);
```

**Transmitting data to the master when requested**
```c
I2C_transmitByte(data);
```

Example
-------

The example in `main.c` implements a sample application in which the slave can receive a byte, 
and then will echo the byte when requested.

To compile:
`make`
