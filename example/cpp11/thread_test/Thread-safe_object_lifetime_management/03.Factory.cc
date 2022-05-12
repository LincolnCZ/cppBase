#include <map>
#include <memory>
#include <functional>
#include <cassert>
#include <cstdio>
#include "muduo/base/Mutex.h"

using std::string;
using namespace std::placeholders;  // for _1, _2, _3...

/// 背景：
///   假设有Stock类，代表一只股票的价格。每一只股票有一个唯一的字符串标识，比如Google的key是"NASDAQ:GOOG"，IBM是"NYSE:IBM"。
///   Stock对象是个主动对象，它能不断获取新价格。为了节省系统资源，同一个程序里边每一只出现的股票只有一个Stock对象，如果多处用到同一只股票，
///   那么Stock对象应该被共享。如果某一只股票没有再在任何地方用到，其对应的Stock对象应该析构，以释放资源，这隐含了“引用计数”。

class Stock : muduo::noncopyable {
public:
    Stock(const string &name)
            : name_(name) {
        printf(" Stock[%p] %s\n", this, name_.c_str());
    }

    ~Stock() {
        printf("~Stock[%p] %s\n", this, name_.c_str());
    }

    const string &key() const { return name_; }

private:
    string name_;
};

/// 存在问题：Stock对象永远不会被销毁，因为map里存的是shared_ptr，始终有“铁丝”绑着。
namespace version1 {

    class StockFactory : muduo::noncopyable {
    public:
        std::shared_ptr<Stock> get(const string &key) {
            muduo::MutexLockGuard lock(mutex_);
            std::shared_ptr<Stock> &pStock = stocks_[key];
            if (!pStock) {
                pStock.reset(new Stock(key));
            }
            return pStock;
        }

    private:
        mutable muduo::MutexLock mutex_;
        std::map<string, std::shared_ptr<Stock> > stocks_;
    };

}

/// 修改：数据成员修改为 std::map<string, weak_ptr<Stock> > stocks_; 这样Stock 对象引用计数为0时，将会销毁
///
/// 存在问题：这么做固然Stock对象是销毁了，但是程序却出现了轻微的内存泄漏，为什么？
///   因为stocks_的大小只增不减，stocks_.size()是曾经存活过的Stock对象的总数，
///   即便活的Stock对象数目降为0。或许有人认为这不算泄漏，因为内存并不是彻底遗失不能访问了，
///   而是被某个标准库容器占用了。我认为这也算内存泄漏，毕竟是“战场”没有打扫干净。
namespace version2 {

    class StockFactory : muduo::noncopyable {
    public:
        std::shared_ptr<Stock> get(const string &key) {
            std::shared_ptr<Stock> pStock;
            muduo::MutexLockGuard lock(mutex_);
            std::weak_ptr<Stock> &wkStock = stocks_[key]; // 如果 key 不存在，会默认构造一个
            pStock = wkStock.lock();  // 尝试把“棉线”提升为“铁丝”
            if (!pStock) {
                pStock.reset(new Stock(key));
                wkStock = pStock;  // 这里更新了 stocks_[key]，注意 wkStock 是个引用
            }
            return pStock;
        }

    private:
        mutable muduo::MutexLock mutex_;
        std::map<string, std::weak_ptr<Stock> > stocks_;
    };

}

///修改：利用shared_ptr的定制析构功能，在析构 Stock 对象的同时清理 stocks_。
///
///存在问题：那就是我们把一个原始的StockFactory this指针保存在了boost::function里（***处），
///  这会有线程安全问题。如果这个Stock Factory先于Stock对象析构，那么会core dump。
///  正如Observer在析构函数里去调用Observable::unregister()，而那时Observable对象可能已经不存在了。
namespace version3 {

    class StockFactory : muduo::noncopyable {
    public:
        std::shared_ptr<Stock> get(const string &key) {
            std::shared_ptr<Stock> pStock;
            muduo::MutexLockGuard lock(mutex_);
            std::weak_ptr<Stock> &wkStock = stocks_[key];
            pStock = wkStock.lock();
            if (!pStock) {
                pStock.reset(new Stock(key),
                             std::bind(&StockFactory::deleteStock, this, _1)); // ***
                wkStock = pStock;
            }
            return pStock;
        }

    private:
        void deleteStock(Stock *stock) {
            printf("deleteStock[%p]\n", stock);
            if (stock) {
                muduo::MutexLockGuard lock(mutex_);
                stocks_.erase(stock->key());  // This is wrong, see removeStock below for correct implementation.
            }
            delete stock;  // sorry, I lied
        }

        mutable muduo::MutexLock mutex_;
        std::map<string, std::weak_ptr<Stock> > stocks_;
    };

}

/// 修改：用enable_shared_from_this。这是一个以其派生类为模板类型实参的基类模板，继承它，this指针就能变身为shared_ptr。
///      为了使用shared_from_this()，StockFactory不能是stack object，必须是heap object且由shared_ptr管理其生命期
///
/// 存在问题：StockFactory的生命期似乎被意外延长了。
///         通常Factory对象是个singleton，在程序正常运行期间不会销毁，这里只是为了展示 version 5的弱回调技术。
namespace version4 {

    class StockFactory : public std::enable_shared_from_this<StockFactory>,
                         muduo::noncopyable {
    public:
        std::shared_ptr<Stock> get(const string &key) {
            std::shared_ptr<Stock> pStock;
            muduo::MutexLockGuard lock(mutex_);
            std::weak_ptr<Stock> &wkStock = stocks_[key];
            pStock = wkStock.lock();
            if (!pStock) {
                pStock.reset(new Stock(key),
                             std::bind(&StockFactory::deleteStock, shared_from_this(), _1));
                wkStock = pStock;
            }
            return pStock;
        }

    private:
        void deleteStock(Stock *stock) {
            printf("deleteStock[%p]\n", stock);
            if (stock) {
                muduo::MutexLockGuard lock(mutex_);
                stocks_.erase(stock->key());
            }
            delete stock;
        }

        mutable muduo::MutexLock mutex_;
        std::map<string, std::weak_ptr<Stock> > stocks_;
    };

}

/// 版本：解决了StockFactory 生命期被意外延长问题。
/// 修改：我们可以把weak_ptr绑到boost::function里，这样对象的生命期就不会被延长。
///      然后在回调的时候先尝试提升为shared_ptr，如果提升成功，说明接受回调的对象还健在，那么就执行回调；如果提升失败，就不必劳神了。
///
/// 改进点：本节的StockFactory只有针对单个Stock对象的操作，如果程序需要遍历整个stocks_，稍不注意就会造成死锁或数据损坏，解决方法参见copy-on-write
class StockFactory : public std::enable_shared_from_this<StockFactory>,
                     muduo::noncopyable {
public:
    std::shared_ptr<Stock> get(const string &key) {
        std::shared_ptr<Stock> pStock;
        muduo::MutexLockGuard lock(mutex_);
        std::weak_ptr<Stock> &wkStock = stocks_[key];
        pStock = wkStock.lock();
        if (!pStock) {
            pStock.reset(new Stock(key),
                         std::bind(&StockFactory::weakDeleteCallback,
                                   std::weak_ptr<StockFactory>(shared_from_this()), _1));
            // 上面必须强制把 shared_from_this() 转型为 weak_ptr，才不会延长生命期，
            // 因为boost::bind拷贝的是实参类型，不是形参类型
            wkStock = pStock;
        }
        return pStock;
    }

private:
    static void weakDeleteCallback(const std::weak_ptr<StockFactory> &wkFactory,
                                   Stock *stock) {
        printf("weakDeleteStock[%p]\n", stock);
        std::shared_ptr<StockFactory> factory(wkFactory.lock());// 尝试提升
        if (factory) // 如果 factory 还在，那就清理 stocks_
        {
            factory->removeStock(stock);
        } else {
            printf("factory died.\n");
        }
        delete stock;  // sorry, I lied
    }

    void removeStock(Stock *stock) {
        if (stock) {
            muduo::MutexLockGuard lock(mutex_);
            auto it = stocks_.find(stock->key());
            if (it != stocks_.end() && it->second.expired()) {
                stocks_.erase(stock->key());
            }
        }
    }

private:
    mutable muduo::MutexLock mutex_;
    std::map<string, std::weak_ptr<Stock> > stocks_;
};

void testLongLifeFactory() {
    std::shared_ptr<StockFactory> factory(new StockFactory);
    {
        std::shared_ptr<Stock> stock = factory->get("NYSE:IBM");
        std::shared_ptr<Stock> stock2 = factory->get("NYSE:IBM");
        assert(stock == stock2);
        // stock destructs here
    }
    // factory destructs here
}

void testShortLifeFactory() {
    std::shared_ptr<Stock> stock;
    {
        std::shared_ptr<StockFactory> factory(new StockFactory);
        stock = factory->get("NYSE:IBM");
        std::shared_ptr<Stock> stock2 = factory->get("NYSE:IBM");
        assert(stock == stock2);
        // factory destructs here
    }
    // stock destructs here
}

int main() {
    version1::StockFactory sf1;
    version2::StockFactory sf2;
    version3::StockFactory sf3;
    std::shared_ptr<version4::StockFactory> sf4(new version4::StockFactory);
    std::shared_ptr<StockFactory> sf5(new StockFactory);

    {
        std::shared_ptr<Stock> s1 = sf1.get("stock1");
    }

    {
        std::shared_ptr<Stock> s2 = sf2.get("stock2");
    }

    {
        std::shared_ptr<Stock> s3 = sf3.get("stock3");
    }

    {
        std::shared_ptr<Stock> s4 = sf4->get("stock4");
    }

    {
        std::shared_ptr<Stock> s5 = sf5->get("stock5");
    }

    testLongLifeFactory();
    testShortLifeFactory();
}
