#include "UdpServer.h"
#include "Context.h"
#include "Socket.h"
#include "Log.h"
#include <WS2tcpip.h>

namespace Networking
{
    UdpServer::UdpServer(time_t timeout)
        :m_context(new Context())
        , m_connection(nullptr)
        , m_timeout(timeout)
        , m_running(false)
    {
        m_context->SetOnRecvFromCallback(std::bind(&UdpServer::OnRecvFrom, this,
            std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
        m_context->SetOnSendToCallback(std::bind(&UdpServer::OnSendTo, this,
            std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
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

    bool UdpServer::AsyncWrite(void * data, std::size_t size, sockaddr_in *addr)
    {
        char str[INET_ADDRSTRLEN] = { 0 };
        char key[INET_ADDRSTRLEN + 8] = { 0 };
        inet_ntop(AF_INET, &addr->sin_addr, str, sizeof(str));
        snprintf(key, sizeof(key), "%s:%d", str, ntohs(addr->sin_port));
        auto it = m_conns.find(key);
        if (it != m_conns.end())
        {
            return m_context->AsyncSendTo(it->second.conn.get(), data, size, *addr);
        }
        else
        {
            UdpEndpoint info;
            info.address = str;
            info.port = ntohs(addr->sin_port);
            info.remoteaddr = *addr;
            info.conn.reset(new Connection(m_connection->GetSocket()));
            m_conns[key] = info;
            return m_context->AsyncSendTo(info.conn.get(), data, size, *addr);
        }
        return false;
    }

    bool UdpServer::AsyncBroadcast(void * data, std::size_t size)
    {
        std::unique_lock<std::recursive_mutex> lock(m_mutex);
        for (auto &item : m_conns)
        {
            m_context->AsyncSendTo(item.second.conn.get(), data, size, item.second.remoteaddr);
        }
        return true;
    }

    bool UdpServer::AsyncRead()
    {
        if (m_connection.get() != nullptr)
            return m_context->AsyncRecvFrom(m_connection.get());
        return false;
    }

    void UdpServer::Start()
    {
        if (m_running)
            return;
        m_running = true;
        // Start iocp context
        m_context->Start();
        // Start keepalive check service
        m_thread.reset(new std::thread(std::bind(&UdpServer::MainLoop, this, std::placeholders::_1), this));
    }

    void UdpServer::Stop()
    {
        if (m_running)
        {
            m_running = false;
            m_context->Stop();
            m_thread->join();
        }
    }

    void UdpServer::Status()
    {
        std::unique_lock<std::recursive_mutex> lock(m_mutex);
        std::cout << "total clients: " << m_conns.size() << std::endl << std::endl;
        for (auto &item : m_conns)
        {
            tm t;
            if (localtime_s(&t, &item.second.lastalive))
            {
                char lastalive[32] = { 0 };
                strftime(lastalive, sizeof(lastalive), "%Y-%m-%d %H:%M:%S", &t);
                std::cout << "last alive: " << lastalive << std::endl;
            }
            else
            {
                std::cout << "last alive: " << item.second.lastalive << std::endl;
            }
            std::cout << "address: " << item.second.address.c_str()
                << ":" << item.second.port << std::endl << std::endl;
        }
    }

    void UdpServer::GetUdpEndpoints(std::list<UdpEndpoint>& clients)
    {
        std::unique_lock<std::recursive_mutex> lock(m_mutex);
        for (auto &item : m_conns)
        {
            clients.push_back(item.second);
        }
    }

    void UdpServer::MainLoop(void * data)
    {
        while (m_running)
        {
            time_t now = time(nullptr);
            {
                std::unique_lock<std::recursive_mutex> lock(m_mutex);
                for (auto it = m_conns.begin(); it != m_conns.end();)
                {
                    if (now - it->second.lastalive > m_timeout)
                    {
                        it->second.conn->DetachedSocket();
                        it = m_conns.erase(it);
                        continue;
                    }
                    it++;
                }
            }
            Sleep(1000);
        }
    }

    void UdpServer::OnRecvFrom(const Connection * conn, void * data, std::size_t size, sockaddr *addr)
    {
        if (conn == nullptr || data == nullptr || addr == nullptr)
            return;

        auto mutable_conn = const_cast<Networking::Connection*>(conn);
        sockaddr_in *remoteaddr = (sockaddr_in*)addr;
        char str[INET_ADDRSTRLEN] = { 0 };
        char key[INET_ADDRSTRLEN + 8] = { 0 };
        inet_ntop(AF_INET, &remoteaddr->sin_addr, str, sizeof(str));
        snprintf(key, sizeof(key), "%s:%d", str, ntohs(remoteaddr->sin_port));

        std::unique_lock<std::recursive_mutex> lock(m_mutex);
        auto it = m_conns.find(key);
        if (it != m_conns.end())
        {
            it->second.lastalive = time(nullptr);
        }
        else
        {
            UdpEndpoint info;
            info.address = str;
            info.port = ntohs(remoteaddr->sin_port);
            info.remoteaddr = *remoteaddr;
            info.conn.reset(new Connection(mutable_conn->GetSocket()));
            m_conns[key] = info;
        }
    }

    void UdpServer::OnSendTo(const Connection * conn, std::size_t bytes_transferred, sockaddr *addr)
    {
        //LOG_DEBUG("Successfully written " << bytes_transferred << " bytes");
    }


} // namespace Networking
