
/** \addtogroup hal */
/** @{*/
/* mbed Microcontroller Library
 * Copyright (c) 2006-2013 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef MBED_SPI_API_H
#define MBED_SPI_API_H

#include "device.h"
#include "pinmap.h"
#include "hal/dma_api.h"
#include "hal/buffer.h"

#if DEVICE_SPI

#define SPI_EVENT_ERROR       (1 << 1)
#define SPI_EVENT_COMPLETE    (1 << 2)
#define SPI_EVENT_RX_OVERFLOW (1 << 3)
#define SPI_EVENT_ALL         (SPI_EVENT_ERROR | SPI_EVENT_COMPLETE | SPI_EVENT_RX_OVERFLOW)

#define SPI_EVENT_INTERNAL_TRANSFER_COMPLETE (1 << 30) // Internal flag to report that an event occurred

#define SPI_FILL_WORD         (0xFFFF)
#define SPI_FILL_CHAR         (0xFF)

#if DEVICE_SPI_ASYNCH
/** Asynch SPI HAL structure
 */
typedef struct {
    struct spi_s spi;        /**< Target specific SPI structure */
    struct buffer_s tx_buff; /**< Tx buffer */
    struct buffer_s rx_buff; /**< Rx buffer */
} spi_t;

#else
/** Non-asynch SPI HAL structure
 */
typedef struct spi_s spi_t;

#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \defgroup hal_GeneralSPI SPI Configuration Functions
 *
 * # Defined behavior
 * * ::spi_init initializes the spi_t control structure
 * * ::spi_init configures the pins used by SPI
 * * ::spi_free returns the pins owned by the SPI object to their reset state
 * * ::spi_format sets the number of bits per frame
 * * ::spi_format configures clock polarity and phase
 * * ::spi_format configures master/slave mode
 * * ::spi_frequency sets the SPI baud rate
 * * ::spi_master_write writes a symbol out in master mode and receives a symbol
 * * ::spi_master_block_write writes `tx_length` words to the bus
 * * ::spi_master_block_write reads `rx_length` words from the bus
 * * ::spi_master_block_write returns the maximum of tx_length and rx_length
 * * ::spi_master_block_write specifies the write_fill which is default data transmitted while performing a read
 * * ::spi_get_module returns non-zero if a value is available to read from SPI channel, 0 otherwise
 * * ::spi_slave_read returns a received value out of the SPI receive buffer in slave mode
 * * ::spi_slave_read blocks until a value is available
 * * ::spi_slave_write writes a value to the SPI peripheral in slave mode
 * * ::spi_slave_write blocks until the SPI peripheral can be written to
 * * ::spi_busy returns non-zero if the peripheral is currently transmitting, 0 otherwise
 * * ::spi_master_transfer starts the SPI asynchronous transfer
 * * ::spi_master_transfer writes `tx_len` words to the bus
 * * ::spi_master_transfer reads `rx_len` words from the bus
 * * ::spi_master_transfer specifies the bit width of buffer words
 * * The callback given to ::spi_master_transfer is invoked when the transfer completes (with a success or an error)
 * * ::spi_master_transfer specifies the logical OR of events to be registered
 * * The ::spi_master_transfer function may use the `DMAUsage` hint to select the appropriate async algorithm
 * * ::spi_irq_handler_asynch reads the received values out of the RX FIFO
 * * ::spi_irq_handler_asynch writes values into the TX FIFO
 * * ::spi_irq_handler_asynch checks for transfer termination conditions, such as buffer overflows or transfer complete
 * * ::spi_irq_handler_asynch returns event flags if a transfer termination condition was met, otherwise 0
 * * ::spi_abort_asynch aborts an on-going async transfer
 * * ::spi_active returns non-zero if the SPI port is active or zero if it is not
 *
 * # Undefined behavior
 * * Calling ::spi_init multiple times on the same `spi_t` without ::spi_free
 * * Calling any function other than ::spi_init on a non-initialized or freed `spi_t`
 * * Passing pins that cannot be on the same peripheral
 * * Passing an invalid pointer as `obj` to any function
 * * Passing an invalid pointer as `handler` to ::spi_master_transfer
 * * Calling ::spi_abort while no async transfer is being processed (no transfer or a synchronous transfer)
 *
 * @{
 */

/**
 * \defgroup hal_GeneralSPI_tests SPI hal tests
 * The SPI HAL tests ensure driver conformance to defined behaviour.
 *
 * To run the SPI hal tests use the command:
 *
 *     mbed test -t <toolchain> -m <target> -n tests-mbed_hal_fpga_ci_test_shield-spi
 *
 */

#ifdef DEVICE_SPI_COUNT
/**
 * Returns a variant of the SPIName enum uniquely identifying a SPI peripheral of the device.
 * @param[in]  mosi The pin to use for MOSI
 * @param[in]  miso The pin to use for MISO
 * @param[in]  sclk The pin to use for SCLK
 * @return     An SPI peripheral identifier
 */
SPIName spi_get_peripheral_name(PinName mosi, PinName miso, PinName mclk);
#endif

/** Initialize the SPI peripheral
 *
 * Configures the pins used by SPI, sets a default format and frequency, and enables the peripheral
 * @param[out] obj  The SPI object to initialize
 * @param[in]  mosi The pin to use for MOSI
 * @param[in]  miso The pin to use for MISO
 * @param[in]  sclk The pin to use for SCLK
 * @param[in]  ssel The pin to use for SSEL
 */
void spi_init(spi_t *obj, PinName mosi, PinName miso, PinName sclk, PinName ssel);

/** Release a SPI object
 *
 * TODO: spi_free is currently unimplemented
 * This will require reference counting at the C++ level to be safe
 *
 * Return the pins owned by the SPI object to their reset state
 * Disable the SPI peripheral
 * Disable the SPI clock
 * @param[in] obj The SPI object to deinitialize
 */
void spi_free(spi_t *obj);

/** Configure the SPI format
 *
 * Set the number of bits per frame, configure clock polarity and phase, shift order and master/slave mode.
 * The default bit order is MSB.
 * @param[in,out] obj   The SPI object to configure
 * @param[in]     bits  The number of bits per frame
 * @param[in]     mode  The SPI mode (clock polarity, phase, and shift direction)
 * @param[in]     slave Zero for master mode or non-zero for slave mode
 */
void spi_format(spi_t *obj, int bits, int mode, int slave);

/** Set the SPI baud rate
 *
 * Actual frequency may differ from the desired frequency due to available dividers and bus clock
 * Configures the SPI peripheral's baud rate
 * @param[in,out] obj The SPI object to configure
 * @param[in]     hz  The baud rate in Hz
 */
void spi_frequency(spi_t *obj, int hz);

/**@}*/
/**
 * \defgroup SynchSPI Synchronous SPI Hardware Abstraction Layer
 * @{
 */

/** Write a byte out in master mode and receive a value
 *
 * @param[in] obj   The SPI peripheral to use for sending
 * @param[in] value The value to send
 * @return Returns the value received during send
 */
int  spi_master_write(spi_t *obj, int value);

/** Write a block out in master mode and receive a value
 *
 *  The total number of bytes sent and received will be the maximum of
 *  tx_length and rx_length. The bytes written will be padded with the
 *  value 0xff.
 *
 * @param[in] obj        The SPI peripheral to use for sending
 * @param[in] tx_buffer  Pointer to the byte-array of data to write to the device
 * @param[in] tx_length  Number of bytes to write, may be zero
 * @param[in] rx_buffer  Pointer to the byte-array of data to read from the device
 * @param[in] rx_length  Number of bytes to read, may be zero
 * @param[in] write_fill Default data transmitted while performing a read
 * @returns
 *      The number of bytes written and read from the device. This is
 *      maximum of tx_length and rx_length.
 */
int spi_master_block_write(spi_t *obj, const char *tx_buffer, int tx_length, char *rx_buffer, int rx_length, char write_fill);

/** Check if a value is available to read
 *
 * @param[in] obj The SPI peripheral to check
 * @return non-zero if a value is available
 */
int  spi_slave_receive(spi_t *obj);

/** Get a received value out of the SPI receive buffer in slave mode
 *
 * Blocks until a value is available
 * @param[in] obj The SPI peripheral to read
 * @return The value received
 */
int  spi_slave_read(spi_t *obj);

/** Write a value to the SPI peripheral in slave mode
 *
 * Blocks until the SPI peripheral can be written to
 * @param[in] obj   The SPI peripheral to write
 * @param[in] value The value to write
 */
void spi_slave_write(spi_t *obj, int value);

/** Checks if the specified SPI peripheral is in use
 *
 * @param[in] obj The SPI peripheral to check
 * @return non-zero if the peripheral is currently transmitting
 */
int  spi_busy(spi_t *obj);

/** Get the module number
 *
 * @param[in] obj The SPI peripheral to check
 * @return The module number
 */
uint8_t spi_get_module(spi_t *obj);

/** Get the pins that support SPI MOSI
 *
 * Return a PinMap array of pins that support SPI MOSI in
 * master mode. The array is terminated with {NC, NC, 0}.
 *
 * @return PinMap array
 */
const PinMap *spi_master_mosi_pinmap(void);

/** Get the pins that support SPI MISO
 *
 * Return a PinMap array of pins that support SPI MISO in
 * master mode. The array is terminated with {NC, NC, 0}.
 *
 * @return PinMap array
 */
const PinMap *spi_master_miso_pinmap(void);

/** Get the pins that support SPI CLK
 *
 * Return a PinMap array of pins that support SPI CLK in
 * master mode. The array is terminated with {NC, NC, 0}.
 *
 * @return PinMap array
 */
const PinMap *spi_master_clk_pinmap(void);

/** Get the pins that support SPI CS
 *
 * Return a PinMap array of pins that support SPI CS in
 * master mode. The array is terminated with {NC, NC, 0}.
 *
 * @return PinMap array
 */
const PinMap *spi_master_cs_pinmap(void);

/** Get the pins that support SPI MOSI
 *
 * Return a PinMap array of pins that support SPI MOSI in
 * slave mode. The array is terminated with {NC, NC, 0}.
 *
 * @return PinMap array
 */
const PinMap *spi_slave_mosi_pinmap(void);

/** Get the pins that support SPI MISO
 *
 * Return a PinMap array of pins that support SPI MISO in
 * slave mode. The array is terminated with {NC, NC, 0}.
 *
 * @return PinMap array
 */
const PinMap *spi_slave_miso_pinmap(void);

/** Get the pins that support SPI CLK
 *
 * Return a PinMap array of pins that support SPI CLK in
 * slave mode. The array is terminated with {NC, NC, 0}.
 *
 * @return PinMap array
 */
const PinMap *spi_slave_clk_pinmap(void);

/** Get the pins that support SPI CS
 *
 * Return a PinMap array of pins that support SPI CS in
 * slave mode. The array is terminated with {NC, NC, 0}.
 *
 * @return PinMap array
 */
const PinMap *spi_slave_cs_pinmap(void);

/**@}*/

#if DEVICE_SPI_ASYNCH
/**
 * \defgroup AsynchSPI Asynchronous SPI Hardware Abstraction Layer
 * @{
 */

/** Begin the SPI transfer. Buffer pointers and lengths are specified in tx_buff and rx_buff
 *
 * @param[in] obj       The SPI object that holds the transfer information
 * @param[in] tx        The transmit buffer
 * @param[in] tx_length The number of bytes to transmit
 * @param[in] rx        The receive buffer
 * @param[in] rx_length The number of bytes to receive
 * @param[in] bit_width The bit width of buffer words
 * @param[in] event     The logical OR of events to be registered
 * @param[in] handler   SPI interrupt handler
 * @param[in] hint      A suggestion for how to use DMA with this transfer
 */
void spi_master_transfer(spi_t *obj, const void *tx, size_t tx_length, void *rx, size_t rx_length, uint8_t bit_width, uint32_t handler, uint32_t event, DMAUsage hint);

/** The asynchronous IRQ handler
 *
 * Reads the received values out of the RX FIFO, writes values into the TX FIFO and checks for transfer termination
 * conditions, such as buffer overflows or transfer complete.
 * @param[in] obj     The SPI object that holds the transfer information
 * @return Event flags if a transfer termination condition was met; otherwise 0.
 */
uint32_t spi_irq_handler_asynch(spi_t *obj);

/** Attempts to determine if the SPI peripheral is already in use
 *
 * If a temporary DMA channel has been allocated, peripheral is in use.
 * If a permanent DMA channel has been allocated, check if the DMA channel is in use.  If not, proceed as though no DMA
 * channel were allocated.
 * If no DMA channel is allocated, check whether tx and rx buffers have been assigned.  For each assigned buffer, check
 * if the corresponding buffer position is less than the buffer length.  If buffers do not indicate activity, check if
 * there are any bytes in the FIFOs.
 * @param[in] obj The SPI object to check for activity
 * @return Non-zero if the SPI port is active or zero if it is not.
 */
uint8_t spi_active(spi_t *obj);

/** Abort an SPI transfer
 *
 * @param obj The SPI peripheral to stop
 */
void spi_abort_asynch(spi_t *obj);


#endif

/**@}*/

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // SPI_DEVICE

#endif // MBED_SPI_API_H

/** @}*/
