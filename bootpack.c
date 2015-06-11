/*************************************************************************
    > File Name: bootpack.c
    > Author: VOID_133
    > ################### 
    > Mail: ################### 
    > Created Time: Wed 10 Jun 2015 10:50:12 PM CST
 ************************************************************************/

void io_hlt(void);
void write_mem8(int addr, int data);

void HariMain(void)
{
	int i;
	for(i = 0x0a0000; i <= 0x0affff; i++)
	{
		write_mem8(i, i%16);
	}
	for(;;)
	{
		io_hlt();
	}
}

