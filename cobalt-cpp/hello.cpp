#include <boost/cobalt.hpp>
#include <iostream>

// co_main - start using co_main in c++ coroutines

namespace cobalt = boost::cobalt;

cobalt::main co_main(int argc, char * argv[])
{
	std::cout << "Hello, cobalt c++!" << std::endl;
	co_return 0;
}

