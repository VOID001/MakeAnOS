/*************************************************************************
    > File Name: bootpack.c
    > Author: VOID_133
    > ################### 
    > Mail: ################### 
    > Created Time: Wed 10 Jun 2015 10:50:12 PM CST
 ************************************************************************/

void io_hlt(void);
void io_cli(void);
void io_out8(int port, int data);
int io_load_eflags(void);
void io_store_eflags(int eflags);

void init_palette(void);
void set_palette(int start, int end, unsigned char* rgb);
void boxfill8(unsigned char* vram, int xszie, unsigned char c, int x0, int y0, int x1, int y1);		//填充矩形

/* 定义颜色 */

#define COL8_000000 0
#define COL8_FF0000 1
#define COL8_00FF00 2
#define COL8_FFFF00 3
#define COL8_0000FF 4
#define COL8_FF00FF 5
#define COL8_00FFFF 6
#define COL8_FFFFFF 7
#define COL8_C6C6C6 8
#define COL8_840000 9
#define COL8_008400 10
#define COL8_848400 11
#define COL8_000084 12
#define COL8_840084 13
#define COL8_008484 14
#define COL8_848484 15

struct BOOTINFO {
	char cyls,leds,vmode,reserve;
	short scrnx,scrny;
	char* vram;
};

void HariMain(void)
{
	int xsize,ysize;
	char* vram;		// BYTE PTR
	struct BOOTINFO* binfo;		//Store the bootup information in it

	init_palette();	/* setup the palette */
	binfo = (struct BOOTINFO*) 0xff0;
	vram = binfo -> vram;
	xsize = binfo -> scrnx;
	ysize = binfo -> scrny;

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

	for(;;)
	{
		io_hlt();
	}
}

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
