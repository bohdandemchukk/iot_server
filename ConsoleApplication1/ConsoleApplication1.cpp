#include <iostream>
#include <cstdlib>
#include <thread>
#include <chrono>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <print>
#include "InfluxWriter.h"
#include "HttpServer.h"
#include "Callback.h"
#include "WeatherCache.h"
#include "env_utility.h"
#include "ConnectionPool.h"



namespace beast = boost::beast;
namespace asio = boost::asio;


int main() {
	try {

		const std::string SERVER_URL { env::require("MQTT_URL") };

		const std::string CLIENT_ID{ "cpp-server" };
		const std::string TOPIC{ "project/weather" };
		const std::string USERNAME { env::require("MQTT_USERNAME") };
		const std::string PASSWORD { env::require("MQTT_PASSWORD") };

		//const std::string INFLUX_HOST{ env::read_env_or_default("INFLUXDB_HOST", "localhost")};
		const std::string INFLUX_HOST{"localhost"};
		const std::string INFLUX_PORT{ env::read_env_or_default("INFLUXDB_PORT", "8181")};
		const std::string INFLUX_DB{ env::read_env_or_default("INFLUXDB_BUCKET", "weather_db")};

		asio::io_context io_context;
		WeatherCache cache{};
		ConnectionPool pool{io_context, INFLUX_HOST, INFLUX_PORT, 5};
		HttpServer httpServer{pool, io_context, 8080, cache, INFLUX_HOST, INFLUX_PORT, INFLUX_DB};
		InfluxWriter writer{pool, INFLUX_HOST, INFLUX_PORT, INFLUX_DB};

		mqtt::async_client client{ SERVER_URL, CLIENT_ID };
		mqtt::connect_options con_opts{};
		mqtt::ssl_options ssl_opts{};
		ssl_opts.set_verify(true);
		con_opts.set_ssl(ssl_opts);
		con_opts.set_user_name(USERNAME);
		con_opts.set_password(PASSWORD);

		Callback cb{io_context, writer, cache};
		client.set_callback(cb);

		asio::signal_set signals{io_context, SIGINT, SIGTERM};
		signals.async_wait([&](const boost::system::error_code& ec, int signum) {
			if (ec) return;
			std::println("[MAIN] Signal {} received, shutting down server...", signum);
			client.disconnect()->wait();
			io_context.stop();
		});

		std::println("[MAIN] Connecting to MQTT...");
		client.connect(con_opts)->wait();
		std::println("[MAIN] Connected to MQTT!");

		client.subscribe(TOPIC, 1)->wait();

		std::println("[MAIN] Subscribed to MQTT topic: {}", TOPIC);
		
		httpServer.run();

		io_context.run();
	}
	catch (const mqtt::exception& e) {
		std::println("[MAIN] Caught exception: {}", e.what());
		return 1;
	}

	
	return 0;
}