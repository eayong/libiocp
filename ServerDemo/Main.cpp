#include "Server.h"
#include "Connection.h"
#include <iostream>
#include <stdlib.h>

#ifdef _DEBUG
#pragma comment(lib, "libiocp_d.lib")
#else
#pragma comment(lib, "libiocp.lib")
#endif

class MyServer : public Networking::Server
{
public:

    void OnRead(const Networking::Connection* conn, void* data, std::size_t size)
    {
        /// test broadcast echo
        //std::string value((char*)data, size);
        //auto mutable_conn = const_cast<Networking::Connection*>(conn);
        //std::cout << "Receive[" << mutable_conn->GetSocket() << "]: " << value.c_str() << std::endl;
        // AsyncBroadcast(data, size);

        /// test save file
        SaveFile(data, size);
    }

    void OnWrite(const Networking::Connection* conn, std::size_t bytes_transferred)
    {
        // std::cout << "Successfully written " << bytes_transferred << " bytes" << std::endl;
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
    FILE * m_fp;
};


int main(int argc, char *argv[])
{
    MyServer server;

    if (!server.Init("localhost", 8080))
    {
        std::cout << "Init server failed." << std::endl;
        return -1;
    }

    server.OpenFile();
    
    server.Start();

    char line[1024] = { 0 };

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