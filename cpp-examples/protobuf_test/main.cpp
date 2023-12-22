#include <iostream>
#include "addressbook.pb.h"

using namespace tutorial;
using namespace std;

// 直接使用 copyFrom 是有问题的，即使src和dst的定义完全一致也是有问题的。
// CopyFrom 只适用于同一定义之间
void testCopyFromError() {
    Src src;
    src.set_f1(123);
    src.set_f2("hello");
    cout << "src:" << src.DebugString() << endl;
    Dst dst; //如果使用Src dst，则可以
    dst.CopyFrom(src);
//    dst.set_f3("world");
    cout << "dst:" << dst.DebugString() << endl;
}

int main(int argc, const char *argv[]) {
    //--------------------------------------------------------------------
    //初始化并序列化
    AddressBook addr_book;
    auto book_info = addr_book.mutable_book_info(); //成员是message
    book_info->set_title("tutorial address book");

    //增加Person结点
    Person *pPerson = addr_book.add_person_info(); //成员是repeated message
    //Person *pPerson = addr_book.mutable_person_info(0);//有问题的写法，访问越界
    pPerson->set_name("lcz");
    pPerson->set_id(123);
    pPerson->set_email("lcz@qq.com");

    //增加PhoneNumber节点1
    Person::PhoneNumber *pPersonPhonenum = pPerson->add_phone();
    pPersonPhonenum->set_number("021-8888-8888");
    //增加PhoneNumber节点2
    pPersonPhonenum = pPerson->add_phone();
    pPersonPhonenum->set_number("138-8888-8888");
    pPersonPhonenum->set_type(Person::MOBILE);

    uint32_t size = addr_book.ByteSize();//获取二进制字节序的大小
    unsigned char byteArray[size];
    addr_book.SerializeToArray(byteArray, size);//序列化函数：输出到字节流

//	//test
//	{
//		cout << "before" << addr_book.DebugString() << endl;
//		addr_book.person_info(0).set_name("test");
//		cout << "after" << addr_book.DebugString() << endl;
//	}


    //--------------------------------------------------------------------------
    //以下对应的是解析了
    AddressBook help_addr_book;
    help_addr_book.ParseFromArray(byteArray, size);//反序列化函数：从字节流解析

    cout << "***********book info******************" << endl;
    // 指针访问，适用用修改
    //auto help_book_info = help_addr_book.mutable_book_info();
    //cout << "title: " << help_book_info->title() << endl;
    //const 引用访问，只读
    auto help_book_info = help_addr_book.book_info();
    cout << "title: " << help_book_info.title() << endl;

    cout << "***********person info******************" << endl;
    for (int i = 0; i < help_addr_book.person_info_size(); ++i) {
        Person help_per = help_addr_book.person_info(i);
        cout << "id: " << help_per.id() << endl;
        cout << "name: " << help_per.name() << endl;
        cout << "email: " << help_per.email() << endl;

        for (int j = 0; j < help_per.phone_size(); ++j) {
            Person::PhoneNumber *help_pn = help_per.mutable_phone(j);
            cout << "phone_type: " << help_pn->type() << endl;
            cout << "phone_number: " << help_pn->number() << endl;
        }
    }

//    testCopyFromError();

    return 0;
}