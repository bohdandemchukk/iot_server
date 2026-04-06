#ifndef WEATHERCACHE_H
#define WEATHERCACHE_H

#include <optional>
#include <string>
#include <mutex>

struct WeatherData {
    float temperature{};
    float humidity{};
    float pressure{};
    std::string raw{};
};

class WeatherCache {

private:
    std::optional<WeatherData> m_latest{};
    std::mutex m_mutex{};

public:
    void update(const WeatherData& data);
    std::optional<WeatherData> get();
};

#endif