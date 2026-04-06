#include "Callback.h"
#include <string>

void Callback::message_arrived(mqtt::const_message_ptr msg) {
	
	std::string payload{ msg->to_string() };
	std::cout << "Message received: " << payload << '\n';

	WeatherData data{};
	data.raw = payload;
	m_cache.update(data);

	std::string influx_line{ "weather " + payload };

	m_writer.write(influx_line);
}