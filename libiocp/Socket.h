#ifndef __SOCKET_H__
#define __SOCKET_H__

#include <WinSock2.h>
#include "NonCopyable.h"

namespace Networking
{

    class Socket : Utils::NonCopyable
    {
    public:
        Socket();
        ~Socket();

    public:
        // Initialize the socket and address
        bool Init(const char *address, unsigned int port);

        // Returns native socket
        inline SOCKET& Native();

        // Returns address associated with the socket
        inline sockaddr_in& GetAddress();

        // Bind socket to address. User when the socket is listening (server side)
        bool BindForListen();
        
        // Bind socket to NULL address. User when the socket is connecting to the server (client side)
        bool BindForConnect();

    private:
        // Resolv string to really address
        unsigned long ResolveAddress(const char *address);

    private:
        // Native socket
        SOCKET m_socket;

        // Native address
        sockaddr_in m_socket_address;

        // Flag the socket was initialized
        bool m_inited;
    };

    inline SOCKET & Socket::Native()
    {
        return m_socket;
    }

    inline sockaddr_in & Socket::GetAddress()
    {
        return m_socket_address;
    }

} // namespace Networking

#endif // !__SOCKET_H__
