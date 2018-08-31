#ifndef ___SERVICES___H___
#define ___SERVICES___H___

#include <sys/thread.h>		// for THREAD-type

#define PC_POWER_LED_DDR			DDRD
#define PC_POWER_LED_OUT			PORTD
#define PC_POWER_LED_IN				PIND
#define PC_POWER_LED				4

#define PC_POWER_SW_DDR				DDRD
#define PC_POWER_SW_OUT				PORTD
#define PC_POWER_SW_IN				PIND
#define PC_POWER_SW					1

#define KEY_DDR						DDRE
#define KEY_OUT						PORTE
#define KEY_IN						PINE
#define KEY							4

#define OTDR_DATA_RDY_DDR			DDRD
#define OTDR_DATA_RDY_OUT			PORTD
#define OTDR_DATA_RDY_IN			PIND
#define OTDR_DATA_RDY				0

#define OTDR_RESET_DDR				DDRD
#define OTDR_RESET_OUT				PORTD
#define OTDR_RESET_IN				PIND
#define OTDR_RESET					5

#define AC_POWER_DDR				DDRB
#define	AC_POWER_OUT				PORTB
#define AC_POWER_IN					PINB
#define	AC_POWER					6

#define II_POWER_DDR				DDRB
#define	II_POWER_OUT				PORTB
#define II_POWER_IN					PINB
#define	II_POWER					7

#define NOT_LOAD_DDR				DDRD
#define NOT_LOAD_OUT				PORTD
#define NOT_LOAD_IN					PIND
#define NOT_LOAD					6

#define OTDR_CHECK_COUNT			10000

#define MAX_OTAU_CHANNELS			12

#define	EEPROM_RTU_NUMBER_ADDRESS	0x106
#define EEPROM_START_TIMER_MIN		0x103
#define EEPROM_START_TIMER_SEC		0x104
#define EEPROM_SWITCH_STATE			0x105

#define EEPROM_INI					0x109		//480b
#define EEPROM_INI_LEN				480

#define NEW_PROGRAM_FLASH_ADDRESS	0x10000
#define MAX_NEW_PROGRAM_SIZE		0xF000

#define BOOT_ADDRESS				0x1F000
#define MAX_BOOT_SIZE				0x1000

#define OTAU_CAN_BLIND				0x80

extern u_char otdr_rs232_manual;

extern u_char loading_min;
extern u_char loading_sec;

extern u_char win_loaded;
extern u_char con_otdr;

extern u_char power_led_flag;
extern u_char power_sw_flag;

//*******************
// управление кнопкой 
//*******************

void init_key(void);

void enable_int_key(void);

void disable_int_key(void);

//*****************
// управление RESET
//*****************

void init_power_pc(void);

void enable_int_power_sw(void);

void disable_int_power_sw(void);

void reset_ext(void);

u_long get_rtu_number(void);

void set_rtu_number(u_long number);

u_char get_start_time_min(void);

void set_start_time_min(u_char min);

u_char get_start_time_sec(void);

void set_start_time_sec(u_char sec);

u_char get_power_sw(void);

void set_power_sw(u_char sw_state);

//*******************
// INI
//*******************

void read_ini(u_char *buf);

void write_ini(u_char *buf);

//*******************
// управление AC и II
//*******************
void ac_ii_init(void);

u_char set_power_ac(int bt);

u_char set_power_ii(int bt);

//****************
// управление OTAU
//****************

void otau_init(void);

u_char otau_reset(void);

u_char otau_set_channel(u_char channel);

u_char otau_get_channel(void);

u_char otau_get_count_channels(void);

u_char otau_check_alive(void);

//****************
// управление OTDR
//****************

void otdr_init(void);

void otdr_reset(void);

void init_timer1(void);

void enable_timer1(void);

void disable_timer1(void);

void stop_timer1(void);

void charon_reset(void);

void flash_boot(u_char *buff);

THREAD(OTAU_Thread, arg);

THREAD(WIN_Load_Thread, arg);

THREAD(Power_Led_Thread, arg);

THREAD(Power_Sw_Thread, arg);

u_char boot(u_char *buff) __attribute__ ((section (".bootloader")));

#endif	// ___SERVICES___H___
