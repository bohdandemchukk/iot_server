#include "InfluxReader.h"
#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <stdexcept>

namespace beast = boost::beast;
namespace asio = boost::asio;

InfluxReader::InfluxReader(asio::io_context& io_context, const std::string& host, const std::string& port, const std::string& database)
    : m_io_context{io_context}, m_host{host}, m_port{port}, m_stream{m_io_context}, m_token(std::getenv("INFLUXDB3_AUTH_TOKEN")),
    m_database{database}
{
    connect();
}


void InfluxReader::connect() {
    try {
        asio::ip::tcp::resolver resolver{m_io_context};
        auto results {resolver.resolve(m_host, m_port)};
        m_stream.connect(results);
        std::cout << "Influx Reader connected! " << '\n';
    } catch (const std::exception& e) {
        std::cerr << "Influx Reader couldn't connect: " << e.what() << '\n';
    }
}


std::string InfluxReader::query(const std::string& sql) {
    try {
        return doQuery(sql);
    } catch (...) {
        std::cout << "InfluxReader: reconnecting after failure...\n";
        connect();
        return doQuery(sql);
    }
}

std::string InfluxReader::doQuery(const std::string& sql) {
    std::string target {"/api/v3/query_sql?db=" + m_database + "&q=" + url_encode(sql)};

    beast::http::request<beast::http::string_body> request{beast::http::verb::get, target, 11};
    request.set(beast::http::field::host, m_host);
    request.set(beast::http::field::authorization, "Bearer " + m_token);
    request.set(beast::http::field::connection, "keep-alive");
    request.prepare_payload();

    m_stream.expires_after(std::chrono::seconds(10));

    beast::http::write(m_stream, request);

    beast::flat_buffer buffer{};
    beast::http::response<beast::http::string_body> response{};
    beast::http::read(m_stream, buffer, response);

    if (response.result() != beast::http::status::ok) {
        throw std::runtime_error("InfluxDB query failed: " + std::to_string(response.result_int()) + " " + response.body());
    }

    return response.body();
}

std::string InfluxReader::url_encode(const std::string& value) {
    std::string result{};
    for (char c : value) {
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            result += c;
        } else if (c == ' ') {
            result += '+';
        } else {
            char buf[4];
            snprintf(buf, sizeof(buf), "%%%02X", (unsigned char)c);
            result += buf;
        }
    }
    return result;
}