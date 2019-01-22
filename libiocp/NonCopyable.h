#ifndef __UTILS_NON_COPYABLE_H__
#define __UTILS_NON_COPYABLE_H__

namespace Utils
{
    class NonCopyable
    {
    protected:
        constexpr NonCopyable() = default;
        ~NonCopyable() = default;
        NonCopyable(const NonCopyable &) = delete;
        NonCopyable &operator= (const NonCopyable &) = delete;
    };
} // namespace Utils

#endif // !__UTILS_NON_COPYABLE_H__
