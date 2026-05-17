#ifndef CONNECTIONPOOL_H
#define CONNECTIONPOOL_H

#include <boost/beast/core.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/experimental/channel.hpp>
#include <string>
#include <memory>

namespace beast = boost::beast;
namespace asio = boost::asio;

class ConnectionPool {
public:
    ConnectionPool(asio::io_context& io_context, std::string host, std::string port, std::size_t pool_size = 5);

    

    struct ConnectionGuard {
        beast::tcp_stream* stream;
        ConnectionPool* pool;


        ConnectionGuard(beast::tcp_stream* s, ConnectionPool* p)
            : stream{s}, pool{p} {}

        ~ConnectionGuard() {
            if (stream) pool->release(stream);
        }

        ConnectionGuard(const ConnectionGuard&) = delete;
        ConnectionGuard& operator=(const ConnectionGuard&) = delete;

        ConnectionGuard(ConnectionGuard&& other) noexcept
            : stream{other.stream}, pool{other.pool}
        {
            other.stream = nullptr;
        }
    };

    asio::awaitable<ConnectionGuard> acquire();

private:
    void release(beast::tcp_stream* stream);
    asio::awaitable<void> connect(beast::tcp_stream& stream);

    asio::io_context& m_io_context;
    std::string m_host;
    std::string m_port;


    using CHANNEL = asio::experimental::channel<void(boost::system::error_code, beast::tcp_stream*)>;
    CHANNEL m_channel;
    std::vector<std::unique_ptr<beast::tcp_stream>> m_streams;
};


#endif