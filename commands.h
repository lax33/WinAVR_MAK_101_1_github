#ifndef ___COMMANDS___H___
#define ___COMMANDS___H___

#define	CMD_OTAU_GET_CHANNEL		"otau_get_channel\r\n"
	// "1\r\n" .. "8\r\n"
	// ANSWER_ERROR							- BAD
#define CMD_OTAU_GET_COUNT_CHANNELS	"otau_get_count_channels\r\n"
	// "1\r\n", "2\r\n", "4\r\n", "8\r\n"
	// ANSWER_ERROR							- BAD
#define	CMD_OTAU_SET_CHANNEL		"otau_set_channel %d\r\n"	// ? = 1..12
	// ANSWER_OK							- OK
	// ANSWER_ERROR							- BAD
#define	CMD_OTAU_CHECK_ALIVE		"otau_check_alive\r\n"
	// ANSWER_OK							- OK
	// ANSWER_ERROR							- BAD
#define CMD_OTAU_RESET				"otau_reset\r\n"
	// ANSWER_OK							- OK
	// ANSWER_ERROR							- BAD
#define	CMD_PC_RESET				"pc_reset\r\n"
	// ANSWER_OK							- OK
#define	CMD_OTDR_RESET				"otdr_reset\r\n"
	// ANSWER_OK							- OK
	// ANSWER_BUSY							- BUSY
	// ANSWER_ERROR							- BAD
#define	CMD_OTDR_RESET_				"otdr_reset_\r\n"
	// ANSWER_OK							- OK
	// ANSWER_BUSY							- BUSY
	// ANSWER_ERROR							- BAD
#define	CMD_GET_TEMPERATURE			"get_temperature\r\n"
	// "+26.5\r\n"
	// "-1.0\r\n"
#define	CMD_HAVE_EXT_RS232_PLUGGED	"have_ext_rs323_plugged\r\n"
	// ANSWER_YES							- YES
	// ANSWER_NO							- NO
#define CMD_PC_LOADED				"pc_loaded\r\n"
	// ANSWER_OK							- OK
#define CMD_MEAS					"meas\r\n"
	// ANSWER_OK							- OK
#define CMD_STOP_MEAS				"stop_meas\r\n"
	// ANSWER_OK							- OK
#define CMD_ALARM					"alarm %d %s\r\n"
	// ANSWER_OK							- OK
#define CMD_ALARM_CRITICAL			"alarm_critical %d %s\r\n"
	// ANSWER_OK							- OK
#define CMD_GET_RTU_NUMBER			"get_rtu_number\r\n"
	// "123\r\n"
	// "6\r\n"
#define CMD_FREE_OTDR_PORT			"free_otdr_port\r\n"
	// ANSWER_OK							- OK
	// ANSWER_BUSY							- BUSY
#define CMD_BAD_WORK_ENABLE			"bad_work_enable\r\n"
	// ANSWER_OK							- OK
	// ANSWER_BUSY							- BUSY
#define CMD_CONTROL_BREAK			"control_break\r\n"
	// ANSWER_OK							- OK
	// ANSWER_BUSY							- BUSY
#define CMD_CHARON_RESET			"charon_reset\r\n"
	// ANSWER_OK							- OK
	
#define	CMD_II_SET_POWER		"ii_set_power %d\r\n"	// ? = 0..1
	// ANSWER_OK							- OK
	// ANSWER_ERROR							- BAD
	
#define	CMD_AC_SET_POWER		"ac_set_power %d\r\n"	// ? = 0..1
	// ANSWER_OK							- OK
	// ANSWER_ERROR							- BAD
	
#define CMD_INI_WR				"ini_write\r\n"			//write ini offset 0 length 480 
//		>- OK <- data(480) >- OK
	// ANSWER_OK							- OK
	// ANSWER_ERROR							- BAD
	
#define CMD_INI_RD				"ini_read\r\n"			//write ini offset 0 length 480
	// ANSWER_ERROR							- BAD
	

#define CMD_START_FLASH_PROCEDURE	"\x13\x13\x13\x13\x13\x13"

//#define CMD_START_FLASH_BOOT		"\x13\x13\x13\x13\x13\x14"

#define CMD_BEGIN_FLASH_DATA		"\x13\x14\x15\x16\x17\x18"
#define CMD_BEGIN_VERIFY_FLASH		"\x13\x14\x15\x16\x17\x19"
#define CMD_START_FLASH				"\x13\x14\x15\x16\x17\x20"

#define ANSWER_OK					"OK\r\n"
#define ANSWER_BUSY					"BUSY\r\n"
#define ANSWER_YES					"YES\r\n"
#define ANSWER_NO					"NO\r\n"
#define ANSWER_ERROR				"ERROR\r\n"
#define ANSWER_ERROR_COMMAND		"ERROR_COMMAND\r\n"

#endif	// ___COMMANDS___H___
