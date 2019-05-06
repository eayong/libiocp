#include "UdpClient.h"
#include "Connection.h"

#include <iostream>
#include <string>
#include <stdlib.h>

#ifdef _DEBUG
#pragma comment(lib, "libiocp_d.lib")
#else
#pragma comment(lib, "libiocp.lib")
#endif

class MyUdpClient : public Networking::UdpClient
{
public:
    void OnRecvFrom(const Networking::Connection * conn, void * data, std::size_t size, sockaddr *addr)
    {
        if (conn == nullptr || data == nullptr || addr == nullptr)
            return;
        std::string value((char*)data, size);
        std::cout << "Got new data. Size: " << size << ", data: " << value.c_str() << std::endl;;
    }

    void OnSendTo(const Networking::Connection* conn, std::size_t bytes_transferred, sockaddr *addr)
    {
        SendFile();
    }

    void OpenFile()
    {
        if (fopen_s(&m_fp, "send.txt", "rb") != 0)
        {
            std::cout << "fopen send.txt error: " << GetLastError() << std::endl;
            return;
        }
    }

    void SendFile()
    {
        if (m_fp)
        {
            char buf[1024] = { 0 };
            size_t rsize = fread(buf, 1, sizeof(buf), m_fp);
            if (rsize > 0)
            {
                AsyncWrite(buf, rsize);
            }
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
    FILE * m_fp;
};


int main(int argc, char *argv[])
{
    MyUdpClient client;

    if (!client.Init("localhost", 8080))
    {
        std::cout << "Init client failed." << std::endl;
        return -1;
    }
    client.OpenFile();

    client.Start();

    char line[2048] = { 0 };

    std::cout << "Input 'bye' to exit." << std::endl << std::endl;
    while (std::cin.getline(line, sizeof(line) - 1))
    {
        if (strlen(line) == 0)
            continue;

        if (strcmp(line, "bye") == 0)
            break;

        if (strcmp(line, "sendfile") == 0)
        {
            client.SendFile();
            std::cout << "finished." << std::endl;
            continue;
        }

        if (!client.AsyncWrite(line, strlen(line)))
            continue;
    }

    client.Stop();
    
    client.CloseFile();

    return 0;
}