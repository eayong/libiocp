#ifndef __UTILS_SINGLETON_H__
#define __UTILS_SINGLETON_H__

#include "NonCopyable.h"
#include<mutex>
#include <memory>

namespace Utils
{
    template<typename T>
    class Singleton : NonCopyable
    {
    public:
        static T& Instance()
        {
            static std::once_flag oc;   //用于call_once的局部静态变量
            std::call_once(oc, [&] {  m_instance = new T(); });
            return *m_instance;
        }
    private:
        static T *m_instance;
    };

    template<typename T>
    T *Singleton<T>::m_instance = nullptr;
}

#endif // !__UTILS_SINGLETON_H__
