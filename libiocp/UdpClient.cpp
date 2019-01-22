#include "UdpClient.h"
#include "Context.h"
#include "Socket.h"
#include "Log.h"

namespace Networking
{
    UdpClient::UdpClient()
        :m_context(new Context())
        , m_connection(nullptr)
    {
        m_context->SetOnConnectedCallback(std::bind(&UdpClient::OnConnected, this, std::placeholders::_1));
        m_context->SetOnDisconnectedCallback(std::bind(&UdpClient::OnDisconnected, this, std::placeholders::_1));
        m_context->SetOnReadCallback(std::bind(&UdpClient::OnRead, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        m_context->SetOnWriteCallback(std::bind(&UdpClient::OnWrite, this, std::placeholders::_1, std::placeholders::_2));
    }

    UdpClient::~UdpClient()
    {
        Stop();
    }

    bool UdpClient::Init(const char * address, unsigned port)
    {
        // Initial context
        if (!m_context->Init(address, port, E_PROTO_UDP))
            return false;

        m_connection.reset(new Connection(m_context->GetSocket().Native()));
        if (!AsyncRead())
        {
            return false;
        }
        return true;
    }

    bool UdpClient::AsyncWrite(void * data, std::size_t size)
    {
        if (m_connection.get() != nullptr)
            return m_context->AsyncSendTo(m_connection.get(), data, size, m_context->GetSocket().GetAddress());
        return false;
    }

    bool UdpClient::AsyncRead()
    {
        if (m_connection.get() != nullptr)
            return m_context->AsyncRecvFrom(m_connection.get());
        return false;
    }

    void UdpClient::Start()
    {
        m_context->Start();
    }

    void UdpClient::Stop()
    {
        m_context->Stop();
    }

    void UdpClient::OnConnected(const Connection * conn)
    {
        LOG_DEBUG("Connect to server successed.");
    }

    void UdpClient::OnDisconnected(const Connection *)
    {
        LOG_DEBUG("Connection disconnected");
    }

    void UdpClient::OnRead(const Connection * conn, void * data, std::size_t size)
    {
        LOG_DEBUG("Got new data. Size: " << size << ", data: " << data);
    }

    void UdpClient::OnWrite(const Connection * conn, std::size_t bytes_transferred)
    {
        LOG_DEBUG("Successfully written " << bytes_transferred << " bytes");
    }

} // namespace Networking
