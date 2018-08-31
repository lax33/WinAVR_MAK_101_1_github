#include "clock.h"
#include <dev/board.h>
#include <net/route.h>			// for NutIpRouteAdd-function
#include <sys/timer.h>			// for NutSleep-function
#include <sys/confnet.h>		// for confnet-data
#include <util/delay.h>

#include "uart.h"
#include "network.h"
#include "services.h"
#include "spi.h"
#include "dallas.h"
#include "text_lcd.h"

// Главная процедура приложения
int main(void)
{
	unsigned char buff[3];
	
	cbi(NOT_LOAD_DDR, NOT_LOAD);
	sbi(NOT_LOAD_OUT, NOT_LOAD);
	cbi(PC_POWER_LED_DDR, PC_POWER_LED);
	sbi(PC_POWER_LED_OUT, PC_POWER_LED);
	if ((!(PC_POWER_LED_IN & (1 << PC_POWER_LED))) ||	(!(NOT_LOAD_IN & (1 << NOT_LOAD))))
	{
		set_power_sw(0x00);
		set_start_time_min(0);
		set_start_time_sec(0);
	}

	buff[0] = 0x00;
	buff[1] = 0x01;
	buff[2] = 0x1F;               // 9bit resolution
	ds_set_resolution(buff);

	display_init();
//	display_set_font();
    display_clear_screen();

	// регистрация и инициализация UART0, UART1
	if (initUart(0, 115200, 0))
		printf("-- Registering UART0 failed\n\r");
	if (initUart(1, 57600, 800))
		printf("-- Registering UART1 failed\n\r");
	
	if (get_rtu_number() == 0xFFFFFF)
	{
		set_start_time_min(0);
		set_start_time_sec(0);
		set_rtu_number(114);
		defaultNet();
	}

	display_show(0x00, 0x00, 0x00, 0x00, get_rtu_number());
	
	
	init_timer1();
//	enable_timer1();
	ac_ii_init();
	otdr_init();
	init_key();
	init_power_pc();

	// перенправление stdout -> запись в UART0
	//                stdin  -> чтение из UART0
    freopen("uart0", "w+b", stdout);
    freopen("uart0", "r+b", stdin);

	// типа вывод инфы
	printf("---===<<< RTU-Diagnostic Unit >>>===---\n\r");
	printf("                   (c)2005-2013        \n\r");
	printf("_______________________________________\n\r");
	printf("           mail-to D.Popesku@beliit.com\n\r");

	// инициализация сетевого адаптера
	initNetwork();

	// регистрация сетевого адаптера
    if (NutRegisterDevice(&DEV_ETHER, 0x8300, 5))
        printf("-- Registering eth0 failed\n\r");
	// настройка сетевого адаптера eth0
    NutNetIfConfig("eth0", confnet.cdn_mac, confnet.cdn_ip_addr, confnet.cdn_ip_mask); /*configure Ethernet*/
	// добавление gateway, если он есть
    if (confnet.cdn_ip_addr && confnet.cdn_gateway)
        NutIpRouteAdd(0, 0, confnet.cdn_gateway, &DEV_ETHER);

    // вывод текущих установок сетевого адаптера eth0
    PrintNetworkParam();

	// запуск потока опроса UART0
	NutThreadCreate("ethRef", ReflectThread, NULL, 512);
	NutThreadCreate("SPI_REC", SPI_Receiver, NULL, 256);
	NutThreadCreate("ethUdpFind", UdpFind, NULL, 512);

	otau_init();

    NutThreadCreate("otau_th", OTAU_Thread, NULL, 256);

//	printf("- DDRD = %x\n\r", DDRD);
//	printf("- POUTD = %x\n\r", PORTD);
//	printf("- PIND = %x\n\r", PIND);

	cbi(NOT_LOAD_DDR, NOT_LOAD);
	sbi(NOT_LOAD_OUT, NOT_LOAD);
	if (NOT_LOAD_IN & (1 << NOT_LOAD))
	{
		NutThreadCreate("WIN_Load", WIN_Load_Thread, NULL, 256);
		NutThreadCreate("Power_Led_Thread", Power_Led_Thread, NULL, 128);
		NutThreadCreate("Power_Sw_Thread", Power_Sw_Thread, NULL, 128);
	}

	printf("\r\nRunning ...\r\n");

	loopNetwork();

	return 0;
}
