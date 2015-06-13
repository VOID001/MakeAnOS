/*************************************************************************
    > File Name: mouse.c
    > Author: VOID_133
    > ################### 
    > Mail: ################### 
    > Created Time: Sat 13 Jun 2015 10:18:43 PM CST
 ************************************************************************/
#include "bootpack.h"

void enable_mouse(struct MOUSE_DEC* mdec)
{
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);
	return ;
}

int mouse_decode(struct MOUSE_DEC* mdec, int dat)
{
	if( mdec -> phase == 0)  //鼠标的0xFA准备被接收
	{
		if( dat == 0xfa )
		{
			mdec -> phase = 1;
		}
		return 0;
	}
	if( mdec -> phase == 1 )		//
	{
		if( (dat & 0xc8) == 0x08)		//判断第一字节是否正确被传输
		{
			mdec -> phase = 2;
			mdec -> buf[0] = dat;
		}
		return 0;
	}
	if( mdec -> phase == 2 )
	{
		mdec -> phase = 3;
		mdec -> buf[1] = dat;
		return 0;
	}
	if( mdec -> phase == 3 )
	{
		mdec -> phase = 1;
		mdec -> buf[2] = dat;
		mdec -> btn = mdec -> buf[0] & 0x07;		//低三位存储鼠标的按键信息
		mdec -> x = mdec -> buf[1];
		mdec -> y = mdec -> buf[2];

		/* 这一小段不懂 */	
		if( (mdec -> buf[0] & 0x10) != 0 )
		{
			mdec -> x |= 0xffffff00;
		}
		if( (mdec -> buf[0] & 0x20) != 0 )
		{
			mdec -> y |= 0xffffff00;
		}
		/* 这一小段不懂 */

		mdec -> y = - mdec -> y;
		return 1;
	}
	return -1;
}

struct FIFO8 mousefifo;

void inthandler2c(int *esp)
{
	unsigned char data;
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
	io_out8(PIC1_OCW2,0x64);
	io_out8(PIC0_OCW2,0x62);
	data = io_in8(PORT_KEYDAT);
	fifo8_push(&mousefifo, data);

	return ;
}
