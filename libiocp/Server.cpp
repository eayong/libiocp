#include "Server.h"
#include "Context.h"
#include "Log.h"
#include <WS2tcpip.h>

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
        if (!m_context->Init(address, port, E_PROTO_TCP))
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

    bool Server::AsyncBroadcast(void * data, std::size_t size)
    {
        std::unique_lock<std::recursive_mutex> lock(m_mutex);
        for (auto &item : m_conns)
        {
            m_context->AsyncWrite(item.second.conn, data, size);
        }
        return true;
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

    void Server::Status()
    {
        std::unique_lock<std::recursive_mutex> lock(m_mutex);
        std::cout << "total clients: " << m_conns.size() << std::endl << std::endl;
        for (auto &item : m_conns)
        {
            std::cout << "socketid: " << item.second.conn->GetSocket() << std::endl;
            std::cout << "address: " << item.second.address.c_str()
                << ":" << item.second.port << std::endl << std::endl;
        }
    }

    void Server::GetClients(std::list<ClientInfo>& clients)
    {
        std::unique_lock<std::recursive_mutex> lock(m_mutex);
        for (auto &item : m_conns)
        {
            clients.push_back(item.second);
        }
    }

    void Server::OnConnected(const Connection * conn)
    {
        LOG_DEBUG("New client connected");
        if (conn == nullptr)
            return;

        auto mutable_conn = const_cast<Networking::Connection*>(conn);
        ClientInfo info;
        sockaddr_in remoteaddr;
        int len = sizeof(remoteaddr);
        memset(&remoteaddr, 0, sizeof(remoteaddr));
        if (setsockopt(mutable_conn->GetSocket(),
            SOL_SOCKET,
            SO_UPDATE_ACCEPT_CONTEXT,
            (char *)&m_context->GetSocket().Native(),
            sizeof(m_context->GetSocket().Native())) != 0)
        {
            LOG_ERROR("Failed to setsockopt SO_UPDATE_ACCEPT_CONTEXT");
            return;
        }
        if (getpeername(mutable_conn->GetSocket(), (sockaddr*)&remoteaddr, &len) == 0)
        {
            char str[INET_ADDRSTRLEN] = { 0 };
            inet_ntop(AF_INET, &remoteaddr.sin_addr, str, sizeof(str));
            info.address = str;
            info.port = ntohs(remoteaddr.sin_port);
        }
        info.conn = mutable_conn;
        std::unique_lock<std::recursive_mutex> lock(m_mutex);
        m_conns[mutable_conn->GetSocket()] = info;
    }

    void Server::OnDisconnected(const Connection * conn)
    {
        LOG_DEBUG("Client disconnected, error: " << WSAGetLastError());
        if (conn == nullptr)
            return;
        auto mutable_conn = const_cast<Networking::Connection*>(conn);

        std::unique_lock<std::recursive_mutex> lock(m_mutex);
        m_conns.erase(mutable_conn->GetSocket());
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
