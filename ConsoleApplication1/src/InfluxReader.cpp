#include "../include/InfluxReader.h"
#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <stdexcept>
#include "../utility/env_utility.h"
#include <print>

namespace beast = boost::beast;
namespace asio = boost::asio;

InfluxReader::InfluxReader(ConnectionPool& pool, std::string host, std::string port, std::string database)
    : m_pool{pool},
    m_host{std::move(host)}, 
    m_port{std::move(port)}, 
    m_token { env::require("INFLUXDB3_AUTH_TOKEN") },
    m_database{std::move(database)}
{
}


/*asio::awaitable<std::expected<void, std::string>> InfluxReader::connect() {
    try {
        asio::ip::tcp::resolver resolver{m_io_context};
        auto results { co_await resolver.async_resolve(m_host, m_port, asio::use_awaitable)};

        m_stream.expires_after(std::chrono::seconds(5));

        co_await m_stream.async_connect(results, asio::use_awaitable);
        std::println("[InfluxReader] Connected to {}:{}", m_host, m_port);
        
        co_return std::expected<void, std::string>{};
    } catch (const std::exception& e) {
        co_return std::unexpected(std::string("[InfluxReader] Failed to connect: ") + e.what());
    }
}
*/


asio::awaitable<std::expected<std::string, std::string>> InfluxReader::query(const std::string& sql) {
    /*if (!m_stream.socket().is_open()) {
        if (auto result { co_await connect() }; !result) {
            co_return std::unexpected(result.error());
        }
    }
    
    auto result {co_await doQuery(sql)};
    if (!result) {
        std::println("[InfluxReader] Reconnecting after failure...");

        beast::error_code ec;
        m_stream.socket().shutdown(asio::ip::tcp::socket::shutdown_both, ec);
        m_stream.socket().close(ec);

        if (auto result {co_await connect()}; !result) {
            co_return std::unexpected(result.error());
        }

        co_return co_await doQuery(sql);
    };

    co_return result;
    */

    auto result {co_await doQuery(sql)};

    if (!result) {
        std::println("[InfluxReader] Query failed, retrying...");
        co_return co_await doQuery(sql);
    }

    co_return result;
}




asio::awaitable<std::expected<std::string, std::string>> InfluxReader::doQuery(std::string_view sql) {

    try {

        ConnectionPool::ConnectionGuard guard = co_await m_pool.acquire();
        beast::tcp_stream& stream { *guard.stream };
        std::string target {"/api/v3/query_sql?db=" + m_database + "&q=" + url_encode(sql)};

        beast::http::request<beast::http::string_body> request{beast::http::verb::get, target, 11};
        request.set(beast::http::field::host, m_host);
        request.set(beast::http::field::authorization, "Bearer " + m_token);
        request.set(beast::http::field::connection, "keep-alive");
        request.prepare_payload();

        stream.expires_after(std::chrono::seconds(10));

        co_await beast::http::async_write(stream, request, asio::use_awaitable);

        beast::flat_buffer buffer{};
        beast::http::response<beast::http::string_body> response{};
        co_await beast::http::async_read(stream, buffer, response, asio::use_awaitable);

        if (response.result() != beast::http::status::ok) {
            co_return std::unexpected(std::string("[InfluxReader] doQuery failed: ") + std::to_string(response.result_int()) + " " + response.body());
        }

        co_return response.body();
    } catch (const std::exception& e) {
        co_return std::unexpected(std::string("[InfluxReader] doQuery caught an exception: ") + e.what());
    }
}

std::string InfluxReader::url_encode(std::string_view value) {
    std::string result{};
    result.reserve(value.size());
    for (unsigned char c : value) {
        if (std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            result += c;
        } else if (c == ' ') {
            result += '+';
        } else {
            char buf[4];
            std::snprintf(buf, sizeof(buf), "%%%02X", (unsigned char)c);
            result += buf;
        }
    }
    return result;
}
