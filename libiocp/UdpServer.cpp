#include "UdpServer.h"
#include "Context.h"
#include "Socket.h"
#include "Log.h"

namespace Networking
{
    UdpServer::UdpServer()
        :m_context(new Context())
        , m_connection(nullptr)
    {
        m_context->SetOnConnectedCallback(std::bind(&UdpServer::OnConnected, this, std::placeholders::_1));
        m_context->SetOnDisconnectedCallback(std::bind(&UdpServer::OnDisconnected, this, std::placeholders::_1));
        m_context->SetOnReadCallback(std::bind(&UdpServer::OnRead, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        m_context->SetOnWriteCallback(std::bind(&UdpServer::OnWrite, this, std::placeholders::_1, std::placeholders::_2));
    }

    UdpServer::~UdpServer()
    {
        Stop();
    }

    bool UdpServer::Init(const char * address, unsigned port)
    {
        // Initial context
        if (!m_context->Init(address, port, E_PROTO_UDP))
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

    bool UdpServer::AsyncWrite(void * data, std::size_t size)
    {
        if (m_connection.get() != nullptr)
            return m_context->AsyncSendTo(m_connection.get(), data, size, m_context->GetSocket().GetAddress());
        return false;
    }

    bool UdpServer::AsyncRead()
    {
        if (m_connection.get() != nullptr)
            return m_context->AsyncRecvFrom(m_connection.get());
        return false;
    }

    void UdpServer::Start()
    {
        m_context->Start();
    }

    void UdpServer::Stop()
    {
        m_context->Stop();
    }

    void UdpServer::OnConnected(const Connection * conn)
    {
        LOG_DEBUG("Connect to server successed.");
    }

    void UdpServer::OnDisconnected(const Connection *)
    {
        LOG_DEBUG("Connection disconnected");
    }

    void UdpServer::OnRead(const Connection * conn, void * data, std::size_t size)
    {
        LOG_DEBUG("Got new data. Size: " << size << ", data: " << data);
    }

    void UdpServer::OnWrite(const Connection * conn, std::size_t bytes_transferred)
    {
        LOG_DEBUG("Successfully written " << bytes_transferred << " bytes");
    }

} // namespace Networking
