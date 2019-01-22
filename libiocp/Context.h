#ifndef __NETWORKING_CONTEXT_H__
#define __NETWORKING_CONTEXT_H__


#include "Socket.h"
#include "Exceptions.h"
#include "Connection.h"
#include "WinSockInitializer.h"
#include "Overlapped.h"
#include "NonCopyable.h"
#include "Singleton.h"

#include <functional>
#include <list>

namespace Networking
{

    class Acceptor;
    class Connector;

    namespace Callback
    {
        typedef std::function<void(const Connection*, std::size_t)> OnWrite;

        typedef std::function<void(const Connection*, void*, std::size_t)> OnRead;

        typedef std::function<void(const Connection*)> OnConnected;

        typedef std::function<void(const Connection*)> OnDisconnected;
    }

   // Main Network context.
   // Processes main async operations (AsyncRead, AsyncWrite) and handles all processed operations
    class Context : Utils::NonCopyable
    {
        friend Acceptor;
        friend Connector;

    public:
        Context();
        ~Context();

        static const std::size_t WaittingMilliSec = 1000;

       // Initialize context with address and port
        bool Init(const char* address, unsigned port, EProtoType proto);

       // Start async read operation
        bool AsyncRead(const Connection* conn);

       // Start async write operation
        bool AsyncWrite(const Connection* conn, void* data, std::size_t size);

        // Start async recvfrom operation
        bool AsyncRecvFrom(const Connection* conn);

        // Start async sendto operation
        bool AsyncSendTo(const Connection* conn, void* data, std::size_t size,
            SOCKADDR_IN &remote_addr);

        // Start to run server
        void Start();

        // Stop server
        void Stop();

        // Get socket
        Socket& GetSocket();

        // unable
        void GetAcceptExSockaddrs(Connection * conn, sockaddr &local, sockaddr &remote);

        // OnWrite callback called when data was successfully transferred via socket
        inline void SetOnWriteCallback(Callback::OnWrite callback);

        // OnRead callback called when data was successfully read from socket
        inline void SetOnReadCallback(Callback::OnRead callback);

        // OnClientConnected callback called when new client connected to listening socket
        inline void SetOnConnectedCallback(Callback::OnConnected callback);

        // OnClientDisconnected callback called when client closed connection
        inline void SetOnDisconnectedCallback(Callback::OnDisconnected callback);

    private:

       // Get completion port handle
        HANDLE GetCompletionPort();

        // Main worker. Processes all completed async operations
        void MainLoop(void *data);

    private:
       // Helper for winsock initialization
        Utils::Singleton<WinSockInitializer> m_winsock;

       // Main socket. Used as listening socket for server and as working socket for client
        Socket m_socket;

       // Completion port handle
        HANDLE m_completion_port;

       // New connections acceptor
        Acceptor* m_acceptor;

        // Server run thread
        std::list<std::unique_ptr<std::thread>> m_threads;

        // Server run flag
        bool m_running;

        Callback::OnWrite m_on_write;
        Callback::OnRead m_on_read;
        Callback::OnConnected m_on_connected;
        Callback::OnDisconnected m_on_disconnected;
    };

    inline void Context::SetOnWriteCallback(Callback::OnWrite callback)
    {
        m_on_write = callback;
    }

    inline void Context::SetOnReadCallback(Callback::OnRead callback)
    {
        m_on_read = callback;
    }

    inline void Context::SetOnConnectedCallback(Callback::OnConnected callback)
    {
        m_on_connected = callback;
    }

    inline void Context::SetOnDisconnectedCallback(Callback::OnDisconnected callback)
    {
        m_on_disconnected = callback;
    }

} // namespace Networking

#endif // !__NETWORKING_CONTEXT_H__

