#include "../include/WeatherCache.h"
#include "../include/WeatherData.h"
#include <optional>

void WeatherCache::update(WeatherData data) {
    std::unique_lock lock{m_mutex};
    m_latest = std::move(data);
}

std::optional<WeatherData> WeatherCache::get() const { 
    std::shared_lock lock{m_mutex};
    return m_latest;
}