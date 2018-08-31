#ifndef ___DALLAS___H___
#define ___DALLAS___H___

#define DS_DDR	DDRD
#define DS_OUT	PORTD
#define DS_IN	PIND
#define DS		7

unsigned char ds_reset(void);

void ds_shout(unsigned char b);

unsigned char ds_shin(void);

void ds_read_rom_command(unsigned char *);

void ds_convert_temperature_command(void);

void ds_read_temperature_command(unsigned char *sign, unsigned char *d, unsigned char *f);

void ds_set_resolution(unsigned char *buff);

#endif	// ___DALLAS___H___
