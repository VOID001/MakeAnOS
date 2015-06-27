#ifndef BOOTPACK_H
#define BOOTPACK_H

#include <stdio.h>
void io_hlt(void);
void io_cli(void);
void io_sti(void);
void io_stihlt(void);
void io_out8(int port, int data); 
int io_in8(int port);
int io_load_eflags(void);
void io_store_eflags(int eflags);
void asm_inthandler21(void);
void asm_inthandler27(void);
void asm_inthandler2c(void);

#define ADR_BOOTINFO 0x00000ff0

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

/* 
 * GDT IDT Prototype 
 * dsctbl.c
 * 
 */
#define ADR_IDT			0x0026f800
#define LIMIT_IDT		0x000007ff
#define ADR_GDT			0x00270000
#define LIMIT_GDT		0x0000ffff
#define ADR_BOTPAK		0x00280000
#define LIMIT_BOTPAK	0x0007ffff
#define AR_DATA32_RW	0x4092
#define AR_CODE32_ER	0x409a
#define AR_INTGATE32	0x008e


void init_gdtidt(void);
void set_segmdesc(struct SEGMENT_DESCRIPTOR* sd, unsigned int limit, int base, int ar);
void set_gatedesc(struct GATE_DESCRIPTOR* gd, unsigned int offset, int selector, int ar);

/* 
 * Display Prototype 
 * graphic.c
 *
 */

void init_palette(void);
void set_palette(int start, int end, unsigned char* rgb);
void boxfill8(unsigned char* vram, int xszie, unsigned char c, int x0, int y0, int x1, int y1);		//填充矩形
void putfont8(char* vram, int xsize, int x, int y, char c, char* font);
void init_screen(char* vram, int xsize, int ysize);
void putfonts8_asc(char* vram, int xsize, int x, int y, char c, unsigned char *str);
void init_mouse_cursor(char* mouse, char bc);
void putblock8_8(char* vram, int vxsize, int pxsize, int pysize, int px0, int py0, char* buf, int bxsize); 

/* 
 * PIC Prototype 
 * int.c
 *
 */


/* 定义PIC的端口地址 */

#define PIC0_ICW1		0x0020
#define PIC0_OCW2		0x0020
#define PIC0_IMR		0x0021
#define PIC0_ICW2		0x0021
#define PIC0_ICW3		0x0021
#define PIC0_ICW4		0x0021
#define PIC1_ICW1		0x00a0
#define PIC1_OCW2		0x00a0
#define PIC1_IMR		0x00a1
#define PIC1_ICW2		0x00a1
#define PIC1_ICW3		0x00a1
#define PIC1_ICW4		0x00a1

struct KEYBUF {		//用于存储当前接受到的字符
	unsigned char data, flag;
};

void init_pic(void);
void inthandler27(int* esp);


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

/*
 * FIFO Prototype
 * fifo.c
 *
 */
struct FIFO8{
	unsigned char* buf;			//缓冲区的地址
	int front,end,size,free,flags;	//队列的参数
};

void fifo8_init(struct FIFO8* fifo, int size, unsigned char* buf);
int fifo8_push(struct FIFO8* fifo, unsigned char data);
int fifo8_pop(struct FIFO8* fifo);
int fifo8_status(struct FIFO8* fifo);

/*
 * Mouse Prototype
 * mouse.c
 *
 */



#define KEYCMD_WRITE_MODE 0x60
#define KEYCMD_SENDTO_MOUSE 0xd4
#define MOUSECMD_ENABLE 0xf4



struct MOUSE_DEC{			//用来将鼠标传输来的数据解析出来
	unsigned char buf[3];
	unsigned char phase;
	int x, y, btn;
};


void enable_mouse(struct MOUSE_DEC* mdec);
int mouse_decode(struct MOUSE_DEC* mdec, int dat);
void inthandler2c(int* esp); 

/*
 * Keyboard Prototype
 * keyboard.c
 *
 */

#define PORT_KEYDAT 0x0060
#define PORT_KEYSTA 0x0064
#define PORT_KEYCMD 0x0064
#define KEYSTA_SEND_NOTREADY 0x02
#define KBC_MODE 0x47

void wait_KBC_sendready(void);
void init_keyboard(void);
void inthandler21(int* esp);

/*
 * Memory Prototype
 * memory.c
 *
 */

#define MEMMAN_FREES 4096
#define EFLAGS_AC_BIT 0x00040000		
#define CR0_CACHE_DISABLE 0x600000005
struct FREEINFO{
	unsigned int addr, size;
};

struct MEMMAN{
	int frees, maxfrees, lostsize, losts;
	struct FREEINFO free[MEMMAN_FREES];
};

unsigned int memtest_sub(unsigned int start, unsigned int end);
unsigned int memtest(unsigned int start, unsigned int end);
void memman_init(struct MEMMAN* man);
unsigned int memman_total(struct MEMMAN* man);
unsigned int memman_alloc(struct MEMMAN* man, unsigned int size);
int memman_free(struct MEMMAN* man, unsigned int addr, unsigned int size);
unsigned int memman_free_4k(struct MEMMAN* man, unsigned int addr, unsigned int size);
unsigned int memman_alloc_4k(struct MEMMAN* man, unsigned int size);

/*
 * Sheet Prototype
 * sheet.c
 *
 */

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

struct SHTCTL* shtctl_init(struct MEMMAN* memman, unsigned char* vram, int xsize, int ysize);
struct SHTCTL* sheet_alloc(struct SHTCTL* ctl);
void sheet_setbuf(struct SHEET* sht, unsigned char* buf, int xsize, int ysize, int col_inv);
void sheet_updown(struct SHTCTL* ctl, struct SHEET* sht, int height);
void sheet_refresh(struct SHTCTL* ctl);
void sheet_slide(struct SHTCTL* ctl, struct SHEET* sht, int vx0, int vy0);
void sheet_free(struct SHTCTL* ctl, struct SHEET* sht);


#endif
