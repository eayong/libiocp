#include "Client.h"
#include "Connection.h"

#include <iostream>

#pragma comment(lib, "libiocp.lib")

class MyServer : public Networking::Client
{
public:
    void OnRead(const Networking::Connection* conn, void* data, std::size_t size)
    {
        std::string value((char*)data, size);
        auto mutable_conn = const_cast<Networking::Connection*>(conn);
        std::cout << "Receive[" << mutable_conn->GetSocket() << "]: " << value.c_str() << std::endl;
    }

    void OnWrite(const Networking::Connection* conn, std::size_t bytes_transferred)
    {
        // std::cout << "Successfully written " << bytes_transferred << " bytes" << std::endl;
    }
};


int main(int argc, char *argv[])
{
    MyServer client;

    if (!client.Init("localhost", 8080))
    {
        std::cout << "Init client failed." << std::endl;
        return -1;
    }

    client.Start();

    char line[1024] = { 0 };

    std::cout << "Input 'bye' to exit." << std::endl << std::endl;
    while (std::cin.getline(line, sizeof(line) - 1))
    {
        if (strcmp(line, "bye") == 0)
            break;

        if (strlen(line) == 0)
            continue;

        if (!client.AsyncWrite(line, strlen(line)))
            continue;
    }

    client.Stop();

    return 0;
}