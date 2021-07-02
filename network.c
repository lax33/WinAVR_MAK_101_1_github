#include "clock.h"

#include <dev/board.h>
#include <util/delay.h>
#include <arpa/inet.h>		// for inet_addr-function
#include <sys/confnet.h>	// for confnet-data
#include <sys/version.h>
#include <string.h>			// for strlen
#include <avr/version.h>

#include "network.h"
#include "uart.h"
#include "commands.h"
#include "dallas.h"
#include "services.h"
#include "spi.h"
#include "text_lcd.h"

//#define PROGRAMM_VERSION	0x0104	// last good version
//#define PROGRAMM_VERSION	0x0105	// last good version + bad work enabled
//#define PROGRAMM_VERSION	0x0106	// last version (NutOS v 4.0.3, WinAVR 21-04-2006) + one delay
//#define PROGRAMM_VERSION	0x0107	// last version (NutOS v 4.0.3, WinAVR 21-04-2006) + all delay
//#define PROGRAMM_VERSION	0x0200	// rtu_v2 (NutOS v 4.2.1, WinAVR 21-04-2006)
//#define PROGRAMM_VERSION	0x0201	// + otau_key
//#define PROGRAMM_VERSION	0x0202	// + otau_key + сброс контроль + отобр. загрузки Винды (NutOS v 4.2.1, WinAVR 21-04-2006)
//#define PROGRAMM_VERSION	0x0300	// + otau_key + сброс контроль + отобр. загрузки Винды (NutOS v 4.4.1, WinAVR 21-04-2006)
//#define PROGRAMM_VERSION	0x0301	// + otau_key + сброс контроль + отобр. загрузки Винды + кнопка on/off (NutOS v 4.4.1, WinAVR 21-04-2006)
//#define PROGRAMM_VERSION	0x0302	// + otau_key + сброс контроль + отобр. загрузки Винды + кнопка on/off + сброс PC (NutOS v 4.4.1, WinAVR 21-04-2006)
//#define PROGRAMM_VERSION	0x0303	// + otau_key + сброс контроль + отобр. загрузки Винды + кнопка on/off + сброс PC (NutOS v 4.4.1, WinAVR 21-04-2006)
//#define PROGRAMM_VERSION	0x0304	// + otau_key + сброс контроль + отобр. загрузки Винды + кнопка on/off + сброс PC (NutOS v 4.4.1, WinAVR 21-04-2006) + без переключателя
//#define PROGRAMM_VERSION	0x0305	// + otau_key + сброс контроль + отобр. загрузки Винды + кнопка on/off + сброс PC (NutOS v 4.4.1, WinAVR 21-04-2006) + без переключателя + utoa
//#define PROGRAMM_VERSION	0x0400	// + otau_key + сброс контроль + отобр. загрузки Винды + кнопка on/off + сброс PC (NutOS v 4.4.1, WinAVR 21-04-2006) + без переключателя + utoa
//#define PROGRAMM_VERSION	0x0401	// + otau_key + сброс контроль + отобр. загрузки Винды + кнопка on/off + сброс PC (NutOS v 4.4.1, WinAVR 21-04-2006) + без переключателя + utoa + SPI 1.8Mbit
//#define PROGRAMM_VERSION	0x0402	// + otau_key + сброс контроль + отобр. загрузки Винды + кнопка on/off + сброс PC (NutOS v 4.4.1, WinAVR 21-04-2006) + без переключателя + utoa + SPI 1.8Mbit + SS
//#define PROGRAMM_VERSION	0x0403	// + otau_key + сброс контроль + отобр. загрузки Винды + кнопка on/off + сброс PC (NutOS v 4.4.1, WinAVR 21-04-2006) + без переключателя + utoa + SPI 1.8Mbit + SS + reset charon
//#define PROGRAMM_VERSION	0x0404	// + otau_key + сброс контроль + отобр. загрузки Винды + кнопка on/off + сброс PC (NutOS v 4.4.1, WinAVR 21-04-2006) + без переключателя + utoa + SPI 1.8Mbit + SS + reset charon + доп reset OTDR + 20mks перед выходом из SPI_READ_BUF
//#define PROGRAMM_VERSION	0x0405	// + otau_key + сброс контроль + отобр. загрузки Винды + кнопка on/off + сброс PC (NutOS v 4.4.1, WinAVR 21-04-2006) + без переключателя + utoa + SPI 1.8Mbit + SS + reset charon + доп reset OTDR + 20mks перед выходом из SPI_READ_BUF + ожидание 1s перед опросом переключателя
//#define PROGRAMM_VERSION	0x0406	// + otau_key + сброс контроль + отобр. загрузки Винды + кнопка on/off + сброс PC (NutOS v 4.4.1, WinAVR 21-04-2006) + без переключателя + utoa + SPI 1.8Mbit + SS + reset charon + доп reset OTDR + 20mks перед выходом из SPI_READ_BUF + ожидание 1s перед опросом переключателя + введена команда сброса переключателя
//#define PROGRAMM_VERSION	0x0407	// + otau_key + сброс контроль + отобр. загрузки Винды + кнопка on/off + сброс PC (NutOS v 4.4.1, WinAVR 21-04-2006) + без переключателя + utoa + SPI 1.8Mbit + SS + reset charon + доп reset OTDR + 20mks перед выходом из SPI_READ_BUF + ожидание 1s перед опросом переключателя + введена команда сброса переключателя + сброс переключателя при сбросе Чарона
//#define PROGRAMM_VERSION	0x0408	// + otau_key + сброс контроль + отобр. загрузки Винды + кнопка on/off + сброс PC (NutOS v 4.4.1, WinAVR 21-04-2006) + без переключателя + utoa + SPI 1.8Mbit + SS + reset charon + доп reset OTDR + 20mks перед выходом из SPI_READ_BUF + ожидание 1s перед опросом переключателя + введена команда сброса переключателя + сброс переключателя при сбросе Чарона + увеличил время перед запросом количиства портов
//#define PROGRAMM_VERSION	0x0409	// + otau_key + сброс контроль + отобр. загрузки Винды + кнопка on/off + сброс PC (NutOS v 4.4.1, WinAVR 21-04-2006) + без переключателя + utoa + SPI 1.8Mbit + SS + reset charon + доп reset OTDR + 20mks перед выходом из SPI_READ_BUF + ожидание 1s перед опросом переключателя + введена команда сброса переключателя + сброс переключателя при сбросе Чарона + увеличил время перед запросом количиства портов + add BUS
//#define PROGRAMM_VERSION	0x0500	// RTU_V5 (NutOS v 5.1, WinAVR 2010)
//#define PROGRAMM_VERSION	0x0501	// RTU_V5 (NutOS v 5.1, WinAVR 2010) + комм для АС
//#define PROGRAMM_VERSION	0x0502	// RTU_V5 (NutOS v 5.1, WinAVR 2010) + комм для АС + задержка между переклюбчением и вычитыванием порта
//#define PROGRAMM_VERSION	0x0503	// RTU_V5 (NutOS v 5.1, WinAVR 2010) + комм для АС + задержка между переклюбчением и вычитыванием порта + 24bit номер
//#define PROGRAMM_VERSION	0x0504	// RTU_V5 (NutOS v 5.1, WinAVR 2010) + комм для АС + задержка между переклюбчением и вычитыванием порта + 24bit номер + INI
//#define PROGRAMM_VERSION	0x0505	// RTU_V5 (NutOS v 5.1, WinAVR 2010) + комм для АС + задержка между переклюбчением и вычитыванием порта + 24bit номер + INI + add BOP
//define PROGRAMM_VERSION	0x0506	// RTU_V5 (NutOS v 5.1, WinAVR 2010) + комм для АС + задержка между переклюбчением и вычитыванием порта + 24bit номер + INI + add BOP + meas command on 23 port
//#define PROGRAMM_VERSION	0x0507	// RTU_V5 (NutOS v 5.1, WinAVR 2010) + комм для АС + задержка между переклюбчением и вычитыванием порта + 24bit номер + INI + add BOP + meas command on 23 port + off time
//#define PROGRAMM_VERSION	0x0508	// RTU_V5 (NutOS v 5.1, WinAVR 2010) + комм для АС + задержка между переклюбчением и вычитыванием порта + 24bit номер + INI + add BOP + meas command on 23 port + off time + remove last simbol NUM in DISPLEY
//#define PROGRAMM_VERSION	0x0509	// RTU_V5 (NutOS v 5.1, WinAVR 2010) + комм для АС + задержка между переклюбчением и вычитыванием порта + 24bit номер + INI + add BOP + meas command on 23 port + off time + remove last simbol NUM in DISPLEY + 7 simbols of number
//#define PROGRAMM_VERSION	0x050A	// RTU_V5 (NutOS v 5.1, WinAVR 2010) + комм для АС + задержка между переклюбчением и вычитыванием порта + 24bit номер + INI + add BOP + meas command on 23 port + off time + remove last simbol NUM in DISPLEY + 7 simbols of number + ПОДКЛЮЧ.К OTDR
//#define PROGRAMM_VERSION	0x050C	// v5.12 RTU_V5 (NutOS v 5.1, WinAVR 2010) + комм для АС + задержка между переклюбчением и вычитыванием порта + 24bit номер + INI + add BOP + meas command on 23 port + off time + remove last simbol NUM in DISPLEY + 7 simbols of number + ПОДКЛЮЧ.К OTDR + ОШИБКА с номером
//define PROGRAMM_VERSION	0x050D	//5.13 RTU_V5 (NutOS v 5.1, WinAVR 2010) + комм для АС + задержка между переклюбчением и вычитыванием порта + 24bit номер + INI + add BOP + meas command on 23 port + off time + remove last simbol NUM in DISPLEY + 7 simbols of number + ПОДКЛЮЧ.К OTDR + ОШИБКА с номером + СБРОС, нет "выкл МАК100...", нет "питание можно отключить" 
#define PROGRAMM_VERSION	0x050E	//5.14 + подключение 28 боп

// Ethernet - переменные
u_char mac[] = {MYMAC};
u_long ip_addr;
u_long ip_mask;
u_long ip_gateway;

u_char haveConnectTelnet = 0x00;
u_char haveConnectReflect = 0x00;
u_char have_reset_command = 0x00;

u_char pc_loaded = 0x00;

FILE *ethTelnetFile;

TCPSOCKET *sockReflect;		// сокет

u_char bad_work_enable = 0x00;

void defaultNet(void)
{
	memcpy(confnet.cdn_mac, mac, sizeof(confnet.cdn_mac));
	memcpy(confnet.cd_name, "eth0", sizeof(confnet.cd_name));
	confnet.cdn_ip_addr = inet_addr(MYIP);
	confnet.cdn_cip_addr = inet_addr(MYIP);
	confnet.cdn_ip_mask = inet_addr(MYMASK);
	confnet.cdn_gateway = inet_addr(MYGATEWAY);
	printf("-- Loading default values and storing to EEPROM\r\n");
	NutNetSaveConfig();
}


void initNetwork(void)
{
	// пытаемся прочитать конфигурацию сетевого адаптера из eeprom
    if (NutNetLoadConfig("eth0"))	// eeprom_addr = 64
	{
		defaultNet();
    }
}

void loopNetwork(void)
{
    TCPSOCKET *sockTelnet;		// сокет
	unsigned char sign, d_temp, f_temp;
	unsigned int d, f, port;
	int cnt, bt, tmp;
	char *buff;

    haveConnectTelnet = 0;
    while (1) 
	{
		// создаём сокет
        sockTelnet = NutTcpCreateSocket();

		// ожидаем коннекта на 23 порту
        NutTcpAccept(sockTelnet, TELNET_PORT);
      	printf("-- Client connected (IP address = %s, Port = %d)\r\n", 
			inet_ntoa(sockTelnet->so_remote_addr), TELNET_PORT);

		// открываем файл сокета
		ethTelnetFile = _fdopen((int)sockTelnet, "r+b");
        haveConnectTelnet = 1;

        /*
         * Call RS232 transmit routine. On return we will be
         * disconnected again.
         */
		buff = malloc(ETH_BUFFERSIZE_TELNET);
		while (haveConnectTelnet) 
		{
			if ((cnt = fread(buff, 1, ETH_BUFFERSIZE_TELNET, ethTelnetFile)) <= 0)
				break;
			buff[cnt] = 0x00;
			//if (strcmp(buff, CMD_START_FLASH_PROCEDURE)) printf("-- command - %s", buff);
			if (haveConnectTelnet)
			{
				if (!strcmp(buff, CMD_OTAU_RESET))
				{
                    if (otau_reset())
						fwrite(ANSWER_OK, 1, strlen(ANSWER_OK), ethTelnetFile);
					else
						fwrite(ANSWER_ERROR, 1, strlen(ANSWER_ERROR), ethTelnetFile);
				}
				else if (!strcmp(buff, CMD_OTAU_GET_CHANNEL))
				{
                    bt = otau_get_channel();
					if (bt <= (otau_get_count_channels() & (~OTAU_CAN_BLIND)))
					{
						sprintf(buff, "%d\r\n", bt);
						printf("--- OTAU_GET_CHANNEL - %s\r\n", buff);
						fwrite(buff, 1, strlen(buff), ethTelnetFile);
					}
					else
					{
						printf("--- OTAU_GET_CHANNEL - ERROR Read OPTICAL SWITCH\r\n");
                        fwrite(ANSWER_ERROR, 1, strlen(ANSWER_ERROR), ethTelnetFile);
					}
				}
				else if (!strcmp(buff, CMD_OTAU_GET_COUNT_CHANNELS))
				{
					bt = otau_get_count_channels() & (~OTAU_CAN_BLIND);
					if (bt)
					{
						sprintf(buff, "%d\r\n", bt);
						fwrite(buff, 1, strlen(buff), ethTelnetFile);
					}
					else
					{
						fwrite(ANSWER_ERROR, 1, strlen(ANSWER_ERROR), ethTelnetFile);
					}
				}
				else if (!strncmp(buff, CMD_OTAU_SET_CHANNEL, 17))
				{
					tmp = sscanf(buff, "otau_set_channel%d\r\n", &bt);
                    if (otau_set_channel(bt))
					{
						printf("--- OTAU_SET_CHANNEL - %d\r\n", bt);
						fwrite(ANSWER_OK, 1, strlen(ANSWER_OK), ethTelnetFile);
						display_set_channel(bt);
					}	
					else
					{
						printf("--- OTAU_SET_CHANNEL - ERROR");
						fwrite(ANSWER_ERROR, 1, strlen(ANSWER_ERROR), ethTelnetFile);
					}	
				}
				else if (!strcmp(buff, CMD_OTAU_CHECK_ALIVE))
				{
					if (otau_check_alive())
						fwrite(ANSWER_OK, 1, strlen(ANSWER_OK), ethTelnetFile);
					else
						fwrite(ANSWER_ERROR, 1, strlen(ANSWER_ERROR), ethTelnetFile);
				}
				else if (!strcmp(buff, CMD_PC_RESET))
				{
					reset_ext();
					fwrite(ANSWER_OK, 1, strlen(ANSWER_OK), ethTelnetFile);
				}
				else if (!strcmp(buff, CMD_OTDR_RESET_))
				{
					have_reset_command = 0x01;
					if (!otdr_rs232_manual)
					{
						haveConnectReflect = 0x00;
						otdr_reset();
						fwrite(ANSWER_OK, 1, strlen(ANSWER_OK), ethTelnetFile);
					}
					else 
						fwrite(ANSWER_BUSY, 1, strlen(ANSWER_BUSY), ethTelnetFile);
					have_reset_command = 0x00;	
				}
				else if (!strcmp(buff, CMD_OTDR_RESET))
				{
					have_reset_command = 0x01;
					if (!otdr_rs232_manual)
					{
						haveConnectReflect = 0x00;
						otdr_reset();
						SPI_write_byte(0x55);
                        SPI_write_byte(0x80);
						cnt = 0x00;
						while ((!SPI_have_data()) && (cnt < 10000))
							cnt++;
                        if (cnt == 10000)
						{
							fwrite(ANSWER_ERROR, 1, strlen(ANSWER_ERROR), ethTelnetFile);
						}
						else
						{
							bt = SPI_read_byte();
							SPI_write_byte(0x08);
							cnt = 0x00;
							while ((!SPI_have_data()) && (cnt < 10000))
								cnt++;
							if (cnt == 10000)
							{
								fwrite(ANSWER_ERROR, 1, strlen(ANSWER_ERROR), ethTelnetFile);
							}
							else
							{
								bt = SPI_read_byte();
								if (bt == 0x68)
									fwrite(ANSWER_OK, 1, strlen(ANSWER_OK), ethTelnetFile);
								else
								{
									fwrite(ANSWER_ERROR, 1, strlen(ANSWER_ERROR), ethTelnetFile);
								}
							}
						}
					}
					else
					{
						fwrite(ANSWER_BUSY, 1, strlen(ANSWER_BUSY), ethTelnetFile);
					}
					have_reset_command = 0x00;
				}
				else if (!strcmp(buff, CMD_GET_TEMPERATURE))
				{
					ds_convert_temperature_command();
					_delay_loop_2(60000);
					_delay_loop_2(60000);
					_delay_loop_2(60000);
					_delay_loop_2(60000);
					_delay_loop_2(60000);
					_delay_loop_2(60000);
					_delay_loop_2(60000);
					ds_read_temperature_command(&sign, &d_temp, &f_temp);
					sprintf(buff, "%c%d.%d\r\n", (sign) ? '-' : '+', d_temp, (f_temp) ? 0 : 5);	// +26.6\r\n
					fwrite(buff, 1, strlen(buff), ethTelnetFile);
				}
				else if (!strcmp(buff, CMD_HAVE_EXT_RS232_PLUGGED))
				{
					if (otdr_rs232_manual)
						fwrite(ANSWER_YES, 1, strlen(ANSWER_YES), ethTelnetFile);
					else
						fwrite(ANSWER_NO, 1, strlen(ANSWER_NO), ethTelnetFile);
				}
				else if (!strcmp(buff, CMD_PC_LOADED))
				{
					pc_loaded = 0x01;
					if (!win_loaded)
					{
					disable_timer1();
					stop_timer1();
					set_start_time_min(0);
					set_start_time_sec(0);
					}
					if (!otdr_rs232_manual)
						display_show(1, 0x00, otau_get_channel(), 0x00, get_rtu_number());
                    fwrite(ANSWER_OK, 1, strlen(ANSWER_OK), ethTelnetFile);
				}
                /*else if (!strcmp(buff, CMD_MEAS))
				{
                    display_show(0x02, otau_get_channel(), 0x00, 0x00);
                    fwrite(ANSWER_OK, 1, strlen(ANSWER_OK), ethTelnetFile);
				}*/
				else if (!strncmp(buff, CMD_ALARM_CRITICAL, 15))
				{
                    sscanf(&buff[15], "%d%d%d\r\n", &port, &d, &f);
                    display_show(0x03, port, d, f, 0x00);
					display_set_cursor_pos(6, 0);
					display_write_byte('!', DISPLAY_DATA_BYTE);
                    fwrite(ANSWER_OK, 1, strlen(ANSWER_OK), ethTelnetFile);
				}
				else if (!strncmp(buff, CMD_ALARM, 6))
				{
                    sscanf(&buff[6], "%d%d%d\r\n", &port, &d, &f);
                    display_show(0x03, port, d, f, 0x00);
                    fwrite(ANSWER_OK, 1, strlen(ANSWER_OK), ethTelnetFile);
				}
				else if (!strcmp(buff, CMD_GET_RTU_NUMBER))
				{
					sprintf(buff, "%lu\r\n", get_rtu_number());
					fwrite(buff, 1, strlen(buff), ethTelnetFile);
				}
				else if (!strcmp(buff, CMD_FREE_OTDR_PORT))
				{
					if (!otdr_rs232_manual)
					{
						haveConnectReflect = 0x00;
						fwrite(ANSWER_OK, 1, strlen(ANSWER_OK), ethTelnetFile);
					}
					else
					{
                        fwrite(ANSWER_BUSY, 1, strlen(ANSWER_BUSY), ethTelnetFile);
					}
				}
				else if (!strcmp(buff, CMD_START_FLASH_PROCEDURE))
				{
					//printf("Enter BOOT procedure\r\n");
					set_start_time_min(0);
					set_start_time_sec(0);
					boot((u_char *)buff);
					printf("Exit BOOT procedure\r\n");
				}
//				else if (!strcmp(buff, CMD_START_FLASH_BOOT))
//				{
//					//printf("Enter BOOT procedure\r\n");
//					flash_boot((u_char *)buff);
//					//printf("Exit BOOT procedure\r\n");
//				}
				else if (!strcmp(buff, CMD_BAD_WORK_ENABLE))
				{
					if (!otdr_rs232_manual)
					{
						haveConnectReflect = 0x00;
						bad_work_enable = 0x01;
						fwrite(ANSWER_OK, 1, strlen(ANSWER_OK), ethTelnetFile);
					}
					else
					{
                        fwrite(ANSWER_BUSY, 1, strlen(ANSWER_BUSY), ethTelnetFile);
					}
				}
				else if (!strcmp(buff, CMD_CONTROL_BREAK))
				{
					if (otdr_rs232_manual)
					{
						otdr_rs232_manual = 0x00;
						if (pc_loaded)
						{
							display_show(1, 0x00, 0x00, 0x00, get_rtu_number());
						}
						else if (con_otdr)
						{		
							display_show(0x08, 0x00, 0x00, 0x00, get_rtu_number());
						}
						else if (win_loaded)
						{
							display_show(0x05, 0x00, 0x00, 0x00, get_rtu_number());
						}
						else
						{
							display_show(0x00, 0x00, 0x00, 0x00, get_rtu_number());
							enable_timer1();
						}
					}
					fwrite(ANSWER_OK, 1, strlen(ANSWER_OK), ethTelnetFile);
				}
				else if (!strcmp(buff, CMD_CHARON_RESET))
				{
					fwrite(ANSWER_OK, 1, strlen(ANSWER_OK), ethTelnetFile);
					charon_reset();
				}
				else if (!strcmp(buff, CMD_MEAS))
				{
					printf("--- Have Meas Command on 23 port\n\r");
					disable_int_key();
					disable_timer1();
                    display_show(0x02, otau_get_channel(), 0x00, 0x00, 0x00);
					fwrite(ANSWER_OK, 1, strlen(ANSWER_OK), ethTelnetFile);
				}
				else if (!strcmp(buff, CMD_STOP_MEAS))
				{
					printf("--- Have Meas Done Command on 23 Port\n\r");
					if (pc_loaded)
					{
						display_show(1, 0x00, 0x00, 0x00, get_rtu_number());
					}
					else if (con_otdr)
					{		
						display_show(0x08, 0x00, 0x00, 0x00, get_rtu_number());
					}
					else if (win_loaded)
					{
						display_show(0x05, 0x00, 0x00, 0x00, get_rtu_number());
					}
					else
					{
						display_show(0x00, 0x00, 0x00, 0x00, get_rtu_number());
						enable_timer1();
					}
					enable_int_key();
					fwrite(ANSWER_OK, 1, strlen(ANSWER_OK), ethTelnetFile);
				}
				else if (!strncmp(buff, CMD_II_SET_POWER, 13))
				{
					tmp = sscanf(buff, "ii_set_power%d\r\n", &bt);
                    if (set_power_ii(bt))
					{
						fwrite(ANSWER_OK, 1, strlen(ANSWER_OK), ethTelnetFile);
					}	
					else
						fwrite(ANSWER_ERROR, 1, strlen(ANSWER_ERROR), ethTelnetFile);
				}
				else if (!strncmp(buff, CMD_AC_SET_POWER, 13))
				{
					tmp = sscanf(buff, "ac_set_power%d\r\n", &bt);
                    if (set_power_ac(bt))
					{
						fwrite(ANSWER_OK, 1, strlen(ANSWER_OK), ethTelnetFile);
					}	
					else
						fwrite(ANSWER_ERROR, 1, strlen(ANSWER_ERROR), ethTelnetFile);
				}
				else if (!strncmp(buff, CMD_INI_WR, strlen(CMD_INI_WR)))
				{
					//fwrite(ANSWER_OK, 1, strlen(ANSWER_OK), ethTelnetFile);
					NutTcpSend(sockTelnet, ANSWER_OK, strlen(ANSWER_OK));
					tmp = cnt - strlen(CMD_INI_WR);
//					printf("--- CMD_INI_WR, cnt = %d\r\n", tmp);
					uint32_t tcard;
					NutTcpGetSockOpt(sockTelnet, SO_RCVTIMEO, &tcard, sizeof(tcard));
//					printf("--- SO_RCVTIMEO = %lu\r\n", tcard);
					tcard = 5000;
					NutTcpSetSockOpt(sockTelnet, SO_RCVTIMEO, &tcard, sizeof(tcard));
					while (tmp != EEPROM_INI_LEN) 
					{
						cnt = fread(buff + tmp + strlen(CMD_INI_WR), 1, EEPROM_INI_LEN - tmp, ethTelnetFile);
						//cnt = NutTcpReceive(sockTelnet, buff + tmp + strlen(CMD_INI_WR), EEPROM_INI_LEN - tmp);
						if (cnt <= 0) break;
						tmp += cnt;
//						printf("--- CMD_INI_WR, cnt = %d\r\n", tmp);
					}
					tcard = 0;
					NutTcpSetSockOpt(sockTelnet, SO_RCVTIMEO, &tcard, sizeof(tcard));
					if (cnt < 0) break;
					if (tmp == EEPROM_INI_LEN)
					{
						write_ini((u_char *)(buff + strlen(CMD_INI_WR)));
						fwrite(ANSWER_OK, 1, strlen(ANSWER_OK), ethTelnetFile);
//						printf("--- CMD_INI_WR, OK\r\n");
					}
					else 
					{
						fwrite(ANSWER_ERROR, 1, strlen(ANSWER_ERROR), ethTelnetFile);
//						printf("--- CMD_INI_WR, ERROR\r\n");
					}
				}
				else if (!strncmp(buff, CMD_INI_RD, strlen(CMD_INI_RD)))
				{
					read_ini((u_char *)buff);
					tmp = 0;
					do
					{
						tmp += NutTcpSend(sockTelnet, buff + tmp, EEPROM_INI_LEN - tmp);
//						printf("--- CMD_INI_RD, cnt = %d\r\n", tmp);
					} while (tmp != EEPROM_INI_LEN);	
				}
				else if (!strncmp(buff, CMD_INI_SIZE, strlen(CMD_INI_SIZE)))   // запрос размера ini-файла ("800\r\n" если если размер 800, если 480 "ERROR_COMMAND\r\n\"
				{
					fwrite(ANSWER_INI_SIZE, 1, strlen(ANSWER_INI_SIZE), ethTelnetFile);
				}
				else
				{
					fwrite(ANSWER_ERROR_COMMAND, 1, strlen(ANSWER_ERROR_COMMAND), ethTelnetFile);
				}
				fflush(ethTelnetFile);
			}
		}
		haveConnectTelnet = 0;
		free(buff);

		// закрываем файл сокета
        fclose(ethTelnetFile);
		// закрываем сокет
        NutTcpCloseSocket(sockTelnet);
      	printf("-- Client disconnected (telnet)\r\n");
    }
}

THREAD(ReflectThread, arg)
{
	int cnt, i;
	char *buff;
	u_long timeout = 1000;
	int err;

    haveConnectReflect = 0;
    while (1) 
	{
		// создаём сокет
	    sockReflect = NutTcpCreateSocket();

		// ожидаем коннекта на 1500 порту
		NutTcpAccept(sockReflect, REFLECT_PORT);
		printf("-- Client connected (IP address = %s, Port = %d)\r\n", 
		inet_ntoa(sockReflect->so_remote_addr), REFLECT_PORT);

		if (!otdr_rs232_manual)
			haveConnectReflect = 1;
		else
			haveConnectReflect = 0;

		NutTcpSetSockOpt(sockReflect, SO_RCVTIMEO, &timeout, sizeof(u_long));

		/*
		* Call RS232 transmit routine. On return we will be
		* disconnected again.
		*/
		buff = malloc(ETH_BUFFERSIZE_INREF);
		while (haveConnectReflect) 
		{
			if ((cnt = NutTcpReceive(sockReflect, buff, ETH_BUFFERSIZE_INREF)) <= 0)
			{
				if ((err = NutTcpError(sockReflect)))
				{
					printf("Have eth0 NutTcpError(sockReflect) : %d\n\r", err);
					break;
				}
			}
			//printf("Have eth0 Data : count = %d data[0] = %X\n\r", cnt, buff[0]);
			if (haveConnectReflect && (cnt == 0x06))
			{
				if ((buff[0] == 0x11) && (buff[1] == 0x12) && (buff[2] == 0x13) &&
					(buff[3] == 0x14) && (buff[4] == 0x15) && (buff[5] == 0x11))
				{
					printf("Have UP\n\r");
					buff[0] = ETH_REFLECT_VERSION;
					NutTcpSend(sockReflect, buff, 1);
					continue;
				}
				else if ((buff[0] == 0x10) && (buff[1] == 0x09) && (buff[2] == 0x08) &&
					(buff[3] == 0x07) && (buff[4] == 0x06) && (buff[5] == 0x10))
				{
					printf("Have DOWN\n\r");
					otdr_reset();
					SPI_write_byte(0x55);
					buff[0] = ETH_REFLECT_VERSION;
					NutTcpSend(sockReflect, buff, 1);
					continue;
				}
				else if ((buff[0] == 0x19) && (buff[1] == 0x18) && (buff[2] == 0x17) &&
					(buff[3] == 0x16) && (buff[4] == 0x15) && (buff[5] == 0x19))
				{
					printf("Have Meas Command\n\r");
					disable_int_key();
					disable_timer1();
                    display_show(0x02, otau_get_channel(), 0x00, 0x00, 0x00);
					buff[0] = ETH_REFLECT_VERSION;
					NutTcpSend(sockReflect, buff, 1);
					continue;
				}
				else if ((buff[0] == 0x05) && (buff[1] == 0x06) && (buff[2] == 0x07) &&
					(buff[3] == 0x08) && (buff[4] == 0x09) && (buff[5] == 0x05))
				{
					printf("Have Meas Done Command\n\r");
					if (pc_loaded)
					{
						display_show(1, 0x00, 0x00, 0x00, get_rtu_number());
					}
					else if (con_otdr)
					{		
						display_show(0x08, 0x00, 0x00, 0x00, get_rtu_number());
					}
					else if (win_loaded)
					{
						display_show(0x05, 0x00, 0x00, 0x00, get_rtu_number());
					}
					else
					{
						display_show(0x00, 0x00, 0x00, 0x00, get_rtu_number());
						enable_timer1();
					}
					buff[0] = ETH_REFLECT_VERSION;
					NutTcpSend(sockReflect, buff, 1);
					enable_int_key();
					continue;
				}
				// DTR_ENABLE
				if ((buff[0] == 0x21) && (buff[1] == 0x22) && (buff[2] == 0x23) &&
					(buff[3] == 0x24) && (buff[4] == 0x25) && (buff[5] == 0x21))
				{
					buff[0] = ETH_REFLECT_VERSION;
					NutTcpSend(sockReflect, buff, 1);
					continue;
				}
				// DTR_DISABLE
				else if ((buff[0] == 0x30) && (buff[1] == 0x29) && (buff[2] == 0x28) &&
					(buff[3] == 0x27) && (buff[4] == 0x26) && (buff[5] == 0x30))
				{
					buff[0] = ETH_REFLECT_VERSION;
					NutTcpSend(sockReflect, buff, 1);
					continue;
				}
				// TEST
				else if ((buff[0] == 0x40) && (buff[1] == 0x39) && (buff[2] == 0x38) &&
					(buff[3] == 0x37) && (buff[4] == 0x36) && (buff[5] == 0x40))
				{
					printf("Have Test command\n\r");
					for (i=0; i < 217; i++)
					{
						NutTcpSend(sockReflect, buff, ETH_BUFFERSIZE_INREF);
					//	printf("Have send: %d packet\n\r", i);						
					}	
					NutTcpSend(sockReflect, buff, 3);
					printf("Have Send test %d byte answare\n\r", ETH_BUFFERSIZE_INREF*217);
					continue;
				}
			}
			if (haveConnectReflect)
				SPI_write_buffer((u_char *)buff, cnt);
		}
		haveConnectReflect = 0;
		free(buff);

		// закрываем сокет
		NutTcpCloseSocket(sockReflect);
      	printf("-- Client disconnected (reflect)\r\n");
		if (!otdr_rs232_manual)
		{
			if (pc_loaded)
			display_show(0x01, 0x00, 0x00, 0x00, get_rtu_number());
			else if (con_otdr)
			{		
				display_show(0x08, 0x00, 0x00, 0x00, get_rtu_number());
			}
			else if (win_loaded)
			{
				display_show(0x05, 0x00, 0x00, 0x00, get_rtu_number());
			}
		}
		if (bad_work_enable)
			while (1);
    }
}

THREAD(UdpFind, arg)
{
    UDPSOCKET *sock;		// сокет
	unsigned char *buff;
	unsigned char *tmp;
	u_short size, i;
	u_long addr;
	uint16_t port;
	u_char haveVersion, haveLongNumber;

	sock = NutUdpCreateSocket(UDP_PORT);
	buff = malloc(ETH_UDP_BUFFERSIZE);
	while (1)
	{
		size = NutUdpReceiveFrom(sock, &addr, &port, buff, ETH_UDP_BUFFERSIZE, UDP_TIMEOUT);
		if (size > 0)
		{
			haveVersion = 0x00;
			haveLongNumber = 0x00;
			buff[size] = 0;
			if (strncmp((char *)buff, UDP_FIND_STRING, strlen(UDP_FIND_STRING)) == 0x00)
			{
				printf("-- Have UDP-Find packet\n\r");
				if (strncmp((char *)buff, UDP_FIND_VERSION_STRING, strlen(UDP_FIND_VERSION_STRING)) == 0x00)
					haveVersion = 0x01;
				if (strncmp((char *)buff, UDP_FIND_NUMBER_STRING, strlen(UDP_FIND_NUMBER_STRING)) == 0x00)
					haveLongNumber = 0x01;
				strcpy((char *)buff, UDP_ANSWER_STRING);
				tmp = buff + strlen(UDP_ANSWER_STRING);
				for (i = 0; i < 6; i++, tmp++)
					*tmp = confnet.cdn_mac[i];
				*((u_long*)(tmp)) = confnet.cdn_ip_addr;
				*((u_long*)(tmp + sizeof(u_long))) = confnet.cdn_ip_mask;
				*((u_long*)(tmp + 2 * sizeof(u_long))) = confnet.cdn_gateway;
				addr = get_rtu_number();
				if (!haveLongNumber) 
				{	
					*((u_int*)(tmp + 3 * sizeof(u_long))) = 0;
				} else {	
					*((u_int*)(tmp + 3 * sizeof(u_long))) = 0xFFFF & addr;
				}	
				*((u_int*)(tmp + 3 * sizeof(u_long) + sizeof(u_int))) = (u_int)PROGRAMM_VERSION;

				if (!haveVersion)
				{
					NutUdpSendTo(sock, 0xFFFFFFFF, port, buff, strlen(UDP_ANSWER_STRING) + 
						6 * sizeof(u_char) + 3 * sizeof(u_long) + 2 * sizeof(u_int));
				}
				else
				{
					for (i = 0; i < 6; i++)
						*((u_char*)(tmp + 3 * sizeof(u_long) + 2 * sizeof(u_int) + i * sizeof(u_char))) = confnet.cdn_mac[i];

					*((u_long*)(tmp + 3 * sizeof(u_long) + 2 * sizeof(u_int) + 6 * sizeof(u_char))) = (u_long)NutVersion();
					*((u_long*)(tmp + 4 * sizeof(u_long) + 2 * sizeof(u_int) + 6 * sizeof(u_char))) = (u_long)__AVR_LIBC_VERSION__;
					*((u_long*)(tmp + 5 * sizeof(u_long) + 2 * sizeof(u_int) + 6 * sizeof(u_char))) = (u_long)__AVR_LIBC_DATE_;
					if (haveLongNumber)
					{
						*((u_char*)(tmp + 6 * sizeof(u_long) + 2 * sizeof(u_int) + 6 * sizeof(u_char))) = (u_char)((addr & 0xFF0000) >> 16);
						NutUdpSendTo(sock, 0xFFFFFFFF, port, buff, strlen(UDP_ANSWER_STRING) + 
						12 * sizeof(u_char) + 6 * sizeof(u_long) + 2 * sizeof(u_int) + sizeof(u_char));
					} else {
						NutUdpSendTo(sock, 0xFFFFFFFF, port, buff, strlen(UDP_ANSWER_STRING) + 
						12 * sizeof(u_char) + 6 * sizeof(u_long) + 2 * sizeof(u_int));
					}
				}
			}
			else if (strncmp((char *)buff, UDP_SET_STRING, strlen(UDP_SET_STRING)) == 0x00)
			{
				tmp = buff + strlen(UDP_SET_STRING);
				for (i = 0; i < 6; i++, tmp++)
					if (*tmp != confnet.cdn_mac[i])
						break;
				if (i == 6)
				{
					printf("-- Have UDP-Set packet\n\r");
					confnet.cdn_cip_addr = *((u_long*)(tmp));
					confnet.cdn_ip_addr = *((u_long*)(tmp));
					confnet.cdn_ip_mask = *((u_long*)(tmp + sizeof(u_long)));
					confnet.cdn_gateway = *((u_long*)(tmp + 2 * sizeof(u_long)));
					addr = *((u_int*)(tmp + 3 * sizeof(u_long)));
					tmp += (3 * sizeof(u_long) + 2 * sizeof(u_int));
					for (i = 0; i < 6; i++)
						confnet.cdn_mac[i] = tmp[i];
					tmp += (6 * sizeof(u_char) + 3 * sizeof(u_long));
					addr += *((u_char*)(tmp)) * 65536;
					//printf("--- new nnumber %lu\n\r", addr);
					set_rtu_number(addr);
					NutNetSaveConfig();
					PrintNetworkParam();
				}
			}
			else if (strncmp((char *)buff, UDP_RESET_STRING, strlen(UDP_RESET_STRING)) == 0x00)
			{
				tmp = buff + strlen(UDP_SET_STRING);
				for (i = 0; i < 6; i++, tmp++)
					if (*tmp != confnet.cdn_mac[i])
						break;
				charon_reset();
			}
		}
	}
	free(buff);
	NutUdpDestroySocket(sock);	
}
