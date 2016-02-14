/*
 * debug_macros.h
 *
 *  Created on: Mar 29, 2014
 *      Author: jje
 */

#ifndef DEBUG_MACROS_H_
#define DEBUG_MACROS_H_


#define DEBUG_OUT(en,str) do {          \
      if ( en ) {                       \
        Serial.println( F(str) );       \
        delay( 0 );                     \
      }                                 \
    } while(0)

#define CDEBUG_OUT(en,str) do {         \
      if ( en ) {                       \
        SerialPrintln( str );           \
        delay( 0 );                     \
      }                                 \
    } while(0)


#define DEBUG_OUT_CHR(en,chr) do {      \
      if ( en ) {                       \
        Serial.write( chr );            \
        Serial.println( F("") );        \
        delay( 0 );                     \
      }                                 \
    } while(0)



#endif /* DEBUG_MACROS_H_ */
