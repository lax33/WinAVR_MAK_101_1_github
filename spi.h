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

#define SPI_BUFF_SIZE	512

void SPI_init(void);

uint8_t SPI_have_data(void);

uint8_t SPI_read_byte(void);

void SPI_write_byte(uint8_t byte);

uint16_t SPI_read_buffer(uint8_t *buff);

void SPI_write_buffer(uint8_t *buff, uint16_t cnt);

THREAD(SPI_Receiver, arg);

#endif	// ___SPI___H___
