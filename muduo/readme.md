muduo release v2.0.2 版本

## 目录
* base：muduo release v2.0.2 版本 base 目录
* net：muduo release v2.0.2 版本 net 目录
* example：muduo 库代码示例

## base
互斥量、条件变量：
* Mutex.h：mutex 封装
  * test：Mutex_test.cc
* Condition.h：条件变量，与 Mutex.h 一同使用
  * BlockingQueue.h：无界阻塞队列（生产者消费者队列）
  * CountDownLatch.{h,cc}：倒计时（CountDownLatch）是一种常用且易用的同步手段。它主要有两种用途：
    * 主线程发起多个子线程，等这些子线程各自都完成一定的任务之后，主线程才继续执行。通常用于主线程等待多个子线程完成初始化。
    * 主线程发起多个子线程，子线程都等待主线程，主线程完成其他一些任务之后通知所有子线程开始执行。通常用于多个子线程等待主线程发出“起跑”命令。
  * BoundedBlockingQueue：有界阻塞队列

线程相关：
* Thread.{h,cc}：线程对象
* ThreadLocal.h：线程局部数据
* ThreadLocalSingleton.h：每个线程一个 singleton
* CurrentThread.h：当前运行线程相关信息
* ThreadPool.{h,cc}：简单的固定大小线程池 --- 实现和容量有限的队列类似

其他：
* Types.h：基本类型的声明，包括muduo::string
* Atomic.h：原子操作与原子整数
* FileUtil.h：读取文件和写入文件
  * test：FileUtil_test.cc
* Exception.h：带stack trace的异常基类
* copyable.h：一个空基类，用于标识（tag）值类型
* nocopyable.h：说明类是不可复制的
* ProcessInfo.{h,cc}：进程信息
  * test：PrecessInfo_test.cc
* Singleton.h：线程安全的 singleton
* StringPiece.h：从 Google 开源代码借用的字符串参数传递类型

时间相关：
* Date.{h,cc}：Julian 日期库（即公历）
* Timestamp.{h,cc}：UTC时间戳
* TimeZone.{h,cc}：时区与夏令时

日志相关：
* LogFile.h：完成滚动日志；日志刷新功能
  * test：LogFile_test.cc
* LogStream.h：包含 FixedBuffer 和 LogStream
  * muduo 没有用标准库中的 iostream，而是自己写的 LogStreamclass，这主要是出于性能原因。
  * test：LogStream_bench.cc   LogStream_test.cc
* Logging.{h,cc}：简单的日志，可搭配AsyncLogging使用
  * test：Logging_test.cc
* AsyncLogging.{h,cc}：多线程异步日志，前后端实现
  * AsyncLogging_test.cc

### 高效的多线程日志
#### 性能需求
为了实现这样的性能指标，muduo日志库的实现有几点优化措施值得一提：
* 时间戳字符串中的日期和时间两部分是缓存的，一秒之内的多条日志只需重新格式化微秒部分【Logger::Impl::formatTime()函数】。例如§5.1出现的3条日志消息中，“20120603 08:02:46”是复用的，每条日志只需要格式化微秒部分（“.125770Z”）。
* 日志消息的前4个字段是定长的，因此可以避免在运行期求字符串长度（不会反复调用strlen【见muduo/base/Logging.cc中的class T和operator<<(LogStream& s, T v)。】）。因为编译器认识memcpy()函数，对于定长的内存复制，会在编译期把它in-line展开为高效的目标代码。
* 线程id是预先格式化为字符串，在输出日志消息时只需简单拷贝几个字节。见CurrentThread::tidString()。
* 每行日志消息的源文件名部分采用了编译期计算来获得basename，避免运行期strrchr(3)开销。见SourceFile-class，这里利用了gcc的内置函数。

#### 多线程异步日志
多线程程序对日志库提出了新的需求：线程安全，即多个线程可以并发写日志，两个线程的日志消息不会出现交织。线程安全不难办到，简单的办法是用一个全局mutex保护IO，或者每个线程单独写一个日志文件，但这两种做法的高效性就堪忧了。前者会造成全部线程抢一个锁，后者有可能让业务线程阻塞在写磁盘操作上。

我认为一个多线程程序的每个进程最好只写一个日志文件，这样分析日志更容易，不必在多个文件中跳来跳去。再说多线程写多个文件也不一定能提速，见§4.7的分析。解决办法不难想到，用一个背景线程负责收集日志消息，并写入日志文件，其他业务线程只管往这个“日志线程”发送日志消息，这称为“异步日志”。

在多线程服务程序中，异步日志（叫“非阻塞日志”似乎更准确）是必需的，因为如果在网络IO线程或业务线程中直接往磁盘写数据的话，写操作偶尔可能阻塞长达数秒之久（原因很复杂，可能是磁盘或磁盘控制器复位）。这可能导致请求方超时，或者耽误发送心跳消息，在分布式系统中更可能造成多米诺骨牌效应，例如误报死锁引发自动failover等。因此，在正常的实时业务处理流程中应该彻底避免磁盘IO，这在使用one loop perthread模型的非阻塞服务端程序中尤为重要，因为线程是复用的，阻塞线程意味着影响多个客户连接。

我们需要一个“队列”来将日志前端的数据传送到后端（日志线程），但这个“队列”不必是现成的Block-ingQueue\<std::string\>，因为不用每次产生一条日志消息都通知（notify()）接收方。

**muduo日志库采用的是双缓冲（double buffering）技术**，基本思路是准备两块buffer：A和B，前端负责往buffer A填数据（日志消息），后端负责将buffer B的数据写入文件。当bufferA写满之后，交换A和B，让后端将buffer A的数据写入文件，而前端则往buffer B填入新的日志消息，如此往复。用两个buffer的好处是在新建日志消息的时候不必等待磁盘文件操作，也避免每条新日志消息都触发（唤醒）后端日志线程。换言之，前端不是将一条条日志消息分别传送给后端，而是将多条日志消息拼成一个大的buffer传送给后端，相当于批处理，减少了线程唤醒的频度，降低开销。另外，为了及时将日志消息写入文件，即便buffer A未满，日志库也会每3秒执行一次上述交换写入操作。

muduo异步日志的性能开销大约是前端每写一条日志消息耗时1.0μs~1.6μs。

## example 目录说明
examples：使用例子
* reactor_test：说明muduo库的设计和实现过程
* thread_test：
  * **Thread-safe_object_lifetime_manager**：对象生命周期管理
  * **copy-on-write**：shared_ptr 实现写时复制
* python：python 例子，说明网络并发模式
  * 00_echo-iterative.py：accept+read/write 并发模式 
  * 01_echo-fork.py
  * 02_echo-thread.py
* **sudoku**：说明网络并发模式
  * 05_server_basic.cc
  * 08_server_threadpoll.cc
  * 09_server_multiloop.cc
  * 11_server_threadpoll.cc
* simple：[UNP]中的五个简单协议，包括echo、daytime、time、discard、chargen等。
  * discard：丢弃所有收到的数据。
    * discard恐怕算是最简单的长连接TCP应用层协议，它只需要关注“三个半事件”中的“消息/数据到达”事件。
  * daytime：服务端accept连接之后，以字符串形式发送当前时间，然后主动断开连接。 
    * daytime是短连接协议，在发送完当前时间后，由服务端主动断开连接。它只需要关注“三个半事件”中的“连接已建立”事件。
  * time：服务端accept连接之后，以二进制形式发送当前时间（从Epoch到现在的秒数），然后主动断开连接；我们需要一个客户程序来把收到的时间转换为字符串。   
    * time协议与daytime极为类似，只不过它返回的不是日期时间字符串，而是一个32-bit整数，表示从1970-01-01 00:00:00Z到现在的秒数。当然，这个协议有“2038年问题”。服务端只需要关注“三个半事件”中的“连接已建立”事件。
  * echo：回显服务，把收到的数据发回客户端。
    * 前面几个协议都是单向接收或发送数据，echo是我们遇到的第一个双向的协议：服务端把客户端发过来的数据原封不动地传回去。它只需要关注“三个半事件”中的“消息/数据到达”事件。
  * chargen：服务端accept连接之后，不停地发送测试数据。
    * Chargen协议很特殊，它只发送数据，不接收数据。而且，它发送数据的速度不能快过客户端接收的速度，因此需要关注“三个半事件”中的半个“消息/数据发送完毕”事件（onWriteComplete）。
  * allinone：将前面五个服务合并成一个。
    * 这里我们把五个服务端用同一个EventLoop跑起来，程序还是单线程的，功能却强大了很多。
* filetransfer:文件传输，示范非阻塞TCP网络程序中如何完整地发送数据。用来说明TcpConnection::send()的使用。
  * download.cc：一次性把文件读入内存，一次性调用send(const string&)发送完毕。这个版本满足除了“内存消耗只与并发连接数有关，跟文件大小无关”之外的健壮性要求。
  * download2.cc：一块一块地发送文件，减少内存使用，用到了WriteCompleteCallback。这个版本满足了上述全部健壮性要求。
  * download3.cc：同2，但是采用shared_ptr来管理FILE*，避免手动调用::fclose(3)。
* asio
  * **chat**：主要目的是介绍如何处理分包。chat实现了TCP封包与拆包（codec）。
    * server_threaded.cc：使用多线程TcpServer，并用mutex来保护共享数据。
    * server_threaded_efficient.cc：对共享数据以§2.8“借shared_ptr实现copy-on-write”的手法来降低锁竞争。 
    * server_threaded_highperformance.cc：采用thread local变量，实现多线程高效转发，这个例子值得仔细阅读理解。
* protobuf
  * **codec**：
* maxconnection：限制服务器的最大并发连接数例子
* asio
  * tutorial：
* **idleconnection**：用timing wheel踢掉空闲连接
* hub：简单的消息广播服务



