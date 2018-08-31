#include "clock.h"

#include <dev/board.h>
#include <util/delay.h>

#include "spi.h"
#include "uart.h"
#include "network.h"
#include "services.h"

void SPI_init(void)
{
	uint8_t i;

	// setup SPI ports
	cbi(SPI_DDR, SPI_MISO);

	sbi(SPI_DDR, SPI_MOSI);
	sbi(SPI_DDR, SPI_CLK);
	sbi(SPI_DDR, SPI_SS);

	sbi(SPI_OUT, SPI_SS);

	// enable SPI - SCK = Fosc / 8
	SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR0);	// set DORD, SPIE
	SPSR = (1 << SPI2X);
	i = SPSR;
	i = SPDR;
}

uint8_t SPI_have_data(void)
{
	return (!(OTDR_DATA_RDY_IN & (1 << OTDR_DATA_RDY)));
}

void SPI_write_byte(uint8_t byte)
{
	cbi(SPI_OUT, SPI_SS);
	_NOP(); 
	SPDR = byte;
	while (!(SPSR & (1 << SPIF)));
	_NOP(); 
	sbi(SPI_OUT, SPI_SS);
}

uint8_t SPI_read_byte(void)
{
    SPI_write_byte(0x55);
	return SPDR;
}

uint16_t SPI_read_buffer(uint8_t *buff)
{
	uint16_t cnt = 0;
//	bool b_ex = true;

	if (OTDR_DATA_RDY_IN & (1 << OTDR_DATA_RDY)) 
		return 0;

sl:	
	cbi(SPI_OUT, SPI_SS);
//	while ((!(OTDR_DATA_RDY_IN & (1 << OTDR_DATA_RDY))) && (cnt < SPI_BUFF_SIZE))
	while (!(OTDR_DATA_RDY_IN & (1 << OTDR_DATA_RDY)))
	{
		SPDR = 0x55;
		while (!(SPSR & (1 << SPIF)));
		buff[cnt++] = SPDR;
		if (cnt >= SPI_BUFF_SIZE) return cnt;
	}
	sbi(SPI_OUT, SPI_SS);

	_delay_loop_2(74);	// 20 mks
	
	if (!(OTDR_DATA_RDY_IN & (1 << OTDR_DATA_RDY))) goto sl;

	return cnt;
}

void SPI_write_buffer(uint8_t *buff, uint16_t cnt)
{
	if (cnt>0)
	{	
		uint16_t i;
		cbi(SPI_OUT, SPI_SS);
		for (i = 0; i < cnt; i++)
		{
			SPDR = buff[i];
			while (!(SPSR & (1 << SPIF)));
		}
		sbi(SPI_OUT, SPI_SS);
		//printf("out byte to OTDR: %d\n\r", cnt);
	}	
}

/*
 * From SPI to Ethernet
 */
THREAD(SPI_Receiver, arg)
{
	uint8_t *buff;
	uint16_t cnt;

	buff = malloc(SPI_BUFF_SIZE);
    while (1)
	{
		if (!have_reset_command)
		{
			cnt = SPI_read_buffer(buff);
			if ((cnt) && (haveConnectReflect) && (!otdr_rs232_manual))
			//{
				NutTcpSend(sockReflect, buff, cnt);
			//	printf("in byte from OTDR: %d\n\r", cnt);
			//}
		}
        NutThreadYield();
    }
	free(buff);
}
