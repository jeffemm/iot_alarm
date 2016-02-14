/*
 * AlarmClock.h
 *
 *  Created on: Mar 29, 2014
 *      Author: jje
 */

#ifndef ALARMCLOCK_H_
#define ALARMCLOCK_H_


/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include "Clock.h"

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************
 * CONSTANTS
 *****************************************************************************/
#define NULL_AAF_PTR  ( (AlarmAction_fptr_t)( 0 ) )

/*****************************************************************************
 * PUBLIC TYPES
 *****************************************************************************/
typedef enum {
    ALARM_ID_0,
    ALARM_ID_1,
    ALARM_ID_2,
    ALARM_ID_3,
    NUM_ALARMS
} AlarmId_t;

typedef void (* AlarmAction_fptr_t)( AlarmId_t idAlarm );
typedef struct {
    uint8_t Hour_u8;
    uint8_t Minute_u8;
    uint8_t Second_u8;
    uint8_t Day_u8;
    uint8_t Flags_u8;
} Alarm_t;


/*****************************************************************************
 * GLOBALS
 *****************************************************************************/

/*****************************************************************************
 * PUBLIC FUNCTION DECLARATIONS
 *****************************************************************************/
void Check_All_Alarms( void );
uint8_t Check_Alarm( uint8_t p_AlarmNum );
uint8_t Set_Alarm( uint8_t p_AlarmNum, uint8_t p_Hour, uint8_t p_Minute, uint8_t p_Second, uint8_t p_Days );
uint8_t Read_Alarm_Nvm( void );
uint8_t Get_Alarm( Alarm_t * alrPtr, uint8_t alrNum );



#ifdef __cplusplus
} // extern "C"
#endif

#endif /* ALARMCLOCK_H_ */
