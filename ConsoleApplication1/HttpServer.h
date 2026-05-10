#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include "InfluxReader.h"
#include "WeatherCache.h"
#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>

namespace beast = boost::beast;
namespace asio = boost::asio;

class HttpServer {

public:
    HttpServer(asio::io_context& io_context, asio::ip::port_type port, 
           WeatherCache& cache, std::string influx_host,
           std::string influx_port, std::string influx_db);

    void run();

private:
    asio::awaitable<void> listen();

    asio::awaitable<beast::http::response<beast::http::string_body>> handle_request(const beast::http::request<beast::http::string_body>& request);
    asio::awaitable<void> handle_session(asio::ip::tcp::socket socket);

    static void add_cors_headers(beast::http::response<beast::http::string_body>& response);

    beast::http::response<beast::http::string_body> error_response(beast::http::status status,
                                                                    unsigned int version,
                                                                    const std::string& message) const;

    asio::io_context& m_io_context;
    asio::ip::tcp::acceptor m_acceptor;
    WeatherCache& m_cache;
    InfluxReader m_reader;

};

#endif