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
	char str_blind[] = "chb\r\n";
	char str[15] = "type?\r\n";
	int cnt, ret_val;

	NutSleep(1000);

	otau_reset();
	NutSleep(2500);

	fwrite(str_blind, 1, strlen(str_blind), uartFile1);
	_delay_loop_2(60000);
	_delay_loop_2(60000);
	fwrite(str, 1, strlen(str), uartFile1);
	_delay_loop_2(60000);
	_delay_loop_2(60000);
	cnt = fread(str, 1, 15, uartFile1);
    str[cnt] = 0x00;
	if (cnt)
	{
		sscanf(&str[6], "%d\r\n", &ret_val);
		otau_count_channels = (u_char)ret_val;
		if (str[cnt - 3] == 'b')
			otau_count_channels |= OTAU_CAN_BLIND;
	}
}

u_char otau_reset(void)
{
	fwrite("reset\r\n", 1, 7, uartFile1);
	return 0x01;
}

u_char otau_get_count_channels(void)
{
	return otau_count_channels;
}

u_char otau_set_channel(u_char channel)
{
	char str[10];

	if (channel <= ((~OTAU_CAN_BLIND) & otau_get_count_channels()))
	{
		sprintf(str, "ch%d\r\n", channel);
		fwrite(str, 1, strlen(str), uartFile1);
		//_delay_loop_2(60000);
		//_delay_loop_2(60000);
		old_tik = NutGetMillis(); 
		return 0x01;
	}
	else
		return 0x00;
}

u_char otau_get_channel(void)
{
	char str[10] = "ch?\r\n";
	int cnt, ret_val;

	if (otau_get_count_channels() == 1)
		return 0x01;
	else
	{
		new_tik = NutGetMillis() - old_tik;
		if (new_tik < 700) NutSleep(700 - new_tik); 
		fwrite(str, 1, strlen(str), uartFile1);
		_delay_loop_2(60000);
		_delay_loop_2(60000);
		cnt = fread(str, 1, 10, uartFile1);
		str[cnt] = 0x00;
		if (cnt && sscanf(str, "%d\r\n", &ret_val))
		{
			return (u_char)ret_val;
		}
		else
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
