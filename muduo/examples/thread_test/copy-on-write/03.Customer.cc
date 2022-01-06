#include <map>
#include <string>
#include <vector>
#include <memory>
#include "muduo/base/Mutex.h"

using std::string;

class CustomerData : muduo::noncopyable
{
 public:
  CustomerData()
    : data_(new Map)
  { }

  // 用 lower_bound 在 entries 里找 stock
  int query(const string& customer, const string& stock) const;

 private:
  typedef std::pair<string, int> Entry;
  typedef std::vector<Entry> EntryList;
  typedef std::map<string, EntryList> Map;
  typedef std::shared_ptr<Map> MapPtr;
  void update(const string& customer, const EntryList& entries);
  void update(const string& message);

  static int findEntry(const EntryList& entries, const string& stock){
    // ...
    return 0;
  };
  // 解析收到的消息，返回新的 MapPtr
  static MapPtr parseData(const string& message){
    // ...
    return nullptr;
  };

  MapPtr getData() const
  {
    muduo::MutexLockGuard lock(mutex_);
    return data_;
  }

  mutable muduo::MutexLock mutex_;
  MapPtr data_;
};

int CustomerData::query(const string& customer, const string& stock) const
{
  MapPtr data = getData();

  // data 一旦拿到，就不再需要锁了。
  // 取数据的时候只有getData()内部有锁，多线程并发读的性能很好。
  Map::const_iterator entries = data->find(customer);
  if (entries != data->end())
    return findEntry(entries->second, stock);
  else
    return -1;
}

// 每次收到一个customer的数据更新
void CustomerData::update(const string& customer, const EntryList& entries)
{
  muduo::MutexLockGuard lock(mutex_); // update 必须全程持锁
  if (!data_.unique())
  {
    MapPtr newData(new Map(*data_));
    // 在这里打印日志，然后统计日志来判断 worst case 发生的次数
    data_.swap(newData);
  }
  assert(data_.unique());
  (*data_)[customer] = entries;
}

// 函数原型有变，此时网络上传来的是完整的Map数据
void CustomerData::update(const string& message)
{
  // 解析新数据，在临界区之外
  MapPtr newData = parseData(message);
  if (newData)
  {
    muduo::MutexLockGuard lock(mutex_);
    data_.swap(newData); // 不要用 data_=newData;
  }
  // 旧数据的析构也在临界区外，进一步缩短了临界区
}

int main()
{
  CustomerData data;
}
