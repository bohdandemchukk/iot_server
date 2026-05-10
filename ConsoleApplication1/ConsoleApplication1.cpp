#include <iostream>
#include <cstdlib>
#include <thread>
#include <chrono>
#include "InfluxWriter.h"
#include "HttpServer.h"
#include "Callback.h"
#include "WeatherCache.h"

const std::string SERVER_URL{std::getenv("MQTT_URL")};
const std::string CLIENT_ID{ "cpp-server" };
const std::string TOPIC{ "project/weather" };
const char* username_env { std::getenv("MQTT_USERNAME") };
const char* password_env { std::getenv("MQTT_PASSWORD") };

const std::string INFLUX_HOST{ std::getenv("INFLUXDB_HOST") ? std::getenv("INFLUXDB_HOST") : "localhost" };
const std::string INFLUX_PORT{ std::getenv("INFLUXDB_PORT") ? std::getenv("INFLUXDB_PORT") : "8181" };
const std::string INFLUX_DB{ std::getenv("INFLUXDB_BUCKET") ? std::getenv("INFLUXDB_BUCKET") : "weather_db" };

const std::string USERNAME{ username_env ? username_env : "" };
const std::string PASSWORD{ password_env ? password_env : "" };



int main() {

	mqtt::async_client client{ SERVER_URL, CLIENT_ID };
	mqtt::connect_options con_opts{};
	mqtt::ssl_options ssl_opts{};

	ssl_opts.set_verify(true);
	con_opts.set_ssl(ssl_opts);
	con_opts.set_user_name(USERNAME);
	con_opts.set_password(PASSWORD);

	asio::io_context io_context;
	asio::io_context http_io_context;

	WeatherCache cache{};

	HttpServer httpServer{http_io_context, 8080, cache, INFLUX_HOST, INFLUX_PORT, INFLUX_DB};
	InfluxWriter writer{io_context, INFLUX_HOST, INFLUX_PORT, INFLUX_DB};
	Callback cb{io_context, writer, cache};
	client.set_callback(cb);
	
	try {
		std::cout << "Connecting to MQTT..." << '\n';
		client.connect(con_opts)->wait();
		std::cout << "Connected to MQTT!" << '\n';

		client.subscribe(TOPIC, 1)->wait();

		std::cout << "Subscribed to MQTT topic: " << TOPIC << '\n';
		std::thread http_thread([&]() { httpServer.run(); });
		http_thread.detach();
		auto work {asio::make_work_guard(io_context)};
		io_context.run();
	}
	catch (const mqtt::exception& exception) {
		std::cerr << exception.what() << '\n';
		return 1;
	}

	client.disconnect()->wait();
	return 0;
}