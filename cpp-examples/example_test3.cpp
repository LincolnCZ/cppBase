#include <stdio.h>
#include <string>
#include <time.h>
#include <iostream>
#include <set>

using namespace std;

string convertTimeStamp2TimeStr(time_t timeStamp)
{
	struct tm *timeinfo = nullptr;
	char buffer[80];
	timeinfo = localtime(&timeStamp);
	strftime(buffer, 80, "%Y%m%d", timeinfo);
	printf("%s\n", buffer);
	return string(buffer);
}

const char *FormatTime(uint32_t iTime)
{
	time_t t;
	struct tm *p;
	t = iTime + 28800;
	p = gmtime(&t);
	char s[80];
	strftime(s, 80, "%Y-%m-%d %H:%M:%S", p);
	string tmp(s);
	printf("%s\n", tmp.c_str());
	return tmp.c_str();
}

bool startsWith(const std::string &str, const std::string prefix)
{
	return (str.rfind(prefix, 0) == 0);
}

#if defined(MAILLIST_SYNC)
#define MAILLISTLOGIC_NAME                  "maillist_syn"
#define MAILLISTLOGIC_CMDID                 101
#elif defined(MAILLIST_NEW_SYNC)
#define MAILLISTLOGIC_NAME                  "maillist_newsync"
#define MAILLISTLOGIC_CMDID                 101
#else
#define MAILLISTLOGIC_NAME                  "mail_list"
#define MAILLISTLOGIC_CMDID                 101
#endif

int main()
{
	cout << MAILLISTLOGIC_NAME << endl;
	return 0;
}

