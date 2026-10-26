#pragma once

#include <cstdint>
#include <optional>
#include <utility>
#include <vector>

#include "engine/Types.hpp"

namespace engine {

// Slot storage addressed by generation-checked handles.
// Freed slots are recycled; their generation is bumped so old handles to the
// slot stop resolving.
template <typename Tag, typename T>
class HandlePool {
public:
    using HandleType = Handle<Tag>;

    HandleType insert(T value)
    {
        std::uint32_t index = 0;
        if (!freeList_.empty()) {
            index = freeList_.back();
            freeList_.pop_back();
        } else {
            index = static_cast<std::uint32_t>(slots_.size());
            slots_.emplace_back();
        }
        Slot& slot = slots_[index];
        slot.value = std::move(value);
        slot.alive = true;
        return HandleType{index, slot.generation};
    }

    [[nodiscard]] T* get(HandleType handle)
    {
        if (!contains(handle)) {
            return nullptr;
        }
        return &*slots_[handle.index].value;
    }

    [[nodiscard]] bool contains(HandleType handle) const
    {
        return handle.valid() && handle.index < slots_.size() && slots_[handle.index].alive &&
               slots_[handle.index].generation == handle.generation;
    }

    // Removes the value and returns it so the caller can release it.
    std::optional<T> remove(HandleType handle)
    {
        if (!contains(handle)) {
            return std::nullopt;
        }
        Slot& slot = slots_[handle.index];
        std::optional<T> value = std::move(slot.value);
        slot.value.reset();
        slot.alive = false;
        if (++slot.generation == 0) {
            slot.generation = 1;  // skip 0, which marks a null handle
        }
        freeList_.push_back(handle.index);
        return value;
    }

    [[nodiscard]] std::size_t size() const { return slots_.size() - freeList_.size(); }

    // Visits every live value, e.g. to release them all on shutdown.
    template <typename Fn>
    void forEach(Fn&& fn)
    {
        for (Slot& slot : slots_) {
            if (slot.alive) {
                fn(*slot.value);
            }
        }
    }

    void clear()
    {
        slots_.clear();
        freeList_.clear();
    }

private:
    struct Slot {
        std::optional<T> value;
        std::uint32_t generation = 1;
        bool alive = false;
    };

    std::vector<Slot> slots_;
    std::vector<std::uint32_t> freeList_;
};

} // namespace engine
