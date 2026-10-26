#pragma once

#include <filesystem>
#include <utility>

namespace engine {

// Resolves asset paths relative to the executable's directory.
// Texture caching and asset handles arrive with the asset milestone.
class Assets {
public:
    explicit Assets(std::filesystem::path root) : root_(std::move(root)) {}

    [[nodiscard]] const std::filesystem::path& root() const { return root_; }
    [[nodiscard]] std::filesystem::path resolve(const std::filesystem::path& relative) const
    {
        return root_ / relative;
    }

private:
    std::filesystem::path root_;
};

} // namespace engine
