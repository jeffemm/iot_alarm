/*
 * AlarmClock.c
 *
 *  Created on: Mar 29, 2014
 *      Author: jje
 */


/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include <stdint.h>
#include <Arduino.h>

#include "common_constants.h"

#include "debug_macros.h"
#include "AlarmClock.h"
#include "Nvm.h"

/*****************************************************************************
 * TYPES
 *****************************************************************************/

/*****************************************************************************
 * CONSTANTS
 *****************************************************************************/
//#define NUM_ALARMS  ((uint8_t)4U)
/* Array to remap Alarm IDs into NVM block IDs */
static const NVM_BlockId_t AlarmNvmIds_ca[NUM_ALARMS] = {
    NVM_ALARM_0,
    NVM_ALARM_1,
    NVM_ALARM_2,
    NVM_ALARM_3 };


/*****************************************************************************
 * GLOBALS
 *****************************************************************************/


/*****************************************************************************
 * LOCALS
 *****************************************************************************/
/* Default is all flags clear and no days active. This keeps alarm disabled
 * by defaults, until NVM read, or alarm is set.
 */
static Alarm_t Alarms_a[NUM_ALARMS] = {
    { 0xFF, 0xFF, 0xFF, 0x00, 0x00 },
    { 0xFF, 0xFF, 0xFF, 0x00, 0x00 },
    { 0xFF, 0xFF, 0xFF, 0x00, 0x00 },
    { 0xFF, 0xFF, 0xFF, 0x00, 0x00 },
};

static AlarmAction_fptr_t AlarmActions_a[NUM_ALARMS] = {
    NULL_AAF_PTR,
    NULL_AAF_PTR,
    NULL_AAF_PTR,
    NULL_AAF_PTR
};

/*****************************************************************************
 * LOCAL FUNCTION DECLARATION
 *****************************************************************************/

/*****************************************************************************
 * FUNCTION DEFINITION
 *****************************************************************************/

/*****************************************************************************
 *  FUNCTION: Check_All_Alarms
 *
 *  INPUTS:
 *      (none)
 *
 *  OUTPUTS:
 *      (none)
 *
 *  DESCRIPTION:
 *      Calls Check_Alarm for all alarms. Return status of Check_Alarm is
 *      ignored. Intended to be used with Alarm Action hanlder functions.
 *
 *****************************************************************************/
void Check_All_Alarms( void )
{
    uint8_t l_inx = 0;

    for ( l_inx = 0; l_inx < NUM_ALARMS; l_inx++ ) {
        ( void )Check_Alarm( (AlarmId_t)l_inx );
    }
} /* Check_All_Alarms */
/*---------------------------------------------------------------------------*/

/*****************************************************************************
 *  FUNCTION: Add_Alarm_Action
 *
 *  INPUTS:
 *      AlarmAction_fptr_t fPtr
 *          Function pointer to alarm action function to assign to alarm
 *      AlarmId_t alarmId
 *          Alarm ID for which action function will be assigned.
 *
 *  OUTPUTS:
 *      AlarmAction_fptr_t (return)
 *          The value of the previous function pointer for the alarm ID.
 *
 *  DESCRIPTION:
 *      This function is used to assign an alarm action function to a specified
 *      alarm. Each alarm can have only one action function assigned. This
 *      will replace a previously assigned function pointer. By specifying
 *      a null function pointer (NULL_AAF_PTR), it can be used to disable
 *      alarm action.
 *
 *      The value of any previously assigned function pointer is returned.
 *      
 *      If the alarm ID provided is not within valid range the action will
 *      not be assigned and the new function pointer value will be returned.
 *      This could be a valid return value if the previously assigned value
 *      is the same as the new value.
 *
 *****************************************************************************/
AlarmAction_fptr_t Add_Alarm_Action( AlarmAction_fptr_t fPtr, AlarmId_t alarmId )
{
    AlarmAction_fptr_t fPtr_old = NULL_AAF_PTR;

    if ( alarmId < NUM_ALARMS ) {
        fPtr_old = AlarmActions_a[ alarmId ];
        AlarmActions_a[ alarmId ] = fPtr;
    } else {
        /* If bad ID return the new function pointer. */
        fPtr_old = fPtr;
    }

    return fPtr_old;
} /* Add_Alarm_Action */
/*---------------------------------------------------------------------------*/

/*****************************************************************************
 *  FUNCTION: Check_Alarm
 *
 *  INPUTS:
 *      uint8_t p_AlarmNum
 *          MAX: 3
 *          MIN: 0
 *          DESC: index of alarm to check
 *
 *  OUTPUTS:
 *      uint8_t (return)
 *          TRUE - current clock time matches alarm time
 *          FALSE - current clock time does not match alarm time, or
 *                  alarm is disabled
 *
 *  DESCRIPTION:
 *      This function compares the current clock time to the alarm time. If
 *      the values match the function returns TRUE, otherwise the function
 *      returns FALSE. The function checks for an exact match of hours, minutes,
 *      and seconds. The function checks that the clock day matches one of
 *      the days selected for the alarm.
 *
 *      To avoid missing an alarm this function needs to be called at least
 *      every 0.5 seconds. If called multiple times withing one second it will
 *      return TRUE every time the clock value matches the alarm value.
 *
 *****************************************************************************/
uint8_t Check_Alarm( uint8_t p_AlarmNum )
{
    Clock_t l_Time;
    uint8_t l_result_u8 = FALSE;

    Get_Time( &l_Time );

    if ( p_AlarmNum < NUM_ALARMS ) {
        if (   ( l_Time.Day_u8 & Alarms_a[ p_AlarmNum ].Day_u8 )
            && ( l_Time.Hour_u8 == Alarms_a[ p_AlarmNum ].Hour_u8 )
            && ( l_Time.Minute_u8 == Alarms_a[ p_AlarmNum ].Minute_u8 )
            && ( l_Time.Second_u8 == Alarms_a[ p_AlarmNum ].Second_u8 ) ) {
            l_result_u8 = TRUE;

            if ( NULL_AAF_PTR != AlarmActions_a[ p_AlarmNum ] ) {
                AlarmActions_a[ p_AlarmNum]( p_AlarmNum );
            }
        } else {
            l_result_u8 = FALSE;
        }
    } else {
        l_result_u8 = FALSE;
    }

    return l_result_u8;
} /* Check_Alarm */
/*---------------------------------------------------------------------------*/

/*****************************************************************************
 *  FUNCTION: Set_Alarm
 *
 *  INPUTS:
 *      uint8_t p_AlarmNum
 *          MAX: 3
 *          MIN: 0
 *          DESC: index of alarm to check
 *      uint8_t p_Hour
 *          MAX: 23
 *          MIN: 0
 *          DESC: zero based hour of the day in 24 hour format for alarm
 *      uint8_t p_Minute
 *          MAX: 59
 *          MIN: 0
 *          DESC: zero based minute of the hour for alarm
 *      uint8_t p_Second
 *          MAX: 59
 *          MIN: 0
 *          DESC: zero based second of the minute for alarm
 *      uint8_t p_Day
 *          ALLOWED: 0x00..0x7F
 *          DESC: days of the week, value should be bitwise ORing of the
 *            defined constants for days of week. Value of 0x00 disables
 *            the alarm.
 *
 *  OUTPUTS:
 *      uint8_t (return)
 *          TRUE - alarm time was set
 *          FALSE - clock time was NOT changed, probably due to an out of
 *                  range parameter value.
 *
 *  DESCRIPTION:
 *      This function sets the alarm time to the provided values. Both the
 *      file scoped variables and the EEPROM values will be modified.
 *
 *****************************************************************************/
uint8_t Set_Alarm( uint8_t p_AlarmNum, uint8_t p_Hour, uint8_t p_Minute, uint8_t p_Second, uint8_t p_Days )
{
    uint8_t l_result_u8 = FALSE;
    uint8_t l_NVM_result_u8 = FALSE;

    if ( p_AlarmNum >= NUM_ALARMS ) {
        //Serial.println( "DEBUG: ERROR - Set_Alarm p_AlarmNum to BlockId failed!" );
        return FALSE;
    } else {
        if (   ( p_Hour < 24 )
                && ( p_Minute < 60 )
                && ( p_Second < 60 )
                && !( p_Days & ~( CLOCK_DAY_MON
                        | CLOCK_DAY_TUE
                        | CLOCK_DAY_WED
                        | CLOCK_DAY_THU
                        | CLOCK_DAY_FRI
                        | CLOCK_DAY_SAT
                        | CLOCK_DAY_SUN ) ) ) {
            /* Input values are all valid, set the alarm */
            Alarms_a[ p_AlarmNum ].Hour_u8   = p_Hour;
            Alarms_a[ p_AlarmNum ].Minute_u8 = p_Minute;
            Alarms_a[ p_AlarmNum ].Second_u8 = p_Second;
            Alarms_a[ p_AlarmNum ].Day_u8   = p_Days;
            /* TODO all alarm flags are cleared, no flags have been defined yet. */
            Alarms_a[ p_AlarmNum ].Flags_u8   = 0x00;

            /* Update EEPROM values for alarm */
            l_NVM_result_u8 = NVM_Write( AlarmNvmIds_ca[ p_AlarmNum],
                                         (uint8_t *)&Alarms_a[ p_AlarmNum ] );

            l_result_u8 = l_NVM_result_u8;
            if ( FALSE == l_NVM_result_u8 ) {
                /* TODO what to do when NVM write fails?
                 * Add a flag to indicate NVM failure.
                 */
                //Serial.println( "DEBUG: ERROR - NVM_Write failed!" );
            }

        } else {
            l_result_u8 = FALSE;
        } /* input values valid */
    } /* p_AlarmNum >= NUM_ALARMS */

    return l_result_u8;
} /* Set_Alarm */
/*---------------------------------------------------------------------------*/


/*****************************************************************************
 *  FUNCTION: Read_Alarm_Nvm
 *
 *  INPUTS:
 *      (none)
 *
 *  OUTPUTS:
 *      uint8_t (return)
 *          TRUE - read successful
 *          FALSE - NVM read failed
 *
 *  DESCRIPTION:
 *      Read alarm values stored in NVM into RAM.
 *
 *****************************************************************************/
uint8_t Read_Alarm_Nvm( void )
{
    uint8_t l_inx = 0;
    uint8_t l_status = TRUE;
    
    for ( l_inx = 0; l_inx < NUM_ALARMS; l_inx++ ) {
        if ( FALSE == NVM_Read( AlarmNvmIds_ca[ l_inx ],
                                (uint8_t *)&Alarms_a[ l_inx ] ) ) {
            l_status = FALSE;
            /* Set to default values to disable alarm, NVM data unreliable */
            Alarms_a[ l_inx ].Hour_u8 = 0xFF;
            Alarms_a[ l_inx ].Minute_u8 = 0xFF;
            Alarms_a[ l_inx ].Second_u8 = 0xFF;
            Alarms_a[ l_inx ].Day_u8 = 0x00;
            Alarms_a[ l_inx ].Flags_u8 = 0x00;
        }
    }

    return l_status;
} /* Read_Alarm_Nvm */
/*---------------------------------------------------------------------------*/

/*****************************************************************************
 *  FUNCTION: Get_Alarm
 *
 *  INPUTS:
 *      Alarm_t * alrPtr
 *          DESC: pointer to Alarm_t structure that will be populated
 *                with alarm values
 *      uint8_t alrNum
 *          DESC: alarm number
 *
 *  OUTPUTS:
 *      uint8_t (return)
 *          0 - failed, bad index
 *          1 - success
 *
 *  DESCRIPTION:
 *      Copies alarm values into structure passed as pointer
 *
 *****************************************************************************/
uint8_t Get_Alarm( Alarm_t * alrPtr, uint8_t alrNum )
{
    if ( alrNum > NUM_ALARMS ) {
        return 0;
    } else {
        alrPtr->Hour_u8 = Alarms_a[ alrNum ].Hour_u8;
        alrPtr->Minute_u8 = Alarms_a[ alrNum ].Minute_u8;
        alrPtr->Second_u8 = Alarms_a[ alrNum ].Second_u8;
        alrPtr->Day_u8 = Alarms_a[ alrNum ].Day_u8;
        return 1;
    }
} /* Get_Alarm */
/*---------------------------------------------------------------------------*/
