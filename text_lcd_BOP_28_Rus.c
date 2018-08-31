#include "clock.h"

#include <dev/board.h>

#include <string.h>			// strlen-function
#include <stdio.h>			// sprintf
#include <util/delay.h>

#include "text_lcd.h"
#include "services.h"

const unsigned char font[][8] = {
	{0x1F, 0x11, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00},	// √	- 0x00	-
	{0x11, 0x11, 0x11, 0x0F, 0x01, 0x01, 0x01, 0x00},	// „	- 0x01			
	{0x0A, 0x04, 0x11, 0x13, 0x15, 0x19, 0x11, 0x00},	// …	- 0x02	
	{0x11, 0x11, 0x13, 0x15, 0x19, 0x11, 0x11, 0x00},	// »	- 0x03	-
	{0x1F, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x00},	// ѕ	- 0x04	-
	{0x11, 0x11, 0x11, 0x0A, 0x04, 0x08, 0x10, 0x00},	// ”	- 0x05	-
	{0x0F, 0x11, 0x11, 0x0F, 0x05, 0x09, 0x11, 0x00},	// я	- 0x06	-
	{0x1F, 0x11, 0x10, 0x1E, 0x11, 0x11, 0x1E, 0x00}};	// Ѕ	- 0x07

/*const unsigned char font2[][8] = {
	{0x11, 0x11, 0x11, 0x0F, 0x01, 0x01, 0x01, 0x00},	// „	- 0x00
	{0x15, 0x15, 0x15, 0x0E, 0x15, 0x15, 0x15, 0x00},	// ∆	- 0x01
	{0x12, 0x15, 0x15, 0x1D, 0x15, 0x15, 0x12, 0x00},	// ё	- 0x02
	{0x11, 0x11, 0x13, 0x15, 0x19, 0x11, 0x11, 0x00},	// »	- 0x03
	{0x1F, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x00},	// ѕ	- 0x04
	{0x11, 0x11, 0x11, 0x19, 0x15, 0x15, 0x19, 0x00},	// џ	- 0x05
	{0x0F, 0x05, 0x05, 0x09, 0x11, 0x1F, 0x11, 0x11},	// ƒ	- 0x06
	{0x0F, 0x05, 0x05, 0x05, 0x05, 0x15, 0x09, 0x00}};	// Ћ	- 0x07*/

u_char current_font = 0x00;

const unsigned char displays[][16] = {
	{0x7, 'O', 0x4, '-', '2', '8', ' ', ' ', ' ', ' ', ' ', '0', '0', 'x', 'x', 'x'},
	{' ', ' ', ' ', ' ', '3', 'A', 0x0, 'P', 0x5, '3', 'K', 'A', ' ', ' ', ' ', ' '},

	{0x7, 'O', 0x4, '-', '2', '8', ' ', ' ', ' ', ' ', ' ', '0', '0', 'x', 'x', 'x'},
	{' ', ' ', ' ', ' ', ' ', 0x0, 'O', 'T', 'O', 'B', ' ', ' ', ' ', ' ', ' ', ' '},

	{' ', ' ', ' ', 0x3, '3', 'M', 'E', 'P', 'E', 'H', 0x3, 'E', ' ', ' ', ' ', ' '},
	{' ', ' ', ' ', 0x4, 'O', 'P', 'T', ' ', '-', ' ', '0', 'x', ' ', ' ', ' ', ' '},

	{'A', 'B', 'A', 'P', 0x3, 0x6, '?', ' ', 0x4, 'O', 'P', 'T', '-', '0', 'x', ' '},
	{' ', ' ', ' ', ' ', ' ', ' ', ' ', 'x', '.', 'x', '0', '0', '0', ' ', 'K', 'M'},

	{0x7, 'O', 0x4, '-', '2', '8', ' ', ' ', ' ', ' ', ' ', '0', '0', 'x', 'x', 'x'},
	{'P', 0x5, 0x1, 'H', 'O', 0x2, ' ', 'S', 'C', '/', 'A', 'P', 'C', '-', '0', 'x'}

//	{'M', 'A', 'K', ' ', '1',  '0',  '0',  ' ', ' ', ' ', ' ', '0', '0', 'x', 'x', 'x'},		// font2
//	{' ', ' ', ' ', ' ', 0x01, 0x06, 0x03, 'T', 'E', '.', '.', '.', ' ', ' ', ' ', ' '},

//	{' ', 'B', 0x05, 'K', 0x07, '.',  ' ', 'M', 'A', 'K', ' ', '1', '0', '0', ' ', ' '},	// font2
//	{' ', ' ', ' ', ' ', 0x01, 0x06, 0x03, 'T', 'E', '.', '.', '.', ' ', ' ', ' ', ' '},

//	{' ', ' ', ' ', ' ', 0x04, 0x03, 'T', 'A', 'H', 0x03, 'E', ' ', ' ', ' ', ' ', ' '},		// font2
//	{'M', 'O', 0x01, 'H', 'O', ' ', 'O', 'T', 'K', 0x07, 0x02, 0x00, 0x03, 'T', 'b', ' '}
};

//---------------------------------------------------------
// инициализаци€ диспле€.
//---------------------------------------------------------

void display_set_font(void)
{
	u_char i, *pf = (u_char *)font;

	display_write_byte(0x40, DISPLAY_COMMAND_BYTE);
    for (i = 0; i < 64; i++)
		display_write_byte(pf[i], DISPLAY_DATA_BYTE);

	current_font = 0x01;
}


void display_init(void)
{
	LCD_DDR |= ((1 << LCD_RS) | (1 << LCD_RW) | (1 << LCD_E) | (1 << LCD_D4) | (1 << LCD_D5) | 
		(1 << LCD_D6) | (1 << LCD_D7));

	cbi(LCD_OUT, LCD_RS);	// выбор IR
	cbi(LCD_OUT, LCD_RW);	// запись
	_delay_loop_2(55294);	// 55294 * 4 cycles = 15ms
	
	sbi(LCD_OUT, LCD_E);	// запись тетрады 3h
	sbi(LCD_OUT, LCD_D4);
	sbi(LCD_OUT, LCD_D5);
	cbi(LCD_OUT, LCD_D6);
	cbi(LCD_OUT, LCD_D7);
    _delay_loop_2(1);
	cbi(LCD_OUT, LCD_E);
	_delay_loop_2(15114);	// 15114 * 4 cycles = 4100mks
    
	sbi(LCD_OUT, LCD_E);	// запись тетрады 3h
	_delay_loop_2(1);
	cbi(LCD_OUT, LCD_E);
	_delay_loop_2(369);		// 369 * 4 cycles = 100mks

	sbi(LCD_OUT, LCD_E);	// запись тетрады 3h
	_delay_loop_2(1);
	cbi(LCD_OUT, LCD_E);
	_delay_loop_2(147);		// 147 * 4 cycles = 40mks

	sbi(LCD_OUT, LCD_E);	// запись тетрады 2h
    cbi(LCD_OUT, LCD_D4);
	_delay_loop_2(1);
	cbi(LCD_OUT, LCD_E);
	_delay_loop_2(147);		// 147 * 4 cycles = 40mks
	
	// DL = 0 (4 бит данных - по тетрадам)
	// N = 1 (2 троки), F = 0 (символы - 5*8)
	// {0 0 1 DL N F - -} - байт команды
    display_write_byte(0x28, DISPLAY_COMMAND_BYTE);

	// D = 1 (включить дисплей), C = 0 (выключить курсор)
	// B = 0 (выключить мигание)
	// {0 0 0 0 1 D C B} - байт команды
	display_write_byte(0x0C, DISPLAY_COMMAND_BYTE);

	// I/D = 1 (курсор сдвигаетс€ вправо)
	// S = 0 (нет сдвига диспле€)
	// {0 0 0 0 0 1 I/D S} - байт команды
	display_write_byte(0x06, DISPLAY_COMMAND_BYTE);
	
	display_set_font();
	
}

//void display_set_font2(void)
//{
//	u_char i, *pf = (u_char *)font2;
//
//	display_write_byte(0x40, DISPLAY_COMMAND_BYTE);
//    for (i = 0; i < 64; i++)
//		display_write_byte(pf[i], DISPLAY_DATA_BYTE);
//
//	current_font = 0x02;
//}

//---------------------------------------------------------
// вывод NUM дл€ BOP на LCD в заданной позиции
//---------------------------------------------------------
void display_show_string_BOP(u_char x, u_char y, u_char *str)
{
	if (*str != 0x00)
	{
	
		display_set_cursor_pos(x + 1, y);

		while (*(str + 1) != 0x00)
		{
			display_write_byte(*str, DISPLAY_DATA_BYTE);
			str++;
		}
	}	
}

//---------------------------------------------------------
// очистка экрана.
//---------------------------------------------------------
void display_clear_screen(void)
{
	// команда очистки
	// {0 0 0 0 0 0 0 1} - байт команды
	display_write_byte(0x01, DISPLAY_COMMAND_BYTE);
}

void display_set_channel(u_int port)
{
}

void display_show(u_char index, u_int port, u_int d_len, u_int f_len, u_long num)
{
	u_char *pstr = (u_char *)displays, i, buff[9];

	/*if ((get_power_sw() == 0x01) && (index != 0x07))
		index = 0x06;

	if (index >= 0x05)
	{
		if (current_font != 0x02)
			display_set_font2();
	}
	else
	{
		if (current_font != 0x01)
			display_set_font();
	}*/
	
	pstr += (index << 5);

	display_set_cursor_pos(0, 0);
	for (i = 0; i < 16; i++)
		display_write_byte(*pstr++, DISPLAY_DATA_BYTE);
	display_set_cursor_pos(0, 1);
	for (i = 0; i < 16; i++)
		display_write_byte(*pstr++, DISPLAY_DATA_BYTE);
	switch (index)
	{
		case 0x00 :
			sprintf((char *)buff, "%08lu", num);
			display_show_string_BOP(16 - strlen((char *)buff), 0, buff);
		//	loading_min = get_start_time_min();
		//	loading_sec = get_start_time_sec();
		//	display_show_loading_time();
			break;
		case 0x01 :
			sprintf((char *)buff, "%08lu", num);
			display_show_string_BOP(16 - strlen((char *)buff), 0, buff);
			break;
		case 0x02 :
			utoa(port, (char *)buff, 10);
			display_show_string(12 - strlen((char *)buff), 1, buff);
			break;
		case 0x03 :
			utoa(port, (char *)buff, 10);
			display_show_string(15 - strlen((char *)buff), 0, buff);
			utoa(d_len, (char *)buff, 10);
			display_show_string(8 - strlen((char *)buff), 1, buff);
			utoa(f_len, (char *)buff, 10);
			display_show_string(9, 1, buff);
			break;
		case 0x04 :
			sprintf((char *)buff, "%08lu", num);
			display_show_string_BOP(16 - strlen((char *)buff), 0, buff);
			utoa(d_len, (char *)buff, 10);
			display_show_string(16 - strlen((char *)buff), 1, buff);
			break;
		case 0x05 :
		//	sprintf((char *)buff, "%07lu", num);
		//	display_show_string(16 - strlen((char *)buff), 0, buff);
			break;
		case 0x06 :
			break;
		case 0x07 :
			break;
	}
}

/*void display_show_loading_time(void)
{
    u_char buff[3];

	utoa(loading_min, (char *)buff, 10);
	display_set_cursor_pos(11, 1);
	if (loading_min < 10)
	{
		display_write_byte('0', DISPLAY_DATA_BYTE);
		display_write_byte(buff[0], DISPLAY_DATA_BYTE);
	}
	else
	{
		display_write_byte(buff[0], DISPLAY_DATA_BYTE);
		display_write_byte(buff[1], DISPLAY_DATA_BYTE);
	}

	utoa(loading_sec, (char *)buff, 10);
	display_set_cursor_pos(14, 1);
	if (loading_sec < 10)
	{
		display_write_byte('0', DISPLAY_DATA_BYTE);
		display_write_byte(buff[0], DISPLAY_DATA_BYTE);
	}
	else
	{
		display_write_byte(buff[0], DISPLAY_DATA_BYTE);
		display_write_byte(buff[1], DISPLAY_DATA_BYTE);
	}
}*/

//---------------------------------------------------------
// показывать курсор на дисплее.
//---------------------------------------------------------
void display_show_cursor(void)
{
	// D = 1 (включить дисплей), C = 1 (включить курсор)
	// B = 0 (выключить мигание)
	// {0 0 0 0 1 D C B} - байт команды
    display_write_byte(0x0E, DISPLAY_COMMAND_BYTE);
}

//---------------------------------------------------------
// спр€тать курсор на дисплее.
//---------------------------------------------------------
void display_hide_cursor(void)
{
	// D = 1 (включить дисплей), C = 1 (включить курсор)
	// B = 0 (выключить мигание)
	// {0 0 0 0 1 D C B} - байт команды
	display_write_byte(0x0C, DISPLAY_COMMAND_BYTE);
}

//---------------------------------------------------------
// установка адреса курсора LCD
//---------------------------------------------------------
void display_set_cursor_pos(u_char x, u_char y)
{
	// формирование байта команды :
	// {1 AD AD AD AD AD AD AD}
    display_write_byte(0x80 | ((y & 0x01) << 0x06) | (x & 0x0F), DISPLAY_COMMAND_BYTE);
}
	
//---------------------------------------------------------
// запись байта данных или команд
//---------------------------------------------------------
void display_write_byte(u_char byte, u_char byte_type)
{
	if (byte_type == DISPLAY_DATA_BYTE)
		sbi(LCD_OUT, LCD_RS);
	else
		cbi(LCD_OUT, LCD_RS);

	cbi(LCD_OUT, LCD_RW);	// запись
	_NOP();

	sbi(LCD_OUT, LCD_E);	// передача старшей тетрады
	cbi(LCD_OUT, LCD_D7);
	cbi(LCD_OUT, LCD_D6);
	cbi(LCD_OUT, LCD_D5);
	cbi(LCD_OUT, LCD_D4);
    if (byte & (1 << 7))
		sbi(LCD_OUT, LCD_D7);
    if (byte & (1 << 6))
		sbi(LCD_OUT, LCD_D6);
    if (byte & (1 << 5))
		sbi(LCD_OUT, LCD_D5);
    if (byte & (1 << 4))
		sbi(LCD_OUT, LCD_D4);
	_delay_loop_2(1);
	cbi(LCD_OUT, LCD_E);

	_delay_loop_2(147);		// 147 * 4 cycles = 40mks
	
	sbi(LCD_OUT, LCD_E);	// передача младшей тетрады
	cbi(LCD_OUT, LCD_D7);
	cbi(LCD_OUT, LCD_D6);
	cbi(LCD_OUT, LCD_D5);
	cbi(LCD_OUT, LCD_D4);
    if (byte & (1 << 3))
		sbi(LCD_OUT, LCD_D7);
    if (byte & (1 << 2))
		sbi(LCD_OUT, LCD_D6);
    if (byte & (1 << 1))
		sbi(LCD_OUT, LCD_D5);
    if (byte & (1 << 0))
		sbi(LCD_OUT, LCD_D4);
	_delay_loop_2(1);
	cbi(LCD_OUT, LCD_E);
	
	/// display_wait_for_work();	// ожидание готовности диспле€
    _delay_loop_2(147);		// 147 * 4 cycles = 40mks
}

//---------------------------------------------------------
// вывод строки на LCD в заданной позиции
//---------------------------------------------------------
void display_show_string(u_char x, u_char y, u_char *str)
{
	display_set_cursor_pos(x, y);

    while (*str != 0x00)
	{
		display_write_byte(*str, DISPLAY_DATA_BYTE);
		str++;
	}
}

//---------------------------------------------------------
// разрешение записи в дисплей.
//---------------------------------------------------------
void display_can_write(void)
{
	LCD_DDR |= ((1 << LCD_D7) | (1 << LCD_D6) | (1 << LCD_D5) | (1 << LCD_D4));
}

//---------------------------------------------------------
// разрешение чтени€ диспле€.
//---------------------------------------------------------
void display_can_read(void)
{
	LCD_DDR &= ~((1 << LCD_D7) | (1 << LCD_D6) | (1 << LCD_D5) | (1 << LCD_D4));
}
	
//---------------------------------------------------------
// ожидаем готовности диспле€
//---------------------------------------------------------
void display_wait_for_work(void)
{
    u_char bt;

	display_can_read();			// разрешаем чтение диспле€
	cbi(LCD_OUT, LCD_RS);		// выбор IR
	sbi(LCD_OUT, LCD_RW);		// чтение
	_NOP();

	while (1)
	{
		sbi(LCD_OUT, LCD_E);	// читаем первую тетраду в аккумул€тор
		_delay_loop_2(1);		// и заносим бит зан€тости диспле€ BF в T
		bt = LCD_IN;
		cbi(LCD_OUT, LCD_E);

		_delay_loop_2(1);

		sbi(LCD_OUT, LCD_E);	// читаем вторую тетраду, хот€ она нам и не нужна
		_delay_loop_2(1);
		cbi(LCD_OUT, LCD_E);

		if (!(bt & (1 << LCD_D7)))
			break;
	}

	display_can_write();		// разрешаем запись в дисплей
}
