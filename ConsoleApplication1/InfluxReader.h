#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <string>

namespace beast = boost::beast;
namespace asio = boost::asio;


class InfluxReader {
private:
    std::string m_host{};
    std::string m_port{};
    std::string m_token{};
    std::string m_database{};

    asio::io_context& m_io_context;
    boost::beast::tcp_stream m_stream;
    std::string url_encode(const std::string& value);

public:
    InfluxReader(asio::io_context& io_context, const std::string& host, const std::string& port, const std::string& database);

    void connect();
    std::string query(const std::string& sql);
    std::string doQuery(const std::string& sql);


};