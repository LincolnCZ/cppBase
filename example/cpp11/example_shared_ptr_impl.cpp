#include "shape.h"
#include <utility>  // std::swap

/// 编译器生成copy-constructor 和 move-constructor的时机：
///   用户如果没有自己提供一个拷贝构造函数（必须形如 Obj(Obj&) 或 Obj(const Obj&)；不是模板），编译器会隐式声明一个。
///   用户如果没有自己声明拷贝构造函数、拷贝赋值函数、移动赋值函数和析构函数，编译器会隐式声明一个移动构造函数。
/// 拷贝是拷贝，移动是移动，拷贝就不要影响右边。反过来讲，如果纯粹是移动，也不应该发生异常的。

/// 要让你设计的对象支持移动的话，通常需要下面几步：
//   你的对象应该有分开的拷贝构造和移动构造函数（除非你只打算支持移动，不支持拷贝——如 unique_ptr）。
//   你的对象应该有 swap 成员函数，支持和另外一个对象快速交换成员。
//   在你的对象的名空间下，应当有一个全局的 swap 函数，调用成员函数 swap 来实现交换。支持这种用法会方便别人（包括你自己在将来）在其他对象里
//     包含你的对象，并快速实现它们的 swap 函数。
//   实现通用的 operator=。
//   上面各个函数如果不抛异常的话，应当标为 noexcept。这对移动构造函数尤为重要。移动构造函数、移动赋值运算符和 swap 函数一般需要保证不抛异常
//     并标为 noexcept（析构函数通常不抛异常且自动默认为 noexcept，不需要标）。


// 为什么shared_count类作为smart_ptr的内部类编译不过，而必须作为外部类呢？
//    移进去的话，smart_ptr<circle>::shared_count 和 smart_ptr<shape>::shared_count 成了两个完全不相关的类型，
//    它们的指针（在不做强制类型转换时）也不能互相赋值，不好。
class shared_count {
public:
    shared_count() noexcept: count_(1) {}

    void add_count() noexcept {
        ++count_;
    }

    long reduce_count() noexcept {
        return --count_;
    }

    long get_count() const noexcept {
        return count_;
    }

private:
    long count_;
};

template<typename T>
class smart_ptr {
public:
    /// 模板的各个实例间并不天然就有 friend 关系，因而不能互访私有成员 ptr_ 和 shared_count_。
    template<typename U>
    friend
    class smart_ptr;

    explicit smart_ptr(T *ptr = nullptr) : ptr_(ptr) {
        if (ptr) {
            shared_count_ = new shared_count();
        }
    }

    /// destructor
    ~smart_ptr() {
        if (ptr_ && !shared_count_->reduce_count()) {
            delete ptr_;
            delete shared_count_;
        }
    }

    /// copy-constructor
    smart_ptr(const smart_ptr &other) {
        // 对于拷贝构造的情况，我们需要在指针非空时把引用数加一，并复制共享计数的指针
        ptr_ = other.ptr_;
        if (ptr_) {
            other.shared_count_->add_count();
            shared_count_ = other.shared_count_;
        }
    }

    /// copy-constructor
    /// 拷贝构造函数为什么有一个泛型版本 还有一个非泛型版本?
    ///   这是一个很特殊的、甚至有点恼人的情况。如果没有非泛型版本，编译器看到没有拷贝构造函数，会生成一个缺省的拷贝构造函数。
    ///   这样，同样类型的smart_ptr的拷贝构造会是错误的。
    template<typename U>
    smart_ptr(const smart_ptr<U> &other) noexcept {
        ptr_ = other.ptr_;
        if (ptr_) {
            other.shared_count_->add_count();
            shared_count_ = other.shared_count_;
        }
    }

    /// move-constructor
    /// 为了子类指针向基类指针的转换，让smart_ptr<circle> 自动转换成 smart_ptr<shape>
    ///   增加了smart_ptr<U>&& other
    ///
    /// 需要注意，这个构造函数不被编译器看作移动构造函数
    ///   理论上，这里的模板参数smart_ptr<U>&&是万能引用，既可以引用左值，又可以引用右值，万能引用在【完美转发】中大有用武之地。
    ///   因此这段代码所表达的是一个构造函数模板，实例化后可能是拷贝构造函数，也可能是移动构造函数。 --- 补充：在函数模板中，如果参数列表是
    ///   带“&&”的模板参数，那么这个参数的类型不是右值引用，而是万能引用。
    template<typename U>
    explicit smart_ptr(smart_ptr<U> &&other) noexcept {
        // 对于移动构造的情况，我们不需要调整引用数，直接把 other.ptr_ 置为空，认为 other 不再指向该共享对象即可。
        ptr_ = other.ptr_;
        if (ptr_) {
            shared_count_ = other.shared_count_;
            other.ptr_ = nullptr;
        }
    }

    /// 为了实现指针类型转换而增加的构造函数
    ///   智能指针需要实现类似的函数模板。实现本身并不复杂，但为了实现这些转换，我们需要添加构造函数，允许在对智能指针内部的指针对象赋值时，
    ///   使用一个现有的智能指针的共享计数。
    template<typename U>
    smart_ptr(const smart_ptr<U> &other, T *ptr) noexcept {
        ptr_ = ptr;
        if (ptr_) {
            other.shared_count_->add_count();
            shared_count_ = other.shared_count_;
        }
    }

    void swap(smart_ptr &rhs) noexcept {
        using std::swap;
        swap(ptr_, rhs.ptr_);
        swap(shared_count_, rhs.shared_count_);
    }

    /// move assigment, copy assigment
    ///   利用 copy and swap idiom 实现
    smart_ptr &operator=(smart_ptr rhs) noexcept {
        rhs.swap(*this);
        return *this;
    }

    // 指针相关操作
    T *get() const noexcept {
        return ptr_;
    }

    long use_count() const noexcept {
        if (ptr_) {
            return shared_count_->get_count();
        } else {
            return 0;
        }
    }

    T &operator*() const noexcept {
        return *ptr_;
    }

    T *operator->() const noexcept {
        return ptr_;
    }

    operator bool() const noexcept {
        return ptr_;
    }

private:
    T *ptr_;
    shared_count *shared_count_;
};

/// 全局的 swap 函数
template<typename T>
void swap(smart_ptr<T> &lhs, smart_ptr<T> &rhs) noexcept {
    lhs.swap(rhs);
}

/// 实现c++里面的四种指针类型转换：把 U 类型的smart pointer转化为 T 类型
///   static_cast reinterpret_cast const_cast dynamic_cast
template<typename T, typename U>
smart_ptr<T> static_pointer_cast(const smart_ptr<U> &other) noexcept {
    T *ptr = static_cast<T *>(other.get());
    return smart_ptr<T>(other, ptr);
}

template<typename T, typename U>
smart_ptr<T> reinterpret_pointer_cast(const smart_ptr<U> &other) noexcept {
    T *ptr = reinterpret_cast<T *>(other.get());
    return smart_ptr<T>(other, ptr);
}

template<typename T, typename U>
smart_ptr<T> const_pointer_cast(const smart_ptr<U> &other) noexcept {
    T *ptr = const_cast<T *>(other.get());
    return smart_ptr<T>(other, ptr);
}

template<typename T, typename U>
smart_ptr<T> dynamic_pointer_cast(const smart_ptr<U> &other) noexcept {
    T *ptr = dynamic_cast<T *>(other.get());
    return smart_ptr<T>(other, ptr);
}

int main() {
    smart_ptr<circle> ptr1(new circle());
    printf("use count of ptr1 is %ld\n", ptr1.use_count());
    smart_ptr<shape> ptr2;
    printf("use count of ptr2 was %ld\n", ptr2.use_count());
    ptr2 = ptr1;
    printf("use count of ptr2 is now %ld\n", ptr2.use_count());
    if (ptr1) {
        puts("ptr1 is not empty");
    }
    smart_ptr<circle> ptr3 = dynamic_pointer_cast<circle>(ptr2);
    printf("use count of ptr3 is %ld\n", ptr3.use_count());
}

