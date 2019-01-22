#ifndef __UTILS_GUARD_H__
#define __UTILS_GUARD_H__

#include "NonCopyable.h"
#include <functional>

namespace Utils
{
    // Resource guard based on RAII
    class Guard : NonCopyable
    {
    public:
        typedef std::function<void()> ScopeExitFunc;

        Guard(ScopeExitFunc func)
            : m_remover(func)
        {
        }

        ~Guard()
        {
            if (m_remover)
                m_remover();
        }

        void Release()
        {
            m_remover = nullptr;
        }

    private:
        ScopeExitFunc m_remover;
    };

} // namespace Utils

#endif // !__UTILS_GUARD_H__

