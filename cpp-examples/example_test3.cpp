#include <iostream>
#include <stack>
#include <queue>
#include <vector>
#include <map>
#include <complex>
#include <list>
#include <set>
#include <algorithm>
#include <unordered_set>
#include <unordered_map>

using namespace std;

template<typename T>
std::ostream &operator<<(std::ostream &ostr, const std::set<T> &m) {
    for (auto &i: m) {
        ostr << i << '\t';
    }
    return ostr;
}

template<typename K, typename V>
std::ostream &operator<<(std::ostream &ostr, const std::map<K, V> &m) {
    for (auto &i: m) {
        ostr << "[" << i.first << "]=" << i.second << '\t';
    }
    return ostr;
}

template<typename T>
std::ostream &operator<<(std::ostream &ostr, const std::vector<T> &vec) {
    for (auto &i: vec) {
        ostr << i << '\t';
    }
    return ostr;
}

template<typename T>
std::ostream &operator<<(std::ostream &ostr, const std::vector<vector<T>> &vec) {
    for (auto &i: vec) {
        ostr << i << '\t';
    }
    return ostr;
}

template<typename T>
std::ostream &operator<<(std::ostream &ostr, const std::list<T> &list) {
    for (auto &i: list) {
        ostr << " " << i;
    }
    return ostr;
}

///-------------------------------------------------------------------------------
class Trie {
public:
    Trie() {
        root = new TrieNode('/');
    }

    void insert(string word) {
        TrieNode *p = root;
        for (char c: word) {
            if (p->children[c - 'a'] == nullptr) {
                p->children[c - 'a'] = new TrieNode(c);
            }
            p = p->children[c - 'a'];
        }
        p->isEnding = true;
    }

    bool search(string word) {
        TrieNode *p = root;
        for (char c: word) {
            if (p->children[c - 'a'] == nullptr) {
                return false;
            }
            p = p->children[c - 'a'];
        }
        return p->isEnding;
    }

    bool startsWith(string prefix) {
        TrieNode *p = root;
        for (char c: prefix) {
            if (p->children[c - 'a'] == nullptr) {
                return false;
            }
            p = p->children[c - 'a'];
        }
        return true;
    }

    // 多模式串匹配
    // 替代方案：使用AC自动机
    void match(vector<string> words) {
        for (const auto &word: words) {
            if (search(word)) {
                cout << "find : " << word << endl;
            }
        }
    }

private:
    struct TrieNode {
        explicit TrieNode(char c) : data(c) {
            children.assign(26, nullptr);
        }

        char data;
        bool isEnding = false;
        vector<TrieNode *> children;
    };

    TrieNode *root;
};

int main() {
    static int index = 0;
    map<string, string> testMap = {
            {"1", "11"},
            {"2", "22"}
    };
    cout << "map:" << testMap << endl;

    set<string> testSet = {
            "1", "2", "3"
    };
    cout << "set:" << testSet << endl;

    vector<int> testVec = {1, 2, 3};
    vector<vector<int>> testVec2 = {{1, 2},
                                    {3, 4}};
    cout << "vector:" << testVec << endl;
    cout << "vector vector :" << testVec2 << endl;

}








