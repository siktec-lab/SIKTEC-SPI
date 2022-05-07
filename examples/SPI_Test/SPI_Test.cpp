/******************************************************************************/
// Created by: SIKTEC.
// Release Version : 1.0.1
// Creation Date: 2022-04-12
// Copyright 2022, SIKTEC.
// 
/******************************************************************************/
/*****************************      NOTES       *******************************
    -> Simple SPI declaration and 3 bytes write example.
*******************************************************************************/
/*****************************      Changelog       ****************************
1.0.1:
    -> initial release
*******************************************************************************/

#include <Arduino.h>
#include <SIKTEC_SPI.h>

//The spi device SC pin:
#define SLAVE_CS_PIN 16

//Create a SIKTEC_SPI spi object:
SIKTEC_SPI spi(SLAVE_CS_PIN);

void setup() {

    //Initialize Serial:
    Serial.begin(115200);
    while (!Serial) { delay(10); }

    Serial.println("SPI Begin.");
    spi.begin(); //Will star the spi -> define pins and all is needed.


    Serial.println("SPI Write.");
    uint8_t buf[3] = {0XFF};
    spi.write(buf, 3); //Example writing 3 bytes to a slave.

    Serial.println("SPI End.");
    spi.end(); //End and release the SPI.

}

void loop() {
    Serial.println("SPI In loop.");
    while (1) { ; }
}