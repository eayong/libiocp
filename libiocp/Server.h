#ifndef __NETWORKING_SERVER_H__
#define __NETWORKING_SERVER_H__

#include "Acceptor.h"
#include "NonCopyable.h"

#include <cstddef>
#include <WinSock2.h>
#include <mswsock.h>
#include <functional>
#include <memory>
#include <thread>
#include <mutex>
#include <string>
#include <map>

namespace Networking
{

    class Connection;
    class Context;

    struct ClientInfo
    {
        std::string address;
        int port;
        Connection *conn;
    };

    // Asynchronous Tcp server based on IO Completion ports
    class Server : Utils::NonCopyable
    {
    public:
        Server();
        virtual ~Server();

    public:
        // Initialization. Must be called before any other method
        bool Init(const char* address, unsigned port);

        // Start async write operation
        bool AsyncWrite(const Connection* conn, void* data, std::size_t size);

        // Start async broadcast operation
        bool AsyncBroadcast(void* data, std::size_t size);

        // Start async read operation
        bool AsyncRead(const Connection* conn);

        // Start to run server
        void Start();

        // Stop server
        void Stop();

        // Show client's status
        void Status();

        // Async call handlers
        virtual void OnConnected(const Networking::Connection* conn);
        virtual void OnDisconnected(const Networking::Connection* /*conn*/);
        virtual void OnRead(const Networking::Connection* conn, void* data, std::size_t size);
        virtual void OnWrite(const Networking::Connection* conn, std::size_t bytes_transferred);

    private:
        // Networking context
        std::unique_ptr<Context> m_context;

        // New connections acceptor
        std::unique_ptr<Acceptor> m_acceptor;

        std::map<unsigned int, ClientInfo> m_conns;
        std::recursive_mutex m_mutex;
    };

} // namespace Networking

#endif // !__NETWORKING_SERVER_H__
