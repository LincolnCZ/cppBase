#include <iostream>
#include "addressbook.pb.h"

int main(int argc, const char *argv[]) {
    //--------------------------------------------------------------------
    //初始化并序列化
    addressbook::AddressBook addr_book;
    addressbook::Person *p_per = addr_book.add_person_info();//增加结点

    p_per->set_name("lcz");
    p_per->set_id(1219);
    std::cout << "before clear(), id = " << p_per->id() << std::endl;
    p_per->clear_id();
    std::cout << "after clear(), id = " << p_per->id() << std::endl;
    p_per->set_id(1087);
    if (!p_per->has_email())
        p_per->set_email("lcz@qq.com");

    addressbook::Person::PhoneNumber *p_per_phonenum = p_per->add_phone();//增加结点
    p_per_phonenum->set_number("021-8888-8888");
    //增加另一个结点
    p_per_phonenum = p_per->add_phone();
    p_per_phonenum->set_number("138-8888-8888");
    p_per_phonenum->set_type(addressbook::Person::MOBILE);

    uint32_t size = addr_book.ByteSize();////获取二进制字节序的大小
    unsigned char byteArray[size];
    addr_book.SerializeToArray(byteArray, size);//序列化函数：输出到字节流

    //--------------------------------------------------------------------------
    //以下对应的是解析了
    addressbook::AddressBook help_addr_book;
    help_addr_book.ParseFromArray(byteArray, size);//反序列化函数：从字节流解析
    addressbook::Person help_per = help_addr_book.person_info(0);

    std::cout << "*****************************" << std::endl;
    std::cout << "id: " << help_per.id() << std::endl;
    std::cout << "name: " << help_per.name() << std::endl;
    std::cout << "email: " << help_per.email() << std::endl;

    for (int i = 0; i < help_per.phone_size(); ++i) {
        addressbook::Person::PhoneNumber *help_pn = help_per.mutable_phone(i);
        std::cout << "phone_type: " << help_pn->type() << std::endl;
        std::cout << "phone_number: " << help_pn->number() << std::endl;
    }
    std::cout << "*****************************" << std::endl;

    return 0;
}