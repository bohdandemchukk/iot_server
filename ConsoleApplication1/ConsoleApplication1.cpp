#include <iostream>
#include <mqtt/async_client.h>
#include <cstdlib>
#include <thread>
#include <chrono>

const std::string SERVER_URL{"ssl://4c6fd5b08f67481cb2b10335dd04aeba.s1.eu.hivemq.cloud:8883"};
const std::string CLIENT_ID{ "cpp-server" };
const std::string TOPIC{ "project/weather" };
const char* username_env { std::getenv("MQTT_USERNAME") };
const char* password_env { std::getenv("MQTT_PASSWORD") };

const std::string USERNAME{ username_env };
const std::string PASSWORD{ password_env };


class Callback: public virtual mqtt::callback {
public:
	void message_arrived(mqtt::const_message_ptr msg) override {
		std::cout << "Message received: " << msg->to_string() << '\n';
	}
};
int main() {

	mqtt::async_client client{ SERVER_URL, CLIENT_ID };
	mqtt::connect_options con_opts{};
	mqtt::ssl_options ssl_opts{};

	ssl_opts.set_verify(true);
	con_opts.set_ssl(ssl_opts);
	con_opts.set_user_name(USERNAME);
	con_opts.set_password(PASSWORD);

	Callback cb{};
	client.set_callback(cb);
	
	try {
		std::cout << "Connecting..." << '\n';
		client.connect(con_opts)->wait();
		std::cout << "Connected!" << '\n';

		client.subscribe(TOPIC, 1)->wait();

		std::cout << "Subscribed to topic: " << TOPIC << '\n';

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