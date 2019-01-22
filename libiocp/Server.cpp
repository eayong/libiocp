#include "Server.h"
#include "Context.h"
#include "Log.h"

namespace Networking
{
    Server::Server()
        :m_context(new Context())
        , m_acceptor(new Acceptor(*m_context.get()))
    {
        m_context->SetOnWriteCallback(std::bind(&Server::OnWrite, this, std::placeholders::_1, std::placeholders::_2));
        m_context->SetOnReadCallback(std::bind(&Server::OnRead, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        m_context->SetOnConnectedCallback(std::bind(&Server::OnConnected, this, std::placeholders::_1));
        m_context->SetOnDisconnectedCallback(std::bind(&Server::OnDisconnected, this, std::placeholders::_1));
    }

    Server::~Server()
    {
        Stop();
    }

    bool Server::Init(const char * address, unsigned port)
    {
        // Initial context
        if (!m_context->Init(address, port))
            return false;

        // Initial acceptor 
        if (!m_acceptor->Init() || !m_acceptor->Start())
            return false;

        return true;
    }

    bool Server::AsyncWrite(const Connection * conn, void * data, std::size_t size)
    {
        return m_context->AsyncWrite(conn, data, size);
    }

    bool Server::AsyncRead(const Connection * conn)
    {
        return m_context->AsyncRead(conn);
    }

    void Server::Start()
    {
        m_context->Start();
    }

    void Server::Stop()
    {
        m_context->Stop();
    }

    void Server::OnConnected(const Connection * conn)
    {
        LOG_DEBUG("New client connected");
    }

    void Server::OnDisconnected(const Connection *)
    {
        LOG_DEBUG("Client disconnected");
    }

    void Server::OnRead(const Connection * conn, void * data, std::size_t size)
    {
        LOG_DEBUG("Got new data. Size: " << size << ", data: " << data);
    }

    void Server::OnWrite(const Connection * conn, std::size_t bytes_transferred)
    {
        LOG_DEBUG("Successfully written " << bytes_transferred << " bytes");
    }

} // namespace Networking
