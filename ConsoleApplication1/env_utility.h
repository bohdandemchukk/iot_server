#ifndef ENV_UTILITY_H
#define ENV_UTILITY_H

#include <cstdlib>
#include <format>
#include <expected>
#include <string>
#include <string_view>

inline std::expected<std::string, std::string> read_env(std::string_view env) {
    if (const char* value {std::getenv(env.data())}) {
        return std::string{value};
    }

    return std::unexpected(std::format("Missing required environment variable: {}", env));
}

#endif