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

void HariMain(void)
{
	int i;
	char* p;		// BYTE PTR

	init_palette();	/* setup the palette */

	for(i = 0x0a0000; i <= 0x0affff; i++)
	{
		p = i;
		*p = i%16;
	}
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
