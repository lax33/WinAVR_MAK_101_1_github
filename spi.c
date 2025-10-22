#include <dev/board.h>

#include "spi.h"
#include "uart.h"
#include "network.h"
#include "services.h"

uint8_t useEthernet	= 0x00;

void SPI_init(void)
{
	u_int i;

	// setup SPI ports
	sbi(SPI_OUT, SPI_SS);
	sbi(SPI_DDR, SPI_SS);

	cbi(SPI_DDR, SPI_MISO);

	sbi(SPI_DDR, SPI_MOSI);
	sbi(SPI_DDR, SPI_CLK);

//	sbi(SPI_SEL_DDR, SPI_SEL);
//	sbi(SPI_SEL_OUT, SPI_SEL);

	// enable SPI - SCK = Fosc / 4
	SPCR = (1 << SPE) | (1 << MSTR) | (1 << CPHA) | (1 << SPR0);	// set DORD, SPIE
	i = SPSR;
	i = SPDR;
}

uint8_t SPI_have_data(void)
{
	return (!(OTDR_DATA_RDY_IN & (1 << OTDR_DATA_RDY)));
}

u_char SPI_write_byte(u_char byte)
{
	SPDR = byte;
	_NOP();
	while (!(SPSR & (1 << SPIF)));
	return SPDR;
}

u_char SPI_read_byte(void)
{
	return SPI_write_byte(0x55);
}

u_char SPI_read_buffer(u_char *buff)
{
	u_char cnt = 0;

	if (!SPI_have_data()) return cnt;

	cbi(SPI_OUT, SPI_SS);
	_NOP();
	_NOP();
	_NOP();

	while (SPI_have_data() && (cnt < SPI_BUFF_SIZE))
		buff[cnt++] = SPI_read_byte();

	_NOP();
	_NOP();
	_NOP();
	sbi(SPI_OUT, SPI_SS);

	return cnt;
}

void SPI_write_buffer(u_char *buff, u_char cnt)
{
	u_char i;

	cbi(SPI_OUT, SPI_SS);
	_NOP();
	_NOP();
	_NOP();

	for (i = 0; i < cnt; i++)
		SPI_write_byte(buff[i]);

	_NOP();
	_NOP();
	_NOP();
	sbi(SPI_OUT, SPI_SS);
}

THREAD(SPI_Receiver, arg)
{
	u_char *buff;
	int cnt;

	buff = malloc(SPI_BUFF_SIZE);
	while (1)
	{
		if (!have_reset_command)
		{
			cnt = SPI_read_buffer(buff);
			if ((cnt) && (!otdr_rs232_manual))
			{
				if ((useEthernet) && (haveConnectReflect))
					NutTcpSend(sockReflect, buff, cnt);
				else
					fwrite(buff, 1, cnt, uartFile0);
			}
		}
		NutThreadYield();
	}
	free(buff);
}
