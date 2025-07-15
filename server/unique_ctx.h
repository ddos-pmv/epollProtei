#pragma once

#include "unique_fd.h"
#include <deque>

namespace protei
{
    class UniqueCtx
    {
    public:
        explicit UniqueCtx(int fd, size_t initial_buf_size = 1024)
            : fd_(fd), buffer_(initial_buf_size)
        {
        }

        ~UniqueCtx() = default;
        UniqueCtx(const UniqueCtx &) = delete;
        UniqueCtx &operator=(const UniqueCtx &) = delete;
        UniqueCtx(UniqueCtx &&) = default;
        UniqueCtx &operator=(UniqueCtx &&) = default;

        int fd()
        {
            return fd_.get();
        }

        std::vector<char> &buffer() { return buffer_; }
        const std::vector<char> &buffer() const { return buffer_; }

    private:
        UniqueFd fd_;
        std::vector<char> buffer_;
    };

    using CtxSharedPtr = std::shared_ptr<UniqueCtx>;
}