#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/beast/version.hpp>
#include <string>
#include <expected>

namespace beast = boost::beast;
namespace asio = boost::asio;


class InfluxReader {

public:
    InfluxReader(asio::io_context& io_context, std::string host, std::string port, std::string database);

   
    asio::awaitable<std::expected<std::string, std::string>> query(const std::string& sql);
    

private:

    asio::awaitable<std::expected<void, std::string>> connect();

    asio::awaitable<std::expected<std::string, std::string>> doQuery(std::string_view sql);

    std::string m_host{};
    std::string m_port{};
    std::string m_token{};
    std::string m_database{};

    asio::io_context& m_io_context;
    boost::beast::tcp_stream m_stream;
    static std::string url_encode(std::string_view value);
};