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
        m_context->SetOnRecvFromCallback(std::bind(&UdpClient::OnRecvFrom, this,
            std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
        m_context->SetOnSendToCallback(std::bind(&UdpClient::OnSendTo, this,
            std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
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

        if (!m_context->GetSocket().BindForConnect())
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

    void UdpClient::OnRecvFrom(const Connection * conn, void * data, std::size_t size, sockaddr *addr)
    {
        if (conn == nullptr || data == nullptr || addr == nullptr)
            return;
    }

    void UdpClient::OnSendTo(const Connection * conn, std::size_t bytes_transferred, sockaddr *addr)
    {
        //LOG_DEBUG("Successfully written " << bytes_transferred << " bytes");
    }

} // namespace Networking
