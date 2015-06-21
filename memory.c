#include "bootpack.h"
#define EFLAGS_AC_BIT 0x00040000		
#define CR0_CACHE_DISABLE 0x600000005

unsigned int memtest(unsigned int start, unsigned int end)
{
	char flg486 = 0;
	unsigned int eflg, cr0, i;

	/* 确认CPU是 386 还是 486 以上的 */
	io_store_eflags(eflg);
	eflg = io_load_eflags();
	if ((eflg & EFLAGS_AC_BIT) != 0)		//如果为386 即使AC设为1 也会自动回到0
	{
		flg486 = 1;
	}
	eflg &= ~EFLAGS_AC_BIT;
	io_store_eflags(eflg);

	if(flg486 != 0)
	{
		cr0 = load_cr0();
		cr0 |= CR0_CACHE_DISABLE;			//Memtest 的时候禁止Cache
		store_cr0(cr0);
	}

	i = memtest_sub(start, end);

	if(flg486 != 0)
	{
		cr0 = load_cr0();
		cr0 &= ~CR0_CACHE_DISABLE;			//允许Cache
		store_cr0(cr0);
	}

	return i;
}

#define MEMMAN_FREES 4096

struct FREEINFO{
	unsigned int addr, size;
};

struct MEMMAN{
	int frees, maxfrees, lostsize, losts;
	struct FREEINFO free[MEMMAN_FREES];
};

void memman_init(struct MEMMAN* man)
{
	man->frees = 0;			// 可用信息数目 
	man->maxfrees = 0;		// Free 的最大值
	man->lostsize = 0;
	man->losts = 0;
	return ;
}

unsigned int memman_total(struct MEMMAN* man)		//返回内存当前可用空间
{
	unsigned int i, t = 0;
	for(i = 0; i < man->frees; i++)
	{
		t += man->free[i].size;
	}
	return t;
}

unsigned int memman_alloc(struct MEMMAN* man, unsigned int size)
{
	unsigned int i, a;
	for(i = a; i < man->frees; i++)
	{
		if(man->free[i].size >= size)				//可以分配
		{
			a = man->free[i].addr;
			man->free[i].addr += size;
			man->free[i].size -= size;
			if(man->free[i].size <= 0)
			{
				man->frees --;
				for(; i < man->frees; i++)
				{
					man->free[i] = man->free[i + 1];
				}
			}
			return a;
		}
	}
	return 0;
}

int memman_free(struct MEMMAN* man, unsigned int addr, unsigned int size)	//内存释放
{
	int i, j;
	for(i = 0;i < man->frees; i++)
	{
		if(man->free[i].addr > addr) 
		{
			break;
		}
	}
	/* free[i - 1].addr < addr < free[i].addr */
	if(i > 0)				//i的前面有位置
	{
		if(man->free[i - 1].addr + man->free[i - 1].size == addr)		//如果可以与前面的内存归纳到一起的话
		{
			man->free[i - 1].size += size;
			if(i < man->frees)
			{
				if(addr + size == man->free[i].addr)					//如果还可以和后面的内存归纳到一起的话
				{
					man->free[i - 1].size += man->free[i].size;
					man->frees--;
					for(; i < man->frees; i++)
					{
						man->free[i] = man->free[i + 1];
					}
				}
			}
			return 0;
		}
	}
	if(i < man->frees)
	{
		if(addr + size == man->free[i].addr)
		{
			man->free[i].addr = addr;
			man->free[i].size += size;
			return 0;
		}
	}
	if(man->frees < MEMMAN_FREES)
	{
		for(j = man->frees; j > i; j--)
		{
			man->free[j] = man->free[j - 1];
		}
		man->frees++;
		if(man->maxfrees < man->frees)
		{
			man->maxfrees = man->frees;
		}
		man->free[i].addr = addr;
		man->free[i].size = size;
		return 0;
	}
	man->losts++;
	man->lostsize += size;
	return -1;
}

unsigned int memman_alloc_4k(struct MEMMAN* man, unsigned int size)
{
	unsigned int a;
	size = (size + 0xfff) & 0xfffff000;
	a = memman_alloc(man, size);
	return a;
}

unsigned int memman_free_4k(struct MEMMAN* man, unsigned int addr, unsigned int size)
{
	int i;
	size = (size + 0xfff) & 0xfffff000;
	i = memman_free(man, addr, size);
	return i;
}


