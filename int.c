/*************************************************************************
    > File Name: int.c
    > Author: VOID_133
    > ################### 
    > Mail: ################### 
    > Created Time: Fri 12 Jun 2015 04:11:03 PM CST
 ************************************************************************/
#include "bootpack.h"

void init_pic(void)
{
	io_out8(PIC0_IMR, 0xff); /* 禁止所有中断 */
	io_out8(PIC1_IMR, 0xff); /* 同上*/

	io_out8(PIC0_ICW1, 0x11); /* 边沿触发模式 */
	io_out8(PIC0_ICW2, 0x20); /* IRQ0-7 由 INT 20 -27 接收 */
	io_out8(PIC0_ICW3, 1 << 2); /* PIC1 由 IRQ2 连接 */
	io_out8(PIC0_ICW4, 0x01); /* 无缓冲区模式 */

	io_out8(PIC0_IMR, 0xfb);  /* 11111011 PIC1以外全部禁止 */
	io_out8(PIC1_IMR, 0xff);  /* 11111111 禁止所有中断 */

	return ;
}

void inthandler21(int* esp)		//键盘中断
{
	struct BOOTINFO* binfo = (struct BOOTINFO*) ADR_BOOTINFO;
	boxfill8(binfo -> vram, binfo -> scrnx, COL8_000000, 0, 0, 32 * 8 - 1, 15);
	boxfill8(binfo -> vram, binfo -> scrnx, COL8_000000, 0, 0, binfo -> scrnx, binfo -> scrny );
	putfonts8_asc(binfo -> vram, binfo -> scrnx, 0, 0, COL8_FFFFFF, "INT 21 (IRQ-1): PS/2 keyboard\nKeyboard is Pressed");

	for(;;)
	{
		io_hlt();
	}
}

void inthandler2c(int* esp)		//鼠标中断
{
	struct BOOTINFO* binfo = (struct BOOTINFO*) ADR_BOOTINFO;
	boxfill8(binfo -> vram, binfo -> scrnx, COL8_000000, 0, 0, 32 * 8 - 1, 15);
	putfonts8_asc(binfo -> vram, binfo -> scrnx, 0, 0, COL8_FFFFFF, "INT 2c (IRQ-12): PS/2 mouse");

	for(;;)
	{
		io_hlt();
	}
}

void inthandler27(int* esp)		//PIC 设置中断
{
	io_out8(PIC0_OCW2, 0x67);
	return ;
}
