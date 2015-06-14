/*************************************************************************
    > File Name: bootpack.c
    > Author: VOID_133
    > ################### 
    > Mail: ################### 
    > Created Time: Wed 10 Jun 2015 10:50:12 PM CST
 ************************************************************************/

#include "bootpack.h"

extern struct FIFO8 keyfifo;
extern struct FIFO8 mousefifo;
 
unsigned int memtest_sub(unsigned int start, unsigned int end);
unsigned int memtest(unsigned int start, unsigned int end);

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
	i = memtest(0x00400000, 0xbfffffff) / (1024 * 1024);
	sprintf(str, "Memory: %dMB", i);
	putfonts8_asc(binfo -> vram, binfo -> scrnx, 0, 32, COL8_FFFFFF, str);
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


#define EFLAGS_AC_BIT 0x00040000		
#define CR0_CACHE_DISABLE 0x600000005

unsigned int memtest(unsigned int start, unsigned int end)
{
	char flg486 = 0;
	unsigned int eflg, cr0, i;

	/* 确认CPU是 386 还是 486 以上的 */
	io_store_eflags(eflg);
	eflg = io_load_eflags();
	if ((eflg & EFLAGS_AC_BIT) != 0)		//如果为386 即使AC设为1 也会自动回到0
	{
		flg486 = 1;
	}
	eflg &= ~EFLAGS_AC_BIT;
	io_store_eflags(eflg);

	if(flg486 != 0)
	{
		cr0 = load_cr0();
		cr0 |= CR0_CACHE_DISABLE;			//Memtest 的时候禁止Cache
		store_cr0(cr0);
	}

	i = memtest_sub(start, end);

	if(flg486 != 0)
	{
		cr0 = load_cr0();
		cr0 &= ~CR0_CACHE_DISABLE;			//允许Cache
		store_cr0(cr0);
	}

	return i;
}

//unsigned int memtest_sub(unsigned int start, unsigned int end) { unsigned int i, *p, old, pat0 = 0xaa55aa55, pat1 = 0x55aa55aa; for(i = start; i <= end; i += 0x1000) { p = (unsigned int *) (i + 0xffc); old = *p; *p = pat0; *p ^= 0xffffffff; if(*p != pat1) { not_memory: *p = old; break; } *p ^= 0xffffffff; if(*p != pat0) { goto not_memory; } *p = old; } return i; }
