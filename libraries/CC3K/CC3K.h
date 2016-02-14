/*
 * CC3K.h
 *
 *  Created on: Mar 8, 2014
 *      Author: jje
 */

#ifndef CC3K_H_
#define CC3K_H_

//#define CC3K_DEBUG_L1
//#define CC3K_DEBUG_L2


//#include <Adafruit_CC3000.h>
//#include <Adafruit_CC3000_Server.h>

#include "Arduino.h"
#include "ccspi.h"
#include "utility/hci.h"
#include "utility/socket.h"
#include "utility/netapp.h"
#include "utility/wlan.h"
#include "CC3000_MDNS.h"


class CC3K {
public:
#if 0
    // Values set to match Adafruit CC3000 libraries
    typedef enum {
        WLAN_SECURITY_UNSEC   = 0,
        WLAN_SECURITY_WEP     = 1,
        WLAN_SECURITY_WPA     = 2,
        WLAN_SECURITY_WPA2    = 3,
        WLAN_SECURITY_NOTSET,
    } WLAN_Security_t;
#endif

    CC3K( );
	//CC3K( uint8_t csPin, uint8_t irqPin, uint8_t vbatPin );
	virtual ~CC3K();

	void Manager( void );
	uint8_t Connect( void );
	uint8_t Disconnect( void );
	bool RetrieveIP( void );
    bool Connected( void );
    void EnableAutoReconnect( void );
    void DisableAutoReconnect( void );
    void EnableSmartConfig( void );
    void DisableSmartConfig( void );
    //uint8_t SetSSID( char *ssid, uint8_t len );
    //uint8_t SetPassword( char *password, uint8_t len );
    //uint8_t SetSecurity( CC3K::WLAN_Security_t security );


private:
	typedef enum {
	    CC3K_STATE_INIT,
	    CC3K_STATE_CONNECT,
	    //CC3K_STATE_DHCP_REQ,
	    CC3K_STATE_DHCP_WAIT,
	    CC3K_STATE_CONNECTED,
	    CC3K_STATE_DISCONNECT,
	    CC3K_STATE_DISCONNECTED,
	    CC3K_STATE_RECONNECT_WAIT,
	    CC3K_STATE_ERROR,
	} State_t;

	typedef enum {
	    CC3K_CMD_NONE,
	    CC3K_CMD_CONNECT,
	    CC3K_CMD_DISCONNECT,
	} Command_t;

    //static const uint8_t FALSE = 0;
    //static const uint8_t TRUE = 1;

    static const uint32_t CC3K_DHCP_WAIT_MS = 60000;

    // Values below are for Uno board
    static const uint8_t p_CSpin = 10;
    static const uint8_t p_IRQpin = 3;
    static const uint8_t p_IRQnum = 1;
    static const uint8_t p_VbatPin = 5;
    static const uint8_t p_SPIspeed = SPI_CLOCK_DIV2;
    static const char * p_mDNSname;

    MDNSResponder    p_mdns;

	State_t          p_State;
	Command_t        p_Command;
	//Adafruit_CC3000  p_cc3000;
    uint8_t          p_SSID_Set;
    //uint8_t          p_Password_Set;
    uint8_t          p_AutoReconnect;
    uint8_t          p_UseSmartConfig;
    // TODO add a function to set reconnect time
    uint32_t         p_ReconnectTime;
    //char             p_WLAN_SSID[33];
    //char             p_WLAN_Password[16];
    //WLAN_Security_t  p_WLAN_Security;
    uint32_t         p_IPaddress;
    uint32_t         p_Netmask;
    uint32_t         p_Gateway;
    uint32_t         p_DHCPserver;
    uint32_t         p_DNSserver;
};

#endif /* CC3K_H_ */
