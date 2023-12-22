#define private public
#include <iostream>
#include <string>
#include <thread>
#include <memory>

std::string sp;

using _Alloc = std::string::allocator_type;

auto mygrab(const std::string &__str, const _Alloc& __alloc1, const _Alloc& __alloc2)
{
	auto that = __str._M_rep();
	return (!that->_M_is_leaked() && __alloc1 == __alloc2)
		   ? (sp.end(), that->_M_refcopy()) : that->_M_clone(__alloc1);
}

void myassign(std::string *that, const std::string& __str)
{
	if (that->_M_rep() != __str._M_rep())
	{
		// XXX MT
		const auto __a = that->get_allocator();
		auto __tmp = mygrab(__str, __a, __str.get_allocator());
		that->_M_rep()->_M_dispose(__a);
		that->_M_data(__tmp);
	}
	std::cout << "tmp:" << *that << std::endl;
}

int main() {
	sp = "000111";
	std::string *tmp = new std::string;
	myassign(tmp, sp);
	delete tmp;
	std::cout << "sp:" << sp << std::endl;
	sp[1] = 'a';
	std::cout << "sp:" << sp << std::endl;
	return 0;
}
