#include "InfluxWriter.h"
#include <iostream>
#include <chrono>
namespace beast = boost::beast ;
namespace asio = boost::asio;

InfluxWriter::InfluxWriter(asio::io_context& io_context, const std::string& host, const std::string& port, const std::string& database)
	: m_io_context{io_context}, m_host{host}, m_port{port}, m_stream{m_io_context}, m_database{database}, m_INFLUXDB3_AUTH_TOKEN { std::getenv("INFLUXDB3_AUTH_TOKEN") }
	{
		connect();
	}

InfluxWriter::~InfluxWriter() {
	disconnect();
}


void InfluxWriter::connect() {

	if (m_stream.socket().is_open()) {
        disconnect();
    }

	asio::ip::tcp::resolver resolver{m_io_context};
	auto ip {resolver.resolve(m_host, m_port)};
	m_stream.connect(ip);
	std::cout << "Influx Writer connected!" << '\n';
}

bool InfluxWriter::isConnected() const {
		return m_stream.socket().is_open();
}

void InfluxWriter::disconnect() {
    beast::error_code ec;
    m_stream.socket().shutdown(asio::ip::tcp::socket::shutdown_both, ec);
    m_stream.socket().close(ec);
}


void InfluxWriter::doWrite(const std::string& line) {
	beast::http::request<beast::http::string_body> request {
		beast::http::verb::post,
		"/api/v3/write_lp?db=" + m_database,
		11
	};

	request.set(beast::http::field::host, m_host);
	request.set(beast::http::field::authorization, "Bearer " + m_INFLUXDB3_AUTH_TOKEN);
	request.set(beast::http::field::connection, "keep-alive");
	request.body() = line;
	request.prepare_payload();

	m_stream.expires_after(std::chrono::seconds(5));

	beast::http::write(m_stream, request);
	
	beast::http::response<beast::http::string_body> response{};
	beast::flat_buffer buffer{};
	beast::http::read(m_stream, buffer, response);

	std::cout << "InfluxDB response: " << response.result_int() << '\n';

	if (response.result() != beast::http::status::no_content && response.result() != beast::http::status::ok) 
	{
    	throw std::runtime_error("Influx write failed: " + response.body());
	}
}

void InfluxWriter::write(const std::string& line) {

	try {
		doWrite(line);
	} catch (...) {
		std::cout << "Reconnect after failure...\n";
        connect();
        doWrite(line);
	}

	

	

}

