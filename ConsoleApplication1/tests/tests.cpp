#include <gtest/gtest.h>
#include "WeatherData.h"
#include "env_utility.h"
#include "WeatherCache.h"
#include "env_utility.h"
#include <thread>

TEST(WeatherCacheTest, EmptyOnStart) {
    WeatherCache cache{};
    EXPECT_FALSE(cache.get().has_value());
}

TEST(WeatherCacheTest, UpdateAndGet) {
    WeatherCache cache{};
    cache.update(WeatherData{.raw = "temperature=25.3"});
    ASSERT_TRUE(cache.get().has_value());
    EXPECT_EQ(cache.get()->raw, "temperature=25.3");
}

TEST(WeatherCacheTest, UpdateOverwritesPrevious) {
    WeatherCache cache{};
    cache.update(WeatherData{.raw = "first"});
    cache.update(WeatherData{.raw = "second"});
    EXPECT_EQ(cache.get()->raw, "second");
}

TEST(WeatherCacheTest, ConcurrentReadWrite) {
    WeatherCache cache{};
    std::vector<std::thread> threads;

    for (int i = 0; i < 10; i++) {
        threads.emplace_back([&cache, i]() {
            cache.update(WeatherData{.raw = std::to_string(i)});
        });
        threads.emplace_back([&cache]() {
            cache.get();
        });
    }
    for (auto& t : threads) t.join();
    EXPECT_TRUE(cache.get().has_value());
}


TEST(EnvTest, MissingVar) {
    auto result {env::read_env("NONEXISTENT_VAR_XYZ_123")};
    EXPECT_FALSE(result.has_value()); 
}

TEST(EnvTest, MissingVarErrorMessage) {
    auto result {env::read_env("NONEXISTENT_VAR_XYZ_123")};
    ASSERT_FALSE(result.has_value());
    EXPECT_TRUE(result.error().contains("NONEXISTENT_VAR_XYZ_123")); 
}

TEST(EnvTest, PresentVar) {
    setenv("TEST_VAR", "hello", 1);
    auto result {env::read_env("TEST_VAR")};
    ASSERT_TRUE(result.has_value());  
    EXPECT_EQ(result.value(), "hello"); 
    unsetenv("TEST_VAR");
}

// require — тестуємо через виняток
TEST(EnvTest, RequireThrowsOnMissing) {
    EXPECT_THROW(
        env::require("NONEXISTENT_VAR_XYZ_123"),
        std::runtime_error                       
    );
}

TEST(EnvTest, RequireReturnsValueWhenPresent) {
    setenv("TEST_VAR", "hello", 1);
    EXPECT_NO_THROW({
        auto result {env::require("TEST_VAR")};
        EXPECT_EQ(result, "hello");
    });
    unsetenv("TEST_VAR");
}

TEST(EnvTest, GetOrDefault) {
    auto result {env::read_env_or_default("NONEXISTENT_VAR_XYZ_123", "default")};
    EXPECT_EQ(result, "default");     
}

TEST(EnvTest, GetOrReturnsValueWhenPresent) {
    setenv("TEST_VAR", "hello", 1);
    auto result {env::read_env_or_default("TEST_VAR", "default")};
    EXPECT_EQ(result, "hello");       
    unsetenv("TEST_VAR");
}
