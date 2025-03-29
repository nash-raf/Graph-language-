#include <variant>
#include <string>
#include <iostream>


class utilities
{
    public:
    utilities() = delete;
    static std::string nodeTypeToString(const std::variant<int, double, char, bool, std::string>& value) {
        return std::visit([](auto&& v) -> std::string {
            using U = std::decay_t<decltype(v)>;
            if constexpr (std::is_same_v<U, std::string>) {
                return v;
            } else if constexpr (std::is_same_v<U, char>) {
                return std::string(1, v);
            } else if constexpr (std::is_same_v<U, bool>) {
                return v ? "true" : "false";
            } else if constexpr (std::is_arithmetic_v<U>) {
                return std::to_string(v);
            } else {
                return {}; // Fallback for unknown types
            }
        }, value);
    }

};
