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
 
struct SHEET {
	unsigned char* buf;
	int bxsize, bysize, vx0, vy0, col_inv, height, flags;
};

#define MAX_SHEETS 256

struct SHTCTL {
	unsigned char* vram;
	int xsize, ysize, top;
	struct SHEET* sheets[MAX_SHEETS];
	struct SHEET sheets0[MAX_SHEETS];
};

void HariMain(void)
{
	unsigned char* str;
	unsigned int memtotal;
	struct MEMMAN* memman = (struct MEMMAN*) MEMMAN_ADDR;
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
	mousex = ( binfo -> scrnx - 16 ) / 2;
	mousey = ( binfo -> scrny - 16) / 2;
	
	init_screen(binfo -> vram, binfo -> scrnx, binfo -> scrny);
	init_mouse_cursor(mouse_cursor, COL8_008484);
	putblock8_8(binfo -> vram,binfo -> scrnx, 16, 16, mousex, mousey, mouse_cursor, 16);
	memtotal = memtest(0x00400000, 0xbfffffff);
	memman_init(memman);
	memman_free(memman, 0x00001000, 0x0009e000);
	memman_free(memman, 0x00400000, memtotal - 0x00400000);
	sprintf(str, "Memory: %d MB, free %d KB", memtotal / (1024 * 1024),memman_total(memman) / 1024);
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

#define MEMMAN_FREES 4096

struct FREEINFO{
	unsigned int addr, size;
};

struct MEMMAN{
	int frees, maxfrees, lostsize, losts;
	struct FREEINFO free[MEMMAN_FREES];
};

void memman_init(struct MEMMAN* man)
{
	man->frees = 0;			// 可用信息数目 
	man->maxfrees = 0;		// Free 的最大值
	man->lostsize = 0;
	man->losts = 0;
	return ;
}

unsigned int memman_total(struct MEMMAN* man)		//返回内存当前可用空间
{
	unsigned int i, t = 0;
	for(i = 0; i < man->frees; i++)
	{
		t += man->free[i].size;
	}
	return t;
}

unsigned int memman_alloc(struct MEMMAN* man, unsigned int size)
{
	unsigned int i, a;
	for(i = a; i < man->frees; i++)
	{
		if(man->free[i].size >= size)				//可以分配
		{
			a = man->free[i].addr;
			man->free[i].addr += size;
			man->free[i].size -= size;
			if(man->free[i].size <= 0)
			{
				man->frees --;
				for(; i < man->frees; i++)
				{
					man->free[i] = man->free[i + 1];
				}
			}
			return a;
		}
	}
	return 0;
}

int memman_free(struct MEMMAN* man, unsigned int addr, unsigned int size)	//内存释放
{
	int i, j;
	for(i = 0;i < man->frees; i++)
	{
		if(man->free[i].addr > addr) 
		{
			break;
		}
	}
	/* free[i - 1].addr < addr < free[i].addr */
	if(i > 0)				//i的前面有位置
	{
		if(man->free[i - 1].addr + man->free[i - 1].size == addr)		//如果可以与前面的内存归纳到一起的话
		{
			man->free[i - 1].size += size;
			if(i < man->frees)
			{
				if(addr + size == man->free[i].addr)					//如果还可以和后面的内存归纳到一起的话
				{
					man->free[i - 1].size += man->free[i].size;
					man->frees--;
					for(; i < man->frees; i++)
					{
						man->free[i] = man->free[i + 1];
					}
				}
			}
			return 0;
		}
	}
	if(i < man->frees)
	{
		if(addr + size == man->free[i].addr)
		{
			man->free[i].addr = addr;
			man->free[i].size += size;
			return 0;
		}
	}
	if(man->frees < MEMMAN_FREES)
	{
		for(j = man->frees; j > i; j--)
		{
			man->free[j] = man->free[j - 1];
		}
		man->frees++;
		if(man->maxfrees < man->frees)
		{
			man->maxfrees = man->frees;
		}
		man->free[i].addr = addr;
		man->free[i].size = size;
		return 0;
	}
	man->losts++;
	man->lostsize += size;
	return -1;
}

struct SHTCTL* shtctl_init(struct MEMMAN* memman, unsigned char* vram, int xsize, int ysize)
{
	struct SHTCTL* ctl;
	int i;
	ctl = (struct SHTCTL*) memman_alloc_4k(memman, sizeof(struct SHTCTL));
	if(ctl == 0)
	{
		return ctl;
	}
	ctl->vram = vram;
	ctl->xsize = xsize;
	ctl->ysize = ysize;
	ctl->top = -1;
	for(i = 0; i < MAX_SHEETS; i++)
	{
		ctl->sheets0[i].flags = 0;
	}
}
