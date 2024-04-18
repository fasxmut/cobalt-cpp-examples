#include <boost/beast.hpp>
#include <boost/asio.hpp>
#include <boost/cobalt.hpp>
#include <iostream>

namespace beast = boost::beast;
namespace asio = boost::asio;
namespace cobalt = boost::cobalt;
using asio::ip::tcp;
namespace http = beast::http;

class session_class: virtual public std::enable_shared_from_this<session_class>
{
protected:
	std::string host;
	std::string port;
	
	beast::tcp_stream stream;
	tcp::resolver resolver;
	
	http::request<http::string_body> req;
	http::response<http::string_body> res;
	
	beast::flat_buffer buffer;
public:
	virtual ~session_class() = default;
public:
	session_class(const std::string & host__, const std::string & port__):
		host{host__},
		port{port__},
		stream{cobalt::this_thread::get_executor()},
		resolver{cobalt::this_thread::get_executor()}
	{
	}
public:
	cobalt::promise<void> run()
	{
		req.version(11);
		req.method(http::verb::get);
		req.target("/");
		req.set(http::field::host, host);
		
		co_await this->connect();
		co_await this->write_read();
	}
public:
	cobalt::promise<void> connect()
	{
		tcp::resolver::results_type results = co_await resolver.async_resolve(host, port, cobalt::use_op);
		tcp::endpoint ep = co_await stream.async_connect(results, cobalt::use_op);
	}
public:
	cobalt::promise<void> write_read()
	{
		unsigned long bytes = co_await http::async_write(stream, req, cobalt::use_op);
		bytes = co_await http::async_read(stream, buffer, res, cobalt::use_op);
		std::cout
			<< "------------------------------------------------------------\n"
			<< res << std::endl
			<< "------------------------------------------------------------\n"
			<< res.body() << std::endl
			<< "------------------------------------------------------------\n"
			<< res.body().data() << std::endl
			<< "------------------------------------------------------------\n"
			;
	}
};

cobalt::main co_main(int argc, char * argv[])
{
	try
	{
		co_await std::make_shared<session_class>("localhost", "4444")->run();
	}
	catch (const std::exception & err)
	{
		std::cerr << "std::exception =>\n" << err.what() << std::endl;
		co_return 79;
	}
	co_return 0;
}

