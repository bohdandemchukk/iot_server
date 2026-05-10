#ifndef INFLUXWRITER_H
#define INFLUXWRITER_H

#include <string>
#include <expected>
#include <iostream>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/ip/tcp.hpp>

namespace beast = boost::beast;
namespace asio = boost::asio;

class InfluxWriter {
public:

	InfluxWriter(asio::io_context& io_context, std::string host, std::string port, std::string database);
	~InfluxWriter();

	
	asio::awaitable<std::expected<void, std::string>> write(std::string line);

private:

	asio::awaitable<std::expected<void, std::string>> connect();
	asio::awaitable<std::expected<void, std::string>> doWrite(std::string_view line);
	void disconnect() noexcept;

	asio::io_context& m_io_context;
	beast::tcp_stream m_stream;
	std::string m_host{};
	std::string m_port{};
	std::string m_database{};
	std::string m_token {};
};

#endif