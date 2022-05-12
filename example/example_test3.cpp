#include <iostream>
#include <stack>
#include <queue>
#include <vector>
#include <map>
#include <complex>
#include <list>
#include <set>
#include <unordered_set>
#include <algorithm>

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

///--------------------------------------------------------------------------------


//weight: 物品重量，n: 物品个数，w: 背包可承载重量
int knapsack(vector<int> &weight, vector<int> count, int n, int w) {
    //记录状态
    vector<vector<int>> dp(n, vector<int>(w + 1));
    //第 0 个物品
    for (int i = 0; i < min(w / weight[0], count[i]); ++i) {
        dp[0][i * weight[0]] = 1;
    }
    // 状态转移
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j <= w; ++j) {
            int k = min(j / weight[i], count[i]);
            for (int c = 0; c < k; ++c) {
                dp[i][j] += dp[i - 1][j - c * weight[i]];
            }
        }
    }
    // 返回结果
    return dp[n - 1][w];
}

class Solution {
public:
    int minimumTotal(vector<vector<int>> &triangle) {
        int m = triangle.size();
        vector<vector<int>> dp(m, vector<int>(m));
        dp[0][0] = triangle[0][0];

        for (int i = 1; i < m; ++i) {
            dp[i][0] = dp[i - 1][0] + triangle[i][0];
            for (int j = 1; j < i; ++j) {
                dp[i][j] = min(dp[i - 1][j], dp[i - 1][j - 1]) + triangle[i][j];
            }
            dp[i][i] = dp[i - 1][i - 1] + triangle[i][i];
        }

        int result = INT32_MAX;
        cout << dp << endl;
        for (int j = 0; j < m; ++j) {
            result = min(result, dp[m - 1][j]);
        }
        return result;
    }
};


int main() {
    vector<vector<int>> vec = {{2},
                               {3, 4},
                               {6, 5, 7},
                               {4, 1, 8, 3}};
    cout << vec << endl;
    Solution s;
    cout << s.minimumTotal(vec);
}



