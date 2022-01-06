muduo release v2.0.2 版本

## 目录说明
examples：使用例子
* python： python 例子，说明网络并发模式
  * 00_echo-iterative.py：accept+read/write 并发模式 
  * 01_echo-fork.py
  * 02_echo-thread.py
* sudoku：说明网络并发模式
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
  * chat：主要目的是介绍如何处理分包。chat实现了TCP封包与拆包（codec）。
    * server_threaded.cc：使用多线程TcpServer，并用mutex来保护共享数据。
    * server_threaded_efficient.cc：对共享数据以§2.8“借shared_ptr实现copy-on-write”的手法来降低锁竞争。 
    * server_threaded_highperformance.cc：采用thread local变量，实现多线程高效转发，这个例子值得仔细阅读理解。
* protobuf
  * codec：
* 
* 
* 
* 
* maxconnection：限制服务器的最大并发连接数例子
* idleconnection：用timing wheel踢掉空闲连接
* hub：简单的消息广播服务



