#include <iostream>

using namespace std;

class base
{
public:
	base();

	virtual void test();

private:
	char *basePStr;
};

class dumpTest : public base
{
public:
	void test();

private:
	char *childPStr;
};

base::base()
{
	basePStr = "test_info";
}

void base::test()
{
	cout << basePStr << endl;
}

void dumpTest::test()
{
	cout << "dumpTest" << endl;
	delete childPStr;
}

//在main函数里定义一个子类的实例化对象，并调用它的虚函数方法test，test里由于直接delete没有初始化的指针childPStr，肯定会造成coredump。
// 本次我们就希望通过dump文件，找到子类dumpTest的this指针和虚函数指针。
int main()
{
	dumpTest dump;
	dump.test();
	return 0;
}