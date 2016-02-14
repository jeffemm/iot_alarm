/*
 * Clock.h
 *
 *  Created on: Mar 29, 2014
 *      Author: jje
 */

#ifndef CLOCK_H_
#define CLOCK_H_

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************
 * CONSTANTS
 *****************************************************************************/
#define DEBUG_OUT_CLOCK ( 0 )

#define CLOCK_DAY_MON ( (uint8_t)0x01 )
#define CLOCK_DAY_TUE ( (uint8_t)0x02 )
#define CLOCK_DAY_WED ( (uint8_t)0x04 )
#define CLOCK_DAY_THU ( (uint8_t)0x08 )
#define CLOCK_DAY_FRI ( (uint8_t)0x10 )
#define CLOCK_DAY_SAT ( (uint8_t)0x20 )
#define CLOCK_DAY_SUN ( (uint8_t)0x40 )

/*****************************************************************************
 * PUBLIC TYPES
 *****************************************************************************/
typedef struct {
    uint8_t Hour_u8;
    uint8_t Minute_u8;
    uint8_t Second_u8;
    uint8_t Day_u8;
    uint8_t reserved_u8;
} Clock_t;

/*****************************************************************************
 * PUBLIC FUNCTION DECLARATIONS
 *****************************************************************************/
extern uint8_t Update_Time( uint8_t p_Init );
extern uint8_t Set_Time( uint8_t p_Hour, uint8_t p_Minute, uint8_t p_Second, uint8_t p_Day );
extern void Get_Time( Clock_t * clkPtr );



#ifdef __cplusplus
} // extern "C"
#endif

#endif /* CLOCK_H_ */
