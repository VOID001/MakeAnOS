/*************************************************************************
    > File Name: sheet.c
    > Author: VOID_133
    > ################### 
    > Mail: ################### 
    > Created Time: Sat 27 Jun 2015 09:52:50 PM CST
 ************************************************************************/

#include "bootpack.h" 
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
	return ctl;
}

#define SHEET_USE 1

/* 分配一个SHEET给当前系统 ,成功返回分配好的sheet的地址 */

struct SHEET* sheet_alloc(struct SHTCTL* ctl)
{
	struct SHEET* sht;
	int i;
	for(i = 0; i < MAX_SHEETS; i++)
	{
		if(ctl->sheets0[i].flags == 0)
		{
			sht = &ctl->sheets0[i];
			sht->flags = SHEET_USE;
			sht->height = -1;
			return sht;
		}
	}
	return 0;
}

void sheet_setbuf(struct SHEET* sht, unsigned char* buf, int xsize, int ysize, int col_inv)
{
	sht->buf = buf;
	sht->bxsize = xsize;
	sht->bysize = ysize;
	sht->col_inv = col_inv;
	return ;
}

void sheet_refreshsub(struct SHTCTL* ctl, int vx0, int vy0, int vx1, int vy1)		//可以指定刷新区域
{
	int h, bx, by, vx, vy;
	unsigned char *buf, c, *vram = ctl->vram;
	struct SHEET *sht;
	for(h = 0; h <= ctl->top; h++)
	{
		sht = ctl->sheets[h];
		buf = sht->buf;
		for(by = 0; by < sht->bysize; by++)
		{
			vy = sht->vy0 + by;
			for(bx = 0;bx < sht->bxsize; bx++)
			{
				vx = sht->vx0 + bx;
				if(vx >= vx0 && vx < vx1 && vy >=vy0 && vy < vy1)
				{
					c = buf[by * sht->bxsize + bx];
					if(c != sht->col_inv)
					{
						vram[vy * ctl->xsize + vx] = c;
					}
				}
			}
		}
	}
	return ;
}

void sheet_updown(struct SHTCTL* ctl, struct SHEET* sht, int height)
{
	int h, old = sht->height;
	/* 对给定的height 进行修正 */
	if(height > ctl->top + 1)
	{
		height = ctl->top + 1;
	}
	if(height < -1)
	{
		height = -1;
	}
	
	sht->height = height;

	/* 对 sheets 的高度进行调整 */

	if(old > height)			//从高处将图层移到低处
	{
		if(height >= 0)	//该图层夹在当前图层之间了
		{
			/* 将高于它的图层向上提 */
			for(h = old; h>height; h--)
			{
				ctl->sheets[h] = ctl->sheets[h - 1];
				ctl->sheets[h]->height = h;
			}
			ctl->sheets[height] = sht;
		}
		else		//该图层被隐藏
		{
			if(ctl->top > old)		//将top --- old这些图层降下来
			{
				for(h = old; h < ctl->top; h++)
				{
					ctl->sheets[h] = ctl->sheets[h + 1];
					ctl->sheets[h]->height = h;
				}
			}
			ctl->top--;				//图层减少1
		}
	}
	else if(old < height)		//从低处将图层移到高处
	{
		if(old >= 0)			//要操作的图层没有被隐藏
		{
			for(h = old; h < height; h++)
			{
				ctl->sheets[h] = ctl->sheets[h + 1];
				ctl->sheets[h]->height = h;
			}
			ctl->sheets[height] = sht;
		}
		else					//要操作的图层处于隐藏状态
		{
			for(h = ctl->top; h >= height; h--)
			{
				ctl->sheets[h + 1] = ctl->sheets[h];
				ctl->sheets[h + 1]->height = h + 1;
			}
			ctl->sheets[height] = sht;
			ctl->top++;
		}
	}
	sheet_refreshsub(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize);
	return ;
}
/* 刷新图层 */
void sheet_refresh(struct SHTCTL* ctl, struct SHEET* sht, int bx0, int by0, int bx1, int by1)
{
	if(sht->height >= 0)
	{
		sheet_refreshsub(ctl, sht->vx0 + bx0, sht->vy0 + by0, sht->vx0 + bx1, sht->vy0 + by1);
	}
	return ;
}

void sheet_slide(struct SHTCTL* ctl, struct SHEET* sht, int vx0, int vy0)
{
	int old_vx0, old_vy0;
	old_vx0 = sht->vx0;
	old_vy0 = sht->vy0;
	sht->vx0 = vx0;
	sht->vy0 = vy0;
	if(sht->height >= 0)		//非隐藏图层被移动的时候,要刷新
	{
		/* 只更新被移动的部分 */
		sheet_refreshsub(ctl, old_vx0, old_vy0, old_vx0 + sht->bxsize, old_vy0 + sht->bysize);
		sheet_refreshsub(ctl, vx0, vy0, vx0 + sht->bxsize, vy0 + sht->bysize);
	}
	return ;
}

void sheet_free(struct SHTCTL* ctl, struct SHEET* sht)
{
	if(sht->height >= 0)
	{
		sheet_updown(ctl, sht, -1);
	}
	sht->flags = 0;
	return ;
}
