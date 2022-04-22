#include <iostream>
#include <stack>
#include <queue>
#include <vector>
#include <map>
#include <complex>
#include <list>

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


class Solution {
public:
    int result = 0;
    int movingCount(int m, int n, int k) {
        vector<vector<bool>> visited(m, vector<bool>(n, false));
        dfs(0, 0, m, n, k, visited);
        return result;
    }

    void dfs(int row, int col, int m, int n, int k, vector<vector<bool>>& visited){
        visited[row][col] = true;
        result++;

        vector<vector<int>> directions= {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
        for(int i = 0; i < 4; ++i){
            int newRow = row + directions[i][0];
            int newCol = col + directions[i][1];

            cout << "newRow:" << newRow << " newCol:" << newCol << endl;

            if(newRow < 0 || newRow >= m || newCol < 0 || newCol >= n
               || visited[newRow][newCol]
               || !valid(newRow, newCol, k)) {
                continue;
            }
            dfs(newRow, newCol, m, n, k, visited);
        }
    }

    static bool valid(int row, int col, int k){
        int sum = row%10 + row/10 + col%10 + col/10;
        return sum <= k;
    }
};


int main() {
    Solution s;
    cout <<"result:" << s.movingCount(1,2,1) << endl;
    int row = 0, col = 1;
    cout << s.valid(0, 1, 1) << endl;
}


