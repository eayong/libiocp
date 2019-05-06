#ifndef __NETWORKING_CLIENT_H__
#define __NETWORKING_CLIENT_H__

#include "Connector.h"
#include "NonCopyable.h"

#include <cstddef>
#include <WinSock2.h>
#include <mswsock.h>
#include <functional>
#include <memory>
#include <thread>

namespace Networking
{

    class Connection;
    class Context;

    // Asynchronous Tcp server based on IO Completion ports
    class Client : Utils::NonCopyable
    {
    public:
        Client();
        virtual ~Client();

    public:
        // Initialization. Must be called before any other method
        bool Init(const char* address, unsigned port);

        // Start async write operation
        bool AsyncWrite(void* data, std::size_t size);

        // Start async read operation
        bool AsyncRead();

        // Start to run server
        void Start();

        // Stop server
        void Stop();

        // Async call handlers
        virtual void OnConnected(const Networking::Connection* conn);
        virtual void OnDisconnected(const Networking::Connection* /*conn*/);
        virtual void OnRead(const Networking::Connection* conn, void* data, std::size_t size);
        virtual void OnWrite(const Networking::Connection* conn, std::size_t bytes_transferred);

    private:
        void OnInnerConnected(const Networking::Connection* conn);
        void OnInnerDisconnected(const Networking::Connection* conn);

    protected:
        // Networking context
        std::unique_ptr<Context> m_context;

        // New connections acceptor
        std::unique_ptr<Connector> m_connetor;
    
        std::unique_ptr<Connection> m_connection;
    };

} // namespace Networking

#endif // !__NETWORKING_CLIENT_H__
