/*************************************************************************
    > File Name: bootpack.c
    > Author: VOID_133
    > ################### 
    > Mail: ################### 
    > Created Time: Wed 10 Jun 2015 10:50:12 PM CST
 ************************************************************************/

#include "bootpack.h"

#define PORT_KEYDAT 0x0060
#define PORT_KEYSTA 0x0064
#define PORT_KEYCMD 0x0064
#define KEYSTA_SEND_NOTREADY 0x02
#define KEYCMD_WRITE_MODE 0x60
#define KEYCMD_SENDTO_MOUSE 0xd4
#define MOUSECMD_ENABLE 0xf4
#define KBC_MODE 0x47

extern struct FIFO8 keyfifo;
extern struct FIFO8 mousefifo;

struct MOUSE_DEC{
	unsigned char buf[3];
	unsigned char phase;
};

struct MOUSE_DEC mdec;

void wait_KBC_sendready(void);
void enable_mouse(struct MOUSE_DEC* mdec);
void init_keyboard(void);

void HariMain(void)
{
	char* vram;		// BYTE PTR
	unsigned char* str;
	char* mouse_cursor;
	struct BOOTINFO* binfo;		//Store the bootup information in it
	int mousex, mousey;
	unsigned char i;
	unsigned char mouse_phase,mouse_dbuf[3];
	char keybuf[32];			//键盘缓冲区
	char mousebuf[128];			//鼠标缓冲区
	int posx = 0;
	int posy = 0;
	/* 对PIC GDT IDT 进行初始化 如果不进行设置的话,不能使用中断 */
	init_gdtidt();		//初始化GDT IDT
	init_pic();			//初始化PIC
	io_sti();
	io_out8(PIC0_IMR, 0xf9);		//设置PIC 的IMR 使之接受 来自键盘的中断 IRQ1 ,默认的2号端口为开启状态使PIC0与PIC1相连
	io_out8(PIC1_IMR, 0xef);		//设置PIC 的IMR 使之接受 来自鼠标的中断 IRQ12

	init_palette();	/* setup the palette */
	binfo = (struct BOOTINFO*) 0xff0;
	//sprintf(str, "binfo -> vram = %08X\n",binfo -> vram);
	mousex = binfo -> scrnx / 2;
	mousey = binfo -> scrny / 2;
	
	init_screen(binfo -> vram, binfo -> scrnx, binfo -> scrny);
	init_mouse_cursor(mouse_cursor, COL8_008484);
	putblock8_8(binfo -> vram,binfo -> scrnx, 16, 16, mousex, mousey, mouse_cursor, 16);
	//putfonts8_asc(binfo -> vram, binfo -> scrnx, 8, 8, COL8_FFFFFF, "voidOS By VOID001");
	//putfonts8_asc(binfo -> vram, binfo -> scrnx, 8, 30, COL8_FFFFFF, str);

	fifo8_init(&keyfifo, 32, keybuf);
	fifo8_init(&mousefifo, 128, mousebuf);
	init_keyboard();
	io_cli();
	enable_mouse(&mdec);

	for(;;)
	{
		io_cli();
		if( fifo8_status(&keyfifo) == 0 && fifo8_status(&mousefifo) == 0 )
		{
			io_stihlt();
		}
		else if( fifo8_status(&keyfifo) != 0 )
		{
			i = fifo8_pop(&keyfifo);
			io_sti();
			sprintf(str, "%02X", i);
			boxfill8(binfo -> vram, binfo -> scrnx, COL8_008484, 0, 16, 15, 31);
			putfonts8_asc(binfo -> vram, binfo -> scrnx, 0, 16, COL8_FFFFFF, str);
		}
		else if( fifo8_status(&mousefifo) != 0 )
		{
			i = fifo8_pop(&mousefifo);
			io_sti();
			if( mouse_decode(&mdec, i) == 1)
			{
				sprintf(str, "%02X %02X %02X", mdec.buf[0], mdec.buf[1], mdec.buf[2]);
				boxfill8(binfo -> vram, binfo -> scrnx, COL8_008484, 32, 16, 32 + 8 * 8 -1, 31);
				putfonts8_asc(binfo -> vram, binfo -> scrnx, 32, 16, COL8_FFFFFF, str);
			}
		}
	}
}

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

void enable_mouse(struct MOUSE_DEC* mdec)
{
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);
	return ;
}

int mouse_decode(struct MOUSE_DEC* mdec, unsigned char dat)
{
	if( mdec -> phase == 0)  //鼠标的0xFA准备被接收
	{
		if( dat == 0xfa )
		{
			mdec -> phase = 1;
		}
		return 0;
	}
	else if( mdec -> phase == 1 )
	{
		mdec -> phase = 2;
		mdec -> buf[0] = dat;
		return 0;
	}
	else if( mdec -> phase == 2 )
	{
		mdec -> phase = 3;
		mdec -> buf[1] = dat;
		return 0;
	}
	else if( mdec -> phase == 3 )
	{
		mdec -> phase = 1;
		mdec -> buf[2] = dat;
		return 1;
	}
	return -1;
}
