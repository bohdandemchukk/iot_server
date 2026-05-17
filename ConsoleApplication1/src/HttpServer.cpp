#include "../include/HttpServer.h"
#include <iostream>
#include <print>
#include <string_view>
#include <regex>

namespace beast = boost::beast;
namespace asio = boost::asio;

HttpServer::HttpServer(ConnectionPool& pool, asio::io_context& io_context, asio::ip::port_type port,
                       WeatherCache& cache, std::string influx_host,
                        std::string influx_port, std::string influx_db)
    : m_io_context{io_context}, 
      m_acceptor{m_io_context, {asio::ip::tcp::v4(), port}},
      m_cache{cache}, 
      m_reader{pool, std::move(influx_host), std::move(influx_port), std::move(influx_db)}
{
    m_acceptor.set_option(asio::socket_base::reuse_address(true));
}


bool HttpServer::is_valid_date(std::string_view date) {
    std::regex pattern{R"(\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}(\.\d+)?(Z)?)"};
    return std::regex_match(date.begin(), date.end(), pattern);
}

void HttpServer::run() {
    asio::co_spawn(m_io_context, listen(), asio::detached);
    std::println("[HTTP Server] Listening on port 8080 (hardcoded)");
}


asio::awaitable<void> HttpServer::listen() {
    while (true) {
        asio::ip::tcp::socket socket{m_io_context};
        co_await m_acceptor.async_accept(socket, asio::use_awaitable);

        asio::co_spawn(m_io_context, handle_session(std::move(socket)), [](std::exception_ptr e_ptr) -> void {
            if (e_ptr) {
                try {std::rethrow_exception(e_ptr); }
                catch (const std::exception& e) {
                    std::println("[HTTP Server] Session caught exception: {}", e.what());
                }
            }
        });
    }
}

asio::awaitable<void> HttpServer::handle_session(asio::ip::tcp::socket socket) {
    beast::flat_buffer buffer{};
    beast::http::request<beast::http::string_body> request{};
    beast::error_code ec;

    try {
        co_await beast::http::async_read(socket, buffer, request, asio::use_awaitable);
    
        beast::http::response<beast::http::string_body> response {co_await handle_request(request)};

        co_await beast::http::async_write(socket, response, asio::use_awaitable);  
    } catch (const std::exception& e) {
        std::println("[HTTP Server] Session request caught exception: {}", e.what());
    }

    socket.shutdown(asio::ip::tcp::socket::shutdown_send, ec);
    if (ec) {
        std::println("[HTTP Server] Socket shutdown error: {}", ec.message());
    }
}

asio::awaitable<beast::http::response<beast::http::string_body>> HttpServer::handle_request(const beast::http::request<beast::http::string_body>& request) {

    const auto version { request.version() };
    const std::string_view target { request.target() };

    std::println("[HTTP Server] {} {}", std::string(request.method_string()), std::string(target));

    std::println("[HTTP Server] Target: '{}'", std::string(target));

    if (target == "/latest") {
        auto data { m_cache.get() };

        if (!data) { 
            std::println("[HTTP Server] Latest data was not found!");
            co_return error_response(beast::http::status::not_found, version, "Latest data was not found!");
        };

        beast::http::response<beast::http::string_body> response{
            beast::http::status::ok, version
        };

        response.set(beast::http::field::content_type, "application/json");
        response.body() = R"({"data":")" + data->raw + R"("})";
        response.prepare_payload();
        add_cors_headers(response);

        std::println("Latest data was successfully found and returned!");
        co_return response;
    }

    if (target == "/last_hour") {
        
        const std::string sql {
            "SELECT * FROM weather WHERE time > now() - interval '1 hour'"
        };

        auto result {co_await m_reader.query(sql)};

        if (!result) {
            co_return error_response(beast::http::status::internal_server_error, version, result.error());
        }

        if (result.value().empty() || result.value() == "[]") {
            co_return error_response(beast::http::status::not_found, version, "No data found for last hour");
        }

        std::println("[HTTP Server] Last Hour query result: {}", result ? result.value() : result.error());

        beast::http::response<beast::http::string_body> response{
            beast::http::status::ok, version
        };

        response.set(beast::http::field::content_type, "application/json");
        response.body() = result.value();
        response.prepare_payload();
        add_cors_headers(response);
        std::println("Last hour data was successfully found and returned!");
        co_return response;
    }

    if (target.starts_with("/range")) {
        std::string from{};
        std::string to{};

        auto pos {target.find("?")};
        if (pos != std::string::npos) {
            std::string query {target.substr(pos + 1)};
            std::stringstream ss {query};
            std::string item{};

            while(std::getline(ss, item, '&')) {
                auto eq {item.find('=')};
                if (eq != std::string::npos) {
                    auto key {item.substr(0, eq)};
                    auto value {item.substr(eq +1)};

                    if (key == "from") { from = value; }
                    if (key == "to") { to = value; }
                }
            }
        }
        
        if (from.empty() || to.empty()) {
            co_return error_response(beast::http::status::bad_request, version, "Missing 'from' and/or 'to' parameters");
        }

        if (!is_valid_date(from) || !is_valid_date(to)) {
            co_return error_response(beast::http::status::bad_request, version, "Invalid date format. Expected: YYYY-MM-DDTHH:MM:SS.000Z. For example: 2026-05-16T09:00:00.000Z");
        }

        std::string sql {
            "SELECT * FROM weather WHERE time >= '" + from + "' AND time <= '" + to + "'"
        };

        auto result {co_await m_reader.query(sql)};

        if (!result) {
            co_return error_response(beast::http::status::internal_server_error, version, result.error());
        }

        if (result.value().empty() || result.value() == "[]") {
            co_return error_response(beast::http::status::not_found, version, "No data found for given range");
        }

        std::println("[HTTP Server] Range query result: {}", result ? result.value() : result.error());

        beast::http::response<beast::http::string_body> response{
            beast::http::status::ok, version
        };

        response.set(beast::http::field::content_type, "application/json");
        response.body() = result.value();
        response.prepare_payload();
        add_cors_headers(response);
        co_return response;

    }

    co_return error_response(beast::http::status::not_found, version, "Endpoint was not found!");
}




void HttpServer::add_cors_headers(beast::http::response<beast::http::string_body>& response) {
    response.set(beast::http::field::access_control_allow_origin, "*");
    response.set(beast::http::field::access_control_allow_methods, "GET, POST, OPTIONS");
    response.set(beast::http::field::access_control_allow_headers, "Content-Type");
}

beast::http::response<beast::http::string_body> HttpServer::error_response(
    beast::http::status status,
    unsigned int version,
    const std::string& message) const 
{
    beast::http::response<beast::http::string_body> response{status, version};
    response.set(beast::http::field::content_type, "application/json");
    response.body() = R"({"error":")" + message + R"("})";
    response.prepare_payload();
    add_cors_headers(response);
    return response;
}