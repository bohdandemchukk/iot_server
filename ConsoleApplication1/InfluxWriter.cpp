#include "InfluxWriter.h"
#include <iostream>

namespace beast = boost::beast ;
namespace asio = boost::asio;

InfluxWriter::InfluxWriter(const std::string& host, const std::string& port, const std::string& database)
	: m_host{host}, m_port{port}, m_stream{m_io_context}, m_database{database}, m_INFLUXDB3_AUTH_TOKEN { std::getenv("INFLUXDB3_AUTH_TOKEN") }
	{
		connect();
	}

InfluxWriter::~InfluxWriter() {
	disconnect();
}


void InfluxWriter::connect() {

	if (m_stream.socket().is_open()) {
        beast::error_code ec;
        m_stream.socket().close(ec);
    }

	asio::ip::tcp::resolver resolver{m_io_context};
	auto ip {resolver.resolve(m_host, m_port)};
	m_stream.connect(ip);
	std::cout << "Connected to InfluxDB";
}

bool InfluxWriter::isConnected() const {
		return m_stream.socket().is_open();
}

void InfluxWriter::disconnect() {
	beast::error_code error_code{};
	m_stream.socket().shutdown(asio::ip::tcp::socket::shutdown_both, error_code);
}

void InfluxWriter::write(const std::string& line) {

	if (!isConnected()) {
		std::cout << "Reconnecting..." << '\n';
		connect();
	}

	beast::http::request<beast::http::string_body> request {
		beast::http::verb::post,
		m_database,
		11
	};

	request.set(beast::http::field::host, "localhost");
	request.set(beast::http::field::authorization, "Bearer " + m_INFLUXDB3_AUTH_TOKEN);
	request.body() = line;
	request.prepare_payload();

	beast::http::write(m_stream, request);
	
	beast::http::response<beast::http::string_body> response{};
	beast::flat_buffer buffer{};
	beast::http::read(m_stream, buffer, response);

	std::cout << "InfluxDB response: " << response.result_int() << '\n';

}

