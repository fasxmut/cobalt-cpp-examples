#include <boost/cobalt.hpp>
#include <iostream>

namespace cobalt = boost::cobalt;

cobalt::main co_main(int argc, char * argv[])
{
	std::cout << "Hello, cobalt c++!" << std::endl;
	co_return 0;
}

