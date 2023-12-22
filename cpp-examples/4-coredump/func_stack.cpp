#include <iostream>

int param(int a, int b, int c, int d, int e, int f, int g, int h)
{
	int sum = 0;
	char buff[1024] = {0};
	sum = sum + a;
	sum = sum + b;
	sum = sum + c;
	sum = sum + d;
	sum = sum + e;
	sum = sum + f;
	sum = sum + g;
	sum = sum + h;
	return sum;
}

int main()
{
	int ret = 0;
	int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8;
	ret = param(a, b, c, d, e, f, g, h);
	std::cout << ret << std::endl;
	return 0;
}