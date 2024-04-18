#include <boost/cobalt.hpp>
#include <iostream>

// cobalt::generator - co_await many values returned by co_yield and co_return .

namespace cobalt = boost::cobalt;

class range_box
{
public:
	cobalt::generator<int> fn() const
	{
		int i = 0;
		while (true)
			co_yield i++;
		// no co_return here.
	}
	cobalt::generator<float> func() const
	{
		for (float i=1.4f; i<=6.0f; ++i)
			co_yield i;
		co_return 123.45f;	// Be careful! Use co_return here.
	}
};

cobalt::main co_main(int argc, char * argv[])
{
	auto rt1 = range_box{}.fn();
	auto rt2 = range_box{}.func();

	while (rt2)
		std::cout << "rt2 returned: " << co_await rt2 << std::endl;

	for (int i=0; rt1 && (i<10); ++i)
		std::cout << "rt1 returned: " << co_await rt1 << std::endl;

	co_return 0;
}

