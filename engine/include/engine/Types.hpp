#pragma once

#include <compare>
#include <cstdint>

namespace engine {

// Opaque, generation-checked handle to an engine-owned resource.
// A handle whose slot has been freed and reused no longer resolves, so stale
// references are detected instead of silently aliasing a new resource.
template <typename Tag>
struct Handle {
    std::uint32_t index = 0;
    std::uint32_t generation = 0;  // 0 is never issued: a default handle is null

    [[nodiscard]] constexpr bool valid() const { return generation != 0; }
    constexpr explicit operator bool() const { return valid(); }
    constexpr auto operator<=>(const Handle&) const = default;
};

using BufferHandle = Handle<struct BufferTag>;
using TextureHandle = Handle<struct TextureTag>;
using PipelineHandle = Handle<struct PipelineTag>;

} // namespace engine
