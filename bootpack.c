/*************************************************************************
    > File Name: bootpack.c
    > Author: VOID_133
    > ################### 
    > Mail: ################### 
    > Created Time: Wed 10 Jun 2015 10:50:12 PM CST
 ************************************************************************/

#include "bootpack.h"
#define MEMMAN_ADDR 0x003c0000

extern struct FIFO8 keyfifo;
extern struct FIFO8 mousefifo;
void HariMain(void)
{
	unsigned char* str;
	unsigned int memtotal;
	struct MEMMAN* memman = (struct MEMMAN*) MEMMAN_ADDR;
	char mouse_cursor[256];
	struct BOOTINFO* binfo = (struct BOOTINFO *) ADR_BOOTINFO;		//Store the bootup information in it
	int mousex, mousey;
	struct MOUSE_DEC mdec;
	int i;
	char keybuf[32];			//键盘缓冲区
	char mousebuf[128];			//鼠标缓冲区
	struct SHTCTL* shtctl;		//建立图层控制单元
	struct SHEET* sht_back, *sht_mouse; //准备两个图层 用于显示背景和鼠标
	unsigned char *buf_back, buf_mouse[256];
	/* 对PIC GDT IDT 进行初始化 如果不进行设置的话,不能使用中断 */
	init_gdtidt();		//初始化GDT IDT
	init_pic();			//初始化PIC
	io_sti();
	fifo8_init(&keyfifo, 32, keybuf);
	fifo8_init(&mousefifo, 128, mousebuf);
	io_out8(PIC0_IMR, 0xf9);		//设置PIC 的IMR 使之接受 来自键盘的中断 IRQ1 ,默认的2号端口为开启状态使PIC0与PIC1相连
	io_out8(PIC1_IMR, 0xef);		//设置PIC 的IMR 使之接受 来自鼠标的中断 IRQ12
	init_keyboard();
	enable_mouse(&mdec);

	
	memtotal = memtest(0x00400000, 0xbfffffff);
	memman_init(memman);
	memman_free(memman, 0x00001000, 0x0009e000);
	memman_free(memman, 0x00400000, memtotal - 0x00400000);

	init_palette();	/* setup the palette */
	shtctl = shtctl_init(memman, binfo->vram, binfo->scrnx, binfo->scrny);
	sht_back = sheet_alloc(shtctl);
	sht_mouse = sheet_alloc(shtctl);
	buf_back =  (unsigned char*) memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);
	sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1);
	sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99);
	init_screen(buf_back, binfo -> scrnx, binfo -> scrny);
	init_mouse_cursor(buf_mouse, 99);
	sheet_slide(shtctl, sht_back, 0, 0);
	mousex = ( binfo -> scrnx - 16 ) / 2;
	mousey = ( binfo -> scrny - 16) / 2;
	sheet_slide(shtctl, sht_mouse, mousex, mousey);
	sheet_updown(shtctl, sht_back, 0);
	sheet_updown(shtctl, sht_mouse, 1);
	sprintf(str,"(%3d, %3d)",mousex,mousey);
	putfonts8_asc(buf_back, binfo->scrnx, 0, 0, COL8_FFFFFF, str);
	sprintf(str, "Memory: %d MB, free %d KB", memtotal / (1024 * 1024),memman_total(memman) / 1024);

	putfonts8_asc(buf_back, binfo->scrnx, 0, 32, COL8_FFFFFF, str);
	sheet_refresh(shtctl, sht_back, 0, 0, binfo->scrnx, 48);


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
			boxfill8(buf_back, binfo -> scrnx, COL8_008484, 0, 16, 15, 31);
			putfonts8_asc(buf_back, binfo -> scrnx, 0, 16, COL8_FFFFFF, str);
			sheet_refresh(shtctl, sht_back, 0, 16, 16, 32);
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
				boxfill8(buf_back, binfo -> scrnx, COL8_008484, 32, 16, 32 + 15 * 8 - 1, 31);
				putfonts8_asc(buf_back, binfo -> scrnx, 32, 16, COL8_FFFFFF, str);
				sheet_refresh(shtctl, sht_back, 32, 16, 32 + 15 * 8, 32);
				/* 以上用来显示鼠标信息 */
				/* 使用图层之后就不需要自己写清除原来鼠标指针的操作了,全部使用图层操作即可 */
				//boxfill8(buf_back, binfo -> scrnx, COL8_008484, mousex, mousey, mousex + 15, mousey + 15);	//清除原来鼠标指针
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
				if (mousex > binfo -> scrnx - 1)
				{
					mousex = binfo -> scrnx - 1;
				}
				if (mousey > binfo -> scrny - 1)
				{
					mousey = binfo -> scrny - 1;
				}
				sprintf(str, "(%3d, %3d)", mousex, mousey);
				boxfill8(buf_back, binfo -> scrnx, COL8_008484, 0, 0, 79, 15);
				putfonts8_asc(buf_back, binfo -> scrnx, 0, 0, COL8_FFFFFF, str);
				//putblock8_8(buf_back, binfo -> scrnx, 16, 16, mousex, mousey, mouse_cursor, 16);
				sheet_refresh(shtctl, sht_back, 0, 0, 80, 16);
				sheet_slide(shtctl, sht_mouse, mousex, mousey);
			}
		}
	}
}


