#include <gtest/gtest.h>
#include "utility/utils.h"
using namespace std;
UTILITY_NAMESPACE_USE

TEST(MapUtilTest, KeyIter) {
    map<int,int> m;
    m[1] = 0;
    m[2] = 0;
    m[3] = 0;
    set<int> s(key_begin(m), key_end(m));
    EXPECT_TRUE(s.count(1));
    EXPECT_TRUE(s.count(2));
    EXPECT_TRUE(s.count(3));
    EXPECT_FALSE(s.count(0));
}

TEST(MapUtilTest, MapKeySet) {
    map<int,int> m;
    m[1] = 0;
    m[2] = 0;
    m[3] = 0;
    set<int> s;
    s.insert(1);
    s.insert(2);
    s.insert(3);

    EXPECT_EQ(s, map_keyset(m));
}

TEST(UtilTest, Defer) {
    string s;
    {
        s.push_back('0');
        auto _defer0 = defer([&s] { s.push_back('0'); });

        s.push_back('1');
        auto _defer1 = defer([&s] { s.push_back('1'); });

        s.push_back('2');
        auto _defer2 = defer([&s] { s.push_back('2'); });
    }
    EXPECT_EQ(s, "012210");
}