#include "Udp.h"
#include "Context.h"
#include "Socket.h"
#include "Log.h"

namespace Networking
{
    UDP::UDP()
        :m_context(new Context())
        , m_connection(nullptr)
    {
        m_context->SetOnRecvFromCallback(std::bind(&UDP::OnRecvFrom, this,
            std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
        m_context->SetOnSendToCallback(std::bind(&UDP::OnSendTo, this,
            std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    }

    UDP::~UDP()
    {
        Stop();
    }

    bool UDP::Init(const char * local_addr, unsigned local_port)
    {
        // Initial context
        if (!m_context->Init(local_addr, local_port, E_PROTO_UDP))
            return false;

        if (!m_context->GetSocket().BindForListen())
            return false;

        m_connection.reset(new Connection(m_context->GetSocket().Native()));

        if (!AsyncRead())
        {
            return false;
        }
        return true;
    }

    bool UDP::AsyncWrite(void * data, std::size_t size,
        const char * remote_addr, unsigned remote_port)
    {
        SOCKADDR_IN sock_addr;
        unsigned long resolved_address = m_socket.ResolveAddress(remote_addr);
        if (resolved_address == INADDR_NONE)
        {
            LOG_ERROR("Failed to resolve address");
            return false;
        }

        memset(&sock_addr, 0, sizeof(sock_addr));
        sock_addr.sin_family = AF_INET;
        sock_addr.sin_addr.s_addr = resolved_address;
        sock_addr.sin_port = htons(remote_port);
        if (m_connection.get() != nullptr)
            return m_context->AsyncSendTo(m_connection.get(), data, size, sock_addr);
        return false;
    }

    bool UDP::AsyncRead()
    {
        if (m_connection.get() != nullptr)
            return m_context->AsyncRecvFrom(m_connection.get());
        return false;
    }

    void UDP::Start()
    {
        m_context->Start();
    }

    void UDP::Stop()
    {
        m_context->Stop();
    }

    void UDP::OnRecvFrom(const Connection * conn, void * data, std::size_t size, sockaddr *addr)
    {
        if (conn == nullptr || data == nullptr || addr == nullptr)
            return;
    }

    void UDP::OnSendTo(const Connection * conn, std::size_t bytes_transferred, sockaddr *addr)
    {
        //LOG_DEBUG("Successfully written " << bytes_transferred << " bytes");
    }

} // namespace Networking
