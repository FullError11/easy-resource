#pragma once

#include <cstdint>
#include <string>

struct Version {
    std::uint16_t major;
    std::uint16_t minor;
    std::uint32_t patch;

    constexpr explicit Version(std::uint16_t major, std::uint16_t minor, std::uint32_t patch)
        : major(major), minor(minor), patch(patch) {}

    [[nodiscard]] std::string to_string() const {
        return std::string{"v"}
            .append(std::to_string(major))
            .append(".")
            .append(std::to_string(minor))
            .append(".")
            .append(std::to_string(patch));
    }

    [[nodiscard]] constexpr std::uint64_t to_u64() const {
        return (static_cast<std::uint64_t>(major) << 48)
             | (static_cast<std::uint64_t>(minor) << 32)
             | static_cast<std::uint64_t>(patch);
    }
};

namespace ercc {
    inline constexpr Version VERSION = Version(0, 1, 0);
}

