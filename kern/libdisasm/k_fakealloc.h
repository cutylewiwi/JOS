#ifndef K_FAKE_ALLOC_H
#define K_FAKE_ALLOC_H
#ifndef JOS_KERNEL
# error "This is a JOS kernel header; user programs should not #include it"
#endif
#include <kern/pmap.h>

// 警告：该文件定义的所有函数都是超级 Hack，仅用于 Lab3 Challenge2

char area[64][1024];
bool status[64] = { 0 };

// 警告：该函数只会分配一页，参数无用
void *
fake_calloc(size_t n, size_t size)
{
	int i;
	for (i = 0; i < 64; i++)
		if (!status[i])
		{
			status[i] = true;
			memset(area[i], 0, 1024);
			return area[i];
		}

	return NULL;
}

void
fake_free(void *p)
{
	status[((char *)p - (char *)area[0]) / 1024] = false;
}
#endif