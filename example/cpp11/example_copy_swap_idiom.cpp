///对于一个函数来说，异常安全(https://en.cppreference.com/w/cpp/language/exceptions)被形式化为3个等级，从高到低依次为：
///
///* No-throw guarantee: 该函数确保不会抛出异常。注意这并不意味着该函数执行的过程中完全没有异常。如果它可以捕获所有可能的异常并进行处理，
///     那么也算这个安全级别。另一种达到该等级的方法是靠返回 bool 值来表示操作是否成功（只有当这个函数的失败也是“正常”的时候才有意义，不然只是
///     将异常的处理责任转嫁给了 caller. 和函数式编程中的 Maybe monad 一样）。
///Strong exception safety： 该函数可能失败并抛出异常。但失败后程序的内部状态不发生变化（程序退回到调用函数之前的状态）。这个要求常见于
///     对数据库的操作中。实现的方法：分离出可能抛出异常的部分作为一个单独的函数。在调用这个函数的时候用临时变量保存结果；如果一切正常再引入改变
///     （如文件读写、数据库操作等）。
///Basic exception safety：该函数可能抛出异常，程序的状态也可能改变，但是不会发生 memory leak，或者使一个 object 处于非正常状态。例如
///     一个函数在 free 之前抛出异常了，则会出现 memory leak。
///
///我们这里讨论的 copy-swap-idiom 的目的是达到 avoiding code duplication, and providing a strong exception guarantee.

#include <algorithm> // std::copy
#include <cstddef> // std::size_t

class dumb_array {
public:
    /// (default) constructor
    explicit dumb_array(std::size_t size = 0)
            : mSize(size),
              mArray(mSize ? new int[mSize]() : nullptr) {
    }

    /// copy-constructor
    dumb_array(const dumb_array &other)
            : mSize(other.mSize),
              mArray(mSize ? new int[mSize] : nullptr) {
        // note that this is non-throwing, because of the data
        // types being used; more attention to detail with regards
        // to exceptions must be given in a more general case, however
        std::copy(other.mArray, other.mArray + mSize, mArray);
    }

    /// destructor
    ~dumb_array() {
        delete[] mArray;
    }

    /// 方法1
    ///
    /// 需要设置 mArray = nullptr 的原因：
    ///     因为如果运算符中的任何其他代码抛出，则可能会调用 dumb_array 的析构函数；如果发生这种情况而未将其设置为 null，
    ///     我们将尝试删除已删除的内存!我们通过将其设置为 null 来避免这种情况，因为删除 null 是一种无操作。
    ///
    /// 存在问题：
    ///（1）需要进行自我赋值判别。
    ///     这个判别有两个目的：是一个阻止冗余代码的一个简单的方法；可以防止出现bug（删除数组接着又进行复制操作）。在其他时候不会有什么问题，
    ///     只是使得程序变慢了。自我赋值在程序中比较少见，所以大部分情况下这个判别是多余的。这样，如果没有这个判别也能够正常工作就更好了。
    ///（2）只提供了 Basic exception safety 保证。
    ///     如果new int[mSize]失败，那么 *this 就被修改了（数组大小是错误的，数组也丢失了）。
    ///（3）代码冗余。
    ///     核心代码只有两行即分配空间和拷贝。如果要实现比较复杂的资源管理，那么代码的膨胀将会导致非常严重的问题。
//    dumb_array &operator=(const dumb_array &other) {
//        if (this != &other) // (1)
//        {
//            // get rid of the old data...
//            delete[] mArray; // (2)
//            mArray = nullptr; // (2) *(需要设置为nullptr)
//
//            // ...and put in the new
//            mSize = other.mSize; // (3)
//            mArray = mSize ? new int[mSize] : nullptr; // (3)
//            std::copy(other.mArray, other.mArray + mSize, mArray); // (3)
//        }
//
//        return *this;
//    }

    /// 方法2
    ///
    /// 修改：修正了方法1中（2）的问题，提供 Strong exception safety 保证，但是（1）和（3）还是存在
//    dumb_array& operator=(const dumb_array& other)
//    {
//        if (this != &other) // (1)
//        {
//            // get the new data ready before we replace the old
//            std::size_t newSize = other.mSize;
//            int* newArray = newSize ? new int[newSize]() : nullptr; // (3)
//            std::copy(other.mArray, other.mArray + newSize, newArray); // (3)
//
//            // replace the old data (all are non-throwing)
//            delete [] mArray;
//            mSize = newSize;
//            mArray = newArray;
//        }
//
//        return *this;
//    }

    /// 将swap函数设置为 friend 的原因: https://stackoverflow.com/questions/5695548/public-friend-swap-member-function
    /// 交换dumb_array，而且交换是很有效率的进行：它只是交换指针和数组大小，而不是重新分配空间和拷贝整个数组。
    friend void swap(dumb_array &first, dumb_array &second) {// nothrow
        // enable ADL (not necessary in our case, but good practice)
        using std::swap;

        // by swapping the members of two objects,
        // the two objects are effectively swapped
        swap(first.mSize, second.mSize);
        swap(first.mArray, second.mArray);
    }

    /// 方法3：copy and swap。
    ///     将赋值分为了拷贝构造和交换两步，异常只可能在第一步发生；而第一步如果发生异常的话，this 对象完全不受任何影响。
    ///     无论拷贝构造成功与否，结果只有赋值成功和赋值没有效果两种状态，而不会发生因为赋值破坏了当前对象这种场景。
    ///
    /// 不足：通常，我们最好遵循比较有用的规则是：不要拷贝函数参数。你应该按值传递参数，让编译器来完成拷贝工作
//    dumb_array &operator=(const dumb_array &other) {
//        dumb_array temp(other); // （1）调用 copy-constructor
//        swap(*this, temp); // （2）调用swap
//
//        return *this;
//    }

    /// 方法4：同方法3，改为传值，完美！达到了避免代码冗余, 提供 Strong exception safety 保证的目的.
    /// copy assigment， move assignment
    ///
    /// 参数改为传值，operator=()的参数在接收参数的时候，会调用构造函数:
    ///     如果调用的是 copy-constructor，那赋值操作就是 copy assigment;
    ///     如果调用的是 move-constructor，那么赋值操作就是移动 move assigment。
    dumb_array &operator=(dumb_array other) { // (1)
        swap(*this, other); //(2)
        return *this;
    }

    /// move-constructor
    ///   移动构造函数通常应为 noexcept，否则即使移动有意义，某些代码(例如 std::vector 调整大小逻辑)也会使用复制构造函数。
    ///   当然，只有在里面的代码没有抛出异常的情况下才标记为noexcept。
    ///   例如：当 push_back、insert、reserve、resize 等函数导致内存重分配时，或当 insert、erase 导致元素位置移动时，vector 会试图
    ///   把元素“移动”到新的内存区域。vector 通常保证强异常安全性，如果元素类型没有提供一个保证不抛异常的移动构造函数，vector 通常会使用拷贝
    ///   构造函数。因此，对于拷贝代价较高的自定义元素类型，我们应当定义移动构造函数，并标其为 noexcept，或只在容器中放置对象的智能指针。
    ///1.为什么一定强制移动构造函数不要抛出异常？移动构造函数抛出异常后，catch处理不可以吗？
    ///   答：catch住也没有用了。仔细想一下，我现在要把vector里的两个对象移到一个新的vector，移第一个成功，第二个时有异常，然后vector该
    ///   怎么办？现在两个vector都废掉了。
    ///2.为什么拷贝构造函数被允许抛出异常？
    ///   答：拷贝不影响旧的容器。即使发生异常，至少老的那个还是好的。这就是异常安全。
    dumb_array(dumb_array &&other) noexcept
            : dumb_array() {// initialize via default constructor, C++11 only
        swap(*this, other);
    }

private:
    std::size_t mSize;
    int *mArray;
};