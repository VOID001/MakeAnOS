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

struct MOUSE_DEC{			//用来将鼠标传输来的数据解析出来
	unsigned char buf[3];
	unsigned char phase;
	int x, y, btn;
};


void wait_KBC_sendready(void);
void enable_mouse(struct MOUSE_DEC* mdec);
void init_keyboard(void);
int mouse_decode(struct MOUSE_DEC* mdec, int dat);
 
void HariMain(void)
{
	unsigned char* str;
	char mouse_cursor[256];
	struct BOOTINFO* binfo;		//Store the bootup information in it
	int mousex, mousey;
	struct MOUSE_DEC mdec;
	int i;
	char keybuf[32];			//键盘缓冲区
	char mousebuf[128];			//鼠标缓冲区
	/* 对PIC GDT IDT 进行初始化 如果不进行设置的话,不能使用中断 */
	init_gdtidt();		//初始化GDT IDT
	init_pic();			//初始化PIC
	io_sti();
	io_out8(PIC0_IMR, 0xf9);		//设置PIC 的IMR 使之接受 来自键盘的中断 IRQ1 ,默认的2号端口为开启状态使PIC0与PIC1相连
	io_out8(PIC1_IMR, 0xef);		//设置PIC 的IMR 使之接受 来自鼠标的中断 IRQ12

	init_palette();	/* setup the palette */
	binfo = (struct BOOTINFO*) 0xff0;
	//sprintf(str, "binfo -> vram = %08X\n",binfo -> vram);
	mousex = ( binfo -> scrnx - 16 ) / 2;
	mousey = ( binfo -> scrny - 16) / 2;
	
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
				sprintf(str, "[lcr %4d %4d]", mdec.x, mdec.y);
				if( (mdec.btn & 0x01) !=0 )
				{
					str[1] = 'L';
				}
				if( (mdec.btn & 0x02) !=0 )
				{
					str[3] = 'R';
				}
				if( (mdec.btn & 0x04) !=0 )
				{
					str[2] = 'C';
				}
				boxfill8(binfo -> vram, binfo -> scrnx, COL8_008484, 32, 16, 32 + 15 * 8 - 1, 31);
				putfonts8_asc(binfo -> vram, binfo -> scrnx, 32, 16, COL8_FFFFFF, str);
				/* 以上用来显示鼠标信息 */
				boxfill8(binfo -> vram, binfo -> scrnx, COL8_008484, mousex, mousey, mousex + 15, mousey + 15);	//清除原来鼠标指针
				mousex += mdec.x;
				mousey += mdec.y;
				if( mousex < 0 )
				{
					mousex = 0;
				}
				if (mousey < 0 )
				{
					mousey = 0;
				}
				if (mousex > binfo -> scrnx - 16)
				{
					mousex = binfo -> scrnx - 16;
				}
				if (mousey > binfo -> scrny - 16)
				{
					mousey = binfo -> scrny - 16;
				}
				sprintf(str, "(%3d, %3d)", mousex, mousey);
				boxfill8(binfo -> vram, binfo -> scrnx, COL8_008484, 0, 0, 79, 15);
				putfonts8_asc(binfo -> vram, binfo -> scrnx, 0, 0, COL8_FFFFFF, str);
				putblock8_8(binfo -> vram, binfo -> scrnx, 16, 16, mousex, mousey, mouse_cursor, 16);
			}
		}
	}
}


//void HariMain(void)
//{
//	struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
//	char s[40], mcursor[256], keybuf[32], mousebuf[128];
//	int mx, my, i;
//	struct MOUSE_DEC mdec;
//
//	init_gdtidt();
//	init_pic();
//	io_sti(); /* IDT/PICÌú»ªIíÁœÌÅCPUÌèÝÖ~ðð */
//	fifo8_init(&keyfifo, 32, keybuf);
//	fifo8_init(&mousefifo, 128, mousebuf);
//	io_out8(PIC0_IMR, 0xf9); /* PIC1ÆL[{[hðÂ(11111001) */
//	io_out8(PIC1_IMR, 0xef); /* }EXðÂ(11101111) */
//
//	init_keyboard();
//
//	init_palette();
//	init_screen(binfo->vram, binfo->scrnx, binfo->scrny);
//	mx = (binfo->scrnx - 16) / 2; /* æÊÉÈéæ€ÉÀWvZ */
//	my = (binfo->scrny - 28 - 16) / 2;
//	init_mouse_cursor(mcursor, COL8_008484);
//	putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);
//	sprintf(s, "(%3d, %3d)", mx, my);
//	putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, s);
//
//	enable_mouse(&mdec);
//
//	for (;;) {
//		io_cli();
//		if (fifo8_status(&keyfifo) + fifo8_status(&mousefifo) == 0) {
//			io_stihlt();
//		} else {
//			if (fifo8_status(&keyfifo) != 0) {
//				i = fifo8_pop(&keyfifo);
//				io_sti();
//				sprintf(s, "%02X", i);
//				boxfill8(binfo->vram, binfo->scrnx, COL8_008484,  0, 16, 15, 31);
//				putfonts8_asc(binfo->vram, binfo->scrnx, 0, 16, COL8_FFFFFF, s);
//			} else if (fifo8_status(&mousefifo) != 0) {
//				i = fifo8_pop(&mousefifo);
//				io_sti();
//				if (mouse_decode(&mdec, i) != 0) {
//					/* f[^ª3oCgµÁœÌÅ\Š */
//					sprintf(s, "[lcr %4d %4d]", mdec.x, mdec.y);
//					if ((mdec.btn & 0x01) != 0) {
//						s[1] = 'L';
//					}
//					if ((mdec.btn & 0x02) != 0) {
//						s[3] = 'R';
//					}
//					if ((mdec.btn & 0x04) != 0) {
//						s[2] = 'C';
//					}
//					boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 32, 16, 32 + 15 * 8 - 1, 31);
//					putfonts8_asc(binfo->vram, binfo->scrnx, 32, 16, COL8_FFFFFF, s);
//					/* }EXJ[\ÌÚ® */
//					boxfill8(binfo->vram, binfo->scrnx, COL8_008484, mx, my, mx + 15, my + 15); /* }EXÁ· */
//					mx += mdec.x;
//					my += mdec.y;
//					if (mx < 0) {
//						mx = 0;
//					}
//					if (my < 0) {
//						my = 0;
//					}
//					if (mx > binfo->scrnx - 16) {
//						mx = binfo->scrnx - 16;
//					}
//					if (my > binfo->scrny - 16) {
//						my = binfo->scrny - 16;
//					}
//					sprintf(s, "(%3d, %3d)", mx, my);
//					boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 0, 0, 79, 15); /* ÀWÁ· */
//					putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, s); /* ÀW­ */
//					putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16); /* }EX`­ */
//				}
//			}
//		}
//	}
//}

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
