/** ***************************************************************************
  @file 
  @brief LTEm SPI communication abstraction declarations.

  @author Greg Terrell/Jensen Miller, LooUQ Incorporated

  \loouq

  @warning Internal dependencies, changes only as directed by LooUQ staff.

-------------------------------------------------------------------------------

LooUQ-LTEmC // Software driver for the LooUQ LTEm series cellular modems.
Copyright (C) 2017-2023 LooUQ Incorporated

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
Also add information on how to contact you by electronic and paper mail.

**************************************************************************** */


#ifndef __LQPLATFORM_SPI_H__
#define __LQPLATFORM_SPI_H__

#include <stdint.h>

#define SPI_NO_IRQ_PROTECTION -1                            ///< SPI is not protected during IRQ servicing


/**
 * @brief Ordering of data bits on SPI bus
 */
typedef enum
{
    spiBitOrder_lsbFirst = 0x0,
    spiBitOrder_msbFirst = 0x1
} spiBitOrder_t;


/* Arduino SPI
#define spi_MODE0 0x02
#define spi_MODE1 0x00
#define spi_MODE2 0x03
#define spi_MODE3 0x01
*/

/**
 * @brief SPI standard data modes
 */
typedef enum
{
    spiDataMode_0 = 0x02,
    spiDataMode_1 = 0x00,
    spiDataMode_2 = 0x03,
    spiDataMode_3 = 0x01
} spiDataMode_t;


/**
 * @brief Structure describing an abstracted SPI interface, this is the object LooUQ devices reference for SPI communications.
 */
typedef struct lq-spi_tag
{
    uint32_t dataRate;                                      ///< bit data rate of SPI bus
    spiDataMode_t dataMode;                                 ///< SPI data mode for SPI bus
    spiBitOrder_t bitOrder;                                 ///< SPI bit ordering for SPI bus
    uint8_t clkPin;                                         ///< Host clock pin for SPI bus
    uint8_t misoPin;                                        ///< Host MISO pin for SPI bus
    uint8_t mosiPin;                                        ///< Host MOSI pin for SPI bus
    uint8_t csPin;                                          ///< Chip select pin for peripheral (CS/SS)
    void* spi;                                              ///< Pointer to framework SPI interface (if required)
} lq_spi_t;


#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief 
 * 
 * @param clkPin 
 * @param misoPin 
 * @param mosiPin 
 * @param csPin 
 * @return lq_spi_t* 
 */
lq_spi_t* lq_spi_createFromPins(uint8_t clkPin, uint8_t misoPin, uint8_t mosiPin, uint8_t csPin);

/**
 * @brief 
 * 
 * @param indx 
 * @param csPin 
 * @return lq_spi_t* 
 */
lq_spi_t* lq_spi_createFromIndex(uint8_t indx, uint8_t csPin);

/**
 * @brief 
 * 
 * @param lq_spi 
 */
void lq_spi_start(lq_spi_t* lq_spi);

/**
 * @brief 
 * 
 * @param lq_spi 
 */
void lq_spi_stop(lq_spi_t* lq_spi);


/**
 * @brief 
 * 
 * @param lq_spi 
 * @param irqNumber 
 */
void lq_spi_usingInterrupt(lq_spi_t* lq_spi, int8_t irqNumber);

/**
 * @brief 
 * 
 * @param lq_spi 
 * @param irqNumber 
 */
void lq_spi_notUsingInterrupt(lq_spi_t* lq_spi, int8_t irqNumber);


/**
 * @brief 
 * 
 * @param lq_spi 
 * @param writeVal 
 * @return uint8_t 
 */
uint8_t lq_spi_transferByte(lq_spi_t* lq_spi, uint8_t writeVal);

/**
 * @brief 
 * 
 * @param lq_spi 
 * @param writeVal 
 * @return uint16_t 
 */
uint16_t lq_spi_transferWord(lq_spi_t* lq_spi, uint16_t writeVal);


/**
 * @brief 
 * 
 * @param lq_spi 
 * @param addressByte 
 * @param txData 
 * @param rxData 
 * @param size 
 */
void lq_spi_transferBuffer(lq_spi_t* lq_spi, uint8_t addressByte, const uint8_t * txData,  uint8_t * rxData, uint32_t size);

/**
 * @brief 
 * 
 * @param lq_spi 
 * @param addressByte 
 * @param buf 
 * @param size 
 */
void lq_spi_writeBuffer(lq_spi_t* lq_spi, uint8_t addressByte, void* buf, uint32_t size);

/**
 * @brief 
 * 
 * @param lq_spi 
 * @param addressByte 
 * @param size 
 * @return void* 
 */
void* lq_spi_readBuffer(lq_spi_t* lq_spi, uint8_t addressByte, uint32_t size);


/* DEPRECATED - To be removed in embedLib v2.1.0 
 ------------------------------------------------------- */
// platformSpi_t* spi_createFromPins(uint8_t clkPin, uint8_t misoPin, uint8_t mosiPin, uint8_t csPin);
// platformSpi_t* spi_createFromIndex(uint8_t indx, uint8_t csPin);

// void spi_start(platformSpi_t* platformSpi);
// void spi_stop(platformSpi_t* platformSpi);

// void spi_usingInterrupt(platformSpi_t* platformSpi, int8_t irqNumber);
// void spi_notUsingInterrupt(platformSpi_t* platformSpi, int8_t irqNumber);

// uint8_t spi_transferByte(platformSpi_t* platformSpi, uint8_t writeVal);
// uint16_t spi_transferWord(platformSpi_t* platformSpi, uint16_t writeVal);

// // void SPIClass::transferBytes(const uint8_t * txData, uint8_t * rxData, uint32_t size)
// void spi_transferBuffer(platformSpi_t* platformSpi, uint8_t addressByte, const uint8_t * txData,  uint8_t * rxData, uint32_t size);

// void spi_writeBuffer(platformSpi_t* platformSpi, uint8_t addressByte, void* buf, uint32_t size);
// void* spi_readBuffer(platformSpi_t* platformSpi, uint8_t addressByte, uint32_t size);
/* --------------------------------------------------------
 */

#ifdef __cplusplus
}
#endif // __cplusplus

#endif  /* !__LQPLATFORM_SPI_H__ */
