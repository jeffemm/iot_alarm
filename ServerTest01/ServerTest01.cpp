///////////////////////////////////////////////////////////////////////////////
// INCLUDE FILES
///////////////////////////////////////////////////////////////////////////////
#include "ServerTest01.h"
#include "debug_macros.h"
#include "common_constants.h"
#include "CC3K.h"
#include "utility/socket.h"
#include "utility/netapp.h"

#include "Clock.h"
#include "AlarmClock.h"
#include "Nvm.h"

///////////////////////////////////////////////////////////////////////////////
// DEBUG OUTPUT CONTROL DEFINES
///////////////////////////////////////////////////////////////////////////////
#define DEBUG_OUTPUT_MAIN             ( 1 )
#define DEBUG_OUTPUT_SEND_RESP        ( 0 )
#define DEBUG_OUTPUT_PIC              ( 0 )
#define DEBUG_OUTPUT_RECV_PROC        ( 1 )
#define DEBUG_OUTPUT_REQ_PROC         ( 0 )
#define DEBUG_OUTPUT_STATE            ( 1 )
#define DEBUG_OUTPUT_SETUP_LISTEN     ( 1 )
#define DEBUG_OUTPUT_BDY              ( 0 )
#define DEBUG_OUTPUT_H_ROOT           ( 0 )
#define DEBUG_OUTPUT_H_LED1           ( 0 )
#define DEBUG_OUTPUT_H_TXT1           ( 0 )
#define DEBUG_OUTPUT_H_TXT2           ( 0 )
#define DEBUG_OUTPUT_FD_STRT          ( 0 )
#define DEBUG_OUTPUT_JSN              ( 0 )


///////////////////////////////////////////////////////////////////////////////
// CONSTANTS
///////////////////////////////////////////////////////////////////////////////
#define NULL_E_PTR  ( (HttpInterfaceElement_t *)0 )
#define SERIAL_BAUD_RATE ( 115200 )
#define MAIN_LOOP_DELAY ( 100 )

#define D_OUT_LED1 ( 8 ) // can't use 13 (on board LED), it's SCK

#define ADAFRUIT_CC3000_IRQ   ( (uint8_t)3 )
#define ADAFRUIT_CC3000_VBAT  ( (uint8_t)5 )
#define ADAFRUIT_CC3000_CS    ( (uint8_t)10 )

#define SERVER_LISTEN_PORT ( 80 ) // listen on port 80 for HTTP request
// TODO figure out what this needs to be, current value is SWAG, include a little extra
// 30 chars should be enough, use 50 for safety
#define HTTP_REQ_LN1_LEN ( 50 ) // max length of first line of http request
// TODO figure out what this needs to be, current value is SWAG, include a little extra
// For sample data 153 chars needed, use 170 to be safe
#define JSON_DATA_LEN_MAX ( 170 ) // max length of JSON data string
#define SOCKET_READ_SIZE ( HTTP_REQ_LN1_LEN + JSON_DATA_LEN_MAX ) // size of char array for reading from a socket for http request
#define SOCKET_WRITE_SIZE ( JSON_DATA_LEN_MAX ) // size of char array for reading from a socket

#define SOCKET_READ_CHUNK ( 90 )
#define SOCKET_SEND_CHUNK ( 90 )


// HTTP Request Processing Status Flag Bit Masks
#define HTTP_REQ_BAD_FLAG       ( 0x0001U )  // bad request
#define HTTP_REQ_ERR_FLAG       ( 0x0002U )  // server error
#define HTTP_REQ_HDR_FLAG       ( 0X0004U )  // end of request header
#define HTTP_REQ_LN1_FLAG       ( 0x0008U )  // end of request header first line
#define HTTP_REQ_CR_FLAG        ( 0x0010U )  // carriage return, used to detect end of header
#define HTTP_REQ_GET_FLAG       ( 0x0020U )  // request method GET
#define HTTP_REQ_PST_FLAG       ( 0x0040U )  // request method POST
//#define HTTP_REQ_URI_FLAG       ( 0x0080U )  // URI complete and valid (currently not used)
#define HTTP_REQ_END_FLAG       ( 0x0100U )  // end of request (header and body)
//#define HTTP_REQ_DAT_FLAG       ( 0x0200U )  // body data complete and valid (currently not used)


///////////////////////////////////////////////////////////////////////////////
// LOCAL TYPEDEFS
///////////////////////////////////////////////////////////////////////////////
typedef enum {
    STATE_INIT,
    STATE_CONNECT,
    STATE_WAIT_CONN,
    STATE_LISTEN,
    STATE_ACCEPT,
    STATE_ERROR
} ServerState_t;

// Maximum number of elements in interface tree
#define MAX_TREE_ELEMENTS 10

/* JSON Data Typedefs
 *  Use to create a tree of JSON data
 */
struct JSONElement_s;
typedef JSONElement_s JSONElement_t;

struct JSONElement_s {
    char *      name;
    uint8_t     len_name;
    char *      data;
    uint8_t     len_data;
    JSONElement_t * child;
    JSONElement_t * sibling;
    JSONElement_t * parent;
};

/* Data structure for defining HTTP interface
 * Each element of URI will have a node in a tree.
 * The root of tree represents an empty URI and name is empty string.
 * The root node has no value, NULL ptr.
 *
 */
struct HttpInterfaceElement_s;
typedef const HttpInterfaceElement_s HttpInterfaceElement_t;

struct HttpInterfaceElement_s {
    const char * const  name;          // element name for URI and JSON data, null terminated
    char *              val;             // pointer to data value in ASCII
    const uint8_t       len;             // data length
    uint8_t (* const func)( char * buf, uint16_t len );   // function pointer to element handler
    HttpInterfaceElement_t * const parent;   // parent node, NULL for root
    HttpInterfaceElement_t * const child;   // first child node, NULL for none
    HttpInterfaceElement_t * const next_sibling; // next sibling node, NULL for none
} ;

// Handler function is only executed for a POST request.
// Expects a point to start of string representing value.
// Handler will validate the contents of the buf char string provided
// len is maximum amount of data available. May be used by handler
// to find closing '}' for data.
// If there is an error in received data return value is 0
// If no problems with data return value is 1


// Forward declarations for all elements
extern HttpInterfaceElement_t e_ROOT;
extern HttpInterfaceElement_t e_LED1;
extern HttpInterfaceElement_t e_Clock;
extern HttpInterfaceElement_t e_Alarms;
extern HttpInterfaceElement_t e_Alarm0;
extern HttpInterfaceElement_t e_Alarm1;
extern HttpInterfaceElement_t e_Alarm2;
extern HttpInterfaceElement_t e_Alarm3;

#if 0
extern HttpInterfaceElement_t e_TXT0;
extern HttpInterfaceElement_t e_TXT1;
extern HttpInterfaceElement_t e_TXT2;

#define E_LEN_TXT2  ( (uint8_t)4U )
static char E_val_TXT2[E_LEN_TXT2] = {'"', 'x', 'y', '"'};
static uint8_t eHandler_TXT2( char * buf, uint16_t len );
HttpInterfaceElement_t e_TXT2 = {
    "TXT2",             // name
    E_val_TXT2,         // val
    E_LEN_TXT2,         // len
    &eHandler_TXT2,     // handler func pointer
    &e_TXT0,           // parent
    NULL_E_PTR,           // child
    NULL_E_PTR,           // next_sibling
};

#define E_LEN_TXT1  ( (uint8_t)4U )
static char E_val_TXT1[E_LEN_TXT1] = {'"', 'A', 'B', '"'};
static uint8_t eHandler_TXT1( char * buf, uint16_t len );
HttpInterfaceElement_t e_TXT1 = {
    "TXT1",             // name
    E_val_TXT1,         // val
    E_LEN_TXT1,         // len
    &eHandler_TXT1,     // handler func pointer
    &e_TXT0,           // parent
    NULL_E_PTR,           // child
    &e_TXT2,           // next_sibling
};

//#define E_LEN_TXT0  ( (uint8_t)0U )
//static char E_val_TXT0[E_LEN_TXT0] = {};
static uint8_t eHandler_TXT0( char * buf, uint16_t len );
HttpInterfaceElement_t e_TXT0 = {
    "TXT0",             // name
    (char *)NULL_PTR,         // val
    0,         // len
    &eHandler_TXT0,     // handler func pointer
    &e_ROOT,           // parent
    &e_TXT1,           // child
    NULL_E_PTR,           // next_sibling
};
#endif

/* Clock/Alarm data format:
 * --------------------------------------------------------------------
 * New Cloc/Alarm Data Format (3 Apr 2014)
 * --------------------------------------------------------------------
 *  - hour, minute, second are 2 digit decimal string values
 *  - for clock day is 1 digit decimal string, 0 is Sunday
 *  - for alarm day is 3 digit decimal string of bit mapped flags value
 *  
 *  Examples:
 *    12:34:56 Tuesday - 1234563
 *    1:59:00 Sunday - 0159000
 *
 *
 *     TODO add alarm flags
 */
static uint8_t eHandler_Alarms( char * buf, uint16_t len );
HttpInterfaceElement_t e_Alarms = {
    "Alarms",             // name
    (char *)NULL_PTR,     // val
    0,                    // len
    &eHandler_Alarms,     // handler func pointer
    &e_ROOT,              // parent
    &e_Alarm0,            // child
    NULL_E_PTR,           // next_sibling
};

#define E_LEN_ALARM  ( (uint8_t)11U )
static char E_val_Alarm0[E_LEN_ALARM] = 
    { '"', '0', '0', '0', '0', '0', '0', '0', '0', '0', '"' };
static uint8_t eHandler_Alarm0( char * buf, uint16_t len );
HttpInterfaceElement_t e_Alarm0 = {
    "A0",              // name
    E_val_Alarm0,      // val
    E_LEN_ALARM,       // len
    &eHandler_Alarm0,  // handler func pointer
    &e_Alarms,         // parent
    NULL_E_PTR,        // child
    &e_Alarm1,         // next_sibling
};

static char E_val_Alarm1[E_LEN_ALARM] = 
    { '"', '0', '0', '0', '0', '0', '0', '0', '0', '0', '"' };
static uint8_t eHandler_Alarm1( char * buf, uint16_t len );
HttpInterfaceElement_t e_Alarm1 = {
    "A1",              // name
    E_val_Alarm1,      // val
    E_LEN_ALARM,       // len
    &eHandler_Alarm1,  // handler func pointer
    &e_Alarms,         // parent
    NULL_E_PTR,        // child
    &e_Alarm2,         // next_sibling
};

static char E_val_Alarm2[E_LEN_ALARM] = 
    { '"', '0', '0', '0', '0', '0', '0', '0', '0', '0', '"' };
static uint8_t eHandler_Alarm2( char * buf, uint16_t len );
HttpInterfaceElement_t e_Alarm2 = {
    "A2",              // name
    E_val_Alarm2,      // val
    E_LEN_ALARM,       // len
    &eHandler_Alarm2,  // handler func pointer
    &e_Alarms,         // parent
    NULL_E_PTR,        // child
    &e_Alarm3,         // next_sibling
};

static char E_val_Alarm3[E_LEN_ALARM] = 
    { '"', '0', '0', '0', '0', '0', '0', '0', '0', '0', '"' };
static uint8_t eHandler_Alarm3( char * buf, uint16_t len );
HttpInterfaceElement_t e_Alarm3 = {
    "A3",              // name
    E_val_Alarm3,      // val
    E_LEN_ALARM,       // len
    &eHandler_Alarm3,  // handler func pointer
    &e_Alarms,         // parent
    NULL_E_PTR,        // child
    NULL_E_PTR,        // next_sibling
};


#define E_LEN_CLOCK  ( (uint8_t)9U )
static char E_val_Clock[E_LEN_CLOCK] =
    { '"', '0', '0', '0', '0', '0', '0', '0', '"' };
static uint8_t eHandler_Clock( char * buf, uint16_t len );
HttpInterfaceElement_t e_Clock = {
    "Clock",             // name
    E_val_Clock,         // val
    E_LEN_CLOCK,         // len
    &eHandler_Clock,     // handler func pointer
    &e_ROOT,             // parent
    NULL_E_PTR,          // child
    &e_Alarms,           // next_sibling
};

#define E_LEN_LED1  ( (uint8_t)1U )
static char E_val_LED1[E_LEN_LED1] = {'0'};
static uint8_t eHandler_LED1( char * buf, uint16_t len );
HttpInterfaceElement_t e_LED1 = {
    "LED1",             // name
    E_val_LED1,         // val
    E_LEN_LED1,         // len
    &eHandler_LED1,     // handler func pointer
    &e_ROOT,           // parent
    NULL_E_PTR,            // child
    &e_Clock,            // next_sibling
};

static uint8_t eHandler_ROOT( char * buf, uint16_t len );
HttpInterfaceElement_t e_ROOT = {
    "",
    (char *)NULL_PTR,
    0,
    &eHandler_ROOT,
    NULL_E_PTR,           // parent
    &e_LED1,        // child
    NULL_E_PTR,       // next_sibling -- for root no sibling
};


///////////////////////////////////////////////////////////////////////////////
// LOCAL DATA
///////////////////////////////////////////////////////////////////////////////
static CC3K theCC3k = CC3K( );

static ServerState_t ServerState;
static int16_t Server_ListenSocket = -1;

///////////////////////////////////////////////////////////////////////////////
// LOCAL FUNCTION DECLARATIONS
///////////////////////////////////////////////////////////////////////////////
static bool SetupListenSocket( void );
static void ProcessIncomingConnections( void );
static void ProcessReceiveStream( int16_t sd );
static void SendHttpResponse( int16_t sd, uint16_t req_flags, char *data, uint16_t len );
static uint8_t SendHttpHeader( int16_t sd, uint8_t req_flags );
static HttpInterfaceElement_t * ProcessHttpRequest( char * ln1_ptr, uint16_t ln1_len, char * body_ptr, uint16_t body_len );
static uint16_t GenerateHttpResponseBody( char * buf, uint16_t len, HttpInterfaceElement_t * eReq );
static uint8_t ProcessPostRequest( HttpInterfaceElement_t * ePtr, char * buf, uint16_t len );
static JSONElement_t * BuildJSONTree( JSONElement_t * eBuf, char * buf, uint16_t len );
static void UpdateInterfaceTime( void );
static void UpdateInterfaceAlarms( void );
static uint8_t eHandler_AlarmX( uint8_t aInx, char * buf, uint16_t len );



///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//
// FUNCTION DEFINITIONS
//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
//
// setup
//
///////////////////////////////////////////////////////////////////////////////
void setup( )
{
    uint8_t l_status_u8;

    Serial.begin( SERIAL_BAUD_RATE );

    DEBUG_OUT( DEBUG_OUTPUT_MAIN, "Starting CC3K_Test application..." );

    pinMode( D_OUT_LED1, OUTPUT );

    ServerState = STATE_INIT;

    if ( FALSE == Read_Alarm_Nvm() ) {
        // TODO alarm NVM read failed
    } else {
        /* Update the alarm HTTP interface elements data */
        UpdateInterfaceAlarms();
    }

    Update_Time ( TRUE );
}

///////////////////////////////////////////////////////////////////////////////
//
// loop
//
///////////////////////////////////////////////////////////////////////////////
void loop( )
{
    ServerState_t l_NextState = STATE_ERROR;
    uint32_t l_now = millis();
    static uint32_t l_then = 0;
    uint8_t l_AlarmStatus = 0;

    theCC3k.Manager( );

    /* NOTE: until time is set, Update_Time will not increment time
     * value, and will never return TRUE.
     */
    if ( Update_Time( FALSE ) ) {
        // Time changed, update the time string in Http Interface Data
        //DEBUG_OUT( DEBUG_OUTPUT_MAIN, "Clock" );
        UpdateInterfaceTime();

        // TODO create alarm action functions and assign
        Check_All_Alarms();
    }

    if ( STATE_INIT == ServerState ) {
        DEBUG_OUT( DEBUG_OUTPUT_STATE, "STATE_INIT");
        l_NextState = STATE_CONNECT;
    } else if ( STATE_CONNECT == ServerState ) {
        DEBUG_OUT( DEBUG_OUTPUT_STATE, "STATE_CONNECT");
        if ( theCC3k.Connect( ) ) {
            l_NextState = STATE_WAIT_CONN;
        } else {
            DEBUG_OUT( DEBUG_OUTPUT_STATE, "STATE_ERROR" );
            l_NextState = STATE_ERROR;
        }
    } else if ( STATE_WAIT_CONN == ServerState ) {
        if ( theCC3k.Connected( ) ) {
            l_NextState = STATE_LISTEN;
        } else {
            l_NextState = STATE_WAIT_CONN;
        }
    } else if ( STATE_LISTEN == ServerState ) {
        DEBUG_OUT( DEBUG_OUTPUT_STATE, "STATE_LISTEN");
        if ( SetupListenSocket( ) ) {
            DEBUG_OUT( DEBUG_OUTPUT_MAIN, "STATE_ACCEPT" );
            l_NextState = STATE_ACCEPT;
        } else {
            DEBUG_OUT( DEBUG_OUTPUT_STATE, "STATE_ERROR");
            l_NextState = STATE_ERROR;
        }
    } else if ( STATE_ACCEPT == ServerState ) {
        if ( (l_now - l_then ) > 1000 ) {
            //DEBUG_OUT( DEBUG_OUTPUT_MAIN, "STATE_ACCEPT" );
            l_then = l_now;
        }
        ProcessIncomingConnections( );
        l_NextState = STATE_ACCEPT; // stay here forever
    } else if ( STATE_ERROR == ServerState ) {
        l_NextState = STATE_ERROR;
    } else {
        DEBUG_OUT( DEBUG_OUTPUT_MAIN, "STATE_ERROR" );
        l_NextState = STATE_ERROR;
    }

    ServerState = l_NextState;
} // loop


///////////////////////////////////////////////////////////////////////////////
//
// UpdateInterfaceTime
//
///////////////////////////////////////////////////////////////////////////////
static void UpdateInterfaceTime( void )
{
    Clock_t l_time;
    uint8_t l_inx = 0;
    uint8_t l_bit;

    Get_Time( &l_time );

    E_val_Clock[1] = (l_time.Hour_u8 / 10) + 0x30U;
    E_val_Clock[2] = (l_time.Hour_u8 % 10) + 0x30U;
    E_val_Clock[3] = (l_time.Minute_u8 / 10) + 0x30U;
    E_val_Clock[4] = (l_time.Minute_u8 % 10) + 0x30U;
    E_val_Clock[5] = (l_time.Second_u8 / 10) + 0x30U;
    E_val_Clock[6] = (l_time.Second_u8 % 10) + 0x30U;

    l_bit = l_time.Day_u8;
    while( l_bit ) {
        l_bit >>= 1;
        l_inx++;
    }
    E_val_Clock[7] = ((l_inx - 1) % 10) + 0x30U;

} // UpdateInterfaceTime

///////////////////////////////////////////////////////////////////////////////
//
// UpdateInterfaceAlarms
//
///////////////////////////////////////////////////////////////////////////////
static void UpdateInterfaceAlarms( void )
{
    Alarm_t l_Alarm;

    if ( Get_Alarm( &l_Alarm, 0 ) ) {
        E_val_Alarm0[1] = (l_Alarm.Hour_u8 / 10) + 0x30U;
        E_val_Alarm0[2] = (l_Alarm.Hour_u8 % 10) + 0x30U;
        E_val_Alarm0[3] = (l_Alarm.Minute_u8 / 10) + 0x30U;
        E_val_Alarm0[4] = (l_Alarm.Minute_u8 % 10) + 0x30U;
        E_val_Alarm0[5] = (l_Alarm.Second_u8 / 10) + 0x30U;
        E_val_Alarm0[6] = (l_Alarm.Second_u8 % 10) + 0x30U;
        E_val_Alarm0[7] = (l_Alarm.Day_u8 / 100) + 0x30U;
        E_val_Alarm0[8] = ((l_Alarm.Day_u8 % 100) / 10) + 0x30U;
        E_val_Alarm0[9] = (l_Alarm.Day_u8 % 10) + 0x30U;
    } else {
        E_val_Alarm0[1] = 0x30U;
        E_val_Alarm0[2] = 0x30U;
        E_val_Alarm0[3] = 0x30U;
        E_val_Alarm0[4] = 0x30U;
        E_val_Alarm0[5] = 0x30U;
        E_val_Alarm0[6] = 0x30U;
        E_val_Alarm0[7] = 0x30U;
        E_val_Alarm0[8] = 0x30U;
        E_val_Alarm0[9] = 0x30U;
    }

    if ( Get_Alarm( &l_Alarm, 1 ) ) {
        E_val_Alarm1[1] = (l_Alarm.Hour_u8 / 10) + 0x30U;
        E_val_Alarm1[2] = (l_Alarm.Hour_u8 % 10) + 0x30U;
        E_val_Alarm1[3] = (l_Alarm.Minute_u8 / 10) + 0x30U;
        E_val_Alarm1[4] = (l_Alarm.Minute_u8 % 10) + 0x30U;
        E_val_Alarm1[5] = (l_Alarm.Second_u8 / 10) + 0x30U;
        E_val_Alarm1[6] = (l_Alarm.Second_u8 % 10) + 0x30U;
        E_val_Alarm1[7] = (l_Alarm.Day_u8 / 100) + 0x30U;
        E_val_Alarm1[8] = ((l_Alarm.Day_u8 % 100) / 10) + 0x30U;
        E_val_Alarm1[9] = (l_Alarm.Day_u8 % 10) + 0x30U;
    } else {
        E_val_Alarm1[1] = 0x30U;
        E_val_Alarm1[2] = 0x30U;
        E_val_Alarm1[3] = 0x30U;
        E_val_Alarm1[4] = 0x30U;
        E_val_Alarm1[5] = 0x30U;
        E_val_Alarm1[6] = 0x30U;
        E_val_Alarm1[7] = 0x30U;
        E_val_Alarm1[8] = 0x30U;
        E_val_Alarm1[9] = 0x30U;
    }

    if ( Get_Alarm( &l_Alarm, 2 ) ) {
        E_val_Alarm2[1] = (l_Alarm.Hour_u8 / 10) + 0x30U;
        E_val_Alarm2[2] = (l_Alarm.Hour_u8 % 10) + 0x30U;
        E_val_Alarm2[3] = (l_Alarm.Minute_u8 / 10) + 0x30U;
        E_val_Alarm2[4] = (l_Alarm.Minute_u8 % 10) + 0x30U;
        E_val_Alarm2[5] = (l_Alarm.Second_u8 / 10) + 0x30U;
        E_val_Alarm2[6] = (l_Alarm.Second_u8 % 10) + 0x30U;
        E_val_Alarm2[7] = (l_Alarm.Day_u8 / 100) + 0x30U;
        E_val_Alarm2[8] = ((l_Alarm.Day_u8 % 100) / 10) + 0x30U;
        E_val_Alarm2[9] = (l_Alarm.Day_u8 % 10) + 0x30U;
    } else {
        E_val_Alarm2[1] = 0x30U;
        E_val_Alarm2[2] = 0x30U;
        E_val_Alarm2[3] = 0x30U;
        E_val_Alarm2[4] = 0x30U;
        E_val_Alarm2[5] = 0x30U;
        E_val_Alarm2[6] = 0x30U;
        E_val_Alarm2[7] = 0x30U;
        E_val_Alarm2[8] = 0x30U;
        E_val_Alarm2[9] = 0x30U;
    }

    if ( Get_Alarm( &l_Alarm, 3 ) ) {
        E_val_Alarm3[1] = (l_Alarm.Hour_u8 / 10) + 0x30U;
        E_val_Alarm3[2] = (l_Alarm.Hour_u8 % 10) + 0x30U;
        E_val_Alarm3[3] = (l_Alarm.Minute_u8 / 10) + 0x30U;
        E_val_Alarm3[4] = (l_Alarm.Minute_u8 % 10) + 0x30U;
        E_val_Alarm3[5] = (l_Alarm.Second_u8 / 10) + 0x30U;
        E_val_Alarm3[6] = (l_Alarm.Second_u8 % 10) + 0x30U;
        E_val_Alarm3[7] = (l_Alarm.Day_u8 / 100) + 0x30U;
        E_val_Alarm3[8] = ((l_Alarm.Day_u8 % 100) / 10) + 0x30U;
        E_val_Alarm3[9] = (l_Alarm.Day_u8 % 10) + 0x30U;
    } else {
        E_val_Alarm3[1] = 0x30U;
        E_val_Alarm3[2] = 0x30U;
        E_val_Alarm3[3] = 0x30U;
        E_val_Alarm3[4] = 0x30U;
        E_val_Alarm3[5] = 0x30U;
        E_val_Alarm3[6] = 0x30U;
        E_val_Alarm3[7] = 0x30U;
        E_val_Alarm3[8] = 0x30U;
        E_val_Alarm3[9] = 0x30U;
    }
} // UpdateInterfaceAlarms

///////////////////////////////////////////////////////////////////////////////
//
// SetupListenSocket
//
///////////////////////////////////////////////////////////////////////////////
static bool SetupListenSocket( void )
{
    int16_t l_socket;
    unsigned long aucDHCP       = 14400;
    unsigned long aucARP        = 3600;
    unsigned long aucKeepalive  = 30;
    unsigned long aucInactivity = 0;

    DEBUG_OUT( DEBUG_OUTPUT_SETUP_LISTEN, "Change timeout" );

    // Change the timeout values to prevent closing the socket due to inactivity
    if ( netapp_timeout_values( &aucDHCP, &aucARP, &aucKeepalive, &aucInactivity ) != 0 ) {
      return false;
    }

    DEBUG_OUT( DEBUG_OUTPUT_SETUP_LISTEN, "Create server socket" );

    l_socket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );

    DEBUG_OUT( DEBUG_OUTPUT_SETUP_LISTEN, "Set socket options" );

    // Create a non-blocking socket
    // setsockopt returns -1 on error, otherwise 0
    char arg = SOCK_ON; // option for non-blocking
    if ( setsockopt( l_socket, SOL_SOCKET, SOCKOPT_ACCEPT_NONBLOCK, &arg, sizeof(arg) ) < 0 ) {
        DEBUG_OUT( DEBUG_OUTPUT_SETUP_LISTEN, "SOCKOPT_ACCEPT_NONBLOCK failed" );
      return false;
    }

    DEBUG_OUT( DEBUG_OUTPUT_SETUP_LISTEN, "Bind server socket" );

    // Bind the socket to a TCP address.
    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(0);     // Listen on any network interface, equivalent to INADDR_ANY in sockets programming.
    address.sin_port = htons( SERVER_LISTEN_PORT );        // Listen on the specified port.
    if ( bind( l_socket, (sockaddr*) &address, sizeof(address) ) < 0 ) {
      return false;
    }

    DEBUG_OUT( DEBUG_OUTPUT_SETUP_LISTEN, "Listen server socket" );

    // Start listening for connections.
    // The backlog parameter is 0 as it is not supported on TI's CC3000 firmware.
    if ( listen( l_socket, 0 ) < 0 ) {
      return false;
    }

    Server_ListenSocket = l_socket;
    return true;
} // SetupListenSocket

///////////////////////////////////////////////////////////////////////////////
//
// ProcessIncommingConnections
//
///////////////////////////////////////////////////////////////////////////////
static void ProcessIncomingConnections( void )
{
    int16_t l_socket;

    // accept() returns -1 on erro -2 for in progress, or socket handle
    //DEBUG_OUT( DEBUG_OUTPUT_PIC, "PIC:00.0" );
    l_socket = accept( Server_ListenSocket, NULL, NULL );
    //DEBUG_OUT( DEBUG_OUTPUT_PIC, "PIC:00.1" );
    if ( l_socket > -1 ) {
        char arg = SOCK_ON;
        DEBUG_OUT( DEBUG_OUTPUT_PIC, "PIC:00.2" );
        // try to set non-blocking
        if ( setsockopt( l_socket, SOL_SOCKET, SOCKOPT_RECV_NONBLOCK, &arg, sizeof(arg) ) < 0 ) {
            DEBUG_OUT( DEBUG_OUTPUT_PIC, "SOCKOPT_RECV_NONBLOCK failed" );
        }
        // There is a connection
        DEBUG_OUT( DEBUG_OUTPUT_PIC, "PIC:01" );
        ProcessReceiveStream( l_socket );
        DEBUG_OUT( DEBUG_OUTPUT_PIC, "PIC:02" );
        // TODO close the socket, may not have read everything yet
        // TODO figure how to wait for send to complete before closing socket
        // There is an event to signal when a socket can be closed - HCI_EVENT_CC3000_CAN_SHUT_DOWN
        // This is event is caught in CC3K handler CC3000_UsynchCallback, but
        // nothing useful done with it.
        // Should be just spin here and wait for the event, or register the
        // need to close the socket and handle it when the event occurs???
        // There seems to be a race condition here. Not clear what triggers
        // the event. Possible that it occurs before we try to call close.
        //
        // Another option found on message board is to check value of
        // tSLInformation.NumberOfSentPackets == tSLInformation.NumberOfReleasedPackets.
        // This indicates all "sent" packets have been transmit over WiFi.
        // No race condition, just a loop to wait. tSLInformation struct is
        // exported by cc3000_common.h
        DEBUG_OUT( DEBUG_OUTPUT_MAIN, "Accepted, and closing..." );
        delay( 100 );
        if ( 0 > closesocket( l_socket ) ) {
            // closesocket returned an error.
            // API docs do not specify what can cause this.
            // TODO may want to reset CC3000, or something???
            DEBUG_OUT( DEBUG_OUTPUT_PIC, "PIC:03 - close error" );
        }
        DEBUG_OUT( DEBUG_OUTPUT_MAIN, "Closed." );
    }
} // ProcessIncomingConnections

///////////////////////////////////////////////////////////////////////////////
//
// ProcessReceiveStream
//
///////////////////////////////////////////////////////////////////////////////
static void ProcessReceiveStream( int16_t sd )
{
    // Read buffer length is sum of HTTP_REQ_LN1_LEN and JSON_DATA_LEN_MAX
    // Once first line of request is received, subsequent reads from socket
    // will be placed after first line data, this is to save first line without
    // need for a second buffer.
    //
    // JSON data string in message body will be left in remainder of buffer
    // intact. If split across reads, data is moved to immediately after LN1,
    // next recv call will follow if needed.
    //

    // HOW SOCKET RECV WORKS
    // Following is based on experiments with some test code.
    // Recv socket was set to non-blocking.
    // Various recv sizes tried.
    //      recv len 80 always worked
    //      recv len 100 never worked
    //      recv len 90 worked for some tests
    //
    // First call to recv after socekt was returned from accept()
    // typically returns 0, unless a delay is added between accept() and recv().
    // With no delay, second call returns full buffer, with some delays before
    // call, and if there is enough recv bytes to fill the buffer.
    // recv() will return less than the full buffer. This seems to be a reliable
    // indication of end of recv data.
    // If recv() called from a loop with almost no delay (do nothing, just call
    // recv() again) it will return zero more than on just first call. But it
    // will eventually return non-zero. Need to check for first non-zero read
    // before waiting for a less than request size read to indicated end.
    //

    static char l_read_buf[ SOCKET_READ_SIZE ]; // only static to avoid stack overflow, better memory use info from build
    int16_t l_recv_cnt = 0; // number of bytes returned by last socket read (-1 on error)
    uint16_t l_http_req_flags = 0;
    uint16_t l_read_start_inx = 0; // index into read buffer for start of next read
    uint16_t l_ln1_end = 0; // index into read buffer pointing to character after end of first line, also size of LN1
    uint16_t l_inx = 0; // index for looping through last chunk of data from socket read, and end of content
    uint8_t l_chunk_cnt = 0; // number of times recv returned non-zero

    // changed from 100 to 500 due to inadequate wait somethimes
    // Perl script has gotten zero reads ~150 times, curl is much faster
    uint16_t l_recv_zero_cnt = 500;

    uint16_t l_read_size = SOCKET_READ_CHUNK;
    uint16_t l_offset = 0;



    while ( 1 ) {
        if ( l_read_start_inx >= SOCKET_READ_SIZE ) {
            // Receive buffer full, call it a bad request
            l_http_req_flags |= HTTP_REQ_BAD_FLAG;
            break;
        }
        if ( (SOCKET_READ_SIZE - l_read_start_inx ) > SOCKET_READ_CHUNK ) {
            l_read_size = SOCKET_READ_CHUNK;
        } else {
            l_read_size = SOCKET_READ_SIZE - l_read_start_inx;
        }
        l_recv_cnt = recv( sd, &l_read_buf[l_read_start_inx], l_read_size, 0 );
#if defined( DEBUG_OUTPUT_MISC )
        for ( l_inx = l_read_start_inx;
              l_inx < ((uint16_t)l_recv_cnt + l_read_start_inx);
              l_inx++ ) {
            if ( '\0' == l_read_buf[l_inx] ) {
                Serial.write( '_' );
            } else {
                Serial.write( l_read_buf[l_inx] );
            }
        }
        Serial.println( l_recv_zero_cnt, DEC );
#endif
        if ( l_recv_cnt > 0 ){
            DEBUG_OUT( DEBUG_OUTPUT_PIC, "PIC:10" );
            l_recv_zero_cnt = 20; // reset this every time non-zero recv

            // Loop through characters in local buffer
            // This only needs to be done if the end of the header
            // has not been found yet. When reading body, it just gets
            // saved in buffer.
            if ( !( HTTP_REQ_HDR_FLAG & l_http_req_flags ) ) {
                for ( l_inx = l_read_start_inx;
                      l_inx < ((uint16_t)l_recv_cnt + l_read_start_inx);
                      l_inx++ ) {
                    // Header processing
                    if ( !( HTTP_REQ_HDR_FLAG & l_http_req_flags ) ) {
                        // Newline
                        if ( '\n' == l_read_buf[ l_inx ] ) {
                            if ( !( HTTP_REQ_LN1_FLAG & l_http_req_flags ) ) {
                                // End of first line of request
                                l_http_req_flags |= HTTP_REQ_LN1_FLAG;
                                // This will save the first line in l_read_buffer
                                l_ln1_end = l_inx; // newline will be overwritten, don't need it
                            } // !HTTP_REQ_LN1_FLAG
                        // Carriage Return
                        } else if ( '\r' == l_read_buf[ l_inx ] ) {
                            if ( HTTP_REQ_CR_FLAG & l_http_req_flags ) {
                                l_http_req_flags |= HTTP_REQ_HDR_FLAG;
                                l_offset = l_ln1_end;
                            } else {
                                l_http_req_flags |= HTTP_REQ_CR_FLAG;
                            }
                        // Any other character
                        } else {
                            l_http_req_flags &= ~HTTP_REQ_CR_FLAG;
                        }
                    // !( HTTP_REQ_HDR_FLAG & l_http_req_flags )
                    } else {
                        // Past end of LN1, need to move data to after
                        // end of LN1. This only happens on first chunk that
                        // has data past end of header. Only in here if
                        // end of header was not found before start of this
                        // loop.
                        // TODO This will leave \n following \r at end of header
                        l_read_buf[ l_offset++ ] = l_read_buf[ l_inx ];
                    }
                } // for ( l_inx = 0; l_inx < l_recv_cnt; l_inx++ )
                // update for next read
                // l_ln1_end will be zero before it is found, won't overwrite
                // any of LN1
                if ( HTTP_REQ_HDR_FLAG & l_http_req_flags ) {
                    l_read_start_inx = l_offset;
                } else {
                    l_read_start_inx = l_ln1_end;
                }
            // !( HTTP_REQ_HDR_FLAG & l_http_req_flags )
            } else {
                l_read_start_inx += l_recv_cnt;
            }
        } else {
            DEBUG_OUT( DEBUG_OUTPUT_PIC, "PIC:11" );
            //Serial.println( l_recv_zero_cnt, DEC );
            l_recv_zero_cnt--;
            if ( 0 == l_recv_zero_cnt ) {
                DEBUG_OUT( DEBUG_OUTPUT_PIC, "PIC:11.1" );
                // If 20 consecutive recv calls return no data, consider
                // it the end of data
                l_http_req_flags |= HTTP_REQ_END_FLAG;
                break;
            }
            // Delay here to let data arrive
            delay( 1 );
        }
        DEBUG_OUT( DEBUG_OUTPUT_PIC, "PIC:12" );

        // This is not really needed, but will leave it to protect against
        // future stupidity
        if (  ( HTTP_REQ_BAD_FLAG & l_http_req_flags )
           || ( HTTP_REQ_ERR_FLAG & l_http_req_flags )
           || ( HTTP_REQ_END_FLAG & l_http_req_flags ) ) {
            // on any errors, exit while loop and send response
            break; // exit while loop
        }
        l_chunk_cnt++;
    } // while ( 1 )

    DEBUG_OUT( DEBUG_OUTPUT_PIC, "\nEnd of receive" );

    if (  ( HTTP_REQ_BAD_FLAG & l_http_req_flags )
       || ( HTTP_REQ_ERR_FLAG & l_http_req_flags ) ) {
        // Do nothing
    } else if ( HTTP_REQ_END_FLAG & l_http_req_flags ) {
        HttpInterfaceElement_t * eReq_ptr;
        // This will determine if it is a POST or GET, do whatever needed
        // for post. For GET just parses URI and returns pointer to element
        // in interface tree structure.
        //
        // POST or GET results in same response, unless an error.
        // Assume all errors are bad request at this point.

        eReq_ptr = ProcessHttpRequest( l_read_buf,
                                       l_ln1_end,
                                       &l_read_buf[l_ln1_end],
                                       (l_read_start_inx - l_ln1_end) );
        if( NULL_E_PTR != eReq_ptr ) {
            DEBUG_OUT( DEBUG_OUTPUT_REQ_PROC, "Proc Req Success" );
            // Generate the response data.
            // Put the response body string in l_read_buf, l_inx is length of body
            // Returns length of body, put in l_recv_cnt.
            l_recv_cnt = GenerateHttpResponseBody( l_read_buf, SOCKET_READ_SIZE, eReq_ptr );
        } else {
            DEBUG_OUT( DEBUG_OUTPUT_REQ_PROC, "Proc Req Error" );
            // error processing request
            l_http_req_flags |= HTTP_REQ_BAD_FLAG;
        }
    } else {
        // how did this happen???
        // make it a server error
        l_http_req_flags |= HTTP_REQ_ERR_FLAG;
    }


    // HTTP req flags indicate any errors
    // l_read_buf has JSON string for response
    // l_inx has length of JSON string
    SendHttpResponse( sd, l_http_req_flags, l_read_buf, l_recv_cnt );
} // ProcessReceiveStream

///////////////////////////////////////////////////////////////////////////////
//
// ProcessHttpRequest
//
///////////////////////////////////////////////////////////////////////////////
static HttpInterfaceElement_t * ProcessHttpRequest( char * ln1_ptr, uint16_t ln1_len, char * body_ptr, uint16_t body_len )
{
    // Return a pointer to the interface element node for the URI
    // This can be used to generate response, return NULL_E_PTR if not found.
    // If this is a POST parse the body and call handler for requested element.
    // Handler is only called for POST.
    // For a GET handler just updates structure value field.
    // Response body string is generated elsewhere.


    uint16_t l_inx;
    uint16_t l_name_inx;
    uint8_t  l_flags = 0;
    HttpInterfaceElement_t * elementPtr = NULL_E_PTR;

    for ( l_inx = 0; l_inx < ln1_len; l_inx++ ) {
        // NOTE this may read past end of ln1_ptr[], but it won't cause
        // any problems, just reading, check below to confirm data wasn't
        // found past end
        if ( ( 'G' == ln1_ptr[ l_inx ] )
                && ( 'E' == ln1_ptr[ l_inx + 1 ] )
                && ( 'T' == ln1_ptr[ l_inx + 2 ] ) ) {
            // GET request
            DEBUG_OUT( DEBUG_OUTPUT_REQ_PROC, "GET");
            l_flags |= HTTP_REQ_GET_FLAG;
            //l_inx = 3;
            break;
        } else if ( ( 'P' == ln1_ptr[ l_inx ] )
                && ( 'O' == ln1_ptr[ l_inx + 1 ] )
                && ( 'S' == ln1_ptr[ l_inx + 2 ] )
                && ( 'T' == ln1_ptr[ l_inx + 3 ] ) ) {
            // POST request
            DEBUG_OUT( DEBUG_OUTPUT_REQ_PROC, "POST");
            l_flags |= HTTP_REQ_PST_FLAG;
            //l_inx = 4;
            break;
        } else {
            // Do nothing
        }
    }
    if ( l_inx >= ln1_len ) {
        // The GET/POST extends past end of LN1, this is an error
        DEBUG_OUT( DEBUG_OUTPUT_REQ_PROC, "Couldn't find GET/POST 1");
        return NULL_E_PTR;
    } else if ( !l_flags ) {
        // GET/POST not found, this is an error
        DEBUG_OUT( DEBUG_OUTPUT_REQ_PROC, "Couldn't find GET/POST 2" );
        return NULL_E_PTR;
    } else {
        // Do nothing
    }

    // Find start of URI from end of GET/POST
    while ( '/' != ln1_ptr[ l_inx ] ) {
        l_inx++;
        if ( l_inx >= ln1_len ) {
            // Start of URI not found, this is an error
            DEBUG_OUT( DEBUG_OUTPUT_REQ_PROC, "Couldn't find URI" );
            return NULL_E_PTR;
        }
    }
    // l_inx points to / at start of URI

    // NOTE: this all assumes URI is followed by a space, and not some other
    // whitespace.
    // TODO is this a valid assumption?? space after URI
    // or what are valid chars for URI, is it an easy range to check??

    // Start with ROOT
    elementPtr = &e_ROOT;
    l_name_inx = 0;
    uint16_t l_token_start = l_inx; // keep track of start of URI
                                    // token to back track if this
                                    // element didn't match token
    while ( l_inx < ln1_len ) {
        //Serial.write( elementPtr->name );
        //Serial.println( F("") );
        if ( elementPtr->name[ l_name_inx ] == ln1_ptr[ l_inx ] ) {
            l_inx++;
            l_name_inx++;
            DEBUG_OUT( DEBUG_OUTPUT_REQ_PROC, "PHR:00" );
        } else {
            if ( ( ( ' ' == ln1_ptr[ l_inx ] ) || ( '/' == ln1_ptr[ l_inx] ) )
                 && ( '\0' == elementPtr->name[ l_name_inx ] ) ) {
                // End of URI token, and end of element name, matched so far
                DEBUG_OUT( DEBUG_OUTPUT_REQ_PROC, "PHR:02" );
                if (    ( ' ' == ln1_ptr[ l_inx ] )
                     || (     ( '/' == ln1_ptr[ l_inx ] )
                           && ( ' ' == ln1_ptr[ l_inx + 1 ] )
                        )
                    ) {
                    // Found the node
                    DEBUG_OUT( DEBUG_OUTPUT_REQ_PROC, "PHR:05" );
                    break;
                } else if ( NULL_E_PTR == elementPtr->child ) {
                    // Not at end of URI, and no child, bad URI
                    DEBUG_OUT( DEBUG_OUTPUT_REQ_PROC, "PHR:06" );
                    return NULL_E_PTR;
                // NULL_E_PTR == elementPtr->child
                } else {
                    elementPtr = elementPtr->child;
                    l_token_start = ++l_inx;
                    l_name_inx = 0;
                    DEBUG_OUT( DEBUG_OUTPUT_REQ_PROC, "PHR:07" );
                }
            } else {
                // Check for more possible elements, siblings only
                if ( NULL_E_PTR == elementPtr->next_sibling ) {
                    // Not a match, and no sibling
                    // Bad URI
                    DEBUG_OUT( DEBUG_OUTPUT_REQ_PROC, "PHR:08" );
                    return NULL_E_PTR;
                } else {
                    elementPtr = elementPtr->next_sibling;
                    l_name_inx = 0;
                    l_inx = l_token_start;
                    DEBUG_OUT( DEBUG_OUTPUT_REQ_PROC, "PHR:09" );
                } // NULL_E_PTR == elementPtr->next_sibling
            }
        } // elementPtr->name[ l_name_inx ] == ln1_ptr[ l_inx ]
    } // while ( l_inx < ln1_len )

    if ( l_inx >= ln1_len ) {
        // Didn't get a match, this is an error
        DEBUG_OUT( DEBUG_OUTPUT_REQ_PROC, "PHR:01" );
        return NULL_E_PTR;
    }

    // elementPtr points to the node.
    if ( HTTP_REQ_PST_FLAG & l_flags ) {
        if ( 0 == ProcessPostRequest( elementPtr, body_ptr, body_len ) ) {
            DEBUG_OUT( DEBUG_OUTPUT_REQ_PROC, "PHR:12" );
            return NULL_E_PTR;
        }
    } else {
        // Don't do anything for GET, data in struct val element is assumed
        // to be most recent value available.
        // Response is generated in GenerateHttpResponseBody function called
        // after this returns.
    }
    return elementPtr;
} // ProcessHttpRequest

///////////////////////////////////////////////////////////////////////////////
//
// ProcessPostRequest
//
///////////////////////////////////////////////////////////////////////////////
static uint8_t ProcessPostRequest( HttpInterfaceElement_t * ePtr, char * buf, uint16_t len )
{
    /* Return:
     *      0 - error: bad data string format
     *      1 - success
     */

    /* TODO this worked for simple tree with only root and one other node
     * Will not work for multiple nodes, unless each node hanlder knows about
     * all of its children, but then it would also need to know if it should
     * call it's siblings also???
     *
     * For nodes with children, only need to call children - no data to
     * process. If a node is requested, it doesn't need to call siblings. If
     * the requested node has a child needs to call all children. Each
     * node handler needs to be aware of itself. Can walk tree to all children.
     * Or can handle all of this from here, just pass data string buffer
     * to each handler. All JSON parsing consolidated here.
     *
     * Build a tree of JSON data here. Have pointers into buffer to save
     * memory. Once JSON tree built, walk the element tree from requested
     * node, calling each handler with information from JSON tree.
     */
    static JSONElement_t JSONElements[MAX_TREE_ELEMENTS];
    JSONElement_t *e = BuildJSONTree( &JSONElements[0], buf, len );
    uint8_t l_status = 0;

    DEBUG_OUT( DEBUG_OUTPUT_REQ_PROC, "PHR:13" );
    if ( (JSONElement_t *)NULL_PTR == e ) {
        DEBUG_OUT( DEBUG_OUTPUT_REQ_PROC, "PHR:14" );
        return 0;
    } else {
        DEBUG_OUT( DEBUG_OUTPUT_REQ_PROC, "PHR:15" );
        // TODO execute handlers with JSON data
        JSONElement_t * eN = e;
        HttpInterfaceElement_t * eL = ePtr;
        uint8_t l_level = 0;
        uint8_t l_inx;

        // NOTE: The URI root node does not have a node in data tree.
        if ( '\0' == ePtr->name[ 0 ] ) {
            DEBUG_OUT( DEBUG_OUTPUT_REQ_PROC, "PHR:15.0" );
            // Root node was requested, need to start with it's child
            if ( NULL_E_PTR == ePtr->child ) {
                DEBUG_OUT( DEBUG_OUTPUT_REQ_PROC, "PHR:15.1" );
                // TODO should never happen, indicates a bad interface tree
                return 0;
            } else {
                DEBUG_OUT( DEBUG_OUTPUT_REQ_PROC, "PHR:15.2" );
                eL = ePtr->child;
            }
        }


        while ( 1 ) {
            DEBUG_OUT( DEBUG_OUTPUT_REQ_PROC, "PHR:16" );
            // Find find eN for current eL
            eN = e; // always start at root of data tree
            /* Use value of l_level to get to correct level of data tree
             * to match level of interface tree.
             */
            l_inx = l_level;
            while ( l_inx > 0 ) {
                DEBUG_OUT( DEBUG_OUTPUT_REQ_PROC, "PHR:16.0" );
                if ( (JSONElement_t *)NULL_PTR == eN->child ) {
                    DEBUG_OUT( DEBUG_OUTPUT_REQ_PROC, "PHR:16.0.1" );
                    if ( (JSONElement_t *)NULL_PTR == eN->sibling ) {
                        DEBUG_OUT( DEBUG_OUTPUT_REQ_PROC, "PHR:16.0.2" );
                        // TODO Not enough levels in data tree???
                        return 0;
                    } else {
                        DEBUG_OUT( DEBUG_OUTPUT_REQ_PROC, "PHR:16.0.3" );
                        eN = eN->sibling;
                    }
                } else {
                    DEBUG_OUT( DEBUG_OUTPUT_REQ_PROC, "PHR:16.0.4" );
                    l_inx--;
                    eN = eN->child;
                }
            }



            // Check for name match
            l_inx = 0;
            do {
                DEBUG_OUT( DEBUG_OUTPUT_REQ_PROC, "PHR:16.1" );
                DEBUG_OUT_CHR( DEBUG_OUTPUT_REQ_PROC, eN->name[ l_inx ] );
                DEBUG_OUT( DEBUG_OUTPUT_REQ_PROC, "--" );
                DEBUG_OUT_CHR( DEBUG_OUTPUT_REQ_PROC, eL->name[ l_inx ] );
                DEBUG_OUT( DEBUG_OUTPUT_REQ_PROC, "--" );
                // Does eN name start with quote? - NO
                // NOTE this will break if char after end of eN->name happens
                // to be NULL. Should never happen because name is always in
                // JSON data string, and all names end with quote.
                if ( eN->name[ l_inx ] == eL->name[ l_inx ] ) {
                    DEBUG_OUT( DEBUG_OUTPUT_REQ_PROC, "PHR:16.1.0" );
                    l_inx++;
                } else {
                    DEBUG_OUT( DEBUG_OUTPUT_REQ_PROC, "PHR:16.1.1" );
                    if (  ( l_inx == eN->len_name )
                       && ( '\0' == eL->name[ l_inx ] ) ) {
                        DEBUG_OUT( DEBUG_OUTPUT_REQ_PROC, "PHR:16.1.2" );
                        // Matched names
                        // l_inx not < len_name, loop will exit, but break
                        // to be safe
                        break;
                    } else {
                        DEBUG_OUT( DEBUG_OUTPUT_REQ_PROC, "PHR:16.1.3" );
                        if ( l_inx == eN->len_name ) {
                            DEBUG_OUT( DEBUG_OUTPUT_REQ_PROC, "PHR:16.1.3.1" );
                        } else {
                            DEBUG_OUT( DEBUG_OUTPUT_REQ_PROC, "PHR:16.1.3.2" );
                        }
                        if ( '\0' == eL->name[ l_inx ] ) {
                            DEBUG_OUT( DEBUG_OUTPUT_REQ_PROC, "PHR:16.1.3.3" );
                        } else {
                            DEBUG_OUT( DEBUG_OUTPUT_REQ_PROC, "PHR:16.1.3.4" );
                        }
                        // Go to next sibling
                        if ( (JSONElement_t *)NULL_PTR == eN->sibling ) {
                            DEBUG_OUT( DEBUG_OUTPUT_REQ_PROC, "PHR:16.1.4" );
                            // TODO no match found
                            return 0;
                        } else {
                            DEBUG_OUT( DEBUG_OUTPUT_REQ_PROC, "PHR:16.1.5" );
                            eN = eN->sibling;
                            l_inx = 0;
                        }
                    }
                }
            // Should never leave loop due to this condition.
            // Need to add one and <= to allow loop to check NULL at end
            // of eL->name. Check to protect against infinite loop only.
            } while ( l_inx <= ( eN->len_name + 1 ) );
            DEBUG_OUT( DEBUG_OUTPUT_REQ_PROC, "PHR:16.2" );

            l_status = eL->func( eN->data, eN->len_data );

            DEBUG_OUT( DEBUG_OUTPUT_REQ_PROC, "PHR:16.3" );
            if ( 0 == l_status ) {
                DEBUG_OUT( DEBUG_OUTPUT_REQ_PROC, "PHR:16.3.1" );
                return 0;
            } else {
                DEBUG_OUT( DEBUG_OUTPUT_REQ_PROC, "PHR:16.3.2" );
                // Get next eL
                // Don't check for siblings of eL==ePtr
                if ( eL == ePtr ) {
                    DEBUG_OUT( DEBUG_OUTPUT_REQ_PROC, "PHR:16.3.3" );
                    if ( NULL_E_PTR == eL->child ) {
                        DEBUG_OUT( DEBUG_OUTPUT_REQ_PROC, "PHR:16.3.4" );
                        // Done
                        break;
                    } else {
                        DEBUG_OUT( DEBUG_OUTPUT_REQ_PROC, "PHR:16.3.5" );
                        l_level++;
                        eL = eL->child;
                    }
                } else {
                    DEBUG_OUT( DEBUG_OUTPUT_REQ_PROC, "PHR:16.3.6" );
                    if ( NULL_E_PTR == eL->child ) {
                        DEBUG_OUT( DEBUG_OUTPUT_REQ_PROC, "PHR:16.3.7" );
                        if ( NULL_E_PTR == eL->next_sibling ) {
                            DEBUG_OUT( DEBUG_OUTPUT_REQ_PROC, "PHR:16.3.8" );
                            // No more nodes on this branch
                            // Go back up and look for more siblings
                            // Don't want to execute handler again until
                            // a sibling is found.
                            do {
                                DEBUG_OUT( DEBUG_OUTPUT_REQ_PROC, "PHR:16.3.9" );
                                if ( NULL_E_PTR != eL->parent ) {
                                    DEBUG_OUT( DEBUG_OUTPUT_REQ_PROC, "PHR:16.3.10" );
                                    eL = eL->parent;
                                    if ( ePtr == eL ) {
                                        DEBUG_OUT( DEBUG_OUTPUT_REQ_PROC, "PHR:16.3.11" );
                                        // Done
                                        return 1;
                                    } else if ( NULL_E_PTR != eL->next_sibling ) {
                                        DEBUG_OUT( DEBUG_OUTPUT_REQ_PROC, "PHR:16.3.12" );
                                        // Go to sibling and run handler
                                        eL = eL->next_sibling;
                                        break; // exit inner do loop, run handler
                                    } else {
                                        DEBUG_OUT( DEBUG_OUTPUT_REQ_PROC, "PHR:16.3.13" );
                                        // Do nothing, next iteration of do loop
                                    }
                                } else {
                                    DEBUG_OUT( DEBUG_OUTPUT_REQ_PROC, "PHR:16.3.14" );
                                    // Done
                                    return 1;
                                }
                            } while ( NULL_E_PTR != eL->parent );
                        } else {
                            DEBUG_OUT( DEBUG_OUTPUT_REQ_PROC, "PHR:16.3.15" );
                            eL = eL->next_sibling;
                        }
                    } else {
                        DEBUG_OUT( DEBUG_OUTPUT_REQ_PROC, "PHR:16.3.16" );
                        l_level++;
                        eL = eL->child;
                    }
                }
            }
        }
        DEBUG_OUT( DEBUG_OUTPUT_REQ_PROC, "PHR:16.3.15" );
    }
    DEBUG_OUT( DEBUG_OUTPUT_REQ_PROC, "PHR:17" );

    // TODO will this ever happen if there isn't an error?
    return 1; // TODO temp code for debug
} // ProcessPostRequest

///////////////////////////////////////////////////////////////////////////////
//
// BuildJSONTree
//
///////////////////////////////////////////////////////////////////////////////
static JSONElement_t * BuildJSONTree( JSONElement_t * eBuf, char * buf, uint16_t len )
{
    /* Return:
     *  NULL_PTR - error: bad data format
     *  success: pointer to root node of JSON tree
     */
    JSONElement_t * eCur = eBuf;
    uint8_t l_node = 0;
    uint8_t l_level = 0; // count levels to make sure all braces are closed
    uint16_t l_inx = 0;
    uint16_t l_start = 0;
    uint8_t l_state = 0;

    eCur->parent = (JSONElement_t *)NULL_PTR;
    eCur->sibling = (JSONElement_t *)NULL_PTR;
    eCur->child = (JSONElement_t *)NULL_PTR;
    eCur->name = (char *)NULL_PTR;
    eCur->data = (char *)NULL_PTR;
    eCur->len_data = 0;
    eCur->len_name = 0;

    while ( l_inx < len ) {
        DEBUG_OUT_CHR( DEBUG_OUTPUT_JSN, buf[l_inx] );
        DEBUG_OUT_CHR( DEBUG_OUTPUT_JSN, (l_level + 0x30) );
        if ( 0 == l_state ) {
            DEBUG_OUT( DEBUG_OUTPUT_JSN, "JSON:00" );
            // Look for the first open brace
            // buf starts after end of request header, probably contains at least
            // one \n or \r
            if ( '{' == buf[ l_inx ] ) {
                l_state = 1;
            } else {
                l_state = 0;
            }
            l_inx++;
        } else if ( 1 == l_state ) {
            DEBUG_OUT( DEBUG_OUTPUT_JSN, "JSON:01" );
            // Next char should be quote at start of name, assuming no white space
            if ( '"' == buf[ l_inx ] ) {
                l_inx++;
                l_start = l_inx;
                eCur->name = &buf[ l_inx ];
                l_state = 2;
            } else {
                // Bad data
                return (JSONElement_t *)NULL_PTR;
            }
        } else if ( 2 == l_state ) {
            DEBUG_OUT( DEBUG_OUTPUT_JSN, "JSON:02" );
            // Find end of name quote
            if ( '"' == buf[ l_inx ] ) {
                // End of name
                eCur->len_name = l_inx - l_start;
                l_inx++;
                l_state = 3;
            } else {
                // Keep looking
                // TODO should check for unexpected chars - ,:{}
                l_inx++;
                l_state = 2;
            }
        } else if ( 3 == l_state ) {
            DEBUG_OUT( DEBUG_OUTPUT_JSN, "JSON:03" );
            // This should be colon after name, assuming no white space
            if ( ':' == buf[ l_inx ] ) {
                l_inx++;
                l_state = 4;
            } else {
                // Bad data
                return (JSONElement_t *)NULL_PTR;
            }
        } else if ( 4 == l_state ) {
            DEBUG_OUT( DEBUG_OUTPUT_JSN, "JSON:04" );
            // Start of data, may be open brace for children
            if ( '{' == buf[ l_inx ] ) {
                // Start of child data, need to add a node to JSON data tree
                l_state = 6;
            } else {
                // Assume it is start of data
                // TODO check for possible non-data chars
                eCur->data = &buf[ l_inx ];
                l_start = l_inx;
                l_inx++;
                l_state = 5;
                // Don't move to next char, start with first data char
                // looking for end
            }
        } else if ( 5 == l_state ) {
            DEBUG_OUT( DEBUG_OUTPUT_JSN, "JSON:05" );
            // Find end of data
            // What marks end of data?
            // End marked by comma or close brace
            // TODO this will not work for quoted data that contains commas
            // or close brace characters
            if ( ',' == buf[ l_inx ] ) {
                // End of data, sibling element follows
                eCur->len_data = l_inx - l_start;
                l_state = 7;
            } else if ( '}' == buf[ l_inx ] ) {
                // End of data, last sibling, return to parent
                eCur->len_data = l_inx - l_start;
                l_state = 8;
            } else {
                // Keep looking
                l_state = 5;
                l_inx++;
            }
        } else if ( 6 == l_state ) {
            DEBUG_OUT( DEBUG_OUTPUT_JSN, "JSON:06" );
            // Add a child node to JSON tree
            l_node++;
            if ( l_node < MAX_TREE_ELEMENTS ) {
                l_level++;
                eCur->child = &eBuf[ l_node ];
                eCur->child->parent = eCur;
                eCur = eCur->child;
                eCur->sibling = (JSONElement_t *)NULL_PTR;
                eCur->name = (char *)NULL_PTR;
                eCur->data = (char *)NULL_PTR;
                eCur->len_data = 0;
                eCur->len_name = 0;
                l_inx++;
                // Go to node name extract
                l_state = 1;
            } else {
                // Too many nodes
                return (JSONElement_t *)NULL_PTR;
            }
        } else if ( 7 == l_state ) {
            DEBUG_OUT( DEBUG_OUTPUT_JSN, "JSON:07" );
            // Add a sibling node to JSON tree
            l_node++;
            if ( l_node < MAX_TREE_ELEMENTS ) {
                /* NOTE: new node sibling is current node and parent of
                 * current node needs to have new node as sibling. This is
                 * required so that if the top level of the tree has siblings
                 * the returned node points to other siblings. If new node is
                 * made sibling of current node, the returned node will not
                 * have any siblings.
                 */
                (&eBuf[ l_node])->sibling = eCur;
                (&eBuf[ l_node])->parent = eCur->parent;
                eCur = &eBuf[ l_node ];
                if ( (JSONElement_t *)NULL_PTR != eCur->parent ) {
                DEBUG_OUT( DEBUG_OUTPUT_JSN, "JSON:07.1" );
                    eCur->parent->child = eCur;
                }
                eCur->child = (JSONElement_t *)NULL_PTR;
                eCur->name = (char *)NULL_PTR;
                eCur->data = (char *)NULL_PTR;
                eCur->len_data = 0;
                eCur->len_name = 0;
                l_inx++;
                // Go to start of node name extract
                l_state = 1;
            } else {
                // Too many nodes
                return (JSONElement_t *)NULL_PTR;
            }
        } else if ( 8 == l_state ) {
            DEBUG_OUT( DEBUG_OUTPUT_JSN, "JSON:08" );
            // End of children, go to parent
            if ( eCur->parent == (JSONElement_t *)NULL_PTR ) {
                DEBUG_OUT( DEBUG_OUTPUT_JSN, "JSON:08.0" );
                // At the top node of data
                // TODO check level for matching braces, anything else?
                // TODO where to go from here?
                if ( 0 == l_level ) {
                    DEBUG_OUT( DEBUG_OUTPUT_JSN, "JSON:08.1" );
                    return eCur;
                } else {
                    DEBUG_OUT( DEBUG_OUTPUT_JSN, "JSON:08.2" );
                    // Braces don't match
                    return (JSONElement_t *)NULL_PTR;
                }
            } else {
                DEBUG_OUT( DEBUG_OUTPUT_JSN, "JSON:08.3" );
                l_level--;
                eCur = eCur->parent;
                // Where to go from here? Is it necessary to check for
                // closing brace? - probably.
                // Is it possible to have more siblings??
                l_inx++;
                l_state = 9;
            }
        } else if ( 9 == l_state ) {
            DEBUG_OUT( DEBUG_OUTPUT_JSN, "JSON:09" );
            // Check for close brace or comma after move from child to parent
            if ( '}' == buf[ l_inx ] ) {
                // Need to go to parent
                l_state = 8;
            } else if ( ',' == buf[ l_inx ] ) {
                // Need to add a sibling
                l_state = 7;
            } else {
                // TODO what does this mean???
                return (JSONElement_t *)NULL_PTR;
            }
        } else {
            DEBUG_OUT( DEBUG_OUTPUT_JSN, "JSON:90" );
            // This should never happen
            return (JSONElement_t *)NULL_PTR;
        }
    } // l_inx < len

    // This should never happen either
    DEBUG_OUT( DEBUG_OUTPUT_JSN, "JSON:91" );
    return (JSONElement_t *)NULL_PTR;
} // BuildJSONTree

///////////////////////////////////////////////////////////////////////////////
//
// GenerateHttpResponseBody
//
///////////////////////////////////////////////////////////////////////////////
static uint16_t GenerateHttpResponseBody( char * buf, uint16_t len, HttpInterfaceElement_t * ePtr )
{
    // Returns length of generated response string, 0 on error

    // Only leaf nodes can have data. Non-leaf nodes will wrap data for
    // all children in braces.
    // Leaf node data strings must have quotes if needed. All data strings
    // are fixed length, as defined in element structure.
    // Node names will have quotes added, with colon following name.
    // Top level braces will be added here. Comas between sibling nodes
    // structures.
    //
    // Example:
    //   {"node_name":"node_data","sibling_node":{"childe_node":"childe_data"}}

    uint16_t l_inx = 0;
    uint8_t l_state = 0;
    uint8_t l_name = 0;
    uint16_t l_data = 0;
    HttpInterfaceElement_t *e;

    if ( NULL_E_PTR == ePtr ) {
        return 0;
    }

    e = ePtr;

#if defined( DEBUG_OUTPUT_MISC )
        Serial.println( ePtr->name );
#endif


    // NOTE: this only works if root node of tree has zero length name
    // and no data.

    while( l_inx < len ) {
        if ( 0 == l_state ) {
            DEBUG_OUT( DEBUG_OUTPUT_BDY, "BDY:0" );
            // Check for zero length name, don't need quote
            // zero length name also means no data, got to child if there is one
            if ( '\0' == e->name[ l_name ] ) {
                if ( NULL_E_PTR != e->child ) {
                    DEBUG_OUT( DEBUG_OUTPUT_BDY, "BDY:0.2" );
                    e = e->child;
                    buf[ l_inx++ ] = '{';
                    l_state = 1;
                } else {
                    DEBUG_OUT( DEBUG_OUTPUT_BDY, "BDY:0.1" );
                    // TODO This should never happen, what needs to be done here?
                }
            } else {
                DEBUG_OUT( DEBUG_OUTPUT_BDY, "BDY:0.3" );
                buf[ l_inx++ ] = '{';
                l_state = 1;
            }
        } else if ( 1 == l_state ) {
            DEBUG_OUT( DEBUG_OUTPUT_BDY, "BDY:01" );
            // Quote at start of element name
            buf[ l_inx++ ] = '"';
            l_state = 2;
        } else if ( 2 == l_state ) {
            DEBUG_OUT( DEBUG_OUTPUT_BDY, "BDY:02" );
            // Copy element name from structure
            if ( e->name[ l_name ] == '\0' ) {
                l_state = 3;
                l_name = 0;
            } else {
                buf[ l_inx++ ] = e->name[ l_name++ ];
                l_state = 2;
            }
        } else if ( 3 == l_state ) {
            DEBUG_OUT( DEBUG_OUTPUT_BDY, "BDY:03" );
            // Quote at end of element name
            buf[ l_inx++ ] = '"';
            l_state = 4;
        } else if ( 4 == l_state ) {
            DEBUG_OUT( DEBUG_OUTPUT_BDY, "BDY:04" );
            // Colon after name
            buf[ l_inx++ ] = ':';
            if ( NULL_E_PTR == e->child ) {
                l_state = 5;
            } else {
                e = e->child;
                l_state = 0;
            }
        } else if ( 5 == l_state ) {
            DEBUG_OUT( DEBUG_OUTPUT_BDY, "BDY:05" );
            // copy data string from leaf node
            if ( l_data >= e->len ) {
                l_data = 0;
                l_state = 6;
            } else {
                buf[ l_inx++ ] = e->val[ l_data++ ];
                l_state = 5;
            }
        } else if ( 6 == l_state ) {
            DEBUG_OUT( DEBUG_OUTPUT_BDY, "BDY:06" );
            // End of data for node, add closing brace if no siblings.
            if ( e != ePtr ) {
                if ( NULL_E_PTR == e->next_sibling ) {
                    DEBUG_OUT( DEBUG_OUTPUT_BDY, "BDY:06.1" );
                    // no siblings
                    l_state = 8;
                } else {
                    DEBUG_OUT( DEBUG_OUTPUT_BDY, "BDY:06.2" );
                    // Go to sibling
                    e = e->next_sibling;
                    // then add the coma
                    l_state = 7;
                }
            } else {
                // Don't check for siblings if this is the requested node
                l_state = 8;
            }
        } else if ( 7 == l_state ) {
            DEBUG_OUT( DEBUG_OUTPUT_BDY, "BDY:07" );
            // Add coma after previous sibling
            buf[ l_inx++ ] = ',';
            l_state = 1; // no brace before sibling
        } else if ( 8 == l_state ) {
            DEBUG_OUT( DEBUG_OUTPUT_BDY, "BDY:08" );
            // End of siblings. Go back to previous node
            if ( e == ePtr ) {
                DEBUG_OUT( DEBUG_OUTPUT_BDY, "BDY:08.1" );
                // At requested node
                if ( '\0' != e->name[ 0 ] ) {
                    DEBUG_OUT( DEBUG_OUTPUT_BDY, "BDY:08.1.1" );
                    // If root node not requested need an extra close brace
                    buf[ l_inx++ ] = '}';
                }
                return l_inx;
            } else {
                DEBUG_OUT( DEBUG_OUTPUT_BDY, "BDY:08.2" );
                // Go to parent and add closing brace
                buf[ l_inx++ ] = '}';
                e = e->parent;
            }
        } else {
            // This should never happen
            return 0;
        }

    } // l_inx < len
    // if the loop ended it was because data was too long for buffer
    DEBUG_OUT( DEBUG_OUTPUT_BDY, "BDY:99" );
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
//
// SendHttpResponse
//
///////////////////////////////////////////////////////////////////////////////
static void SendHttpResponse( int16_t sd, uint16_t req_flags, char *data, uint16_t len )
{
    int16_t l_send_size = 0;
    int16_t l_sent_size = 0;

    DEBUG_OUT( DEBUG_OUTPUT_SEND_RESP, "SendHttpResponse()" );

    if ( !(SendHttpHeader( sd, req_flags )) ) {
        // Head send error
        return;
    }

    DEBUG_OUT( DEBUG_OUTPUT_SEND_RESP, "SHR:00" );
    if ( ( NULL_PTR != data ) && ( 0 != len ) ) {
        DEBUG_OUT( DEBUG_OUTPUT_SEND_RESP, "SHR:01" );

        /* Send body pointed to by data
         * Need to break up send if data is too long. Haven't
         * found any documentation of max length for send. May
         * be limited by stack space.
         * SOCKET_SEND_CHUNK determins max send size
         */
        while ( len > 0 ) {
            if ( len > SOCKET_SEND_CHUNK ) {
                DEBUG_OUT( DEBUG_OUTPUT_SEND_RESP, "SHR:02" );
                l_send_size = SOCKET_SEND_CHUNK;
            } else {
                DEBUG_OUT( DEBUG_OUTPUT_SEND_RESP, "SHR:03" );
                l_send_size = len;
            }
            l_sent_size = send( sd, data, l_send_size, 0 );
            DEBUG_OUT( DEBUG_OUTPUT_SEND_RESP, "SHR:04" );
            if ( l_sent_size < 0 ) {
                DEBUG_OUT( DEBUG_OUTPUT_SEND_RESP, "SOCKET send error body" );
            } else {
                DEBUG_OUT( DEBUG_OUTPUT_SEND_RESP, "SHR:05" );
                data += l_sent_size;
                len -= l_sent_size;
            }
        }
        DEBUG_OUT( DEBUG_OUTPUT_SEND_RESP, "SHR:06" );
    }
} // SendHttpResponse

///////////////////////////////////////////////////////////////////////////////
//
// SendHttpHeader
//
///////////////////////////////////////////////////////////////////////////////
static uint8_t SendHttpHeader( int16_t sd, uint8_t req_flags )
{
    // return 0 on error
    // return 1 on success
    const char l_resp_hdr1_str[] = "HTTP/1.1 ";
    const char l_bad_req_str[] = "400 Bad Request\r\n";
    const char l_srv_err_str[] = "500 Server Error\r\n";
    const char l_resp_ok_str[] = "200 OK\r\n";
    const char l_resp_hdr2_str[] = "Content-Type: application/json\r\n";
    const char l_resp_hdr3_str[] = "Connection: close\r\n\r\n";
    int16_t l_send_size = 0;

    DEBUG_OUT( DEBUG_OUTPUT_SEND_RESP, "SendHttpHeader()" );

    //
    // Send Header
    //
    // NOTE: need to subtract 1 from size of all string literal constants
    // to avoid sending null terminator
    //
    l_send_size = send( sd, l_resp_hdr1_str, sizeof( l_resp_hdr1_str ) - 1, 0 );
    if ( l_send_size < 0 ) {
        return 0;
        DEBUG_OUT( DEBUG_OUTPUT_SEND_RESP, "SOCKET send error01" );
    }

    if ( HTTP_REQ_ERR_FLAG & req_flags ) {
        l_send_size = send( sd, l_srv_err_str, sizeof( l_srv_err_str ) - 1, 0 );
        if ( l_send_size < 0 ) {
            return 0;
            DEBUG_OUT( DEBUG_OUTPUT_SEND_RESP, "SOCKET send error02" );
        }
    } else if ( HTTP_REQ_BAD_FLAG & req_flags ) {
        l_send_size = send( sd, l_bad_req_str, sizeof( l_bad_req_str ) - 1, 0 );
        if ( l_send_size < 0 ) {
            return 0;
            DEBUG_OUT( DEBUG_OUTPUT_SEND_RESP, "SOCKET send error03" );
        }
    } else {
        l_send_size = send( sd, l_resp_ok_str, sizeof( l_resp_ok_str ) - 1, 0 );
        if ( l_send_size < 0 ) {
            return 0;
            DEBUG_OUT( DEBUG_OUTPUT_SEND_RESP, "SOCKET send error04" );
        }
    }
    l_send_size = send( sd, l_resp_hdr2_str, sizeof( l_resp_hdr2_str ) - 1, 0 );
    if ( l_send_size < 0 ) {
        return 0;
        DEBUG_OUT( DEBUG_OUTPUT_SEND_RESP, "SOCKET send error05" );
    }
    l_send_size = send( sd, l_resp_hdr3_str, sizeof( l_resp_hdr3_str ) - 1, 0 );
    if ( l_send_size < 0 ) {
        return 0;
        DEBUG_OUT( DEBUG_OUTPUT_SEND_RESP, "SOCKET send error06" );
    }
    return 1;
} // SendHttpHeader

///////////////////////////////////////////////////////////////////////////////
//
// eHandler_ROOT
//
///////////////////////////////////////////////////////////////////////////////
static uint8_t eHandler_ROOT( char * buf, uint16_t len )
{
    return 1;
}

///////////////////////////////////////////////////////////////////////////////
//
// eHandler_LED1
//
///////////////////////////////////////////////////////////////////////////////
static uint8_t eHandler_LED1( char * buf, uint16_t len )
{
    /* return 0 on error
     * return 1 on success
     */
    /* expected strings for LED1:
     * "LED1":0
     * "LED1":1
     */
    static HttpInterfaceElement_t * const ePtr = &e_LED1;

    DEBUG_OUT( DEBUG_OUTPUT_H_LED1, "LED1:00" );

    if ( len == ePtr->len ) {
        DEBUG_OUT( DEBUG_OUTPUT_H_LED1, "LED1:01" );
        // value should be 0 or 1
        // turn LED on or off based on value
        if ( '0' == buf[ 0 ] ) {
            DEBUG_OUT( DEBUG_OUTPUT_H_LED1, "LED1:02" );
            digitalWrite( D_OUT_LED1, LOW );
            ePtr->val[0] = '0';
        } else if ( '1' == buf[ 0 ] ) {
            DEBUG_OUT( DEBUG_OUTPUT_H_LED1, "LED1:03" );
            digitalWrite( D_OUT_LED1, HIGH );
            ePtr->val[0] = '1';
        } else {
            DEBUG_OUT( DEBUG_OUTPUT_H_LED1, "LED1:04" );
            return 0;
        }
    } else {
        // Data length wrong
        return 0;
        DEBUG_OUT( DEBUG_OUTPUT_H_LED1, "LED1:05" );
    }

    // Made it here, everything worked
    return 1;
}

///////////////////////////////////////////////////////////////////////////////
//
// eHandler_Clock
//
///////////////////////////////////////////////////////////////////////////////
static uint8_t eHandler_Clock( char * buf, uint16_t len )
{
    Clock_t l_time;
    uint8_t l_inx;
    uint8_t l_val;

    /* TODO check for quotes around data string??? */

    l_val = (((uint8_t)buf[1]-0x30) * 10) + ((uint8_t)buf[2]-0x30);
    if ( l_val > 23 ) {
        return 0;
    } else {
        l_time.Hour_u8 = l_val;
    }

    l_val = (((uint8_t)buf[3]-0x30) * 10) + ((uint8_t)buf[4]-0x30);
    if ( l_val > 59 ) {
        return 0;
    } else {
        l_time.Minute_u8 = l_val;
    }

    l_val = (((uint8_t)buf[5]-0x30) * 10) + ((uint8_t)buf[6]-0x30);
    if ( l_val > 59 ) {
        return 0;
    } else {
        l_time.Second_u8 = l_val;
    }

    l_val = (uint8_t)buf[7]-0x30;
    if ( l_val > 6 ) {
        return 0;
    } else {
        l_time.Day_u8 = ((uint8_t)0x01U) << l_val;
    }

    for ( l_inx = 1; l_inx < 8; l_inx ++ ) {
        E_val_Clock[ l_inx ] = buf[ l_inx ];
    }

    Set_Time( l_time.Hour_u8, l_time.Minute_u8, l_time.Second_u8, l_time.Day_u8 );

    return 1;
}

///////////////////////////////////////////////////////////////////////////////
//
// eHandler_Alarms
//
///////////////////////////////////////////////////////////////////////////////
static uint8_t eHandler_Alarms( char * buf, uint16_t len )
{
    return 1;
}

///////////////////////////////////////////////////////////////////////////////
//
// eHandler_Alarm0
//
///////////////////////////////////////////////////////////////////////////////
static uint8_t eHandler_Alarm0( char * buf, uint16_t len )
{
    uint8_t l_inx = 0;
    uint8_t l_status = 0;

    l_status = eHandler_AlarmX( 0, buf, len );

    for ( l_inx = 1; l_inx < 10; l_inx ++ ) {
        E_val_Alarm0[ l_inx ] = buf[ l_inx ];
    }

    return l_status;
}

///////////////////////////////////////////////////////////////////////////////
//
// eHandler_Alarm1
//
///////////////////////////////////////////////////////////////////////////////
static uint8_t eHandler_Alarm1( char * buf, uint16_t len )
{
    uint8_t l_inx = 0;
    uint8_t l_status = 0;

    l_status = eHandler_AlarmX( 1, buf, len );

    for ( l_inx = 1; l_inx < 10; l_inx ++ ) {
        E_val_Alarm1[ l_inx ] = buf[ l_inx ];
    }

    return l_status;
}

///////////////////////////////////////////////////////////////////////////////
//
// eHandler_Alarm2
//
///////////////////////////////////////////////////////////////////////////////
static uint8_t eHandler_Alarm2( char * buf, uint16_t len )
{
    uint8_t l_inx = 0;
    uint8_t l_status = 0;

    l_status = eHandler_AlarmX( 2, buf, len );

    for ( l_inx = 1; l_inx < 10; l_inx ++ ) {
        E_val_Alarm2[ l_inx ] = buf[ l_inx ];
    }

    return l_status;
}

///////////////////////////////////////////////////////////////////////////////
//
// eHandler_Alarm3
//
///////////////////////////////////////////////////////////////////////////////
static uint8_t eHandler_Alarm3( char * buf, uint16_t len )
{
    uint8_t l_inx = 0;
    uint8_t l_status = 0;

    l_status = eHandler_AlarmX( 3, buf, len );

    for ( l_inx = 1; l_inx < 10; l_inx ++ ) {
        E_val_Alarm3[ l_inx ] = buf[ l_inx ];
    }

    return l_status;
}

///////////////////////////////////////////////////////////////////////////////
//
// eHandler_AlarmX
//
///////////////////////////////////////////////////////////////////////////////
static uint8_t eHandler_AlarmX( uint8_t aInx, char * buf, uint16_t len )
{
    Alarm_t l_alarm;
    uint8_t l_val;

    /* TODO check for quotes around data string??? */

    l_val = (((uint8_t)buf[1]-0x30) * 10) + ((uint8_t)buf[2]-0x30);
    if ( l_val > 23 ) {
        return 0;
    } else {
        l_alarm.Hour_u8 = l_val;
    }

    l_val = (((uint8_t)buf[3]-0x30) * 10) + ((uint8_t)buf[4]-0x30);
    if ( l_val > 59 ) {
        return 0;
    } else {
        l_alarm.Minute_u8 = l_val;
    }

    l_val = (((uint8_t)buf[5]-0x30) * 10) + ((uint8_t)buf[5]-0x30);
    if ( l_val > 59 ) {
        return 0;
    } else {
        l_alarm.Second_u8 = l_val;
    }

    l_val = (((uint8_t)buf[7]-0x30) * 100)
          + (((uint8_t)buf[8]-0x30) * 10)
          + ((uint8_t)buf[9]-0x30);
    if ( l_val > 0x7FU ) {
        return 0;
    } else {
        l_alarm.Day_u8 = l_val;
    }

    Set_Alarm( aInx, l_alarm.Hour_u8, l_alarm.Minute_u8, l_alarm.Second_u8, l_alarm.Day_u8 );

    return 1;
}

#if 0
///////////////////////////////////////////////////////////////////////////////
//
// eHandler_TXT0
//
///////////////////////////////////////////////////////////////////////////////
static uint8_t eHandler_TXT0( char * buf, uint16_t len )
{
    return 1;
}

///////////////////////////////////////////////////////////////////////////////
//
// eHandler_TXT1
//
///////////////////////////////////////////////////////////////////////////////
static uint8_t eHandler_TXT1( char * buf, uint16_t len )
{
    /* return 0 on error
     * return 1 on success
     */
    /* expected strings for TXT0:
     * "TXT1":"xx" --> two chars in quotes
     */
    static HttpInterfaceElement_t * const ePtr = &e_TXT1;
    uint8_t l_inx = 0;

    DEBUG_OUT( DEBUG_OUTPUT_H_TXT1, "TXT1:00" );

    if ( len == ePtr->len ) {
        DEBUG_OUT( DEBUG_OUTPUT_H_TXT1, "TXT1:01" );
        for ( l_inx = 0; l_inx < len; l_inx++ ) {
            DEBUG_OUT( DEBUG_OUTPUT_H_TXT2, "TXT1:01.1" );
            if ( ( 0 == l_inx ) && ( '"' != buf[ l_inx ] ) ) {
                DEBUG_OUT( DEBUG_OUTPUT_H_TXT1, "TXT1:02" );
                return 0;
            }
            if ( ( ( len - 1 ) == l_inx ) && ( '"' != buf[ l_inx ] ) ) {
                DEBUG_OUT( DEBUG_OUTPUT_H_TXT1, "TXT1:03" );
                return 0;
            }
            ePtr->val[ l_inx ] = buf[ l_inx ];
        }
        DEBUG_OUT( DEBUG_OUTPUT_H_TXT1, "TXT1:04" );
        return 1;
    } else {
        // Data length wrong
        DEBUG_OUT( DEBUG_OUTPUT_H_TXT1, "TXT1:05" );
        return 0;
    }
    // Should never get here
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
//
// eHandler_TXT2
//
///////////////////////////////////////////////////////////////////////////////
static uint8_t eHandler_TXT2( char * buf, uint16_t len )
{
    /* return 0 on error
     * return 1 on success
     */
    /* expected strings for TXT0:
     * "TXT1":"xx" --> two chars in quotes
     */
    static HttpInterfaceElement_t * const ePtr = &e_TXT2;
    uint8_t l_inx = 0;

    DEBUG_OUT( DEBUG_OUTPUT_H_TXT2, "TXT2:00" );

    if ( len == ePtr->len ) {
        DEBUG_OUT( DEBUG_OUTPUT_H_TXT2, "TXT2:01" );
        for ( l_inx = 0; l_inx < len; l_inx++ ) {
            DEBUG_OUT( DEBUG_OUTPUT_H_TXT2, "TXT2:01.1" );
            if ( ( 0 == l_inx ) && ( '"' != buf[ l_inx ] ) ) {
                DEBUG_OUT( DEBUG_OUTPUT_H_TXT2, "TXT2:02" );
                return 0;
            }
            if ( ( ( len - 1 ) == l_inx ) && ( '"' != buf[ l_inx ] ) ) {
                DEBUG_OUT( DEBUG_OUTPUT_H_TXT2, "TXT2:03" );
                return 0;
            }
            //Serial.write( buf[ l_inx ] );
            //Serial.println("");
            ePtr->val[ l_inx ] = buf[ l_inx ];
            //Serial.write( ePtr->val[ l_inx ] );
            //Serial.println("");
        }
        DEBUG_OUT( DEBUG_OUTPUT_H_TXT2, "TXT2:04" );
        return 1;
    } else {
        // Data length wrong
        DEBUG_OUT( DEBUG_OUTPUT_H_TXT2, "TXT2:05" );
        return 0;
    }
    // Should never get here
    return 0;
}
#endif

