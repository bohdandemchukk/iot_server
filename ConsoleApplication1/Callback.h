#ifndef CALLBACK_H
#define CALLBACK_H

#include "WeatherCache.h"
#include <mqtt/async_client.h>
#include "InfluxWriter.h"

class Callback : public virtual mqtt::callback {
public:

	Callback(InfluxWriter& writer, WeatherCache& cache)
		: m_writer{ writer }, m_cache {cache}
	{}

	void message_arrived(mqtt::const_message_ptr msg) override;

private:
	InfluxWriter& m_writer;
	WeatherCache& m_cache;
};

#endif