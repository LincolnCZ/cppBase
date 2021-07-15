#include <string>
#include <gtest/gtest.h>
#include "utility/strutil.h"
using namespace std;

TEST(StrutilTest, ToStringChar) {
    char i = 0;
    unsigned char u = 0;
    EXPECT_EQ(utility::to_string(i), "0");
    EXPECT_EQ(utility::to_string(u), "0");

    i = -1;
    u = -1;
    EXPECT_EQ(utility::to_string(i), "255");
    EXPECT_EQ(utility::to_string(u), "255");
}

TEST(StrutilTest, ToStringInt) {
    int32_t i = 0;
    uint32_t u = 0;
    EXPECT_EQ(utility::to_string(i), "0");
    EXPECT_EQ(utility::to_string(u), "0");

    i = -1;
    u = -1;
    EXPECT_EQ(utility::to_string(i), "-1");
    EXPECT_EQ(utility::to_string(u), "4294967295");

    i = 1024;
    u = 1024;
    EXPECT_EQ(utility::to_string(i), "1024");
    EXPECT_EQ(utility::to_string(u), "1024");
}

TEST(StrutilTest, ToStringDouble) {
    double f = 0.0;
    EXPECT_EQ(utility::to_string(f), "0");
    f = 1.0;
    EXPECT_EQ(utility::to_string(f), "1");
    f = 1.5;
    EXPECT_EQ(utility::to_string(f), "1.5");
}

TEST(StrutilTest, Stol) {
    bool succ;
    long ret;

    ret = utility::stol("12345", &succ);
    EXPECT_EQ(ret, 12345);
    EXPECT_TRUE(succ);

    ret = utility::stol("-12345", &succ);
    EXPECT_EQ(ret, -12345);
    EXPECT_TRUE(succ);

    ret = utility::stol("a12345", &succ);
    EXPECT_EQ(ret, 0);
    EXPECT_FALSE(succ);

    ret = utility::stol("123a45", &succ);
    EXPECT_EQ(ret, 123);
    EXPECT_TRUE(succ);
}

TEST(StrutilTest, SplitTest) {
    vector<string> target, result;
    utility::split("", ",", result);
    EXPECT_TRUE(result.empty());

    utility::split("abc,def ,hello,,end", ",", result);
    target = {"abc", "def ", "hello", "", "end"};
    EXPECT_EQ(target, result);

    utility::split("a man a plan a canal panama", "a ", result);
    target = {"", "man ", "plan ", "canal panama"};
    EXPECT_EQ(target, result);
}

TEST(StrutilTest, SplitLine) {
    vector<string> target, result;
    utility::splitLine("", result);
    EXPECT_TRUE(result.empty());

    utility::splitLine("\nhello\nworld\r\nend\n", result);
    target = {"", "hello", "world", "end"};
    EXPECT_EQ(target, result);
}

TEST(StrutilTest, Trim) {
    string input, ret;

    input = "   abc def  ";
    ret = utility::trim(input);
    EXPECT_EQ(ret, "abc def");

    input = "     ";
    ret = utility::trim(input);
    EXPECT_TRUE(ret.empty());

    input = " Emt2D0v7w==\r\n";
    ret = utility::trim(input);
    EXPECT_EQ(ret, "Emt2D0v7w==");

    input = "a";
    ret = utility::trim(input);
    EXPECT_EQ(ret, "a");
}