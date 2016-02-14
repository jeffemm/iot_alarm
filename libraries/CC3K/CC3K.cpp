/*
 * CC3K.cpp
 *
 *  Created on: Mar 8, 2014
 *      Author: jje
 */

#include "CC3K.h"


// These global variables are needed by the SPI drivers in ccspi.cpp
uint8_t g_csPin, g_irqPin, g_vbatPin, g_IRQnum, g_SPIspeed;
// Variables used by CC3000 callback from Adafruit library
volatile unsigned long ulSmartConfigFinished;
volatile unsigned long ulCC3000Connected;
volatile unsigned long ulCC3000DHCP;
volatile unsigned long OkToDoShutDown;
volatile unsigned long ulCC3000DHCP_configured;
volatile unsigned char ucStopSmartConfig;

uint8_t pingReportnum;
netapp_pingreport_args_t pingReport;
#define MAX_SOCKETS 32  // can change this
boolean closed_sockets[MAX_SOCKETS] = {false, false, false, false};

// TODO make this a non-constant object member
const char * CC3K::p_mDNSname = "Arduino";


// Callback function needed for CC3000 host driver library, copied from
// Adafruit CC3000 library wit modifications.
//*****************************************************************************
//
//! CC3000_UsynchCallback
//!
//! @param  lEventType Event type
//! @param  data
//! @param  length
//!
//! @return none
//!
//! @brief  The function handles asynchronous events that come from CC3000
//!         device and operates a led for indicate
//
//*****************************************************************************
void CC3000_UsynchCallback(long lEventType, char * data, unsigned char length)
{
  if (lEventType == HCI_EVNT_WLAN_ASYNC_SIMPLE_CONFIG_DONE)
  {
    ulSmartConfigFinished = 1;
    ucStopSmartConfig     = 1;
  }

  if (lEventType == HCI_EVNT_WLAN_UNSOL_CONNECT)
  {
    ulCC3000Connected = 1;
  }

  if (lEventType == HCI_EVNT_WLAN_UNSOL_DISCONNECT)
  {
    ulCC3000Connected = 0;
    ulCC3000DHCP      = 0;
    ulCC3000DHCP_configured = 0;
  }

  if (lEventType == HCI_EVNT_WLAN_UNSOL_DHCP)
  {
    ulCC3000DHCP = 1;
  }

  if (lEventType == HCI_EVENT_CC3000_CAN_SHUT_DOWN)
  {
    OkToDoShutDown = 1;
  }

  if (lEventType == HCI_EVNT_WLAN_ASYNC_PING_REPORT)
  {
    //PRINT_F("CC3000: Ping report\n\r");
    pingReportnum++;
    memcpy(&pingReport, data, length);
  }

  if (lEventType == HCI_EVNT_BSD_TCP_CLOSE_WAIT) {
    uint8_t socketnum;
    socketnum = data[0];
    //PRINT_F("TCP Close wait #"); printDec(socketnum);
    if (socketnum < MAX_SOCKETS)
      closed_sockets[socketnum] = true;
  }
}




#if 0
CC3K::CC3K( uint8_t csPin, uint8_t irqPin, uint8_t vbatPin )
    : p_cc3000( csPin, irqPin, vbatPin, SPI_CLOCK_DIVIDER )
#endif
CC3K::CC3K()
{

	p_State = CC3K_STATE_INIT;
	p_Command = CC3K_CMD_NONE;
    //p_SSID_Set = false;
    //p_Password_Set = false;
    //p_WLAN_Security = WLAN_SECURITY_NOTSET;

	g_csPin = p_CSpin;
	g_irqPin = p_IRQpin;
	g_vbatPin = p_VbatPin;
	g_IRQnum = p_IRQnum;
	g_SPIspeed = p_SPIspeed;

    p_IPaddress = 0;
    p_Netmask = 0;
    p_Gateway = 0;
    p_DHCPserver = 0;
    p_DNSserver = 0;
    p_AutoReconnect = 0;
    p_UseSmartConfig = 1;
    p_ReconnectTime = 60000;
}

CC3K::~CC3K() {
	// TODO Auto-generated destructor stub
}

bool CC3K::RetrieveIP( void )
{
    Serial.println( F("Getting IP info...") );
    if ( ulCC3000DHCP ) {
        tNetappIpconfigRetArgs ipconfig;
        netapp_ipconfig(&ipconfig);

        /* If byte 1 is 0 we don't have a valid address */
        if (ipconfig.aucIP[3] == 0) {
            return false;
        }

        p_IPaddress = ( uint32_t )( *(( uint32_t * )( ipconfig.aucIP )) );
        p_Netmask = ( uint32_t )( *(( uint32_t * )( ipconfig.aucSubnetMask )) );
        p_Gateway = ( uint32_t )( *(( uint32_t * )( ipconfig.aucDefaultGateway )) );
        p_DHCPserver = ( uint32_t )( *(( uint32_t * )( ipconfig.aucDHCPServer )) );
        p_DNSserver = ( uint32_t )( *(( uint32_t * )( ipconfig.aucDNSServer )) );

        #ifdef CC3K_DEBUG_L2
            // Print the IP address info to serial port
            uint8_t l_addr_byte;
            Serial.print( "IP Address: " );
            l_addr_byte = ( uint8_t )( 0xFF & ( p_IPaddress >> 24 ) );
            Serial.print( l_addr_byte, DEC );
            Serial.print( "." );
            l_addr_byte = ( uint8_t )( 0xFF & ( p_IPaddress >> 16 ) );
            Serial.print( l_addr_byte, DEC );
            Serial.print( "." );
            l_addr_byte = ( uint8_t )( 0xFF & ( p_IPaddress >> 8 ) );
            Serial.print( l_addr_byte, DEC );
            Serial.print( "." );
            l_addr_byte = ( uint8_t )( 0xFF & ( p_IPaddress ) );
            Serial.println( l_addr_byte, DEC );
        #endif

        return true;
    } else {
        Serial.println( F("Couln't get IP info.") );
        return false;
    }
} // CC3K::RetrieveIP
/*---------------------------------------------------------------------------*/

bool CC3K::Connected( void )
{
    return ( p_State == CC3K_STATE_CONNECTED );
} // CC3K::Connected
/*---------------------------------------------------------------------------*/

void CC3K::EnableSmartConfig( void )
{
    p_UseSmartConfig = true;
} // CC3K::EnableSmartConfig
/*---------------------------------------------------------------------------*/

void CC3K::DisableSmartConfig( void )
{
    p_UseSmartConfig = false;
} // CC3K::DisableSmartConfig
/*---------------------------------------------------------------------------*/

void CC3K::EnableAutoReconnect( void )
{
    p_AutoReconnect = true;
} // CC3K::EnableAutoReconnect
/*---------------------------------------------------------------------------*/

void CC3K::DisableAutoReconnect( void )
{
    p_AutoReconnect = false;
} // CC3K::DisableAutoReconnect
/*---------------------------------------------------------------------------*/

#if 0
uint8_t CC3K::SetSSID( char *ssid, uint8_t len )
{
    uint8_t l_return_u8 = false;
    uint8_t l_inx_u8;

    if (  ( ssid != NULL ) 
       && ( len <= 16 ) ) {
        for ( l_inx_u8 = 0; l_inx_u8 < len; l_inx_u8++ ) {
            p_WLAN_SSID[ l_inx_u8 ] = ssid[ l_inx_u8 ];
        }
        p_SSID_Set = true;

        l_return_u8 = true;
    } else {
        l_return_u8 = false;
    }

    return l_return_u8;
} // CC3K::SetSSID
/*---------------------------------------------------------------------------*/
#endif

#if 0
uint8_t CC3K::SetPassword( char *password, uint8_t len )
{
    uint8_t l_return_u8 = false;
    uint8_t l_inx_u8;

    if (  ( password != NULL ) 
       && ( len <= 16 ) ) {
        for ( l_inx_u8 = 0; l_inx_u8 < len; l_inx_u8++ ) {
            p_WLAN_Password[ l_inx_u8 ] = password[ l_inx_u8 ];
        }
        p_Password_Set = true;

        l_return_u8 = true;
    } else {
        l_return_u8 = false;
    }

    return l_return_u8;
} // CC3K::SetPassword
/*---------------------------------------------------------------------------*/
#endif

#if 0
uint8_t CC3K::SetSecurity( CC3K::WLAN_Security_t security )
{
    p_WLAN_Security = security;

    return true;
} // CC3K::SetSecurity
/*---------------------------------------------------------------------------*/
#endif

/*****************************************************************************
 *  FUNCTION: CC3K::Connect
 *
 *  INPUTS:
 *      (none)
 *
 *  OUTPUTS:
 *      uint8_t return
 *          true command was accepted
 *          false command was not accepted
 *
 *  MODULE VARIABLES:
 *      Command_t p_State
 *          read & write
 *
 *  DESCRIPTION:
 *      This function sets the value of p_Command to CC3K_CMD_CONNECT
 *      if there is not another command already set.
 *
 *****************************************************************************/
uint8_t CC3K::Connect( void )
{
    uint8_t l_return_u8 = false;

#if 0
    if (  ( true == p_SSID_Set )
       && ( true == p_Password_Set )
       && ( WLAN_SECURITY_NOTSET != p_WLAN_Security )
       && ( CC3K_CMD_NONE == p_Command ) ) {
#endif

    if ( CC3K_CMD_NONE == p_Command ) {
        p_Command = CC3K_CMD_CONNECT;

        l_return_u8 = true;
    } else {
        l_return_u8 = false;
    }

    return l_return_u8;
} /* CC3K::Connect */
/*---------------------------------------------------------------------------*/

/*****************************************************************************
 *  FUNCTION: CC3K::Disconnect
 *
 *  INPUTS:
 *      (none)
 *
 *  OUTPUTS:
 *      uint8_t return
 *          true command was accepted
 *          false command was not accepted
 *
 *  MODULE VARIABLES:
 *      Command_t p_State
 *          read & write
 *
 *  DESCRIPTION:
 *      This function sets the value of p_Command to CC3K_CMD_DISCONNECT
 *      if there is not another command already set.
 *
 *****************************************************************************/
uint8_t CC3K::Disconnect( void )
{
    uint8_t l_return_u8 = false;

    if ( CC3K_CMD_NONE == p_Command ) {
        p_Command = CC3K_CMD_DISCONNECT;

        l_return_u8 = true;
    } else {
        l_return_u8 = false;
    }

    return l_return_u8;
} /* CC3K::Disconnect */
/*---------------------------------------------------------------------------*/

/*****************************************************************************
 *  FUNCTION: CC3K::Manager
 *
 *  INPUTS:
 *      (none)
 *
 *  OUTPUTS:
 *      (none)
 *
 *  MODULE VARIABLES:
 *      State_t p_State
 *          read & write
 *      Command_t p_State
 *          read & write
 *      Adafruit_CC3000 p_cc3000
 *          object used
 *
 *  DESCRIPTION:
 *      This function manages the CC3000 Wifi Module.
 *
 *****************************************************************************/
void CC3K::Manager( void )
{
    static unsigned long l_millis_prev = 0;
    unsigned long l_millis_now;
    State_t l_State_Next = CC3K_STATE_ERROR;

    l_millis_now = millis();


    if ( CC3K_STATE_INIT == p_State ) {
        l_State_Next = CC3K_STATE_DISCONNECTED;
    } else if ( CC3K_STATE_CONNECT == p_State ) {
        /* Initialize the CC3000 Wifi module */
        #ifdef CC3K_DEBUG_L1
            Serial.println( "\nCC3K-CONNECT" );
        #endif

        // TODO should init_spi() and wlan_init() bemoved to init state
        // Thne only be called once. On reconnect only need to call wlan_start()
        // (Do other functions also need to be called everytime wlan_start() is if
        // using SmartConfig???)
        init_spi();
        #ifdef CC3K_DEBUG_L1
            Serial.println( "\nwlan_init()..." );
        #endif
        wlan_init(CC3000_UsynchCallback,
                  sendWLFWPatch, sendDriverPatch, sendBootLoaderPatch,
                  ReadWlanInterruptPin,
                  WlanInterruptEnable,
                  WlanInterruptDisable,
                  WriteWlanPin);
        #ifdef CC3K_DEBUG_L1
            Serial.println( "\nwlan_start()..." );
        #endif
        wlan_start( 0 ); // No patch available
        // Always use smart config
        // TODO check return code
        #ifdef CC3K_DEBUG_L1
            Serial.println( "\nwlan_ioctl_set_connection_policy()..." );
        #endif
        wlan_ioctl_set_connection_policy(0, 0, 1);

        // TODO check return code
        #ifdef CC3K_DEBUG_L1
            Serial.println( "\nwlan_set_event_mask()..." );
        #endif
        wlan_set_event_mask( HCI_EVNT_WLAN_UNSOL_INIT        |
                             //HCI_EVNT_WLAN_ASYNC_PING_REPORT |// we want ping reports
                             //HCI_EVNT_BSD_TCP_CLOSE_WAIT |
                             //HCI_EVNT_WLAN_TX_COMPLETE |
                             HCI_EVNT_WLAN_KEEPALIVE );
        // Adafruit library polls interrupt and waits for connect.
        // Callback function sets connected flag, got to DHCP wait state for
        // connected flag polling. (Don't worry about polling interrupt pin!)
        l_State_Next = CC3K_STATE_DHCP_WAIT;


    } else if ( CC3K_STATE_DHCP_WAIT == p_State ) {
        #ifdef CC3K_DEBUG_L1
            Serial.println( "\nCC3K-DHCP_WAIT" );
        #endif
        // Check for connected flag set in callback
        if ( ulCC3000Connected ) {
            // Connected
            // Also check DHCP flag
            if ( ulCC3000DHCP ) {
                // TODO get IP address - RetrieveIP()
                RetrieveIP();
                // TODO this doesn't seem to work, not sure if it's needed
                // with Adafruit MDNSResponder
                // Enable multicast DNS and give this thing a name
                mdnsAdvertiser( 1, (char *) p_mDNSname, strlen( p_mDNSname ) );

                // TODO this does not work correct with currrent CC3000
                // firmware version, known bug. Need to look for update,
                // and install.
                if( !p_mdns.begin( p_mDNSname, p_IPaddress, 3600 ) ) {
                    // TODO mdns failed???
                    Serial.println( F("mdns.begin failed") );
                }
                Serial.println( F("mdns.begin done.") );

                l_State_Next = CC3K_STATE_CONNECTED;
            } else {
                l_State_Next = CC3K_STATE_DHCP_WAIT;
            }
        } else {
            // Not connected
            l_State_Next = CC3K_STATE_DHCP_WAIT;
        } // ulCC3000Connected


    } else if ( CC3K_STATE_CONNECTED == p_State ) {
        if ( CC3K_CMD_DISCONNECT == p_Command ) {
                l_State_Next = CC3K_STATE_DISCONNECT;
                p_Command = CC3K_CMD_NONE;
        } else {
            if ( ulCC3000Connected ) {
                p_mdns.update();
                l_State_Next = CC3K_STATE_CONNECTED;
            } else {
                if ( p_AutoReconnect ) {
                    l_millis_prev = l_millis_now;
                    l_State_Next = CC3K_STATE_RECONNECT_WAIT;
                } else {
                    l_State_Next = CC3K_STATE_DISCONNECTED;
                }
            }
        }
    } else if ( CC3K_STATE_DISCONNECT == p_State ) {
        #ifdef CC3K_DEBUG_L1
            Serial.println( "CC3K-DISCONNECT" );
        #endif
        // No need to check return value. O on success, other indicates
        // already disconnected
        // TODO this doesn't actually do anything using SmartConfig
        //    wlan_stop will assert reset pin if there is really
        //    a need for this.
        wlan_disconnect( );
        p_IPaddress = 0;
        p_Netmask = 0;
        p_Gateway = 0;
        p_DHCPserver = 0;
        p_DNSserver = 0;
        l_State_Next = CC3K_STATE_DISCONNECTED;
    } else if ( CC3K_STATE_DISCONNECTED == p_State ) {

        if ( CC3K_CMD_CONNECT == p_Command ) {
                l_State_Next = CC3K_STATE_CONNECT;
                p_Command = CC3K_CMD_NONE;
        } else {
            l_State_Next = CC3K_STATE_DISCONNECTED;
        }
    } else if ( CC3K_STATE_RECONNECT_WAIT == p_State ) {

        if ( p_ReconnectTime < ( l_millis_now - l_millis_prev ) ) {
            l_State_Next = CC3K_STATE_CONNECT;
        } else {
            l_millis_prev = l_millis_now;
            l_State_Next = CC3K_STATE_RECONNECT_WAIT;
        }
    } else if ( CC3K_STATE_ERROR == p_State ) {
        static uint8_t l_flag_u8 = false;

        #ifdef CC3K_DEBUG_L1
            Serial.println( "CC3K-ERROR" );
        #endif

        if ( l_flag_u8 ) {
            /* TODO this only works if there is no way out of error state */
        } else {
            /* In error state try to disconnect just in case */
            wlan_disconnect( );
            #if 0
            p_cc3000.disconnect();
#endif
            l_flag_u8 = true;
        }

        /* TODO no way out of error state */
        l_State_Next = CC3K_STATE_ERROR;
    } else {
        l_State_Next = CC3K_STATE_ERROR;
    } /* p_State */

    p_State = l_State_Next;
} /* CC3K::Manager */
/*---------------------------------------------------------------------------*/


