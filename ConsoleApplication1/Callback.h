#ifndef CALLBACK_H
#define CALLBACK_H

#include <mqtt/async_client.h>
#include "InfluxWriter.h"

class Callback : public virtual mqtt::callback {
public:

	Callback(InfluxWriter& writer)
		: m_writer{ writer }
	{}

	void message_arrived(mqtt::const_message_ptr msg) override;

private:
	InfluxWriter& m_writer;
};

#endif