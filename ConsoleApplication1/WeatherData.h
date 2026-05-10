#ifndef WEATHERDATA_H
#define WEATHERDATA_H

#include <string>

struct WeatherData {
    float temperature{};
    float humidity{};
    float pressure{};
    std::string raw{};
};

#endif