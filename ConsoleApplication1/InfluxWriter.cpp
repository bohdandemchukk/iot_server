#include "InfluxWriter.h"


void InfluxWriter::write(const std::string& line) {
	std::string command{ "influxdb3 write --database weather_db \"" + line + "\"" };

	int result{ std::system(command.c_str()) };

	if (result == 0) {
		std::cout << "Wrote to InfluxDB: " << line << '\n';
	}
	else {
		std::cerr << "Failed to write to InfluxDB" << '\n';
	}
}