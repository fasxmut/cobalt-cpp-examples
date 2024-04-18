#include <boost/cobalt.hpp>
#include <boost/beast.hpp>
#include <boost/asio.hpp>
#include <iostream>

namespace beast = boost::beast;
namespace asio = boost::asio;
namespace cobalt = boost::cobalt;
namespace http = beast::http;
using asio::ip::tcp;

class session_class: virtual public std::enable_shared_from_this<session_class>
{
protected:
	beast::tcp_stream stream;
	http::request<http::string_body> req;
	http::response<http::string_body> res;
	beast::flat_buffer buffer;
public:
	virtual ~session_class() = default;
public:
	session_class(tcp::socket && socket):
		stream{std::move(socket)}
	{
		res.set(http::field::content_type, "text/html");
		res.body() = std::string("<html><head><title>c++</title></head><body>Hello c++!</body></html>");
	}
public:
	cobalt::promise<void> run()
	{
		std::cout << "A session is started" << std::endl;
		co_await this->read_write();
	}
	cobalt::promise<void> read_write()
	{
		unsigned long bytes = co_await http::async_read(stream, buffer, req, cobalt::use_op);
		res.keep_alive(req.keep_alive());
		bytes = co_await http::async_write(stream, res, cobalt::use_op);
		co_await this->close();
	}
public:
	cobalt::promise<void> close()
	{
		stream.close();
		std::cout << "A session is closed" << std::endl << std::endl;
		co_return;	// Be careful! use co_return here.
	}
};

class server_class: virtual public std::enable_shared_from_this<server_class>
{
protected:
	std::string host;
	unsigned short port;
	tcp::acceptor acceptor;
public:
	server_class(const std::string & host__, unsigned short port__):
		host{host__},
		port{port__},
		acceptor{
			cobalt::this_thread::get_executor(),
			tcp::endpoint{asio::ip::make_address(host), port}
		}
	{
	}
public:
	cobalt::promise<void> run()
	{
		co_await this->accept();
	}
public:
	cobalt::promise<void> accept()
	{
		tcp::socket socket = co_await acceptor.async_accept(cobalt::use_op);
		co_await std::make_shared<session_class>(std::move(socket))->run();
		co_await this->accept();
	}
};

cobalt::main co_main(int argc, char * argv[])
{
	try
	{
		co_await std::make_shared<server_class>("0.0.0.0", 4444u)->run();
	}
	catch (const std::exception & err)
	{
		std::cerr << "std::exception =>" << err.what() << std::endl;
		co_return 107;
	}
	co_return 0;
}

