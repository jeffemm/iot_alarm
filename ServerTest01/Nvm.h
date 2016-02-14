/*
 * Nvm.h
 *
 *  Created on: Mar 29, 2014
 *      Author: jje
 */

#ifndef NVM_H_
#define NVM_H_

/*****************************************************************************
 * INCLUDES
 *****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************
 * PUBLIC TYPES
 *****************************************************************************/
typedef enum {
    NVM_ALARM_0,
    NVM_ALARM_1,
    NVM_ALARM_2,
    NVM_ALARM_3,
    NUM_NVM_BLOCKS
} NVM_BlockId_t;


/*****************************************************************************
 * PUBLIC FUNCTION DECLARATIONS
 *****************************************************************************/
uint8_t NVM_Write( NVM_BlockId_t p_BlockId, uint8_t *p_Buf );
uint8_t NVM_Read( NVM_BlockId_t p_BlockId, uint8_t *p_Buf );




#ifdef __cplusplus
} // extern "C"
#endif

#endif /* NVM_H_ */
