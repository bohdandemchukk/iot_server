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

const std::string USERNAME{ username_env };
const std::string PASSWORD{ password_env };



int main() {

	mqtt::async_client client{ SERVER_URL, CLIENT_ID };
	mqtt::connect_options con_opts{};
	mqtt::ssl_options ssl_opts{};

	ssl_opts.set_verify(true);
	con_opts.set_ssl(ssl_opts);
	con_opts.set_user_name(USERNAME);
	con_opts.set_password(PASSWORD);

	asio::io_context io_context;
	
	WeatherCache cache{};

	HttpServer httpServer{io_context, 8080, cache};
	InfluxWriter writer{io_context, "localhost", "8181", "weather_db"};
	Callback cb{writer, cache};
	client.set_callback(cb);
	
	try {
		std::cout << "Connecting to MQTT..." << '\n';
		client.connect(con_opts)->wait();
		std::cout << "Connected to MQTT!" << '\n';

		client.subscribe(TOPIC, 1)->wait();

		std::cout << "Subscribed to MQTT topic: " << TOPIC << '\n';
		httpServer.run();
		while (true) {
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
	}
	catch (const mqtt::exception& exception) {
		std::cerr << exception.what() << '\n';
		return 1;
	}

	client.disconnect()->wait();
	return 0;
}