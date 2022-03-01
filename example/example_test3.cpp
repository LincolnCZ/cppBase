#include <iostream>
#include <memory>
#include <unordered_set>

using namespace std;

int main() {
    int i = INT32_MIN;
    std::shared_ptr<int> sP(new int(26));
    cout << "sp use count:" << sP.use_count() << endl;
    std::weak_ptr<int> wP(sP);

    std::unordered_set<std::shared_ptr<int>> bucket;
    {
        std::shared_ptr<int> sP1(wP.lock());
        cout << "sp1 use count:" << sP1.use_count() << endl;
        bucket.insert(sP1);
        for (auto it = bucket.begin(); it != bucket.end(); ++it) {
            cout << "bucket use count :" << it->use_count() << endl;
        }
    }
    {
        std::shared_ptr<int> sP2(wP.lock());
        cout << "sp2 use count:" << sP2.use_count() << endl;

        bucket.insert(sP2);
        for (auto it = bucket.begin(); it != bucket.end(); ++it) {
            cout << "bucket use count :" << it->use_count() << endl;
        }
    }
}