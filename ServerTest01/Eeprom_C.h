/*
 * Eeprom_C.h
 *
 *  Created on: Mar 29, 2014
 *      Author: jje
 */

#ifndef EEPROM_C_H_
#define EEPROM_C_H_


#ifdef __cplusplus
extern "C" {
#endif

void EepromWrite( uint16_t * addr, uint8_t val );
uint8_t EepromRead( uint16_t * addr );


#ifdef __cplusplus
} // extern "C"
#endif

#endif /* EEPROM_C_H_ */
