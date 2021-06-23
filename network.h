#ifndef ___NETWORK___H___
#define ___NETWORK___H___

#include <sys/socket.h>		// for TCPSOCKET-type
#include <sys/thread.h>		// for THREAD-type
#include <stdio.h>			// for FILE-type

// параметры по умолчанию
#define MYMAC   0x00, 0x0A, 0x59, 0x04, 0x01, 0x01 /* free MAC address from Web51 project */
#define MYIP    "192.168.88.101"
#define MYMASK  "255.255.255.0"
#define MYGATEWAY "192.168.88.1"

// порт Telnet
#define TELNET_PORT     23
#define REFLECT_PORT	1500

#define	ETH_BUFFERSIZE_TELNET	1024 //256, 512 (1024 for 28 bop)
#define ETH_BUFFERSIZE_INREF	20
#define ETH_UDP_BUFFERSIZE		128

#define UDP_PORT		9999
#define UDP_TIMEOUT		200
#define UDP_FIND_STRING			"Find IIT RTU-Diagnostic Unit"
#define UDP_FIND_VERSION_STRING	"Find IIT RTU-Diagnostic Unit + Version"
#define UDP_FIND_NUMBER_STRING	"Find IIT RTU-Diagnostic Unit + Version + LongNumber"
#define UDP_ANSWER_STRING		"IIT RTU-Diagnostic Unit"
#define UDP_SET_STRING			"Set IIT RTU-Diagnostic Unit"
#define UDP_RESET_STRING		"Reset IIT RTU-Diagnostic Unit"

#define ETH_REFLECT_VERSION	0x06

extern u_char haveConnectReflect;
extern u_char have_reset_command;

extern u_char pc_loaded;

extern TCPSOCKET *sockReflect;		// сокет

void defaultNet(void);

void initNetwork(void);

void loopNetwork(void);

THREAD(ReflectThread, arg);

THREAD(UdpFind, arg);

#endif	// ___NETWORK___H___
