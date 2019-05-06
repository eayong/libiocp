#ifndef __NETWORKING_UDP_ENDPOINT_H__
#define __NETWORKING_UDP_ENDPOINT_H__

#include "NonCopyable.h"
#include "Socket.h"

#include <cstddef>
#include <WinSock2.h>
#include <mswsock.h>
#include <functional>
#include <memory>
#include <thread>
#include <mutex>
#include <map>
#include <list>
#include <time.h>


namespace Networking
{
    class Connection;
    class Context;

    struct UdpEndpoint
    {
        UdpEndpoint()
            : lastalive(time(nullptr)), port(0), conn(nullptr)
        {
            memset(&remoteaddr, 0, sizeof(remoteaddr)); 
        }

        time_t lastalive;
        int port;
        std::string address;
        sockaddr_in remoteaddr;
        std::shared_ptr<Connection> conn;
    };

    // Asynchronous Tcp server based on IO Completion ports
    class UdpServer : Utils::NonCopyable
    {
    public:
        UdpServer(time_t timeout = 60);
        virtual ~UdpServer();

    public:
        // Initialization. Must be called before any other method
        bool Init(const char* local_addr, unsigned local_port);

        // Start async write operation
        bool AsyncWrite(void* data, std::size_t size, sockaddr_in *addr);

        // Start async broadcast operation
        bool AsyncBroadcast(void* data, std::size_t size);

        // Start async read operation
        bool AsyncRead();

        // Start to run server
        void Start();

        // Stop server
        void Stop();

        // Show client's status
        void Status();

        // Get udp endpoint's info
        void GetUdpEndpoints(std::list<UdpEndpoint> &clients);

        void MainLoop(void *data);

        // Async call handlers
        virtual void OnRecvFrom(const Networking::Connection* conn,
            void* data, std::size_t size, sockaddr *addr);
        virtual void OnSendTo(const Networking::Connection* conn,
            std::size_t bytes_transferred, sockaddr *addr);

    protected:
        // Networking context
        std::unique_ptr<Context> m_context;

        Socket m_socket;

        std::unique_ptr<Connection> m_connection;

        std::map<std::string, UdpEndpoint> m_conns;
        std::recursive_mutex m_mutex;

        // Timeout to detect whether the udp endpoint is alive
        time_t m_timeout;

        std::unique_ptr<std::thread> m_thread;
        bool m_running;
    };

} // namespace Networking

#endif // !__NETWORKING_UDP_ENDPOINT_H__
