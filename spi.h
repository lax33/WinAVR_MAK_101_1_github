#ifndef ___SPI___H___
#define ___SPI___H___

#include <sys/thread.h>			// for THREAD-type

#define SPI_DDR			DDRB
#define SPI_OUT			PORTB
#define SPI_IN			PINB
#define SPI_MISO		3
#define SPI_MOSI		2
#define SPI_CLK			1
#define SPI_SS			0

#define SPI_SEL_DDR		DDRF
#define SPI_SEL_OUT		PORTF
#define SPI_SEL_IN		PINF
#define SPI_SEL			7

#define SPI_BUFF_SIZE	128

extern uint8_t useEthernet;

void SPI_init(void);

uint8_t SPI_have_data(void);

u_char SPI_read_byte(void);

u_char SPI_write_byte(u_char byte);

u_char SPI_read_buffer(u_char *buff);

void SPI_write_buffer(u_char *buff, u_char cnt);

THREAD(SPI_Receiver, arg);

#endif	// ___SPI___H___
