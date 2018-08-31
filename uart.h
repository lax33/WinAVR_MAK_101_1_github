#ifndef ___UART___H___
#define ___UART___H___

#include <stdio.h>			// for FILE-type

#define	UART_BUFFERSIZE	128


extern FILE *uartFile0;
extern FILE *uartFile1;

int initUart(u_char devNum, u_long baud, u_long timeout);

void PrintNetworkParam(void);

// ������� ����� �� UART
void uart0_send(u_char byte) __attribute__ ((section (".uartloader")));

// ���� ����� �� UART
u_char uart0_rec(void) __attribute__ ((section (".uartloader")));

// ������� ������ �� UART
void uart0_send_buff(u_char *buffer, u_int size) __attribute__ ((section (".uartloader")));

// ���� ������ �� UART
void uart0_rec_buff(u_char *buffer, u_int size) __attribute__ ((section (".uartloader")));

// ������� ����� �� UART boot
void uart0_send_boot(u_char byte);

// ���� ����� �� UART boot
u_char uart0_rec_boot(void);

// ������� ������ �� UART boot
void uart0_send_buff_boot(u_char *buffer, u_int size);

// ���� ������ �� UART boot
void uart0_rec_buff_boot(u_char *buffer, u_int size);

//boot
//u_char boot(u_char *buff) __attribute__ ((section (".bootloader")));

#endif	// ___UART___H___
