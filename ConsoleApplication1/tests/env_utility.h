#ifndef ENV_UTILITY_H
#define ENV_UTILITY_H

#include <cstdlib>
#include <format>
#include <expected>
#include <string>
#include <string_view>


namespace env {
    inline std::expected<std::string, std::string> read_env(std::string_view env) {
        if (const char* value {std::getenv(env.data())}) {
            return std::string{value};
        }
        return std::unexpected(std::format("Missing required environment variable: {}", env));
    }

    inline std::string require(std::string_view env) {
        auto result {read_env(env)};
        if (!result) {
            throw std::runtime_error(result.error());
        }
        return *result;
    }

    inline std::string read_env_or_default(std::string_view env, std::string_view def) {
        auto result {read_env(env)};
        if (result) {
            return *result;
        }

        return std::string(def);
    }
}

#endif