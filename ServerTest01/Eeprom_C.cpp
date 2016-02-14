/*
 * Eeprom_C.cpp
 *
 *  Created on: Mar 29, 2014
 *      Author: jje
 */
#include <Arduino.h>
#include <EEPROM.h>
#include "Eeprom_C.h"


void EepromWrite( uint16_t * addr, uint8_t val )
{
    EEPROM.write( (int)addr, val );
}

uint8_t EepromRead( uint16_t * addr )
{
    return EEPROM.read( (int)addr );
}


