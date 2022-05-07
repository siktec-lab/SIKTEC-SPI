/******************************************************************************/
// Created by: SIKTEC.
// Release Version : 1.0.1
// Creation Date: 2022-03-31
// Copyright 2022, SIKTEC.
/******************************************************************************/

/** @file SIKTEC_SPI.cpp */
#include "SIKTEC_SPI.h"

/**
 * @brief  Create an SPI device with the given CS pin and settins
 * 
 * @param  cspin     The arduino pin number to use for chip select
 * @param  freq      The SPI clock frequency to use, defaults to 1MHz
 * @param  dataOrder The SPI data order to use for bits within each byte,
 *                   defaults to SPI_BITORDER_MSBFIRST
 * @param  dataMode  The SPI mode to use, defaults to SPI_MODE0
 * @param  theSPI    The SPI bus to use, defaults to &theSPI
 */
SIKTEC_SPI::SIKTEC_SPI(int8_t cspin, uint32_t freq, BitOrder dataOrder, uint8_t dataMode, SPIClass *theSPI) {
    
    this->_cs           = cspin;
    this->_sck          = -1;
    this->_mosi         = -1;
    this->_miso         = -1;
    this->_spi          = theSPI;
    this->begun         = false;
    this->_spiSetting   = new SPISettings(freq, dataOrder, dataMode);
    this->_freq         = freq;
    this->_dataOrder    = dataOrder;
    this->_dataMode     = dataMode;
    this->_cs_control   = true;
}

/**
 * @brief  Create an SPI device with the given CS pin and settins
 *
 * @param  cspin     The arduino pin number to use for chip select
 * @param  sckpin    The arduino pin number to use for SCK
 * @param  misopin   The arduino pin number to use for MISO, set to -1 if not used
 * @param  mosipin   The arduino pin number to use for MOSI, set to -1 if not used
 * @param  freq      The SPI clock frequency to use, defaults to 1MHz
 * @param  dataOrder The SPI data order to use for bits within each byte, defaults to SPI_BITORDER_MSBFIRST
 * @param  dataMode  The SPI mode to use, defaults to SPI_MODE0
 */
SIKTEC_SPI::SIKTEC_SPI(int8_t cspin, int8_t sckpin, int8_t misopin, int8_t mosipin, uint32_t freq, BitOrder dataOrder, uint8_t dataMode) {
    
    this->_cs           = cspin;
    this->_sck          = sckpin;
    this->_miso         = misopin;
    this->_mosi         = mosipin;
    this->_freq         = freq;
    this->_dataOrder    = dataOrder;
    this->_dataMode     = dataMode;
    this->begun         = false;
    this->_spiSetting   = new SPISettings(freq, dataOrder, dataMode);
    this->_spi          = NULL;
    this->_cs_control   = true;
}

/**
 * @brief  Release memory allocated in constructors
 */
SIKTEC_SPI::~SIKTEC_SPI() {
    if (this->_spiSetting) {
        delete this->_spiSetting;
        this->_spiSetting = nullptr;
    }
}

/**
 * @brief  Initializes SPI bus and sets CS pin high
 * 
 * @returns Always returns true because there's no way to test success of SPI
 */
bool SIKTEC_SPI::begin() {

    #ifdef SIKTEC_SPI_DEBUG
        Serial.println("SPI begin is called.");
    #endif

    pinMode(this->_cs, OUTPUT);
    digitalWrite(this->_cs, HIGH);

    if (this->_spi) { // hardware SPI

        #ifdef SIKTEC_SPI_DEBUG
            Serial.println("Using hardware SPI.");
        #endif
        this->_spi->begin();
    
    } else {
    
        #ifdef SIKTEC_SPI_DEBUG
            Serial.println("Using software SPI.");
        #endif
        pinMode(this->_sck, OUTPUT);
        if ((this->_dataMode == SPI_MODE0) || (this->_dataMode == SPI_MODE1)) {
            // idle low on mode 0 and 1
            digitalWrite(this->_sck, LOW);
        } else {
            // idle high on mode 2 or 3
            digitalWrite(this->_sck, HIGH);
        }
        if (this->_mosi != -1) {
            pinMode(this->_mosi, OUTPUT);
            digitalWrite(this->_mosi, HIGH);
        }
        if (this->_miso != -1) {
            pinMode(this->_miso, INPUT);
        }
    }
    this->begun = true;
    return true;
}

/**
 * @brief ends SPI and releases pins only when using HW SPI.
 * 
 * @return bool 
 */
bool SIKTEC_SPI::end() {
    if (this->_spi) {
        this->_spi->end();
    }
    return true;
}

/**
 * @brief  Transfer (send/receive) one byte over hard/soft SPI
 * 
 * @param  buffer The buffer to send and receive at the same time
 * @param  len    The number of bytes to transfer
 * 
 * @returns void
 */
void SIKTEC_SPI::transfer(uint8_t *buffer, size_t len) {
    if (this->_spi) {
        // hardware SPI:
        #if defined(SPARK)
            this->_spi->transfer(buffer, buffer, len, NULL);
        #elif defined(STM32)
            for (size_t i = 0; i < len; i++) {
                this->_spi->transfer(buffer[i]);
            }
        #else
            this->_spi->transfer(buffer, len);
        #endif

        return;
    }

    uint8_t startbit;
    if (this->_dataOrder == SPI_BITORDER_LSBFIRST) {
        startbit = 0x1;
    } else {
        startbit = 0x80;
    }

    bool towrite, lastmosi = !(buffer[0] & startbit);
    uint8_t bitdelay_us = (1000000 / this->_freq) / 2;

    // for softSPI we'll do it by hand
    for (size_t i = 0; i < len; i++) {
        // software SPI
        uint8_t reply = 0;
        uint8_t send = buffer[i];

        for (uint8_t b = startbit; b != 0; b = (this->_dataOrder == SPI_BITORDER_LSBFIRST) ? b << 1 : b >> 1) {

            if (bitdelay_us) {
                delayMicroseconds(bitdelay_us);
            }

            if (this->_dataMode == SPI_MODE0 || this->_dataMode == SPI_MODE2) {
                towrite = send & b;
                if ((this->_mosi != -1) && (lastmosi != towrite)) {
                    digitalWrite(this->_mosi, towrite);
                    lastmosi = towrite;
                }
                digitalWrite(this->_sck, HIGH);
                if (bitdelay_us) {
                    delayMicroseconds(bitdelay_us);
                }

                if (this->_miso != -1 && digitalRead(this->_miso)) {
                    reply |= b;
                }
                digitalWrite(this->_sck, LOW);

            } else { // if (_dataMode == SPI_MODE1 || _dataMode == SPI_MODE3)
                digitalWrite(this->_sck, HIGH);       

                if (bitdelay_us) {
                    delayMicroseconds(bitdelay_us);
                }

                if (this->_mosi != -1) {
                    digitalWrite(this->_mosi, send & b);
                }
                digitalWrite(this->_sck, LOW);

                if (this->_miso != -1 && digitalRead(this->_miso)) {
                    reply |= b;
                }
            }
            if (this->_miso != -1) {
                buffer[i] = reply;
            }
        }
    }
    return;
}

/**
 * @brief  Transfer (send/receive) one byte over hard/soft SPI
 * 
 * @param  send     The byte to send
 * 
 * @return uint8_t - The byte received while transmitting
 */
uint8_t SIKTEC_SPI::transfer(uint8_t send) {
    uint8_t data = send;
    this->transfer(&data, 1);
    return data;
}

/**
 * @brief  Manually begin a transaction (calls beginTransaction if hardware SPI)
 * 
 * @returns void
 */
void SIKTEC_SPI::beginTransaction() {
    if (this->_spi) {
        this->_spi->beginTransaction(*this->_spiSetting);
    }
}

/**
 * @brief  Manually end a transaction (calls endTransaction if hardware SPI)
 * 
 * @returns void
 */
void SIKTEC_SPI::endTransaction() {
    if (this->_spi) {
        this->_spi->endTransaction();
    }
}

/**
 * @brief Disable and prevent CS toggling by this instance
 * 
 * @returns void
 */
void SIKTEC_SPI::disableCsToggle() {
    this->_cs_control = false;
}

/**
 * @brief Enables CS toggling by this instance
 * 
 * @returns void
 */
void SIKTEC_SPI::enableCsToggle() {
    this->_cs_control = true;
}

/**
 * @brief  Write a buffer or two to the SPI device.
 *    
 * @param  buffer           Pointer to buffer of data to write
 * @param  len              Number of bytes from buffer to write
 * @param  prefix_buffer    Pointer to optional array of data to write before buffer.
 * @param  prefix_len       Number of bytes from prefix buffer to write
 * 
 * @returns bool - Always returns true because there's no way to test success of SPI writes
 */
bool SIKTEC_SPI::write(const uint8_t *buffer, size_t len, const uint8_t *prefix_buffer, size_t prefix_len, bool invertBuffer) {

    //Hardware SPI:
    this->beginTransaction();
    //Toggle CS
    if (this->_cs_control) digitalWrite(this->_cs, LOW);
    // do the writing
    if (invertBuffer) {
        for (size_t i = 0; i < prefix_len; i++) {
            (void)this->transfer(~prefix_buffer[i]);
        }
        for (size_t i = 0; i < len; i++) {
            (void)this->transfer(~buffer[i]);
        }
    } else {
        for (size_t i = 0; i < prefix_len; i++) {
            (void)this->transfer(prefix_buffer[i]);
        }
        for (size_t i = 0; i < len; i++) {
            (void)this->transfer(buffer[i]);
        }
    }
    //Toggle CS
    if (this->_cs_control) digitalWrite(this->_cs, HIGH);

    this->endTransaction();
    return true;
}

/**
 * @brief Write a byte and and read at the same time
 * 
 * @param write     The byte to write
 * 
 * @return uint8_t - the read byte
 */
uint8_t SIKTEC_SPI::write_and_read(uint8_t write) {
    //Toggle CS
    if (this->_cs_control) digitalWrite(this->_cs, LOW);
    //Transfer
    uint8_t returned = this->transfer(write);
    //Toggle CS
    if (this->_cs_control) digitalWrite(this->_cs, HIGH);
    return returned;
}

/**
 * @brief  Write a value N times two to the SPI device with optional prefix data.
 * 
 * @param  buffer           Pointer to buffer of data to write
 * @param  len              Number of bytes from buffer to write
 * @param  prefix_buffer    Pointer to optional array of data to write before buffer.
 * @param  prefix_len       Number of bytes from prefix buffer to write
 *
 * @returns bool - Always returns true because there's no way to test success of SPI writes
 */
bool SIKTEC_SPI::repeated(uint8_t value, size_t times, uint8_t *prefix_buffer, size_t prefix_len) {
    
    //Hardware SPI:
    if (this->_spi) {
        this->_spi->beginTransaction(*this->_spiSetting);
    }
    //Toggle CS
    if (this->_cs_control) digitalWrite(this->_cs, LOW);
    // do the writing
    for (size_t i = 0; i < prefix_len; i++) {
        (void)this->transfer(prefix_buffer[i]);
    }
    for (size_t i = 0; i < times; i++) {
        (void)this->transfer(value);
    }
    //Toggle CS
    if (this->_cs_control) digitalWrite(this->_cs, HIGH);
    if (this->_spi) {
        this->_spi->endTransaction();
    }
    return true;    
}

/**
 * @brief  Read from SPI into a buffer from the SPI device.
 * 
 * @param  buffer       Pointer to buffer of data to read into
 * @param  len          Number of bytes from buffer to read.
 * @param  sendvalue    The 8-bits of data to write when doing the data read, defaults to 0xFF
 * 
 * @return bool - Always returns true because there's no way to test success of SPI writes
 */
bool SIKTEC_SPI::read(uint8_t *buffer, size_t len, uint8_t sendvalue) {
    
    memset(buffer, sendvalue, len); // clear out existing buffer
    
    if (this->_spi) {
        this->_spi->beginTransaction(*this->_spiSetting);
    }
    //Toggle CS
    if (this->_cs_control) digitalWrite(this->_cs, LOW);

    this->transfer(buffer, len);
    
    //Toggle CS
    if (this->_cs_control) digitalWrite(this->_cs, HIGH);

    if (this->_spi) {
        this->_spi->endTransaction();
    }

    return true;
}

/**
 * @brief  Write some data, then read some data from SPI into another buffer.
 *         The buffers can point to same/overlapping locations. This does not
 *         transmit-receive at the same time!
 * @param  write_buffer Pointer to buffer of data to write from
 * @param  write_len    Number of bytes from buffer to write.
 * @param  read_buffer  Pointer to buffer of data to read into.
 * @param  read_len     Number of bytes from buffer to read.
 * @param  sendvalue    The 8-bits of data to write when doing the data read, defaults to 0xFF
 * 
 * @return bool - Always returns true because there's no way to test success of SPI writes
 */
bool SIKTEC_SPI::write_then_read(uint8_t *write_buffer, size_t write_len, uint8_t *read_buffer, size_t read_len, uint8_t sendvalue) {

    if (this->_spi) {
        this->_spi->beginTransaction(*this->_spiSetting);
    }
    //Toggle CS
    if (this->_cs_control) digitalWrite(this->_cs, LOW);
    //Do the writing
    for (size_t i = 0; i < write_len; i++) {
        (void)this->transfer(write_buffer[i]);
    }
    //Do the reading
    for (size_t i = 0; i < read_len; i++) {
        read_buffer[i] = this->transfer(sendvalue);
    }
    //Toggle CS
    if (this->_cs_control) digitalWrite(this->_cs, HIGH);

    if (this->_spi) {
        this->_spi->endTransaction();
    }
    return true;
}

