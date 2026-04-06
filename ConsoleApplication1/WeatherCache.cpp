#include "WeatherCache.h"


void WeatherCache::update(const WeatherData& data) {
    std::lock_guard lock(m_mutex);
    m_latest = data;
}

std::optional<WeatherData> WeatherCache::get() {
    std::lock_guard lock(m_mutex);
    return m_latest;
}