#include <dev/board.h>
#include <avr/boot.h>		// for BOOT-functions


#include <io.h>					// for _ioctl*-function
#include <arpa/inet.h>			// for inet_addr-function
#include <sys/confnet.h>		// for confnet-data

#include "uart.h"
#include "network.h"
#include "spi.h"
#include "services.h"

FILE *uartFile0 = NULL;
FILE *uartFile1 = NULL;

int initUart(u_char devNum, u_long baud, u_long timeout)
{
	int err;
	NUTDEVICE *dev = &DEV_UART0;

	if (devNum)
		dev = &DEV_UART1;
	else
		dev = &DEV_UART0;

	// регистрация UART, указанного в dev
	err = NutRegisterDevice(dev, 0, 0);
	if (err)
		return -1;

	if (devNum)
		uartFile1 = fopen("uart1", "r+b");
	else
		uartFile0 = fopen("uart0", "r+b");

	if (devNum)
	{
		if (!uartFile1)
			return -1;
		else if (!uartFile1)
			return -1;
	}

	if (devNum)
	{
		err = _ioctl(_fileno(uartFile1), UART_SETSPEED, &baud);
		err = _ioctl(_fileno(uartFile1), UART_SETREADTIMEOUT, &timeout);
	}
	else
	{
		err = _ioctl(_fileno(uartFile0), UART_SETSPEED, &baud);
		err = _ioctl(_fileno(uartFile0), UART_SETREADTIMEOUT, &timeout);
	}
	if (err)
		return -1;

	return 0;
}

/*
 *Print current network parameters
 */

void PrintNetworkParam(void)
{
	printf("\r\nEthernet name = %s\r\n", (confnet.cd_name));
	printf("MAC address   = %02X:%02X:%02X:%02X:%02X:%02X\r\n", confnet.cdn_mac[0],confnet.cdn_mac[1],confnet.cdn_mac[2],confnet.cdn_mac[3],confnet.cdn_mac[4],confnet.cdn_mac[5]);
	printf("IP address    = %s\r\n", inet_ntoa(confnet.cdn_ip_addr));
	printf("IP mask       = %s\r\n", inet_ntoa(confnet.cdn_ip_mask));
	printf("GW IP address = %s\r\n", inet_ntoa(confnet.cdn_gateway));
}

/*
 * From RS232 to ???.
 *
THREAD(Receiver, arg)
{
	u_char *buff;
	int cnt;

    while (1)
	{
		buff = malloc(UART_BUFFERSIZE);
		cnt = fread(buff, 1, UART_BUFFERSIZE, uartFile0);
		free(buff);
        NutThreadYield();
    }
}*/

/*u_char boot(u_char *buff)
{
	unsigned long flash_size, flash_count;
	u_char boot_res = 0x00;
	int cnt, i;
	
	cli();

	uart0_send(0xF0);

	// CMD_BEGIN_FLASH_DATA
	uart0_rec_buff(buff, 6);
	boot_res = 0x00;
	if (buff[0] == 0x13)
	if (buff[1] == 0x14)
	if (buff[2] == 0x15)
	if (buff[3] == 0x16)
	if (buff[4] == 0x17)
	if (buff[5] == 0x18)
		boot_res = 0x01;
	if (!boot_res)
	{
		uart0_send(0xF1);
		sei();
		return 0x00;
	}
	uart0_send(0xF0);

	// FLASH_SIZE
	uart0_rec_buff(buff, 4);
	flash_size = ((u_long)buff[0] << 24) + ((u_long)buff[1] << 16) + ((u_long)buff[2] << 8) + ((u_long)buff[3]);
	uart0_send(0xF0);

	// FLASH_DATA
	flash_count = 0;
    while (flash_count < flash_size)
	{
		if (flash_size - flash_count > SPM_PAGESIZE)
			cnt = SPM_PAGESIZE;
		else
			cnt = flash_size - flash_count;

		uart0_rec_buff(buff, cnt);

		for (i = cnt; i < SPM_PAGESIZE; i++)
			buff[i] = 0xFF;
        
		if (flash_count + cnt <= MAX_NEW_PROGRAM_SIZE)
		{
			boot_page_erase((u_long)NEW_PROGRAM_FLASH_ADDRESS + flash_count);
			while(boot_rww_busy())
				boot_rww_enable();
			for(i = 0; i < cnt; i += 2)
				boot_page_fill(i + NEW_PROGRAM_FLASH_ADDRESS + flash_count, (buff[i + 1] << 0x08) + buff[i]);
			boot_page_write((unsigned long)NEW_PROGRAM_FLASH_ADDRESS + flash_count);
			while(boot_rww_busy())
				boot_rww_enable();
		}
		flash_count += cnt;
		uart0_send(0x13);
	}
	uart0_send(0xF0);
	
	// CMD_BEGIN_VERIFY_FLASH
	uart0_rec_buff(buff, 6);
	boot_res = 0x00;
	if (buff[0] == 0x13)
	if (buff[1] == 0x14)
	if (buff[2] == 0x15)
	if (buff[3] == 0x16)
	if (buff[4] == 0x17)
	if (buff[5] == 0x19)
		boot_res = 0x01;
	if (!boot_res)
	{
		uart0_send(0xF1);
		sei();
		return 0x00;
	}
	uart0_send(0xF0);

	// FLASH VERIFY
	flash_count = 0;
    while (flash_count < flash_size)
	{
		if (flash_size - flash_count > SPM_PAGESIZE)
			cnt = SPM_PAGESIZE;
		else
			cnt = flash_size - flash_count;

		uart0_rec_buff(buff, cnt);

		if (flash_count + cnt <= MAX_NEW_PROGRAM_SIZE)
		{
			for(i = 0; i < cnt; i++)
				if (pgm_read_byte_far(i + NEW_PROGRAM_FLASH_ADDRESS + flash_count) != buff[i])
					boot_res = 0x00;
		}
		flash_count += cnt;
		uart0_send(0x13);
	}
	if (boot_res)
		uart0_send(0xF0);
	else
	{
		uart0_send(0xF1);
		sei();
		return 0x00;
	}

	// CMD_START_FLASH
	uart0_rec_buff(buff, 6);
	boot_res = 0x00;
	if (buff[0] == 0x13)
	if (buff[1] == 0x14)
	if (buff[2] == 0x15)
	if (buff[3] == 0x16)
	if (buff[4] == 0x17)
	if (buff[5] == 0x20)
		boot_res = 0x01;
	if (!boot_res)
	{
		uart0_send(0xF1);
		sei();
		return 0x00;
	}
	uart0_send(0xF0);

    flash_count = 0;
    while (flash_count < MAX_NEW_PROGRAM_SIZE)
	{
		for (i = 0; i < SPM_PAGESIZE; i++)
			buff[i] = pgm_read_byte_far(i + flash_count + NEW_PROGRAM_FLASH_ADDRESS);

		boot_page_erase((u_long)flash_count);
		while(boot_rww_busy())
			boot_rww_enable();
		for(i = 0; i < SPM_PAGESIZE; i += 2)
			boot_page_fill(i + flash_count, (buff[i + 1] << 8) + buff[i]);
		boot_page_write((unsigned long)flash_count);
		while(boot_rww_busy())
			boot_rww_enable();
		flash_count += SPM_PAGESIZE;
	}

	uart0_send(0x15);
    
    flash_count = 0;
    while (flash_count < MAX_NEW_PROGRAM_SIZE)
	{
		boot_page_erase((u_long)flash_count + NEW_PROGRAM_FLASH_ADDRESS);
		while(boot_rww_busy())
			boot_rww_enable();
		flash_count += SPM_PAGESIZE;
	}

	uart0_send(0x16);

	charon_reset();

	return 0x01;
}*/

// посылка байта по UART
void uart0_send(u_char byte)
{
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = byte;
}

// приём байта по UART
u_char uart0_rec(void)
{
	while(!(UCSR0A & (1 << RXC0)));
	return UDR0;
}

// посылка буфера по UART
void uart0_send_buff(u_char *buffer, u_int size)
{
	u_int i;
	for (i = 0; i < size; i++)
		uart0_send(buffer[i]);
}

// приём буфера по UART
void uart0_rec_buff(u_char *buffer, u_int size)
{
	u_int i;
	for (i = 0; i < size; i++)
		buffer[i] = uart0_rec();
}

// посылка байта по UART boot
void uart0_send_boot(u_char byte)
{
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = byte;
}

// приём байта по UART boot
u_char uart0_rec_boot(void)
{
	while(!(UCSR0A & (1 << RXC0)));
	return UDR0;
}

// посылка буфера по UART boot
void uart0_send_buff_boot(u_char *buffer, u_int size)
{
	u_int i;
	for (i = 0; i < size; i++)
		uart0_send(buffer[i]);
}

// приём буфера по UART boot
void uart0_rec_buff_boot(u_char *buffer, u_int size)
{
	u_int i;
	for (i = 0; i < size; i++)
		buffer[i] = uart0_rec();
}



