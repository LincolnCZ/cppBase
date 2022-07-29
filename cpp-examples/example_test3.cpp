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

std::ostream &operator<<(std::ostream &ostr, const std::map<int, std::string> &m) {
    for (auto &i: m) {
        ostr << "[" << i.first << "]=" << i.second << '\t';
    }
    return ostr;
}

std::ostream &operator<<(std::ostream &ostr, const std::vector<int> &vec) {
    for (auto &i: vec) {
        ostr << i << '\t';
    }
    return ostr;
}

std::ostream &operator<<(std::ostream &ostr, const std::vector<vector<int>> &vec) {
    for (auto &i: vec) {
        ostr << i << '\t';
    }
    return ostr;
}

std::ostream &operator<<(std::ostream &ostr, const std::vector<char> &vec) {
    for (auto &i: vec) {
        ostr << i << '\t';
    }
    return ostr;
}

std::ostream &operator<<(std::ostream &ostr, const std::vector<vector<char>> &vec) {
    for (auto &i: vec) {
        ostr << i << endl;
    }
    return ostr;
}

void printVec(vector<int> &vec) {
    for (int i = 0; i < vec.size(); ++i) {
        cout << vec[i] << '\t';
    }
    cout << endl;
}

void printVec(vector<string> &vec) {
    for (int i = 0; i < vec.size(); ++i) {
        cout << vec[i] << '\t';
    }
    cout << endl;
}

std::ostream &operator<<(std::ostream &ostr, const std::list<int> &list) {
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

/**
 * Your Trie object will be instantiated and called as such:
 * Trie* obj = new Trie();
 * obj->insert(word);
 * bool param_2 = obj->search(word);
 * bool param_3 = obj->startsWith(prefix);
 */





int main() {
    Solution s;
    vector<int> vec = {1, -1, 3, 4, 5, 6, 1};
    cout << s.maxGap(vec);
}








