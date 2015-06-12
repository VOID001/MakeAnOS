#ifndef BOOTPACK_H
#define BOOTPACK_H
void io_hlt(void);
void io_cli(void);
void io_out8(int port, int data);
int io_load_eflags(void);
void io_store_eflags(int eflags);



extern char hankaku[4096];

struct BOOTINFO {
	char cyls,leds,vmode,reserve;
	short scrnx,scrny;
	char* vram;
};

struct SEGMENT_DESCRIPTOR {			//存放GDT的八字节内容
	short limit_low, base_low;
	char base_mid, access_right;
	char limit_high, base_high;
};

struct GATE_DESCRIPTOR {			//存放IDT的八字节内容
	short offset_low, selector;
	char dw_count, access_right;
	short offset_high;
};

/* GDT IDT Prototype */
void init_gdtidt(void);
void set_segmdesc(struct SEGMENT_DESCRIPTOR* sd, unsigned int limit, int base, int ar);
void set_gatedesc(struct GATE_DESCRIPTOR* gd, unsigned int offset, int selector, int ar);

/* Display Prototype */

void init_palette(void);
void set_palette(int start, int end, unsigned char* rgb);
void boxfill8(unsigned char* vram, int xszie, unsigned char c, int x0, int y0, int x1, int y1);		//填充矩形
void putfont8(char* vram, int xsize, int x, int y, char c, char* font);
void init_screen(char* vram, int xsize, int ysize);
void putfonts8_asc(char* vram, int xsize, int x, int y, char c, unsigned char *str);
void init_mouse_cursor(char* mouse, char bc);
void putblock8_8(char* vram, int vxsize, int pxsize, int pysize, int px0, int py0, char* buf, int bxsize); 

#include <stdio.h>

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

#endif
