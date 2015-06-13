/*************************************************************************
    > File Name: keyboard.c
    > Author: VOID_133
    > ################### 
    > Mail: ################### 
    > Created Time: Sat 13 Jun 2015 10:18:49 PM CST
 ************************************************************************/
#include "bootpack.h"


void wait_KBC_sendready(void)
{
	/* 等待键盘控制电路准备就绪 */
	for(;;)
	{ 
		if((io_in8(PORT_KEYSTA) & KEYSTA_SEND_NOTREADY) == 0) 
		{
			break;
		}
	}
	return ;
}

void init_keyboard(void)
{
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, KBC_MODE);
	return ;
}

#define PORT_KEYDAT		0x0060

struct FIFO8 keyfifo;

void inthandler21(int *esp)
{
	unsigned char data;
	io_out8(PIC0_OCW2, 0x61);
	data = io_in8(PORT_KEYDAT);
	fifo8_push(&keyfifo, data);
	return;
}

