
#ifdef ARDUINO_ARCH_ESP32

#include "esp32-hal-spi.h"

#include "lq-platform_spi.h"
#include <Arduino.h>
#include <SPI.h>


/**
 *	@brief Initialize and configure SPI resource.
 *
 *  @param chipSelLine [in] - GPIO for CS 
 *  \return SPI device
 */
 lq_spi_t* lq_spi_createFromPins(uint8_t clkPin, uint8_t misoPin, uint8_t mosiPin, uint8_t csPin)
{
     lq_spi_t *platformSpi = ( lq_spi_t*)malloc(sizeof( lq_spi_t));
	if (platformSpi == NULL)
	{
		return NULL;
	}	
    platformSpi->dataRate = 2000000U;
    platformSpi->dataMode = spiDataMode_0;
    platformSpi->bitOrder = spiBitOrder_msbFirst;
    platformSpi->clkPin = clkPin;
    platformSpi->misoPin = misoPin;
    platformSpi->mosiPin = mosiPin;
    platformSpi->csPin = csPin;
    platformSpi->spi = new SPIClass(HSPI);
    return platformSpi;
}

/**
 *	@brief Start SPI facility.
 */
void lq_spi_start( lq_spi_t* platformSpi)
{
    digitalWrite(platformSpi->csPin, HIGH);
    pinMode(platformSpi->csPin, OUTPUT);
    ((SPIClass*)platformSpi->spi)->begin(platformSpi->clkPin, platformSpi->misoPin, platformSpi->mosiPin, platformSpi->csPin);
}



/**
 *	@brief Shutdown SPI facility.
 */
void lq_spi_stop( lq_spi_t* spi)
{
    SPI.end();
}


/**
 *	@brief Gaurd SPI resource from recursive interrupts.
 */
void lq_spi_usingInterrupt( lq_spi_t* spi, int8_t irqNumber)
{
    return;
}


/**
 *	@brief Gaurd SPI resource from recursive interrupts.
 */
void lq_spi_notUsingInterrupt( lq_spi_t* spi, int8_t irqNumber)
{
    return;
}



/**
 *	@brief Transfer a byte to the NXP bridge.
 *
 *	@param spi [in] - The SPI device for communications.
 *  @param data [in/out] - The word to transfer to the NXP bridge.
 * 
 *  \returns A 16-bit word received during the transfer.
 */
uint8_t lq_spi_transferByte( lq_spi_t* platformSpi, uint8_t data)
{
    digitalWrite(platformSpi->csPin, LOW);
    ((SPIClass*)platformSpi->spi)->beginTransaction(SPISettings(platformSpi->dataRate, platformSpi->bitOrder, platformSpi->dataMode));

    uint8_t result = ((SPIClass*)platformSpi->spi)->transfer(data);

    digitalWrite(platformSpi->csPin, HIGH);
    ((SPIClass*)platformSpi->spi)->endTransaction();
    return result;
}



/**
 *	@brief Transfer a word (16-bits) to the NXP bridge.
 *
 *	@param spi [in] - The SPI device for communications.
 *  @param data [in/out] - The word to transfer to the NXP bridge.
 * 
 *  \returns A 16-bit word received during the transfer.
 */
uint16_t lq_spi_transferWord( lq_spi_t* platformSpi, uint16_t data)
{
    union { uint16_t val; struct { uint8_t msb; uint8_t lsb; }; } tw;

    digitalWrite(platformSpi->csPin, LOW);
    ((SPIClass*)platformSpi->spi)->beginTransaction(SPISettings(platformSpi->dataRate, platformSpi->bitOrder, platformSpi->dataMode));

    tw.val = data;
    if (platformSpi->bitOrder == spiBitOrder_msbFirst)
    {
        tw.msb = ((SPIClass*)platformSpi->spi)->transfer(tw.msb);
        tw.lsb = ((SPIClass*)platformSpi->spi)->transfer(tw.lsb);
    }
    else
    {
        tw.lsb = ((SPIClass*)platformSpi->spi)->transfer(tw.lsb);
        tw.msb = ((SPIClass*)platformSpi->spi)->transfer(tw.msb);
    }
    digitalWrite(platformSpi->csPin, HIGH);
    ((SPIClass*)platformSpi->spi)->endTransaction();

    return tw.val;
}



/**
 *	@brief Transfer a buffer to the SPI device.
 *
 *	@param spi [in] The SPI device for communications.
 *  @param addressByte [in] Optional address byte sent before buffer, can specify specifics of the I/O being initiated.
 *  @param buf [in/out] - The character pointer to the buffer to transfer to/from.
 *  @param xfer_len [in] - The number of characters to transfer.
 */
void lq_spi_transferBuffer( lq_spi_t* platformSpi, uint8_t addressByte, const uint8_t * txData, uint8_t * rxData, uint32_t size)
{
    digitalWrite(platformSpi->csPin, LOW);
    ((SPIClass*)platformSpi->spi)->beginTransaction(SPISettings(platformSpi->dataRate, platformSpi->bitOrder, platformSpi->dataMode));

    ((SPIClass*)platformSpi->spi)->transfer(addressByte);
    //((SPIClass*)platformSpi->spi)->transfer(buf, xfer_len);

    // void SPIClass::transferBytes(const uint8_t * txData, uint8_t * rxData, uint32_t size)
    ((SPIClass*)platformSpi->spi)->transferBytes(txData, rxData, size);

    digitalWrite(platformSpi->csPin, HIGH);
    ((SPIClass*)platformSpi->spi)->endTransaction();
}


void lq_spi_writeBuffer( lq_spi_t* platformSpi, uint8_t addressByte, uint8_t * txData, uint32_t size)
{
    digitalWrite(platformSpi->csPin, LOW);
    ((SPIClass*)platformSpi->spi)->beginTransaction(SPISettings(platformSpi->dataRate, platformSpi->bitOrder, platformSpi->dataMode));

    ((SPIClass*)platformSpi->spi)->transfer(addressByte);

    // for (uint16_t i = 0; i < size; i++)
    // {
    //     ((SPIClass*)platformSpi->spi)->transferBytes(*(txData + i));
    // }
    ((SPIClass*)platformSpi->spi)->transferBytes(txData, NULL, size);

    digitalWrite(platformSpi->csPin, HIGH);
    ((SPIClass*)platformSpi->spi)->endTransaction();
}


void lq_spi_readBuffer( lq_spi_t* platformSpi, uint8_t addressByte, void* buf, uint16_t xfer_len)
{
    digitalWrite(platformSpi->csPin, LOW);
    ((SPIClass*)platformSpi->spi)->beginTransaction(SPISettings(platformSpi->dataRate, platformSpi->bitOrder, platformSpi->dataMode));

    ((SPIClass*)platformSpi->spi)->transfer(addressByte);
    for (uint16_t i = 0; i < xfer_len; i++)
    {
        *((uint8_t*)(buf + i)) = ((SPIClass*)platformSpi->spi)->transfer(0xFF);
    }

    digitalWrite(platformSpi->csPin, HIGH);
    ((SPIClass*)platformSpi->spi)->endTransaction();
}

#endif