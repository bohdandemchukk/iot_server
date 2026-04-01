#ifndef INFLUXWRITER_H
#define INFLUXWRITER_H

#include <string>
#include <iostream>

class InfluxWriter {
public:
	void write(const std::string& line);
};

#endif