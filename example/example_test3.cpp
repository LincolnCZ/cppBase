#include <iostream>
#include <stack>
#include <queue>
#include <vector>
#include <map>
#include <complex>
#include <list>
#include <set>
#include <unordered_set>

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
int knapsack(vector<int> &weight, int n, int w) {
    //记录状态
    vector<vector<bool>> dp(n, vector<bool>(w + 1, false));
    //第 0 个物品
    for (int i = 0; i < w / weight[0]; ++i) {
        dp[0][i * weight[0]] = true;
    }
    // 状态转移
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j <= w; ++j) {
            int k = j / weight[i];
            for (int c = 0; c < k; ++c) {
                if (dp[i - 1][j - c * weight[i]]) {
                    dp[i][j] = true;
                    break;
                }
            }
        }
    }
    // 返回结果
    for (int j = w; j >= 0; --j) {
        if (dp[n - 1][j]) return j;
    }
    return 0;
}


int main() {

}


template<typename Signature>
class SignalTrivial;

// NOT thread safe !!!
template<typename RET, typename... ARGS>
class SignalTrivial<RET(ARGS...)> {
public:
    typedef std::function<void(ARGS...)> Functor;

    void connect(Functor &&func) {
        functors_.push_back(std::forward<Functor>(func));
    }

    void call(ARGS &&... args) {
        for (const Functor &f: functors_) {
            f(args...);
        }
    }

private:
    std::vector<Functor> functors_;
};



