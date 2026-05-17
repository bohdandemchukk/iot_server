#ifndef CALLBACK_H
#define CALLBACK_H

#include "WeatherCache.h"
#include <mqtt/async_client.h>
#include "InfluxWriter.h"

class Callback : public virtual mqtt::callback {
public:

	Callback(asio::io_context& io_context, InfluxWriter& writer, WeatherCache& cache);
	void message_arrived(mqtt::const_message_ptr msg) override;

private:
	asio::io_context& m_io_context;
	InfluxWriter& m_writer;
	WeatherCache& m_cache;
};

#endif