#pragma once

#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <new>
#include <stdexcept>
#include <string_view>
#include <type_traits>

template <typename HEAD, typename ITEM>
class C_BLOCK
{
public:
    static_assert(std::is_trivially_copyable_v<HEAD> && std::is_trivially_destructible_v<HEAD>);
    static_assert(std::is_trivially_copyable_v<ITEM> && std::is_trivially_destructible_v<ITEM>);

    C_BLOCK(size_t count, size_t text_size)
        : items_offset_(align(sizeof(HEAD), alignof(ITEM)))
    {
        const size_t limit = std::numeric_limits<size_t>::max();
        if (count > (limit - items_offset_) / sizeof(ITEM)) {
            throw std::bad_alloc();
        }
        text_used_ = items_offset_ + sizeof(ITEM) * count;
        if (text_size > limit - text_used_) {
            throw std::bad_alloc();
        }
        size_ = text_used_ + text_size;
        data_ = static_cast<unsigned char *>(std::calloc(1, size_));
        if (data_ == nullptr) {
            throw std::bad_alloc();
        }
        head_ = reinterpret_cast<HEAD *>(data_);
        if (count > 0) {
            items_ = reinterpret_cast<ITEM *>(data_ + items_offset_);
        }
    }

    ~C_BLOCK()
    {
        std::free(data_);
    }

    C_BLOCK(const C_BLOCK &) = delete;
    C_BLOCK &operator=(const C_BLOCK &) = delete;

    HEAD &head()
    {
        return *head_;
    }

    ITEM *items()
    {
        return items_;
    }

    const char *copy(std::string_view text)
    {
        if (data_ == nullptr || text.size() >= size_ - text_used_) {
            throw std::logic_error("result text does not fit");
        }
        char *start = reinterpret_cast<char *>(data_ + text_used_);
        if (!text.empty()) {
            std::memcpy(start, text.data(), text.size());
        }
        start[text.size()] = '\0';
        text_used_ += text.size() + 1;
        return start;
    }

    HEAD *release()
    {
        data_ = nullptr;
        return head_;
    }

private:
    static size_t align(size_t offset, size_t alignment)
    {
        return (offset + alignment - 1) / alignment * alignment;
    }

    unsigned char *data_ = nullptr;
    HEAD *head_ = nullptr;
    ITEM *items_ = nullptr;
    size_t items_offset_ = 0;
    size_t text_used_ = 0;
    size_t size_ = 0;
};
