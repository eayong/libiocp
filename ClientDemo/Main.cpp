#include "Client.h"
#include "Connection.h"

#include <iostream>
#include <string>

#pragma comment(lib, "libiocp.lib")

class MyClient : public Networking::Client
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
    MyClient client;

    if (!client.Init("localhost", 8080))
    {
        std::cout << "Init client failed." << std::endl;
        return -1;
    }

    client.OpenFile();

    client.Start();

    char line[1024] = { 0 };

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