muduo release v2.0.2 版本，base库

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
  * daytime：服务端accept连接之后，以字符串形式发送当前时间，然后主动断开连接。   
  * time：服务端accept连接之后，以二进制形式发送当前时间（从Epoch到现在的秒数），然后主动断开连接；我们需要一个客户程序来把收到的时间转换为字符串。   
  * echo：回显服务，把收到的数据发回客户端。   
  * chargen：服务端accept连接之后，不停地发送测试数据。
* filetransfer:文件传输，示范非阻塞TCP网络程序中如何完整地发送数据。用来说明TcpConnection::send()的使用。
  * download.cc：一次性把文件读入内存，一次性调用send(const string&)发送完毕。这个版本满足除了“内存消耗只与并发连接数有关，跟文件大小无关”之外的健壮性要求。
  * download2.cc：一块一块地发送文件，减少内存使用，用到了WriteCompleteCallback。这个版本满足了上述全部健壮性要求。
  * download3.cc：同2，但是采用shared_ptr来管理FILE*，避免手动调用::fclose(3)。
* asio
  * char：主要目的是介绍如何处理分包。chat实现了TCP封包与拆包（codec）。
  * tutorial:




