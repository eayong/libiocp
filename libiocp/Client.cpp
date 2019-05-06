#include "Client.h"
#include "Context.h"
#include "Log.h"

namespace Networking
{
    Client::Client()
        :m_context(new Context())
        , m_connetor(new Connector(*m_context.get()))
        , m_connection(nullptr)
    {
        m_context->SetOnConnectedCallback(std::bind(&Client::OnInnerConnected, this, std::placeholders::_1));
        m_context->SetOnDisconnectedCallback(std::bind(&Client::OnInnerDisconnected, this, std::placeholders::_1));
        m_context->SetOnReadCallback(std::bind(&Client::OnRead, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        m_context->SetOnWriteCallback(std::bind(&Client::OnWrite, this, std::placeholders::_1, std::placeholders::_2));
    }

    Client::~Client()
    {
        Stop();
    }

    bool Client::Init(const char * address, unsigned port)
    {
        // Initial context
        if (!m_context->Init(address, port, E_PROTO_TCP))
            return false;

        // Initial acceptor 
        if (!m_connetor->Init() || !m_connetor->Start())
            return false;

        return true;
    }

    bool Client::AsyncWrite(void * data, std::size_t size)
    {
        if (m_connection.get() != nullptr)
            return m_context->AsyncWrite(m_connection.get(), data, size);
        return false;
    }

    bool Client::AsyncRead()
    {
        if (m_connection.get() != nullptr)
            return m_context->AsyncRead(m_connection.get());
        return false;
    }

    void Client::Start()
    {
        m_context->Start();
    }

    void Client::Stop()
    {
        m_context->Stop();
    }

    void Client::OnConnected(const Connection * conn)
    {
        LOG_DEBUG("Connect to server successed.");
    }

    void Client::OnDisconnected(const Connection *)
    {
        LOG_DEBUG("Connection disconnected, error: " << WSAGetLastError() );
    }

    void Client::OnRead(const Connection * conn, void * data, std::size_t size)
    {
        LOG_DEBUG("Got new data. Size: " << size << ", data: " << data);
    }

    void Client::OnWrite(const Connection * conn, std::size_t bytes_transferred)
    {
        LOG_DEBUG("Successfully written " << bytes_transferred << " bytes");
    }

    void Client::OnInnerConnected(const Networking::Connection * conn)
    {
        auto mutable_conn = const_cast<Connection*>(conn);
        m_connection.reset(new Connection(mutable_conn->GetSocket()));
        OnConnected(conn);
    }

    void Client::OnInnerDisconnected(const Networking::Connection * conn)
    {
        m_connection.release();
        OnDisconnected(conn);
    }

} // namespace Networking
