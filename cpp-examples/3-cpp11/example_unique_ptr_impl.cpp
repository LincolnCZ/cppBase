#include <utility>  // std::swap/move
#include "shape.h"  // shape/shape_type/create_shape

template<typename T>
class unique_ptr {
public:
    explicit unique_ptr(T *ptr = nullptr)
            : ptr_(ptr) {}

    /// destructor
    ~unique_ptr() {
        delete ptr_;
    }

    /// 因为定义了 move-constructor， 所以 copy-constructor 和 operator assigment 默认是delete

    /// move-constructor
    unique_ptr(unique_ptr &&other) noexcept {
        ptr_ = other.release();
    }

    void swap(unique_ptr &rhs) {
        using std::swap;
        swap(ptr_, rhs.ptr_);
    }

    /// move assigment
    ///   利用 copy and swap idiom 实现
    unique_ptr &operator=(unique_ptr rhs) {
        rhs.swap(*this);
        return *this;
    }

    // 指针相关操作
    T *get() const { return ptr_; }

    T &operator*() const { return *ptr_; }

    T *operator->() const { return ptr_; }

    operator bool() const { return ptr_; }

private:
    T *release() {
        T *ptr = ptr_;
        ptr_ = nullptr;
        return ptr;
    }

private:
    T *ptr_;
};

int main() {
    unique_ptr<shape> ptr1{create_shape(shape_type::circle)};
    //unique_ptr<shape> ptr2 = ptr1;             // 编译出错；因为没有定义copy-constructor
    //unique_ptr<shape> ptr2{ptr1};              // 编译出错；因为没有定义copy-constructor
    unique_ptr<shape> ptr3;
    //ptr3 = ptr1;                               // 编译出错：因为没有定义 copy-assigment
    ptr3 = std::move(ptr1);                   // OK
    unique_ptr<shape> ptr4{std::move(ptr3)};  // OK
}