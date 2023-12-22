#include <algorithm>
#include <vector>
#include <stdio.h>
#include <iostream>
#include <time.h>
#include <fstream>
#include <regex>

using namespace std;

static string getInfileContent(const string &filePath)
{
	ifstream infile(filePath.c_str());
	std::string sContent((std::istreambuf_iterator<char>(infile)),
						 std::istreambuf_iterator<char>());
	return sContent;
}


void replaceContent(string &content, const string &matchBegin_, const string &matchEnd_, const string &replaceBegin,
					const string &replaceEnd)
{
	regex matchBegin(matchBegin_);
	regex matchEnd(matchEnd_);
	bool bFind = false;
	do
	{
		smatch match;
		bFind = regex_search(content, match, matchBegin);
		if (bFind)
		{
			//替换开头
			string beginStr = match[0];
			cout << "beginStr:" << beginStr << endl;
			auto it = content.find(beginStr);
			if (it != string::npos)
			{
				content.replace(it, beginStr.size(), replaceBegin);
			}

			//替换结尾
			string sub = content.substr(it, content.size() - it);
			bFind = regex_search(sub, match, matchEnd);
			if (bFind)
			{
				string endStr = match[0];
				auto subIt = sub.find(endStr);
				if (subIt != string::npos)
				{
					content.replace(it + subIt, endStr.size(), replaceEnd);
				}
			}
		}

		if (!bFind) break;

	} while (true);
}

void replaceTarget(string &content, const string &targetBegin, const string &targetEnd, const string &replaceBegin,
				   const string &replaceEnd)
{
	bool bFind = false;
	do
	{
		auto it = content.find(targetBegin);
		if (it != string::npos)
		{
			bFind = true;
			content.replace(it, targetBegin.size(), replaceBegin);
			string sub = content.substr(it, content.size() - it);
			auto subIt = sub.find(targetEnd);
			if (subIt != string::npos)
			{
				content.replace(it + subIt, targetEnd.size(), replaceEnd);
			}
		}
		else
		{
			bFind = false;
		}
	} while (bFind);
	return;
}

int main()
{
	string filePath = "/Users/llinchengzhong/Downloads/mail.html";
	ifstream infile(filePath.c_str());
	std::string content = getInfileContent(filePath);
	cout << "content size " << content.size() << endl;

	string outFilePath = "/Users/llinchengzhong/Downloads/in.html";
	ofstream fout(outFilePath.c_str());

	clock_t start = clock();
	//1. ---------------------------------------------
	replaceContent(content, R"(<div\s+class="approval_mail_control_title)", R"(</div\s*>)",
				   "<span class=\"approval_mail_control_title", "</span>");
	replaceContent(content, R"(<div\s+class='approval_mail_control_title)", R"(</div\s*>)",
				   "<span class=\'approval_mail_control_title", "</span>");
	replaceContent(content, R"(<div\s+class="approval_mail_control_value)", R"(</div\s*>)",
				   "<span class=\"approval_mail_control_value", "</span>");
	replaceContent(content, R"(<div\s+class='approval_mail_control_value)", R"(</div\s*>)",
				   "<span class='approval_mail_control_value", "</span>");

	//2. ---------------------------------------------
//	replaceTarget(content, "<div class=\"approval_mail_control_title\"", "</div",
//				  "<span class=\"approval_mail_control_title\"", "</span");
//	replaceTarget(content, "<div class=\"approval_mail_control_value\"", "</div",
//				  "<span class=\"approval_mail_control_value\"", "</span");


	fout << content << endl;
	clock_t end = ::clock();
	cout << "cost time:" << (double) (end - start) / CLOCKS_PER_SEC << endl;
//	string s = "ab123cdef456"; // ①
//	regex ex("\\d+");    // ②
//	string result = regex_replace(s, ex, "111"); // ④
//	cout << s << " result: " << result << endl; // ⑤
}
