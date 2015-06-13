/*************************************************************************
    > File Name: fifo.c
    > Author: VOID_133
    > ################### 
    > Mail: ################### 
	> Functions for operating on FIFO
    > Created Time: Sat 13 Jun 2015 07:51:32 AM CST
 ************************************************************************/
#include "bootpack.h"
#define FLAGS_OVERRUN 0x0001

/* Initialize FIFO */
void fifo8_init(struct FIFO8* fifo, int size, unsigned char* buf)
{
	fifo -> size = size;
	fifo -> front = 0;
	fifo -> end = 0;
	fifo -> free = size;
	fifo -> buf = buf;

	return ;
}


/* Push a data into FIFO8 */

int fifo8_push(struct FIFO8* fifo, unsigned char data)
{
	if(fifo -> free == 0)
	{
		fifo -> flags |= FLAGS_OVERRUN;
		return -1;
	}
	fifo -> buf[fifo -> front] = data;
	fifo -> front = (fifo -> front + 1) % fifo -> size;
	fifo -> free--;
	
	return 0;
}

/* Pop a data from FIFO8 */

int fifo8_pop(struct FIFO8* fifo)
{
	int data;
	if(fifo -> free == fifo -> size)
	{
		return -1;
	}
	data = fifo -> buf[fifo -> end];
	fifo -> end = (fifo -> end + 1) % fifo -> size;
	fifo -> free++;

	return data;
}

int fifo8_status(struct FIFO8* fifo)
{
	return fifo -> size - fifo -> free;
}
