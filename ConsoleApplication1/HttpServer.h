#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/ip/tcp.hpp>
#include "WeatherCache.h"
#include "InfluxReader.h"

namespace beast = boost::beast;
namespace asio = boost::asio;


class HttpServer {

private:
    asio::io_context& m_io_context;
    asio::ip::tcp::acceptor m_acceptor;
    WeatherCache& m_cache;
    InfluxReader m_reader;

    beast::http::response<beast::http::string_body> handle_request(beast::http::request<beast::http::string_body>& request);
    void handle_session(asio::ip::tcp::socket socket);

public:
    HttpServer(asio::io_context& io_context, asio::ip::port_type port, WeatherCache& cache);

    void run();


};

#endif