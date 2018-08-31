#ifndef ___TEXT_LCD___H___
#define ___TEXT_LCD___H___

#define LCD_DDR		DDRF
#define LCD_OUT		PORTF
#define LCD_IN		PINF
#define LCD_RS		0
#define LCD_RW		4
#define LCD_E		1
#define LCD_D4		5
#define LCD_D5		2
#define LCD_D6		6
#define LCD_D7		3

#define DISPLAY_COMMAND_BYTE 0
#define DISPLAY_DATA_BYTE 1

void display_init(void);

//void display_set_font(void);

//void display_set_font2(void);

//---------------------------------------------------------
// ������� ������.
//---------------------------------------------------------
void display_clear_screen(void);

void display_show(u_char index, u_int port, u_int d_len, u_int f_len, u_long num);
void display_set_channel(u_int port);

//void display_show_loading_time(void);

//---------------------------------------------------------
// ���������� ������ �� �������.
//---------------------------------------------------------
void display_show_cursor(void);

//---------------------------------------------------------
// �������� ������ �� �������.
//---------------------------------------------------------
void display_hide_cursor(void);

//---------------------------------------------------------
// ��������� ������ ������� LCD
//---------------------------------------------------------
void display_set_cursor_pos(u_char x, u_char y);
	
//---------------------------------------------------------
// ������ ����� ������ ��� ������
//---------------------------------------------------------
void display_write_byte(u_char byte, u_char byte_type);

//---------------------------------------------------------
// ����� ������ �� LCD � �������� �������
//---------------------------------------------------------
void display_show_string(u_char x, u_char y, u_char *str);

//---------------------------------------------------------
// ���������� ������ � �������.
//---------------------------------------------------------
void display_can_write(void);

//---------------------------------------------------------
// ���������� ������ �������.
//---------------------------------------------------------
void display_can_read(void);
	
//---------------------------------------------------------
// ������� ���������� �������
//---------------------------------------------------------
void display_wait_for_work(void);

#endif	// ___TEXT_LCD___H___
