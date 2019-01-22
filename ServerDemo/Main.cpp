#include "Server.h"
#include "Connection.h"
#include <iostream>
#include <map>

#pragma comment(lib, "libiocp.lib")

std::map<SOCKET, Networking::Connection*> g_connections;

class MyServer : public Networking::Server
{
public:
    void OnConnected(const Networking::Connection* conn)
    {
        auto mutable_conn = const_cast<Networking::Connection*>(conn);
        g_connections[mutable_conn->GetSocket()] = mutable_conn;
        std::cout << "New client connected: " << mutable_conn->GetSocket() << std::endl;
    }

    void OnDisconnected(const Networking::Connection* conn)
    {
        auto mutable_conn = const_cast<Networking::Connection*>(conn);
        g_connections.erase(mutable_conn->GetSocket());
        std::cout << "Client disconnected: " << mutable_conn->GetSocket() << std::endl;
    }

    void OnRead(const Networking::Connection* conn, void* data, std::size_t size)
    {
        std::string value((char*)data, size);
        auto mutable_conn = const_cast<Networking::Connection*>(conn);
        std::cout << "Receive[" << mutable_conn->GetSocket() << "]: " << value.c_str() << std::endl;
        for (auto &clien: g_connections)
        {
            AsyncWrite(clien.second, data, size);
        }
    }

    void OnWrite(const Networking::Connection* conn, std::size_t bytes_transferred)
    {
        // std::cout << "Successfully written " << bytes_transferred << " bytes" << std::endl;
    }
};


int main(int argc, char *argv[])
{
    MyServer server;

    if (!server.Init("localhost", 8080))
    {
        std::cout << "Init server failed." << std::endl;
        return -1;
    }

    server.Start();

    char line[1024] = { 0 };

    std::cout << "Input 'bye' to exit." << std::endl;
    while (std::cin.getline(line, sizeof(line) - 1))
    {
        if (strcmp(line, "bye") == 0)
            break;

        if (strlen(line) == 0)
            continue;

        for (auto &clien : g_connections)
        {
            server.AsyncWrite(clien.second, (void*)line, strlen(line));
        }
    }

    server.Stop();

    return 0;
}