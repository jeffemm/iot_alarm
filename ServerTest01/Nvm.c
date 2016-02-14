/*
 * Nvm.c
 *
 *  Created on: Mar 29, 2014
 *      Author: jje
 */

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/
#include <stdint.h>
#include <Arduino.h>
//#include <EEPROM.h>
#include "Eeprom_C.h"

#include "common_constants.h"

#include "Nvm.h"

/*****************************************************************************
 * TYPES
 *****************************************************************************/
typedef struct {
    uint16_t Addr_u16;  /* EEPROM address for start of block */
    uint8_t Size_u8;    /* Size of block in bytes */
    union {
        struct {
            uint8_t UseCRC_bt:1;
            uint8_t unused1_bt:1;
            uint8_t unused2_bt:1;
            uint8_t unused3_bt:1;
            uint8_t unused4_bt:1;
            uint8_t unused5_bt:1;
            uint8_t unused6_bt:1;
            uint8_t unused7_bt:1;
        } b;
        uint8_t w_u8;
    } Flags;
} NVM_Block_t;

/*****************************************************************************
 * CONSTANTS
 *****************************************************************************/

static const NVM_Block_t NVM_Blocks_sa[NUM_NVM_BLOCKS] = {
    { /* Block 0 - NVM_ALARM_0 */
        0, /* Addr_u16 */
        5, /* Size_u8 (does not include CRC) */
        { /* Flags */
            0, /* UseCRC_bt */
            0, /* unused1_bt */
            0, /* unused2_bt */
            0, /* unused3_bt */
            0, /* unused4_bt */
            0, /* unused5_bt */
            0, /* unused6_bt */
            0, /* unused7_bt */
        }
    },
    { /* Block 1 - NVM_ALARM_1 */
        6, /* Addr_u16 */
        5, /* Size_u8 (does not include CRC) */
        { /* Flags */
            0, /* UseCRC_bt */
            0, /* unused1_bt */
            0, /* unused2_bt */
            0, /* unused3_bt */
            0, /* unused4_bt */
            0, /* unused5_bt */
            0, /* unused6_bt */
            0, /* unused7_bt */
        }
    },
    { /* Block 2 - NVM_ALARM_2 */
        12, /* Addr_u16 */
        5, /* Size_u8 (does not include CRC) */
        { /* Flags */
            0, /* UseCRC_bt */
            0, /* unused1_bt */
            0, /* unused2_bt */
            0, /* unused3_bt */
            0, /* unused4_bt */
            0, /* unused5_bt */
            0, /* unused6_bt */
            0, /* unused7_bt */
        }
    },
    { /* Block 3 - NVM_ALARM_3 */
        18, /* Addr_u16 */
        5, /* Size_u8 (does not include CRC) */
        { /* Flags */
            0, /* UseCRC_bt */
            0, /* unused1_bt */
            0, /* unused2_bt */
            0, /* unused3_bt */
            0, /* unused4_bt */
            0, /* unused5_bt */
            0, /* unused6_bt */
            0, /* unused7_bt */
        }
    },
};


/*****************************************************************************
 * GLOBALS
 *****************************************************************************/

/*****************************************************************************
 * LOCALS
 *****************************************************************************/

/*****************************************************************************
 * LOCAL FUNCTION DECLARATION
 *****************************************************************************/

/*****************************************************************************
 * FUNCTION DEFINITION
 *****************************************************************************/

/*****************************************************************************
 *  FUNCTION: NVM_Write
 *
 *  INPUTS:
 *      NVM_BlockId_t p_BlockId
 *          MIN: 0
 *          MAX: NUM_NVM_BLOCKS
 *          DESC: NVM block ID for NVM_Blocks_sa
 *      uint8_t *p_Buf
 *          DESC: pointer to buffer to read EEPROM data from, if NULL return
 *                value will be FALSE
 *
 *  OUTPUTS:
 *      uint8_t (return)
 *          TRUE - write was successful
 *          FALSE - write failed
 *
 *  MODULE VARIABLES:
 *      NVM_Blocks_sa used
 *
 *  DESCRIPTION:
 *      This function writes data to NVM from RAM at the address
 *      of the provided pointer.
 *
 *****************************************************************************/
uint8_t NVM_Write( NVM_BlockId_t p_BlockId, uint8_t *p_Buf )
{
    uint8_t l_return_u8 = FALSE;
    uint8_t l_inx_u8;
    uint16_t l_EEPROM_addr_u8;
    uint8_t *l_ptr_u8;

    if (  ( p_BlockId < NUM_NVM_BLOCKS )
       && ( p_Buf != NULL ) ) {
        l_ptr_u8 = p_Buf;
        l_EEPROM_addr_u8 = NVM_Blocks_sa[p_BlockId].Addr_u16;
        for ( l_inx_u8 = 0; l_inx_u8 < NVM_Blocks_sa[p_BlockId].Size_u8; l_inx_u8++ ) {
            //EEPROM.write( l_EEPROM_addr_u8++, *l_ptr_u8++ );
            EepromWrite( l_EEPROM_addr_u8++, *l_ptr_u8++ );
        }

#if 0 /* CRC not implemented */
        if ( NVM_Blocks_sa[p_BlockId].Flags.b.UseCRC_bt ) {
            /* Calculate CRC and store in EEPROM */
            uint8_t l_crc_u8 = 0;

            l_crc_u8 = CRC8_Calculate( NVM_Blocks_sa[ p_BlockId ].Size_u8, p_Buf );
            EEPROM.write( l_EEPROM_addr_u8, l_crc_u8 );

            //Serial.print( "DEBUG: CRC " );
            //Serial.println( l_crc_u8, DEC );

            l_return_u8 = TRUE;
        } /* NVM_Blocks_sa[p_BlockId].Flags.b.UseCRC  */
#endif
    } else {
        //Serial.println( "DEBUG: ERROR NVM_Write invalid block ID, or NULL pointer!" );
        l_return_u8 = FALSE;
    } /* p_BlockId < NUM_NVM_BLOCKS */

    return l_return_u8;
} /* NVM_Write */
/*---------------------------------------------------------------------------*/
/*****************************************************************************
 *  FUNCTION: NVM_Read
 *
 *  INPUTS:
 *      NVM_BlockId_t p_BlockId
 *          MIN: 0
 *          MAX: NUM_NVM_BLOCKS
 *          DESC: NVM block ID for NVM_Blocks_sa
 *      uint8_t *p_Buf
 *          DESC: pointer to buffer to place EEPROM data, if NULL return value
 *                will be FALSE
 *
 *  OUTPUTS:
 *      uint8_t (return)
 *          TRUE - read was successful
 *          FALSE - read failed
 *
 *  MODULE VARIABLES:
 *      NVM_Blocks_sa used
 *
 *  DESCRIPTION:
 *      This function reads data from NVM into RAM at the address
 *      of the provided pointer.
 *
 *****************************************************************************/
uint8_t NVM_Read( NVM_BlockId_t p_BlockId, uint8_t *p_Buf )
{
    uint8_t l_return_u8 = FALSE;
    uint8_t l_inx_u8;
    uint16_t l_EEPROM_addr_u8;
    uint8_t l_crc_nvm_u8;
    uint8_t *l_ptr_u8;

    if (  ( p_BlockId < NUM_NVM_BLOCKS )
       && ( p_Buf != NULL ) ) {
        l_EEPROM_addr_u8 = NVM_Blocks_sa[p_BlockId].Addr_u16;
        l_ptr_u8 = p_Buf;
        for ( l_inx_u8 = 0; l_inx_u8 < NVM_Blocks_sa[p_BlockId].Size_u8; l_inx_u8++ ) {
            //*l_ptr_u8++ = EEPROM.read( l_EEPROM_addr_u8++ );
            *l_ptr_u8++ = EepromRead( l_EEPROM_addr_u8++ );
        }
        //l_crc_nvm_u8 = EEPROM.read( l_EEPROM_addr_u8 );
#if 0 /* CRC not implemented */
        l_crc_nvm_u8 = EepromRead( l_EEPROM_addr_u8 );

        if ( NVM_Blocks_sa[p_BlockId].Flags.b.UseCRC_bt ) {
            /* CRC calculation and comparison */
            uint8_t l_crc_calc_u8 = 0;

            l_crc_calc_u8 = CRC8_Calculate( NVM_Blocks_sa[ p_BlockId ].Size_u8, p_Buf );

            if ( l_crc_calc_u8 == l_crc_nvm_u8 ) {
                l_return_u8 = TRUE;
            } else {
                //Serial.print( "DEBUG: ERROR NVM_Read CRC mismatch: NVM - " );
                //Serial.print( l_crc_nvm_u8, DEC );
                //Serial.print( " RAM - " );
                //Serial.println( l_crc_calc_u8, DEC );

                l_return_u8 = FALSE;
            }
        } else {
            l_return_u8 = TRUE;
        } /* NVM_Blocks_sa[p_BlockId].Flags.b.UseCRC  */
#else
        l_return_u8 = TRUE;
#endif
    } else {
        //Serial.println( "DEBUG: ERROR NVM_Read invalid block ID, or NULL pointer!" );
        l_return_u8 = FALSE;
    } /* p_BlockId < NUM_NVM_BLOCKS */

    return l_return_u8;
} /* NVM_Read */
/*---------------------------------------------------------------------------*/








