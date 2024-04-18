#include <boost/cobalt.hpp>
#include <boost/process/v2.hpp>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <chrono>

// beast cobalt http cs - http server/client in one single c++ source file.

namespace cobalt = boost::cobalt;
namespace process = boost::process::v2;
namespace asio = boost::asio;
namespace beast = boost::beast;
namespace http = beast::http;
using asio::ip::tcp;

namespace cpp
{
	using uint16 = unsigned short;
	using uint64 = unsigned long;
}

namespace api::server
{

class http_server_session: virtual public std::enable_shared_from_this<api::server::http_server_session>
{
protected:
	http::request<http::string_body> request;
	http::response<http::string_body> response;
	beast::flat_buffer buffer;
	beast::tcp_stream stream;
public:
	virtual ~http_server_session() = default;
public:
	http_server_session(tcp::socket && socket):
		stream{std::move(socket)}
	{
	}
public:
	cobalt::promise<void> run()
	{
		std::cout << "server said: a session is created.\n";
		co_await this->read_write();
		co_await this->close_session();
	}
	cobalt::promise<void> read_write()
	{
		response.set(http::field::content_type, "text/html");
		response.body() = "<html><head><title>c++</title></head><body>Hello c++!</body></html>";

		cpp::uint64 bytes = co_await http::async_read(stream, buffer, request, cobalt::use_op);
		bytes = co_await http::async_write(stream, response, cobalt::use_op);
	}
	cobalt::promise<void> close_session()
	{
		stream.close();
		std::cout << "server said: session is closed." << std::endl;
		co_return;
	}
};

class http_server: virtual public std::enable_shared_from_this<api::server::http_server>
{
protected:
	const std::string ip;
	cpp::uint16 port;
	tcp::acceptor acceptor;
public:
	virtual ~http_server() = default;
public:
	http_server(const std::string_view ip__, cpp::uint16 port__):
		ip{ip__},
		port{port__},
		acceptor{
			cobalt::this_thread::get_executor(),
			tcp::endpoint{asio::ip::make_address(ip), port}
		}
	{
	}
public:
	cobalt::promise<void> run()
	{
		co_await this->accept();
	}
	cobalt::promise<void> accept()
	{
		std::cout << "\n\nHttp server is listening on " << ip << " " << port << " ...." << std::endl;
		tcp::socket socket = co_await acceptor.async_accept(cobalt::use_op);
		co_await std::make_shared<api::server::http_server_session>(std::move(socket))->run();
		co_await this->accept();
	}
};	// class http_server

}	// namespace api::server

namespace api::client
{

class http_client_session: virtual public std::enable_shared_from_this<api::client::http_client_session>
{
protected:
	const std::string host;
	const std::string protocol;

	tcp::resolver resolver;
	beast::tcp_stream stream;

	http::request<http::string_body> request;
	http::response<http::string_body> response;
	beast::flat_buffer buffer;
public:
	virtual ~http_client_session() = default;

public:
	http_client_session(const std::string_view host__, const std::string_view protocol__):
		host{host__},
		protocol{protocol__},
		resolver{cobalt::this_thread::get_executor()},
		stream{cobalt::this_thread::get_executor()}
	{
	}

public:
	cobalt::promise<void> run()
	{
		std::cout << "\n\nclient said: request server ..." << std::endl;
		co_await this->connect();
		co_await this->write_read();
	}
	cobalt::promise<void> connect()
	{
		tcp::resolver::results_type results = co_await resolver.async_resolve(host, protocol, cobalt::use_op);
		tcp::endpoint ep = co_await stream.async_connect(results, cobalt::use_op);
	}
	cobalt::promise<void> write_read()
	{
		request.version(11);
		request.target("/");
		request.set(http::field::host, host);
		request.method(http::verb::get);
		request.set(http::field::user_agent, "boost::beast c++");
		cpp::uint64 bytes = co_await http::async_write(stream, request, cobalt::use_op);
		bytes = co_await http::async_read(stream, buffer, response, cobalt::use_op);
		std::cout << "client said: Got:\n" << response.body().data() << std::endl;
	}
};

}	// namespace api::client

cobalt::main co_main(int argc, char * argv[])
{
	if (argc >= 2 && std::string("server") == argv[1])
	{
		std::cout << "server program is running ...\n";

		const std::string ip = "0.0.0.0";
		const cpp::uint16 port = 4567u;

		co_await std::make_shared<api::server::http_server>(ip, port)->run();
		co_return 0;
	}
	else if (argc >=2 && std::string("client") == argv[1])
	{
		std::cout << "client program is running ...\n";
		while (true)
		{
			co_await std::make_shared<api::client::http_client_session>("localhost", "4567")->run();
			std::this_thread::sleep_for(std::chrono::milliseconds(1500));
			std::cout << "============================================================\n";
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
		}
		co_return 0;
	}
	else
	{
		std::cout << "start ...\n";
		asio::io_context io_context;
		process::process proc_server{io_context, argv[0], {"server"}};
		process::process proc_client{io_context, argv[0], {"client"}};
		proc_server.wait();
		proc_client.wait();
		co_return 0;
	}
	co_return 0;
}

/*
server program is running ...


Http server is listening on 0.0.0.0 4567 ....
client program is running ...


client said: request server ...
server said: a session is created.
server said: session is closed.
client said: Got:
<html><head><title>c++</title></head><body>Hello c++!</body></html>


Http server is listening on 0.0.0.0 4567 ....
============================================================


client said: request server ...
server said: a session is created.
server said: session is closed.


Http server is listening on 0.0.0.0 4567 ....
client said: Got:
<html><head><title>c++</title></head><body>Hello c++!</body></html>
============================================================


client said: request server ...
server said: a session is created.
server said: session is closed.
client said: Got:
<html><head><title>c++</title></head><body>Hello c++!</body></html>


Http server is listening on 0.0.0.0 4567 ....
============================================================


client said: request server ...
server said: a session is created.
server said: session is closed.


Http server is listening on 0.0.0.0 4567 ....
client said: Got:
<html><head><title>c++</title></head><body>Hello c++!</body></html>
============================================================
*/
