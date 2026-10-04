#pragma once

#include <cstddef>
#include <span>
#include <string>
#include <string_view>

namespace erc {

class resource {
    std::string name_;
    std::string path_;
    std::span<const std::byte> data_;
public:
    constexpr explicit resource(std::string_view name, std::string_view path, std::span<const std::byte> data)
        : name_(name), path_(path), data_(data) {}

    [[nodiscard]] std::span<const std::byte> data() const { return data_; }
    [[nodiscard]] std::size_t size() const { return data_.size(); }

    [[nodiscard]] std::string_view name() const { return name_; }
    [[nodiscard]] std::string_view path() const { return path_; }
};

}
