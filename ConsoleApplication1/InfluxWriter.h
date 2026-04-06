#ifndef INFLUXWRITER_H
#define INFLUXWRITER_H

#include <string>
#include <iostream>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/ip/tcp.hpp>

namespace beast = boost::beast ;
namespace asio = boost::asio;

class InfluxWriter {
public:

	InfluxWriter(asio::io_context& io_context, const std::string& host, const std::string& port, const std::string& database);
	~InfluxWriter();
	void doWrite(const std::string& line);
	void write(const std::string& line);

private:

	void connect();
	bool isConnected() const;
	void disconnect();

	
	std::string m_host{};
	std::string m_port{};
	std::string m_database{};
	asio::io_context& m_io_context;
	beast::tcp_stream m_stream;
	std::string m_INFLUXDB3_AUTH_TOKEN {};
};

#endif