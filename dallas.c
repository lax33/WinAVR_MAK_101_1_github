#include "clock.h"

#include <dev/board.h>
#include <util/delay.h>

#include "dallas.h"

/*******************************************************************/
unsigned char ds_reset(void)
{
	unsigned char ret_val = 0x01;

	sbi(DS_DDR, DS);				// output
	sbi(DS_OUT, DS);				// HI

	_delay_loop_1(15);				// 3 us

	sbi(DS_DDR, DS);				// output
	cbi(DS_OUT, DS);				// LO

	_delay_loop_2(2033);			// 4 cycles * 2033 => 271 ns * 2033 = 551 us

	cbi(DS_DDR, DS);				// pull-up
	sbi(DS_OUT, DS);				// HI

	_delay_loop_2(300);				// 4 cycles * 300 => 271 ns * 300 = 81 us

	if (DS_IN & (1 << DS))			// check - RESET_OK
		ret_val = 0x00;
  
	_delay_loop_2(1541);			// 4 cycles * 1541 => 271 ns * 1541 = 418 us

	if (!(DS_IN & (1 << DS)))
        ret_val = 0x00;

	return ret_val;
}

/*******************************************************************/
void ds_shout(unsigned char b)
{
	unsigned char i;

	for (i = 0; i < 8; i++)
	{
		sbi(DS_DDR, DS);			// output
		cbi(DS_OUT, DS);

		_delay_loop_1(10);			// 2 us

		if (b & (1 << i))
			sbi(DS_OUT, DS);

		_delay_loop_2(295);			// 4 cycles * 295 => 271 ns * 295 = 80 us

		sbi(DS_OUT, DS);
		_delay_loop_1(10);			// 2 us
	};
}

/*******************************************************************/
unsigned char ds_shin()
{
	unsigned char i, a;
	a = 0;

	for (i = 0; i < 8; i++)			// bit count
	{
		sbi(DS_DDR, DS);			// output
		cbi(DS_OUT, DS);

		_delay_loop_1(15);			// 3 us

		cbi(DS_DDR, DS);
		sbi(DS_OUT, DS);

		_delay_loop_1(15);			// 3 us

		a >>= 1;
		if (DS_IN & (1 << DS))
			a |= 0x80;

		_delay_loop_2(185);		// 4 cycles * 185 => 271 ns * 185 = 50 us
  }
  return a;
}

/*******************************************************************/
void ds_read_rom_command(unsigned char *buff)
{
	unsigned char i;
	ds_reset();

	ds_shout(0x33);

	for(i = 0; i < 8; i++)
		buff[i] = ds_shin();
}

/*******************************************************************/
void ds_convert_temperature_command(void)
{
	ds_reset();
	ds_shout(0xCC);
	ds_shout(0x44);
}

/*******************************************************************/
void ds_read_temperature_command(unsigned char *sign, unsigned char *d, unsigned char *f)
{
	unsigned char i, buff[9];
    
	ds_reset();
	ds_shout(0xCC);
	ds_shout(0xBE);
	for(i = 0; i < 9; i++)
		buff[i] = ds_shin();
    if ((buff[1] & 0xF8) == 0xF8)
		*sign = 0x01;
	else
		*sign = 0x00;
    *d = ((((int)buff[1] & 0x07) << 0x08) + (int)buff[0]) >> 0x04;
	if (buff[0] & 0x08)
		*f = 0x01;
	else
        *f = 0x00;
}

/*******************************************************************/
void ds_set_resolution(unsigned char *buff)
{
	ds_reset();
	ds_shout(0xCC);
	ds_shout(0x4E);
	ds_shout(buff[0]);
	ds_shout(buff[1]);
	ds_shout(buff[2]);
}
