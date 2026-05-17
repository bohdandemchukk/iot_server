#include "../include/ConnectionPool.h"
#include <print>


ConnectionPool::ConnectionPool(asio::io_context& io_context, std::string host, std::string port, std::size_t pool_size)
    : m_io_context{io_context}, 
    m_host{std::move(host)},
    m_port{std::move(port)},
    m_channel{io_context, pool_size}
{
    for (std::size_t i{0}; i < pool_size; ++i) {
        auto stream {std::make_unique<beast::tcp_stream>(io_context)};

        m_channel.try_send(boost::system::error_code{}, stream.get());
        m_streams.push_back(std::move(stream));
    }
}


asio::awaitable<void> ConnectionPool::connect(beast::tcp_stream& stream) {
    asio::ip::tcp::resolver resolver{m_io_context};
    auto results {co_await resolver.async_resolve(m_host, m_port, asio::use_awaitable)};
    stream.expires_after(std::chrono::seconds(5));
    co_await stream.async_connect(results, asio::use_awaitable);
    std::println("[ConnectionPool] Connected to {}:{}", m_host, m_port);
}

asio::awaitable<ConnectionPool::ConnectionGuard> ConnectionPool::acquire() {
    auto stream { co_await m_channel.async_receive(asio::use_awaitable)};

    if (!stream->socket().is_open()) {
        std::println("[ConnectionPool] Reconnecting...");
        beast::error_code ec;
        stream->socket().close(ec);
        co_await connect(*stream);
    }

    co_return ConnectionGuard{stream, this};
}

void ConnectionPool::release(beast::tcp_stream* stream) {
    m_channel.try_send(boost::system::error_code{}, stream);
    std::println("[ConnectionPool] Connection released back to pool");
}

    