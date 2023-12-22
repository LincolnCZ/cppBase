#include "stdlib.h"

//pStr指针指向的是字符串常量，字符串常量是保存在常量区的，free释放常量区的内存肯定会导致coredump
void dumpCrash()
{
	char *pStr = "test_content";
	free(pStr);
}

int main()
{
	dumpCrash();
	return 0;
}