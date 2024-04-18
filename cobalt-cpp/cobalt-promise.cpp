#include <boost/cobalt.hpp>
#include <iostream>

// cobalt::promise - co_await single value returned by co_return .

namespace cobalt = boost::cobalt;

class cobox_class
{
public:
	cobalt::promise<void> fn() const
	{
		std::cout << "void returned" << std::endl;
		co_return;
	}
	cobalt::promise<int> func() const
	{
		std::cout << "int returned" << std::endl;
		co_return 23456;
	}
};

cobalt::main co_main(int argc, char * argv[])
{
	co_await cobox_class{}.fn();
	std::cout << co_await cobox_class{}.func() << std::endl;
	co_return 0;
}

