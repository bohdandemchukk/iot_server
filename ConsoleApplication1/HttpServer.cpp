#include "HttpServer.h"
#include <iostream>

HttpServer::HttpServer(asio::io_context& io_context, asio::ip::port_type port, WeatherCache& cache)
    : m_io_context{io_context}, m_acceptor{m_io_context, {asio::ip::tcp::v4(), port}},
     m_cache{cache}, m_reader{m_io_context, "localhost", "8181", "weather_db"}
    {
        
    }

beast::http::response<beast::http::string_body> HttpServer::handle_request(beast::http::request<beast::http::string_body>& request) {
    if (request.target() == "/latest") {
        auto data { m_cache.get() };

        if (!data) { 
            std::cout << "Latest data was not found!" << '\n';
            return beast::http::response<beast::http::string_body>{
            beast::http::status::not_found, request.version()
        };}

        beast::http::response<beast::http::string_body> response{
            beast::http::status::ok, request.version()
        };

        response.body() = "{ \"data\": \"" + data->raw + "\" }";
        response.prepare_payload();
        return response;
    }

    if (request.target() == "/last_hour") {

        std::cout << "Inside of /last_hour route" << '\n';
        
        std::string sql {
            "SELECT * FROM weather WHERE time > now() - interval '1 hour'"
        };

        auto result {m_reader.query(sql)};

        std::cout << result;

        beast::http::response<beast::http::string_body> response{
            beast::http::status::ok, request.version()
        };

        response.body() = result;
        response.prepare_payload();
        return response;
    }

    if (request.target().starts_with("/range")) {
        std::string from{};
        std::string to{};

        auto pos {request.target().find("?")};
        if (pos != std::string::npos) {
            std::string query {request.target().substr(pos + 1)};
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

        std::string sql {
            "SELECT * FROM weather WHERE time >= '" + from + "' AND time <= '" + to + "'"
        };

        std::string result {m_reader.query(sql)};

        beast::http::response<beast::http::string_body> response{
            beast::http::status::ok, request.version()
        };

        response.body() = result;
        response.prepare_payload();
        return response;

    }

    return beast::http::response<beast::http::string_body>{
        beast::http::status::not_found, request.version()
    };
}

void HttpServer::handle_session(asio::ip::tcp::socket socket) {
    beast::flat_buffer buffer{};

    beast::http::request<beast::http::string_body> request{};
    beast::http::read(socket, buffer, request);

    auto response {handle_request(request)};

    beast::http::write(socket, response);

    beast::error_code error_code{};
    socket.shutdown(asio::ip::tcp::socket::shutdown_send, error_code);
}

void HttpServer::run() {

    std::cout << "HTTP server is running!" << '\n';
    while (true) {
        asio::ip::tcp::socket socket{m_io_context};

        m_acceptor.accept(socket);

        handle_session(std::move(socket));
    }
}
