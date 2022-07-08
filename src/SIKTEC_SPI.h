/******************************************************************************/
// Created by: SIKTEC.
// Release Version : 1.0.2
// Creation Date: 2022-04-12
// Copyright 2022, SIKTEC.
// 
/******************************************************************************/
/*****************************      NOTES       *******************************
    -> A Lightweight SPI wrapper for HW SPI and Software SPI
    -> A Better api to establish SPI comunication - supports hardware SPI and Software SPI - Auto toggling CS pins and transactions. 
*******************************************************************************/
/*****************************      Changelog       ****************************
1.0.1:
    -> initial release.
1.0.2:
    -> Added other boards (ARDUINO_ARCH_ARC32, __ASR6502__).
    -> Changed bit order enum naming to fix conflict with adafruit libs. 
*******************************************************************************/

/** @file SIKTEC_SPI.h */
#pragma once

// #define SIKTEC_SPI_DEBUG

//------------------------------------------------------------------------//
// INCLUDES:
//------------------------------------------------------------------------//

#include <Arduino.h>
#include <SPI.h>

//------------------------------------------------------------------------//
// DEFAULT ADDRESSES AND CONSTANTS:
//------------------------------------------------------------------------//

// some modern SPI definitions don't have BitOrder enum
#if (defined(__AVR__) && !defined(ARDUINO_ARCH_MEGAAVR)) ||                    \
    defined(ESP8266) || defined(TEENSYDUINO) || defined(SPARK) ||              \
    defined(ARDUINO_ARCH_SPRESENSE) || defined(MEGATINYCORE) ||                \
    defined(DXCORE) || defined(ARDUINO_AVR_ATmega4809) ||                      \
    defined(ARDUINO_AVR_ATmega4808) || defined(ARDUINO_AVR_ATmega3209) ||      \
    defined(ARDUINO_AVR_ATmega3208) || defined(ARDUINO_AVR_ATmega1609) ||      \
    defined(ARDUINO_AVR_ATmega1608) || defined(ARDUINO_AVR_ATmega809) ||       \
    defined(ARDUINO_AVR_ATmega808) || defined(ARDUINO_ARCH_ARC32)

    typedef enum _SIKSPI_BitOrder {
        SIKSPI_BITORDER_MSBFIRST = MSBFIRST,
        SIKSPI_BITORDER_LSBFIRST = LSBFIRST,
    } SIKSPI_BitOrder;

#elif defined(ESP32) || defined(__ASR6501__) || defined(__ASR6502__)

    // some modern SPI definitions don't have BitOrder enum and have different SPI
    // mode defines
    typedef enum _SIKSPI_BitOrder {
    SIKSPI_BITORDER_MSBFIRST = SPI_MSBFIRST,
    SIKSPI_BITORDER_LSBFIRST = SPI_LSBFIRST,
    } SIKSPI_BitOrder;

#else
    // Some platforms have a BitOrder enum but its named MSBFIRST/LSBFIRST

    typedef enum _SIKSPI_BitOrder {
    SIKSPI_BITORDER_MSBFIRST = MSBFIRST,
    SIKSPI_BITORDER_LSBFIRST = LSBFIRST,
    } SIKSPI_BitOrder;

#endif


//------------------------------------------------------------------------//
// SIKTEC_SPI
//------------------------------------------------------------------------//

/** 
 * @brief  The class which defines how we will talk to this device over SPI
*/
class SIKTEC_SPI {

private:

    SPIClass *_spi;
    SPISettings *_spiSetting;
    uint32_t _freq;                     ///< Used freq.
    SIKSPI_BitOrder _dataOrder;                ///< SPI bit order.
    uint8_t _dataMode;                  ///< SPI data mode.
    int8_t _cs, _sck, _mosi, _miso;     
    bool _cs_control;                   ///< this flag is used to prevent the instance controlling the CS pin.

public:

    SIKTEC_SPI(int8_t cspin, uint32_t freq = 1000000, SIKSPI_BitOrder dataOrder = SIKSPI_BitOrder::SIKSPI_BITORDER_MSBFIRST, uint8_t dataMode = SPI_MODE0, SPIClass *theSPI = &SPI);
    SIKTEC_SPI(int8_t cspin, int8_t sck, int8_t miso, int8_t mosi, uint32_t freq = 1000000, SIKSPI_BitOrder dataOrder = SIKSPI_BitOrder::SIKSPI_BITORDER_MSBFIRST, uint8_t dataMode = SPI_MODE0);
    ~SIKTEC_SPI();

    bool begin(); ///< Begin SPI communication.
    bool end();   ///< End SPI communication and release used pins.

    bool write(const uint8_t *buffer, size_t len, const uint8_t *prefix_buffer = NULL, size_t prefix_len = 0, bool invertBuffer = false);
    bool write_then_read(uint8_t *write_buffer, size_t write_len, uint8_t *read_buffer, size_t read_len, uint8_t sendvalue = 0xFF);
    bool read(uint8_t *buffer, size_t len, uint8_t sendvalue = 0xFF);
    bool repeated(uint8_t value, size_t times, uint8_t *prefix_buffer = NULL, size_t prefix_len = 0);
    uint8_t transfer(uint8_t send);
    void transfer(uint8_t *buffer, size_t len);
    uint8_t write_and_read(uint8_t write);

    void beginTransaction();
    void endTransaction();

    void disableCsToggle(); ///< prevents the instance to control CS pin.
    void enableCsToggle();  ///< enables the instance to control CS pin.

    bool begun;

};
