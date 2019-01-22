#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "Socket.h"
#include "Exceptions.h"
#include "Guard.h"
#include "Connection.h"
#include "Log.h"


namespace Networking
{

    Socket::Socket()
        : m_socket(), m_inited(false)
    {
    }
    Socket::~Socket()
    {
        m_inited = false;
    }

    bool Socket::Init(const char *address, unsigned int port, EProtoType proto)
    {
        if (m_inited)
            return true;

        if (address == nullptr)
            return false;

        m_socket = WSASocketW(AF_INET, proto == E_PROTO_TCP ? SOCK_STREAM : SOCK_DGRAM,
            proto == E_PROTO_TCP ? IPPROTO_TCP : IPPROTO_UDP, NULL, 0, WSA_FLAG_OVERLAPPED);
        if (m_socket == INVALID_SOCKET)
        {
            LOG_ERROR("Failed to initialize socket");
            return false;
        }

        // if connection exit it will be auto close socket
        Utils::Guard socket_guard([&] { closesocket(m_socket); });

        unsigned long resolved_address = ResolveAddress(address);
        if (resolved_address == INADDR_NONE)
        {
            LOG_ERROR("Failed to resolve address");
            return false;
        }

        memset(&m_socket_address, 0, sizeof(m_socket_address));
        m_socket_address.sin_family = AF_INET;
        m_socket_address.sin_addr.s_addr = resolved_address;
        m_socket_address.sin_port = htons(port);

        socket_guard.Release();
        m_inited = true;

        return true;
    }

    bool Socket::BindForListen()
    {
        if (!m_inited)
        {
            LOG_ERROR("Socket must be initialized before Bind() call");
            return false;
        }

        const unsigned so_reuseaddr = 1;
        if (setsockopt(
            m_socket, SOL_SOCKET, SO_REUSEADDR,
            reinterpret_cast<const char*>(&so_reuseaddr),
            sizeof(so_reuseaddr)) != 0)
        {
            LOG_ERROR("Failed to reuse address");
            return false;
        }

        if (bind(m_socket, reinterpret_cast<SOCKADDR*>(&m_socket_address), sizeof(m_socket_address)) != 0)
        { 
            LOG_ERROR("Failed to bind socket to address/port");
            return false;
        }
            
        return true;
    }

    bool Socket::BindForConnect()
    {
        // ConnectEx requires the socket to be initially bound
        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));

        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = 0;

        if (bind(m_socket, reinterpret_cast<SOCKADDR*>(&addr), sizeof(addr)) != 0)
        {
            LOG_ERROR("Failed to bound socket");
            return false;
        }
        return true;
    }

    unsigned long Socket::ResolveAddress(const char *address)
    {
        if (!strlen(address))
            return INADDR_ANY;

        hostent* host = gethostbyname(address);
        return inet_addr(inet_ntoa(*reinterpret_cast<struct in_addr *>(*host->h_addr_list)));
    }

} // namespace Networking
