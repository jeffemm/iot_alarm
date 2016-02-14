/*
 * Clock.c
 *
 *  Created on: Mar 29, 2014
 *      Author: jje
 */

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include <stdint.h>
#include "Arduino.h"
#include "common_constants.h"

#include "debug_macros.h"

#include "Clock.h"


/*****************************************************************************
 * TYPES
 *****************************************************************************/

/*****************************************************************************
 * CONSTANTS
 *****************************************************************************/

/*****************************************************************************
 * GLOBALS
 *****************************************************************************/

/*****************************************************************************
 * LOCALS
 *****************************************************************************/
static Clock_t LocalTime = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

/*****************************************************************************
 * LOCAL FUNCTION DECLARATION
 *****************************************************************************/

/*****************************************************************************
 * FUNCTION DEFINITION
 *****************************************************************************/

/*****************************************************************************
 *  FUNCTION: Update_Time
 *
 *  INPUTS:
 *      uint8_t p_Init
 *          DESC: indicates to the function that this is the first
 *                time being called after a reset.
 *          TRUE - first call after a reset, any locallly stored
 *                 static data should be considered invalid, and value
 *                 of millis() has been changed in unpredictable manner
 *          FALSE - not the first call after a reset, locally stored
 *                  static data should be considered valid, and value
 *                  of millis() should be updating in expected manner
 *
 *  OUTPUTS:
 *      0 - time didn't change
 *      1 - time changed
 *
 *  DESCRIPTION:
 *      Function updates the value of the clock time. Clock time is stored
 *      in file scoped variables.
 *
 *      This function utilises the miilis() library function to determine
 *      when one second has passed and updates the clock. If not called faster
 *      than once a second this function may miss updates.
 *
 *      This function assumes the next day starts at time 0hr 0min 0sec.
 *
 *****************************************************************************/
uint8_t Update_Time( uint8_t p_Init )
{
    static unsigned long l_millis_prev = 0;
    unsigned long l_millis_now;
    unsigned long l_millis_diff;
    uint8_t l_ret = 0;

    l_millis_now = millis();

    CDEBUG_OUT( DEBUG_OUT_CLOCK, "CLK:0" );
    if ( p_Init ) {
        CDEBUG_OUT( DEBUG_OUT_CLOCK, "CLK:1" );
        l_millis_prev = l_millis_now;
        l_ret = 0;
    } else {
        CDEBUG_OUT( DEBUG_OUT_CLOCK, "CLK:2" );
        /* This should be roll-over safe, probably would be a good idea
         * to test it sometime. millis() value will roll-over after ~50 days.
         */
        l_millis_diff = l_millis_now - l_millis_prev;
        if ( l_millis_diff >= MILLIS_PER_SECOND ) {
            l_millis_prev += MILLIS_PER_SECOND;

            CDEBUG_OUT( DEBUG_OUT_CLOCK, "CLK:3" );
            l_ret = 1;

            if ( ( 0xFF == LocalTime.Second_u8 )
              || ( 0xFF == LocalTime.Minute_u8 )
              || ( 0xFF == LocalTime.Hour_u8 )
              || ( 0xFF == LocalTime.Day_u8 ) ) {
                /* Clock has not been set since last reset, don't update it. */
            } else {
                CDEBUG_OUT( DEBUG_OUT_CLOCK, "CLK:4" );
                LocalTime.Second_u8++;
                if ( LocalTime.Second_u8 > 59 ) {
                    LocalTime.Second_u8 = 0;
                    LocalTime.Minute_u8++;
                    if ( LocalTime.Minute_u8 > 59 ) {
                        LocalTime.Minute_u8 = 0;
                        LocalTime.Hour_u8++;
                        if ( LocalTime.Hour_u8 > 23 ) {
                            LocalTime.Hour_u8 = 0;
                            LocalTime.Day_u8 <<= 1;
                            if ( LocalTime.Day_u8 > 0x40 ) {
                                LocalTime.Day_u8 = 0x1;
                            } /* Time_s.Day_u8 > 0x40 */
                        } /* Time_s.Hour_u8 > 23 */
                    } /* Time_s.Minute_u8 > 59 */
                } /* Time_s.Second_u8 > 59 */
            } /* clock value valid */
        } else {
            l_ret = 0;
        } /* l_millis_diff > MILLIS_PER_SECOND */
    } /* p_Init */

    return l_ret;
} /* Update_Time */
/*---------------------------------------------------------------------------*/

/*****************************************************************************
 *  FUNCTION: Set_Time
 *
 *  INPUTS:
 *      uint8_t p_Hour
 *          MAX: 23
 *          MIN: 0
 *          DESC: zero based hour of the day in 24 hour format
 *      uint8_t p_Minute
 *          MAX: 59
 *          MIN: 0
 *          DESC: zero based minute of the hour
 *      uint8_t p_Second
 *          MAX: 59
 *          MIN: 0
 *          DESC: zero based second of the minute
 *      uint8_t p_Day
 *          ALLOWED: 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40
 *          DESC: day of the week, value should be one of the defined constants
 *
 *  OUTPUTS:
 *      uint8_t (return)
 *          TRUE - clock time was set
 *          FALSE - clock time was NOT changed, probably due to an out of
 *                  range parameter value.
 *
 *  DESCRIPTION:
 *      Sets the time to the values provided. Updates the file scoped
 *      variables that store the clock time.
 *
 *****************************************************************************/
uint8_t Set_Time( uint8_t p_Hour, uint8_t p_Minute, uint8_t p_Second, uint8_t p_Day )
{
    uint8_t l_result_u8 = FALSE;

    if (   ( p_Hour < 24 )
        && ( p_Minute < 60 )
        && ( p_Second < 60 )
        && (  ( CLOCK_DAY_MON == p_Day )
           || ( CLOCK_DAY_TUE == p_Day )
           || ( CLOCK_DAY_WED == p_Day )
           || ( CLOCK_DAY_THU == p_Day )
           || ( CLOCK_DAY_FRI == p_Day )
           || ( CLOCK_DAY_SAT == p_Day )
           || ( CLOCK_DAY_SUN == p_Day ) ) ) {
        /* Input values are all valid, set the clock */
        LocalTime.Hour_u8 = p_Hour;
        LocalTime.Minute_u8 = p_Minute;
        LocalTime.Second_u8 = p_Second;
        LocalTime.Day_u8 = p_Day;

        l_result_u8 = TRUE;
    } else {
        l_result_u8 = FALSE;
    } /* input values valid */

    return l_result_u8;
} /* Set_Time */
/*---------------------------------------------------------------------------*/

/*****************************************************************************
 *  FUNCTION: Get_Time
 *
 *  INPUTS:
 *      Clock_t * clkPtr
 *          DESC: pointer to Clock_t structure that will be populated
 *                with current local time values
 *
 *  OUTPUTS:
 *      (none)
 *
 *  DESCRIPTION:
 *      Copies current local time values into structure passed as pointer
 *
 *****************************************************************************/
void Get_Time( Clock_t * clkPtr )
{
    clkPtr->Hour_u8 = LocalTime.Hour_u8;
    clkPtr->Minute_u8 = LocalTime.Minute_u8;
    clkPtr->Second_u8 = LocalTime.Second_u8;
    clkPtr->Day_u8 = LocalTime.Day_u8;
} /* Get_Time */
/*---------------------------------------------------------------------------*/






