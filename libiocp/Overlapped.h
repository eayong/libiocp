#ifndef __NETWORKING_OVERLAPPED_H__
#define __NETWORKING_OVERLAPPED_H__


#include "NonCopyable.h"
#include <WinSock2.h>

namespace Networking
{

    class Connection;

    // Overlapped structure, extended with helper fields
    struct Overlapped : Utils::NonCopyable
    {
        enum Type
        {
            Connect,
            Accept,
            Read,
            Write,
            RecvFrom,
            SendTo
        };

        // Native overlapped
        WSAOVERLAPPED overlapped;

        // Type of overlapped. Context used this type to resolve operation
        Type type;

        // Connection associated with current async operation
        Connection* connection;

        // Current winsock buffers
        WSABUF wsa_buf;

        // for udp
        SOCKADDR_IN remote_addr;
        int remote_addr_len;
    };

    // Create and prepare custom overlapped
    inline Overlapped* CreateOverlapped(Overlapped::Type type)
    {
        Overlapped* overlapped = new Overlapped;
        memset(overlapped, 0, sizeof(Overlapped));
        overlapped->type = type;
        overlapped->remote_addr_len = sizeof(overlapped->remote_addr);
        return overlapped;
    }

} // namespace Networking

#endif // !__NETWORKING_OVERLAPPED_H__

