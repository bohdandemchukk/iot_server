#ifndef WEATHERCACHE_H
#define WEATHERCACHE_H

#include <optional>
#include <string>
#include <shared_mutex>
#include <mutex>
#include "WeatherData.h"

class WeatherCache {

public:
    void update(WeatherData data);
    std::optional<WeatherData> get() const;

private:
    std::optional<WeatherData> m_latest{};
    mutable std::shared_mutex m_mutex{};

};

#endif