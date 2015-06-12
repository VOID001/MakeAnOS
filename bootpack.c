/*************************************************************************
    > File Name: bootpack.c
    > Author: VOID_133
    > ################### 
    > Mail: ################### 
    > Created Time: Wed 10 Jun 2015 10:50:12 PM CST
 ************************************************************************/

#include "bootpack.h"

void HariMain(void)
{
	char* vram;		// BYTE PTR
	unsigned char* str;
	char* mouse_cursor;
	struct BOOTINFO* binfo;		//Store the bootup information in it
	int mousex, mousey;

	init_palette();	/* setup the palette */
	binfo = (struct BOOTINFO*) 0xff0;
	sprintf(str, "binfo -> vram = %08X\n",binfo -> vram);
	mousex = binfo -> scrnx / 2;
	mousey = binfo -> scrny / 2;
	
	init_screen(binfo -> vram, binfo -> scrnx, binfo -> scrny);
	init_mouse_cursor(mouse_cursor, COL8_008484);
	putblock8_8(binfo -> vram,binfo -> scrnx, 16, 16, mousex, mousey, mouse_cursor, 16);
	putfonts8_asc(binfo -> vram, binfo -> scrnx, 8, 8, COL8_FFFFFF, "voidOS By VOID001");
	putfonts8_asc(binfo -> vram, binfo -> scrnx, 8, 30, COL8_FFFFFF, str);
	
	for(;;)
	{
		io_hlt();
	}
}



