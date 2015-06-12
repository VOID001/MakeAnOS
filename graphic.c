/*************************************************************************
    > File Name: graphic.c
    > Author: VOID_133
    > ################### 
    > Mail: ################### 
    > Graphic Functions for voidOS
    > Created Time: Fri 12 Jun 2015 02:17:10 PM CST
 ************************************************************************/

#include "bootpack.h"

void init_palette(void)
{
	/* 定义颜色表 */
	/* static char 就相当于汇编的 DB */
	static unsigned char table_rgb[16*3] = {
		0x00,0x00,0x00,		//black
		0xff,0x00,0x00,		//red
		0x00,0xff,0x00,		//green
		0xff,0xff,0x00,		//yellow
		0x00,0x00,0xff,		//blue
		0xff,0x00,0xff,		//purple
		0x00,0xff,0xff,		//light blue
		0xff,0xff,0xff,		//white
		0xc6,0xc6,0xc6,		//grey
		0x84,0x00,0x00,		//dark red
		0x00,0x84,0x00,		//dark green
		0x84,0x84,0x00,		//dark yellow
		0x00,0x00,0x84,		//dark blue
		0x84,0x00,0x84,		//dark purple
		0x00,0x84,0x84,		//light dark blue
		0x84,0x84,0x84		//dark grey
	};
	set_palette(0, 15, table_rgb);
	return ;
}

void set_palette(int start, int end, unsigned char *rgb)
{
	int i,eflags;
	eflags = io_load_eflags();
	io_cli();
	io_out8(0x03c8, start);
	for( i = start; i <= end; i++)
	{
		io_out8(0x03c9, rgb[0] / 4);
		io_out8(0x03c9, rgb[1] / 4);
		io_out8(0x03c9, rgb[2] / 4);
		rgb += 3;
	}
	io_store_eflags(eflags);
	return ;
}

void boxfill8(unsigned char* vram, int xszie, unsigned char c, int x0, int y0, int x1, int y1)		//填充矩形
{
	int x,y;
	for(y = y0; y <= y1; y++)
	{
		for(x = x0; x <= x1; x++)
		{
			vram[y*xszie+x] = c;
		}
	}
	return ;
}

void putfont8(char* vram, int xsize, int x, int y, char c, char* font)
{
	int i;
	char d; /* data */
	char *p;
	for(i = 0; i < 16; i ++)			//输出16*8的点阵字符
	{
		d = font[i];
		p = vram + (y + i) * xsize + x;
		if((d & 0x80) != 0) p[0] = c;
		if((d & 0x40) != 0) p[1] = c;
		if((d & 0x20) != 0) p[2] = c;
		if((d & 0x10) != 0) p[3] = c;
		if((d & 0x08) != 0) p[4] = c;
		if((d & 0x04) != 0) p[5] = c;
		if((d & 0x02) != 0) p[6] = c;
		if((d & 0x01) != 0) p[7] = c;
	}
	return ;
}

void putfonts8_asc(char* vram, int xsize, int x, int y, char c, unsigned char *str)
{
	int storx = x;
	while(*str)
	{
		if(*str == '\n')
		{
			x = storx;
			y += 15;
		}
		else 
		{
			putfont8(vram, xsize, x, y, c, hankaku + *str *16);
			x += 8;
		}
		str ++;
	}
	return ;
}

void init_screen(char* vram, int xsize, int ysize)
{
	boxfill8(vram, xsize, COL8_008484, 0, 0, xsize - 1, ysize - 20);
	boxfill8(vram, xsize, COL8_C6C6C6, 0, ysize - 19,	xsize - 1, ysize - 19);
	boxfill8(vram, xsize, COL8_FFFFFF, 0, ysize - 18, xsize - 1, ysize - 18);
	boxfill8(vram, xsize, COL8_C6C6C6, 0, ysize - 17, xsize - 1, ysize - 1);

	boxfill8(vram, xsize, COL8_008484,  0,         0,          xsize -  1, ysize - 29);
	boxfill8(vram, xsize, COL8_C6C6C6,  0,         ysize - 28, xsize -  1, ysize - 28);
	boxfill8(vram, xsize, COL8_FFFFFF,  0,         ysize - 27, xsize -  1, ysize - 27);
	boxfill8(vram, xsize, COL8_C6C6C6,  0,         ysize - 26, xsize -  1, ysize -  1);

	boxfill8(vram, xsize, COL8_FFFFFF,  3,         ysize - 24, 59,         ysize - 24);
	boxfill8(vram, xsize, COL8_FFFFFF,  2,         ysize - 24,  2,         ysize -  4);
	boxfill8(vram, xsize, COL8_848484,  3,         ysize -  4, 59,         ysize -  4);
	boxfill8(vram, xsize, COL8_848484, 59,         ysize - 23, 59,         ysize -  5);
	boxfill8(vram, xsize, COL8_000000,  2,         ysize -  3, 59,         ysize -  3);
	boxfill8(vram, xsize, COL8_000000, 60,         ysize - 24, 60,         ysize -  3);

	boxfill8(vram, xsize, COL8_848484, xsize - 47, ysize - 24, xsize -  4, ysize - 24);
	boxfill8(vram, xsize, COL8_848484, xsize - 47, ysize - 23, xsize - 47, ysize -  4);
	boxfill8(vram, xsize, COL8_FFFFFF, xsize - 47, ysize -  3, xsize -  4, ysize -  3);
	boxfill8(vram, xsize, COL8_FFFFFF, xsize -  3, ysize - 24, xsize -  3, ysize -  3);

	return ;
}

void init_mouse_cursor(char* mouse, char bc)		//将显示鼠标箭头的信息装载到内存中
{
	static char cursor[16][16] = {		//绘制鼠标箭头
		"**************..",
		"*ooooooooooo*...",
		"*oooooooooo*....",
		"*ooooooooo*.....",
		"*oooooooo*......",
		"*ooooooo*.......",
		"*ooooooo*.......",
		"*oooooooo*......",
		"*oooo**ooo*.....",
		"*ooo*..*ooo*....",
		"*oo*....*ooo*...",
		"*o*......*ooo*..",
		"**........*ooo*.",
		"*..........*ooo*",
		"............*oo*",
		".............***",
	};

	int x, y;
	for(y = 0; y < 16; y ++)
	{
		for(x = 0; x < 16; x++)
		{
			if(cursor[y][x] == '*') mouse[y * 16 + x] = COL8_000000;
			else if(cursor[y][x] == 'o') mouse[y * 16 + x] = COL8_FFFFFF;
			else if(cursor[y][x] == '.') mouse[y * 16 + x] = bc; 
		}
	}
	
	return ;
}

void putblock8_8(char* vram, int vxsize, int pxsize, int pysize,
		int px0, int py0, char* buf, int bxsize)		//把buf的内容输出到屏幕上
{
	int x, y;
	for(y = 0; y < pysize; y++)
	{
		for(x = 0; x < pxsize; x++)
		{
			vram[(py0 + y) * vxsize + (px0 + x)] = buf [y * bxsize + x];
		}
	}

	return ;
}

