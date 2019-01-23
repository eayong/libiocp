#include "UdpServer.h"
#include "Connection.h"
#include <iostream>
#include <stdlib.h>
#include <string>

#pragma comment(lib, "libiocp.lib")

class MyUdpServer : public Networking::UdpServer
{
public:

    void OnRecvFrom(const Networking::Connection* conn, void* data, std::size_t size, sockaddr *addr)
    {
        UdpServer::OnRecvFrom(conn, data, size, addr);

        // test broadcast echo
        // Broadcase(data, size);

        // test save file
        SaveFile(data, size);
    }

    void Broadcase(void* data, std::size_t size)
    {
        std::string value((char*)data, size);
        std::cout << "Got new data. Size: " << size << ", data: " << value.c_str() << std::endl;
        AsyncBroadcast(data, size);
    }

    void OpenFile()
    {
        if (fopen_s(&m_fp, "recv.txt", "wb") != 0)
        {
            std::cout << "fopen recv.txt error: " << GetLastError() << std::endl;
            return;
        }
    }

    void SaveFile(void* data, std::size_t size)
    {
        if (m_fp)
        {
            size_t wsize = fwrite(data, 1, size, m_fp);
            if (wsize != size)
            {
                std::cout << "fwrite error: " << GetLastError() << std::endl;
            }
            fflush(m_fp);
        }
    }

    void CloseFile()
    {
        if (m_fp)
        {
            fclose(m_fp);
            m_fp = nullptr;
        }
    }

private:
    FILE *m_fp;
};


int main(int argc, char *argv[])
{
    MyUdpServer server;

    if (!server.Init("localhost", 8080))
    {
        std::cout << "Init server failed." << std::endl;
        return -1;
    }
    server.OpenFile();

    server.Start();

    char line[2048] = { 0 };

    std::cout << "Input 'bye' to exit." << std::endl;
    while (std::cin.getline(line, sizeof(line) - 1))
    {
        if (strlen(line) == 0)
            continue;

        if (strcmp(line, "bye") == 0)
            break;

        if (strcmp(line, "status") == 0)
        {
            server.Status();
            continue;
        }

        server.AsyncBroadcast(line, strlen(line));
    }

    server.Stop();

    server.CloseFile();

    return 0;
}
