#ifndef __WIN_SOCK_INITIALIZER_H__
#define __WIN_SOCK_INITIALIZER_H__

#include "NonCopyable.h"
#include <WinSock2.h>

#pragma comment(lib, "ws2_32.lib")

// This class helps initialize winsock
class WinSockInitializer : Utils::NonCopyable
{
public:
    WinSockInitializer()
        : m_version_hi(2), m_version_low(2)
    {
        m_wsa_inited = !WSAStartup(MAKEWORD(m_version_hi, m_version_low), &m_wsa_data);
    }

    bool IsInitialized()
    {
        return m_wsa_inited && LOBYTE(m_wsa_data.wVersion) == m_version_low && HIBYTE(m_wsa_data.wVersion) == m_version_hi;
    }

    ~WinSockInitializer()
    {
        if (m_wsa_inited)
        {
            WSACleanup();
            m_wsa_inited = false;
        }
    }

private:
    // Flag of successfull WSA startup
    bool m_wsa_inited;

    // Hi version of WSA
    unsigned m_version_hi;

    // Low version of WSA
    unsigned m_version_low;

    // WSA init data
    WSADATA m_wsa_data;
};

#endif // !__WIN_SOCK_INITIALIZER_H__

