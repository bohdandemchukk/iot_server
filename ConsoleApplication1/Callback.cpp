#include <string>
#include <print>
#include "WeatherData.h"
#include "Callback.h"
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>

namespace asio = boost::asio;


Callback::Callback(asio::io_context& io_context, InfluxWriter& writer, WeatherCache& cache)
		: m_io_context{io_context}, m_writer{ writer }, m_cache {cache}
	{}

void Callback::message_arrived(mqtt::const_message_ptr msg) {
	std::string payload{ msg->to_string() };
	std::println("[MQTT] Message received: {}", payload);

	asio::co_spawn(m_io_context, 
		m_writer.write("weather " + payload),
		[](std::exception_ptr e_ptr, std::expected<void, std::string> result) {
			if (e_ptr) {
				try { std::rethrow_exception(e_ptr); }
				catch (const std::exception& e) {
					std::println("[InfluxWriter] Exception while writing: {}", e.what());
				}
			} else if (!result) {
				std::println("[InfluxWriter] Write error: {}", result.error());
			}
		}
	);
		 
	m_cache.update(WeatherData{.raw = std::move(payload)});
}