#include "clock.h"

#include <dev/board.h>
#include <util/delay.h>
#include <string.h>			// for strlen
#include <avr/eeprom.h>		// foor EEPROM-functions
#include <avr/boot.h>		// for BOOT-functions
#include <avr/wdt.h>		// for WDT-timer
#include <sys/event.h>
#include <sys/timer.h>

#include "services.h"
#include "uart.h"
#include "spi.h"
#include "network.h"
#include "text_lcd.h"
#include "commands.h"

u_char otdr_rs232_manual = 0x00;

u_char enable_timer1_flag = 0x00;
u_char loading_min;
u_char loading_sec;

u_char otau_count_channels = 0x01;

u_char win_loaded = 0x00;
u_char con_otdr = 0x00;

u_char power_led_flag = 0x00;
u_char power_sw_flag = 0x00;

uint32_t new_tik, old_tik = 0; 

static HANDLE otau_event;
static HANDLE power_sw_event;

//*******************
// управление кнопкой 
//*******************

void init_key(void)
{
	cbi(KEY_DDR, KEY);
	sbi(KEY_OUT, KEY);

	EICRB |= (1 << ISC41);
	EICRB &= ~(1 << ISC40);

	enable_int_key();
}

void enable_int_key(void)
{
	EIMSK |= (1 << INT4);
}

void disable_int_key(void)
{
	EIMSK &= (~(1 << INT4));
}

//*****************
// управление RESET
//*****************

void init_power_pc(void)
{
	cbi(PC_POWER_LED_DDR, PC_POWER_LED);
	sbi(PC_POWER_LED_OUT, PC_POWER_LED);

	cbi(PC_POWER_SW_DDR, PC_POWER_SW);
	sbi(PC_POWER_SW_OUT, PC_POWER_SW);

	EICRB |= (1 << ISC11);
	EICRB &= ~(1 << ISC10);

	enable_int_power_sw();
}

void enable_int_power_sw(void)
{
	EIMSK |= (1 << INT1);
}

void disable_int_power_sw(void)
{
	EIMSK &= (~(1 << INT1));
}

void reset_ext(void)
{
	u_int i;

	disable_int_power_sw();
	sbi(PC_POWER_SW_DDR, PC_POWER_SW);

	cbi(PC_POWER_SW_OUT, PC_POWER_SW);
	for (i = 0; i < 307; i++)
		_delay_loop_2(60000);	// delay 5s
	sbi(PC_POWER_SW_OUT, PC_POWER_SW);

	for (i = 0; i < 61; i++)
		_delay_loop_2(60000);	// delay 5s

	cbi(PC_POWER_SW_OUT, PC_POWER_SW);
	for (i = 0; i < 31; i++)
		_delay_loop_2(60000);	// delay 5s
	sbi(PC_POWER_SW_OUT, PC_POWER_SW);

	cbi(PC_POWER_SW_DDR, PC_POWER_SW);
	enable_int_power_sw();
}

u_long get_rtu_number(void)
{
	return eeprom_read_dword((u_long *)EEPROM_RTU_NUMBER_ADDRESS) & 0xFFFFFF;
}

void set_rtu_number(u_long number)
{
	eeprom_write_word((u_int *)EEPROM_RTU_NUMBER_ADDRESS, number & 0xFFFF);
	eeprom_write_byte((u_char *)EEPROM_RTU_NUMBER_ADDRESS + 2, (number & 0xFF0000) >> 16); 
}


u_char get_start_time_min(void)
{
    //return eeprom_read_byte((u_char *)EEPROM_START_TIMER_MIN);
	return 13;
}

void set_start_time_min(u_char min)
{
	//return eeprom_write_byte((u_char *)EEPROM_START_TIMER_MIN, min);
}

u_char get_start_time_sec(void)
{
    //return eeprom_read_byte((u_char *)EEPROM_START_TIMER_SEC);
	return 13;
}

void set_start_time_sec(u_char sec)
{
	//return eeprom_write_byte((u_char *)EEPROM_START_TIMER_SEC, sec);
}

u_char get_power_sw(void)
{
	return eeprom_read_byte((u_char *)EEPROM_SWITCH_STATE);
}

void set_power_sw(u_char sw_state)
{
	return eeprom_write_byte((u_char *)EEPROM_SWITCH_STATE, sw_state);
}

//*******************
// INI
//*******************

void read_ini(u_char *buf)
{
	eeprom_read_block(buf, (u_char *)EEPROM_INI, EEPROM_INI_LEN);
}

void write_ini(u_char *buf)
{
	eeprom_write_block(buf, (u_char *)EEPROM_INI, EEPROM_INI_LEN);
}


//*******************
// управление AC и II
//*******************

void ac_ii_init(void)
{
	sbi(AC_POWER_DDR, AC_POWER);
	sbi(II_POWER_DDR, II_POWER); 
	cbi(AC_POWER_OUT, AC_POWER);
	cbi(II_POWER_OUT, II_POWER);
}

u_char set_power_ac(int bt)
{
	if (bt == 0) {
		cbi(AC_POWER_OUT, AC_POWER);
		return 1;
	}
	if (bt == 1) {
		sbi(AC_POWER_OUT, AC_POWER);
		return 1;
	}
	return 0;
}

u_char set_power_ii(int bt)
{
	if (bt == 0) {
		cbi(II_POWER_OUT, II_POWER);
		return 1;
	}
	if (bt == 1) {
		sbi(II_POWER_OUT, II_POWER);
		return 1;
	}
	return 0;
}

//****************
// управление OTAU
//****************

void otau_init(void)
{
	int cnt = 0;	
	char comm[] = "<INFO_?>";
	char str[69];
	char cnt_channel_str[2];
	char otau_type[7];
	char orig_type[] = "HC-MEMS";
	char *pnt = str + 12; // указатель на число каналов в ответе otau
	
	// ответ otau
	// <HC-MEMS-1ЎБ4-S-162-M3-9-90-10-FA_VER3.11_SN20240507009_ 01.08.0488>
	// <HC-MEMS-1ЎБ28-S-162-M3-9-90-10-FA_VER3.11_SN20240507001_ 01.08.2416>
	//            12,13 count channels
		
	otau_reset();
	
	NutSleep(500);	
	
	fwrite(comm, 1, 8, uartFile1);
	
	NutSleep(500);
	
	cnt = fread(str, 1, 69, uartFile1);
			
	//printf("--- _otau_answer_cnt-:%d\r\n", cnt);                // debug
	//printf("--- _otau_answer_cnt_channel_str-:%s\r\n", str);    // debug 	
	
	if (!cnt)
	{
		printf("--- otau_initialization_ERR\r\n");
		otau_count_channels = 0x01;
		return;
	}	
	
	strncpy(otau_type, str + 1, 7);	
	
	if (0 != my_strnicmp(otau_type, orig_type, 7))
	{
		printf("--- 1_otau_initialization_ERR\r\n");
		otau_count_channels = 0x01;
		return;
	}
	
	strncpy(cnt_channel_str, pnt, 2);	
	
	//printf("--- _otau_cnt_channel_str-:%s\r\n", cnt_channel_str);    // debug
		
	sscanf(cnt_channel_str, "%hhu", &otau_count_channels);
	
	otau_set_channel(0);
	
	NutSleep(500);
	
	//printf("--- otau_initialization ok %c\r\n", otau_count_channels);	   // debug		
}

u_char otau_reset(void)
{
	int result;
	char str[22];
	char str_answer[8];
	char comm[] = "<RESET>";
	char answ[] = "<RESET_OK>";	
	
	//printf("--- _COMMAND - %s\r\n", comm);	// debug
	
	fwrite(comm, 1, 7, uartFile1);	
	
	NutSleep(500);
	
	int cnt = fread(str, 1, 22, uartFile1);
	
	if (!cnt)
	{
		printf("--- _reset_ERR\r\n");
		return 0x00;
	}	
	
	//printf("--- _OTAU_ANSWER_RESET_CNT - %d\r\n", cnt);		// debug
	//printf("--- _OTAU_ANSWER_RESET_STR -:%s\r\n", str);		// debug
	
	strncpy(str_answer, str, 10); 
		
		result = my_strnicmp(str_answer, answ, 10);
		//printf("--- result - %d\r\n", result);				// debug
		//printf("--- str_answer - %s\r\n", str_answer);		// debug
		
	
	if(0 == my_strnicmp(str_answer, answ, 10))
	{
		//printf("--- 1_reset_OK\r\n");							// debug
		return 0x01;
	}
	
	printf("--- 2_reset_ERR\r\n");
	return 0x00;
}

u_char otau_get_count_channels(void)
{
	return otau_count_channels;
}

u_char otau_set_channel(u_char channel)
{
	char comm[15];
	char str[18];
	char channel_str[2];
	char *pnt = str + 15;
	char answer_set_chan_OK[] = "OK";
	
	if (channel <= otau_get_count_channels())
	{
	
		if (channel >= 0 && channel <= 9)
		{
			sprintf(comm, "<OSW_01_SW_00%d>", channel);
		}
		else
		{
			sprintf(comm, "<OSW_01_SW_0%d>", channel);
		}
		
	// <OSW_01_SW_xxx> - команда уст. порта 15- байт
	// <OSW_01_SW_002_OK> - ответ otau 18 -байт
	//                14,15 - signs (OK) 
	
		fwrite(comm, 1, 15, uartFile1);
	
		NutSleep(500);
	
		int cnt = fread(str, 1, 18, uartFile1);
		
		if (!cnt)
		{
			printf("--- set_channel_ERR\r\n");
			return 0x00;
		}	
		
		//printf("--- _OTAU_SET_CHANNEL_CNT - %d\r\n", cnt);		// debug
		//printf("--- _OTAU_SET_CHANNEL_STR -:%s\r\n", str);		// debug
	
		strncpy(channel_str, pnt, 2);
	
		//printf("--- _SET_CHANNEL_ANSWER_%s\r\n", channel_str);	// debug
	
		if (0 == my_strnicmp(channel_str, answer_set_chan_OK, 2))
		{
			//printf("--- set_channel_OK\r\n");						// debug
			return 0x01;
		}
			printf("--- 1_set_channel_ERR\r\n");
			return 0x00;	
	
	}
	
	printf("--- 2_set_channel_ERR\r\n");
	return 0x00;	
}

u_char otau_get_channel(void)
{
	char str[11];
	char comm[] = "<OSW_A_?>";	
	char channel_str[3];
	int channel;
	char *pnt = str + 7;
	int cnt = 0;
	// <OSW_A_001> - otau's answer
	//        -3- signs of the number channel

	if (otau_get_count_channels() == 1)
		{
			return 0x01;
		}
		else
		{
			//printf("--- _COMMAND - %s\r\n", comm);					// debug	
	
			fwrite(comm, 1, 9, uartFile1);
	
			NutSleep(500);
	
			cnt = fread(str, 1, 11, uartFile1);

			//printf("--- _OTAU_ANSWER_CHANNAL_CNT - %d\r\n", cnt);		// debug
			//printf("--- _OTAU_ANSWER_CHANNAL_STR -:%s\r\n", str);		// debug

			strncpy(channel_str, pnt, 3);
	
				if (3 && sscanf(channel_str, "%d", &channel))
				{
					return channel;
				}
			
			printf("--- get_channel_ERR\r\n");
			return 0xFF;
		}	
}

u_char otau_check_alive(void)
{
	if (otau_get_channel())
		return 0x01;
	else
		return 0x00;
}

int my_strnicmp(const char *s1, const char *s2, size_t n)  // сравнение двух строк, n знаков
{
	if (n == 0) return 0;

	do {
	if (tolower((unsigned char)*s1) != tolower((unsigned char)*s2++))
	return tolower((unsigned char)*s1) - tolower((unsigned char)*--s2);
	if (*s1++ == 0)
	break;
	} while (--n != 0);

	return 0;
}

//****************
// управление OTDR
//****************

void otdr_init(void)
{
	SPI_init();

	cbi(OTDR_DATA_RDY_DDR, OTDR_DATA_RDY);
	sbi(OTDR_DATA_RDY_OUT, OTDR_DATA_RDY);

	cbi(OTDR_RESET_OUT, OTDR_RESET);
	sbi(OTDR_RESET_DDR, OTDR_RESET);
}

void otdr_reset(void)
{
    u_char i;

	sbi(OTDR_RESET_OUT, OTDR_RESET);
    _delay_loop_2(3686);		// 3686 * 4 cycles = 1ms
	cbi(OTDR_RESET_OUT, OTDR_RESET);
    for (i = 0; i < 74; i++)
		_delay_loop_2(50000);	// 50000 * 4 * 74 = 1s
}

void init_timer1(void)
{
    TCNT1H = 0x1F;
    TCNT1L = 0x02;
    TCCR1B = (1 << CS12);	// 1 time in 1 sec
	TIMSK |= (1 << TOIE1);
}

void enable_timer1(void)
{
	enable_timer1_flag = 0x01;
}

void disable_timer1(void)
{
	enable_timer1_flag = 0x00;
}

void stop_timer1(void)
{
	TIMSK &= ~(1 << TOIE1);
    TCCR1B = 0;	// stop
}

void charon_reset(void)
{
	wdt_enable(WDTO_15MS);
	while(1);	
}

SIGNAL(SIG_OVERFLOW1)
{
    TCNT1H = 0x1F;
    TCNT1L = 0x02;
	loading_min = get_start_time_min();
	loading_sec = get_start_time_sec() + 1;
	if (loading_sec >= 60)
	{
		loading_sec = 0;
		loading_min++;
	}
    if (loading_min >= 60)        
        loading_min = 0;
	set_start_time_min(loading_min);
	set_start_time_sec(loading_sec);
	if (enable_timer1_flag)
		display_show(0x00, 0x00, 0x00, 0x00, get_rtu_number());
}

SIGNAL(SIG_INTERRUPT4)
{
	NutEventPostFromIrq(&otau_event);
	EIFR |= (1 << INTF4);
}

THREAD(OTAU_Thread, arg)
{
	unsigned int pressed;

	while (1)
	{
		NutEventWaitNext(&otau_event, NUT_WAIT_INFINITE);
		pressed = 0x00;
		if (otdr_rs232_manual)
		{
			while (!(KEY_IN & (1 << KEY)) && (pressed != 10000))
			{
				pressed++;
				_delay_loop_2(369);	// 100uS
			}
			if (pressed == 10000)
			{
				otdr_rs232_manual = 0x00;
				if (pc_loaded)
				{
					display_show(0x01, 0x00, otau_get_channel(), 0x00, get_rtu_number());
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
			else if (pressed > 50)
			{
				if (otau_get_channel() >= ((~OTAU_CAN_BLIND) & otau_get_count_channels()))
				{
					if (OTAU_CAN_BLIND & otau_get_count_channels())
						otau_set_channel(0);
					else
						otau_set_channel(1);
				}
				else
				{
					otau_set_channel(otau_get_channel() + 1);
				}
				display_show(0x04, 0x00, otau_get_channel(), 0x00, get_rtu_number());
			}
		}
		else
		{
			while (!(KEY_IN & (1 << KEY)) && (pressed != 10000))
			{
				pressed++;
				_delay_loop_2(369);	// 100uS
			}
			if (pressed == 10000)
			{
				disable_timer1();
				otdr_rs232_manual = 0x01;
				display_show(0x04, 0x00, otau_get_channel(), 0x00, get_rtu_number());
			}
		}
		while (!(KEY_IN & (1 << KEY)));
	}
}

THREAD(WIN_Load_Thread, arg)
{
	u_char ch;

    while (1)
	{
		scanf("%c", &ch);
		if (ch == 0x13)
		{
			printf("%c", 0x0D);
			win_loaded = 0x01;
			if (!pc_loaded)
			{
				disable_timer1();
				stop_timer1();
				set_start_time_min(0);
				set_start_time_sec(0);
			}
			if (!otdr_rs232_manual)
			{
				if (pc_loaded)
					display_show(0x01, 0x00, 0x00, 0x00, get_rtu_number());
				else
					display_show(0x05, 0x00, 0x00, 0x00, get_rtu_number());
			}
//			NutThreadExit();
		}
		if (ch == 0x14)
		{
			printf("%c", 0x0D);
			con_otdr = 0x01;
			if (!pc_loaded)
			{
				disable_timer1();
				stop_timer1();
				set_start_time_min(0);
				set_start_time_sec(0);
			}
			if (!otdr_rs232_manual)
			{
				display_show(0x08, 0x00, 0x00, 0x00, get_rtu_number());
			}
//			NutThreadExit();
		}
		if ((ch & 0x80) == 0x80)
		{
			printf("%c", 0x0D);
			display_show(0x09, ch & 0x7F, 0x00, 0x00, 0x00);
//			NutThreadExit();
		}
        NutThreadYield();
    }
}

SIGNAL(SIG_INTERRUPT1)
{
	NutEventPostFromIrq(&power_sw_event);
	EIFR |= (1 << INTF1);
	disable_int_power_sw();
}

THREAD(Power_Led_Thread, arg)
{
	NutSleep(30000);

	while (1)
	{
	
		
		if (!(PC_POWER_LED_IN & (1 << PC_POWER_LED)))
		{
			power_led_flag = 0x01;
			disable_timer1();
			stop_timer1();
			set_start_time_min(0);
			set_start_time_sec(0);
			disable_int_key();
			display_show(0x0a, 0x00, 0x00, 0x00, 0x00);  // "СБРОС" ВМЕСТО "ПИТАНИЕ МОЖНО ОТКЛЮЧИТЬ" 
			set_power_sw(0x00);
			
		}
		else if (power_led_flag)
			charon_reset();

		NutThreadYield();
	}
}

THREAD(Power_Sw_Thread, arg)
{
	NutSleep(3000);

    while (1)
	{
        if (!NutEventWaitNext(&power_sw_event, 100))
		{
			//NutSleep(30);

			if (power_led_flag)
				charon_reset();

			if (get_power_sw() != 0x01)
			{
				set_power_sw(0x01);
				disable_timer1();
				stop_timer1();
				set_start_time_min(0);
				set_start_time_sec(0);
				disable_int_key();
				//display_show(0x06, 0x00, 0x00, 0x00, 0x00);  // ПИШЕТ "ВЫКЛ. МАК 100 ЖДИТЕ
			}

			while (!(PC_POWER_SW_IN & (1 << PC_POWER_SW)))
				NutThreadYield();

			enable_int_power_sw();
		}
		NutThreadYield();
    }
}

void flash_boot(u_char *buff)
{
	u_int	flash_size, flash_count, cnt, i;
	u_char	boot_res;
	
	cli();

	uart0_send_boot(0xF0);
	
	// CMD_BEGIN_FLASH_DATA
	uart0_rec_buff_boot(buff, 6);
	if ((buff[0] != 0x13) || (buff[1] != 0x14) || (buff[2] != 0x15) || (buff[3] != 0x16) || (buff[4] != 0x17) || (buff[5] != 0x18))
	{
		uart0_send_boot(0xF1);
		sei();
		return;
	}
	uart0_send_boot(0xF0);

	// FLASH_SIZE
	uart0_rec_buff_boot(buff, 2);
	flash_size = (((u_int)buff[0] << 8) + ((u_int)buff[1]));
	uart0_send_boot(0xF0);

	// FLASH_DATA
	flash_count = 0;
    while (flash_count < flash_size)
	{
		if (flash_size - flash_count > SPM_PAGESIZE)
			cnt = SPM_PAGESIZE;
		else
			cnt = flash_size - flash_count;

		uart0_rec_buff_boot(buff, cnt);

		for (i = cnt; i < SPM_PAGESIZE; i++)
			buff[i] = 0xFF;
        
		if (flash_count + cnt <= MAX_BOOT_SIZE)
		{
			boot_page_erase((u_long)BOOT_ADDRESS + flash_count);
			while(boot_rww_busy())
				boot_rww_enable();
			for(i = 0; i < cnt; i += 2)
				boot_page_fill(i + BOOT_ADDRESS + flash_count, (buff[i + 1] << 0x08) + buff[i]);
			boot_page_write((unsigned long)BOOT_ADDRESS + flash_count);
			while(boot_rww_busy())
				boot_rww_enable();
		}
		flash_count += cnt;
		uart0_send_boot(0x13);
	}
	uart0_send_boot(0xF0);
	
	// CMD_BEGIN_VERIFY_FLASH
	uart0_rec_buff_boot(buff, 6);
	if ((buff[0] != 0x13) || (buff[1] != 0x14) || (buff[2] != 0x15) || (buff[3] != 0x16) || (buff[4] != 0x17) || (buff[5] != 0x19))
	{
		uart0_send_boot(0xF1);
		sei();
		return;
	}
	uart0_send_boot(0xF0);

	// FLASH VERIFY
	boot_res = 0x01;
	flash_count = 0;
    while (flash_count < flash_size)
	{
		if (flash_size - flash_count > SPM_PAGESIZE)
			cnt = SPM_PAGESIZE;
		else
			cnt = flash_size - flash_count;

		uart0_rec_buff_boot(buff, cnt);

		if (flash_count + cnt <= MAX_BOOT_SIZE)
		{
			for(i = 0; i < cnt; i++)
				if (pgm_read_byte_far(i + BOOT_ADDRESS + flash_count) != buff[i])
				{
					boot_res = 0x00;
				}	
		}
		flash_count += cnt;
		uart0_send_boot(0x13);
	}
	if (boot_res)
	{
		uart0_send_boot(0xE0);
	}	
	else
	{
		uart0_send_boot(0xE1);
		flash_count = 0;
		while (flash_count < MAX_BOOT_SIZE)
		{
			for(i = 0; i < SPM_PAGESIZE; i++)
				buff[i] = pgm_read_byte_far(i + BOOT_ADDRESS + flash_count);

			uart0_send_buff_boot(buff, SPM_PAGESIZE);

			flash_count += SPM_PAGESIZE;
			uart0_send_boot(0x13);
		}

	}	
	sei();
}


u_char boot(u_char *buff)
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

	wdt_enable(WDTO_15MS);
	while(1);	
	

	return 0x01;
}
