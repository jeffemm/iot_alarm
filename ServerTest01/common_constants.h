/*
 * common_constants.h
 *
 *  Created on: Mar 29, 2014
 *      Author: jje
 */

#ifndef COMMON_CONSTANTS_H_
#define COMMON_CONSTANTS_H_

#define TRUE   ((uint8_t)1U)
#define FALSE  ((uint8_t)0U)

#define NULL_PTR  ( (void *)0 )

#define MILLIS_PER_SECOND ((unsigned long)1000UL)


/* Board Specific Configuration */
#define ARDUINO_BOARD_UNO

#ifdef ARDUINO_BOARD_UNO
    #define PROCESSOR_ATMEGA328
#else
    #error Board type not defined!
#endif

/* Processor Specific Configuration */
#ifdef PROCESSOR_ATMEGA328
    #define EEPROM_SIZE_BYTES ( 1024 )
#else
    #error Processor type not defined!
#endif


#endif /* COMMON_CONSTANTS_H_ */
