#include "InfluxWriter.h"
#include "env_utility.h"
#include <iostream>
#include <chrono>
#include <cstdlib>
#include <print>

namespace beast = boost::beast;
namespace asio = boost::asio;

InfluxWriter::InfluxWriter(asio::io_context& io_context, std::string host, std::string port, std::string database)
	: m_io_context{io_context},
	  m_host{std::move(host)}, 
	  m_port{std::move(port)}, 
	  m_stream{m_io_context}, 
	  m_database{std::move(database)}, 
	  m_token {env::require("INFLUXDB3_AUTH_TOKEN")}
	{
	}

InfluxWriter::~InfluxWriter() {
	disconnect();
}

void InfluxWriter::disconnect() noexcept {
    beast::error_code ec;
    m_stream.socket().shutdown(asio::ip::tcp::socket::shutdown_both, ec);
    m_stream.socket().close(ec);
}

asio::awaitable<std::expected<void, std::string>> InfluxWriter::connect() {
	try {
		if (m_stream.socket().is_open()) disconnect();
		std::println("[DEBUG] host='{}' port='{}'", m_host, m_port);
		asio::ip::tcp::resolver resolver{m_io_context};
		auto ip { co_await resolver.async_resolve(m_host, m_port, asio::use_awaitable)};
		
		m_stream.expires_after(std::chrono::seconds{5});
		co_await m_stream.async_connect(ip, asio::use_awaitable);

		std::println("[InfluxWriter] Connected to {}:{}", m_host, m_port);
		co_return std::expected<void, std::string>{};
	} catch (const std::exception& e) {
		co_return std::unexpected(std::string("[InfluxWriter] Failed to connect: ") + e.what());
	}
}


asio::awaitable<std::expected<void, std::string>> InfluxWriter::doWrite(std::string_view line) {

	try {
		beast::http::request<beast::http::string_body> request {
			beast::http::verb::post,
			"/api/v3/write_lp?db=" + m_database,
			11
		};

		request.set(beast::http::field::host, m_host);
		request.set(beast::http::field::authorization, "Bearer " + m_token);
		request.set(beast::http::field::connection, "keep-alive");
		request.body() = line;
		request.prepare_payload();

		m_stream.expires_after(std::chrono::seconds(5));

		co_await beast::http::async_write(m_stream, request, asio::use_awaitable);
		
		beast::http::response<beast::http::string_body> response{};
		beast::flat_buffer buffer{};
		co_await beast::http::async_read(m_stream, buffer, response, asio::use_awaitable);

		std::println("[InfluxWriter] InfluxDB Response: {}", response.result_int());
		
		if (response.result() != beast::http::status::no_content && response.result() != beast::http::status::ok) 
		{
			co_return std::unexpected(std::string("[InfluxWriter] Write Request failed: ") + response.body());
		}

		co_return std::expected<void, std::string>{};
		}
	 catch (const std::exception& e) {
		co_return std::unexpected(std::string("[InfluxWriter] doWrite error: ") + e.what());
	}
}

asio::awaitable<std::expected<void, std::string>> InfluxWriter::write(std::string line) {
	if (!m_stream.socket().is_open()) {
        if (auto result {co_await connect()}; !result) {
			co_return std::unexpected(result.error());
		}
    }

	auto result { co_await doWrite(line) };
	if (!result) {
		std::println("[InfluxWriter] Failed to write, reconnecting...");

		if (auto result {co_await connect()}; !result) {
			co_return std::unexpected(result.error());
		}

		co_return co_await doWrite(line);
	}

	co_return std::expected<void, std::string>{};
}

