#include "Context.h"
#include "Acceptor.h"
#include "Log.h"
#include <errno.h>

namespace Networking
{

    Context::Context() 
        : m_acceptor(nullptr)
        , m_running(false)
    {
    }

    Context::~Context()
    {
        Stop();
    }

    bool Context::Init(const char* address, unsigned port, EProtoType proto)
    {
        if (address == nullptr)
            return false;

        if (!m_winsock.Instance().IsInitialized())
        {
            LOG_ERROR("Failed to initialize WinSock, maybe wrong version");
            return false;
        }

        // Create a handle for the completion port
        m_completion_port = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
        if(m_completion_port == INVALID_HANDLE_VALUE)
        {
            LOG_ERROR("Failed to create IO Completion port");
            return false;
        }

        // Init the socket
        m_socket.Init(address, port, proto);

        // Associate the socket with the completion port
        if(CreateIoCompletionPort(reinterpret_cast<HANDLE>(m_socket.Native()), m_completion_port, 0, 0) == INVALID_HANDLE_VALUE)
        {
            LOG_ERROR("Failed to associate listening socket with the IO Completion port");
            return false;
        }

        return true;
    }

    bool Context::AsyncRead(const Connection* conn)
    {
        if (conn == nullptr)
            return false;

        auto overlapped = conn->GetReadOverlapped();
        overlapped->wsa_buf.len = overlapped->connection->ReadBufferSize;
        overlapped->wsa_buf.buf = reinterpret_cast<CHAR*>(overlapped->connection->GetReadBuffer());

        DWORD flags = 0;
        DWORD bytes_transferred = 0;

        auto recv_result = WSARecv(overlapped->connection->GetSocket(), &overlapped->wsa_buf, 1, &bytes_transferred, &flags, reinterpret_cast<LPWSAOVERLAPPED>(overlapped), NULL);
        if (recv_result == NULL || (recv_result == SOCKET_ERROR && WSAGetLastError() == WSA_IO_PENDING))
        {
            return true;
        }

        LOG_ERROR("Failed to receive data");
        return false;
    }

    bool Context::AsyncWrite(const Connection* conn, void* data, std::size_t size)
    {
        if (conn == nullptr || data == nullptr)
            return false;

        auto mutable_conn = const_cast<Connection*>(conn);

        if (mutable_conn->GetWriteBufferSize() < size)
            mutable_conn->ResizeWriteBuffer(size);

        memcpy_s(mutable_conn->GetWriteBuffer(), mutable_conn->GetWriteBufferSize(), data, size);

        mutable_conn->SetSentBytes(0);
        mutable_conn->SetTotalBytes(size);

        auto overlapped = mutable_conn->GetWriteOverlapped();
        overlapped->wsa_buf.len = size;
        overlapped->wsa_buf.buf = reinterpret_cast<CHAR*>(mutable_conn->GetWriteBuffer());

        DWORD bytes;
        auto send_result = WSASend(mutable_conn->GetSocket(), &overlapped->wsa_buf, 1, &bytes, 0, reinterpret_cast<LPWSAOVERLAPPED>(overlapped), NULL);

        if (send_result != NULL && (send_result != SOCKET_ERROR || WSAGetLastError() != WSA_IO_PENDING))
        {
            LOG_ERROR("Failed to send data");
            return false;
        }

        return true;
    }

    bool Context::AsyncRecvFrom(const Connection * conn)
    {
        if (conn == nullptr)
            return false;

        auto overlapped = conn->GetRecvFromOverlapped();
        overlapped->wsa_buf.len = overlapped->connection->ReadBufferSize;
        overlapped->wsa_buf.buf = reinterpret_cast<CHAR*>(overlapped->connection->GetReadBuffer());

        DWORD flags = 0;
        DWORD bytes_transferred = 0;
        auto recv_result = WSARecvFrom(overlapped->connection->GetSocket(),
            &overlapped->wsa_buf, 1, &bytes_transferred, &flags,
            (SOCKADDR*)&(overlapped->remote_addr), &(overlapped->remote_addr_len),
            reinterpret_cast<LPWSAOVERLAPPED>(overlapped), NULL);
        if (recv_result == NULL || (recv_result == SOCKET_ERROR && WSAGetLastError() == WSA_IO_PENDING))
        {
            return true;
        }

        LOG_ERROR("Failed to receive data");
        return false;
    }

    bool Context::AsyncSendTo(const Connection * conn, void * data, std::size_t size,
        SOCKADDR_IN & remote_addr)
    {
        if (conn == nullptr || data == nullptr)
            return false;

        auto mutable_conn = const_cast<Connection*>(conn);

        if (mutable_conn->GetWriteBufferSize() < size
            || mutable_conn->GetWriteBuffer() == nullptr)
            mutable_conn->ResizeWriteBuffer(size);

        memcpy_s(mutable_conn->GetWriteBuffer(), mutable_conn->GetWriteBufferSize(), data, size);

        mutable_conn->SetSentBytes(0);
        mutable_conn->SetTotalBytes(size);

        auto overlapped = mutable_conn->GetSendToOverlapped();
        overlapped->wsa_buf.len = size;
        overlapped->wsa_buf.buf = reinterpret_cast<CHAR*>(mutable_conn->GetWriteBuffer());

        DWORD bytes = 0;
        overlapped->remote_addr = remote_addr;
        overlapped->remote_addr_len = sizeof(remote_addr);
        auto send_result = WSASendTo(mutable_conn->GetSocket(),
            &overlapped->wsa_buf, 1, &bytes, 0,
            (SOCKADDR*)&(overlapped->remote_addr), overlapped->remote_addr_len,
            reinterpret_cast<LPWSAOVERLAPPED>(overlapped), NULL);
        if (send_result != NULL && (send_result != SOCKET_ERROR || WSAGetLastError() != WSA_IO_PENDING))
        {
            LOG_ERROR("Failed to send data");
            return false;
        }

        return true;
    }

    void Context::Start()
    {
        if (m_running)
            return;

        m_running = true;
        SYSTEM_INFO sysinfo;
        GetSystemInfo(&sysinfo);

        for (unsigned int i = 0; i < sysinfo.dwNumberOfProcessors; i++)
        {
            std::unique_ptr<std::thread> thr(new std::thread(std::bind(&Context::MainLoop, this, std::placeholders::_1), this));
            m_threads.push_back(std::move(thr));
        }
    }

    void Context::Stop()
    {
        if (m_running)
        {
            SYSTEM_INFO sysinfo;
            GetSystemInfo(&sysinfo);

            for (unsigned int i = 0; i < sysinfo.dwNumberOfProcessors; i++)
            {
                PostQueuedCompletionStatus(m_completion_port, 0, (DWORD)NULL, NULL);
            }
            for (auto &thr : m_threads)
            {
                thr->join();
            }
            m_threads.clear();
            m_running = false;
        }
    }

    void Context::MainLoop(void *data)
    {
        DWORD bytes_transferred = 0;
        ULONG_PTR completion_key = 0;
        DWORD Flags = 0;
        Overlapped* overlapped = nullptr;
        int error = 0;
        while (m_running)
        {
            GetQueuedCompletionStatus(m_completion_port, &bytes_transferred, &completion_key, reinterpret_cast<LPOVERLAPPED*>(&overlapped), INFINITE);
            if (!overlapped)
            {
                break;
            }

            error = WSAGetLastError();
            if (error != 0 && error != WSA_IO_PENDING && error != ERROR_MORE_DATA)
            {
                if (m_on_disconnected)
                    m_on_disconnected(overlapped->connection);
                continue;
            }

            if (overlapped->type == Overlapped::Type::Accept)
            {
                // server accepted new connection from client
                if (m_acceptor)
                    m_acceptor->Start();

                if (m_on_connected)
                    m_on_connected(overlapped->connection);
                AsyncRead(overlapped->connection);
                continue;
            }

            if (overlapped->type == Overlapped::Type::Connect)
            {
                // client connected to the server
                if (m_on_connected)
                    m_on_connected(overlapped->connection);

                AsyncRead(overlapped->connection);
                continue;
            }

            if (bytes_transferred == 0)
            {
                // remote side disconnected
                if (m_on_disconnected)
                    m_on_disconnected(overlapped->connection);

                delete overlapped->connection;
                overlapped = nullptr;

                continue;
            }

            if (overlapped->type == Overlapped::Type::Read)
            {
                // async read operation fihished
                if (m_on_read)
                    m_on_read(overlapped->connection, overlapped->connection->GetReadBuffer(), bytes_transferred);

                AsyncRead(overlapped->connection);
                continue;
            }

            if (overlapped->type == Overlapped::Type::Write)
            {
                // async write operation fihished or only the part
                auto conn = overlapped->connection;

                conn->SetSentBytes(conn->GetSentBytes() + bytes_transferred);

                if (conn->GetSentBytes() < conn->GetTotalBytes())
                {
                    // need to write more
                    overlapped->wsa_buf.len = conn->GetTotalBytes() - conn->GetSentBytes();
                    overlapped->wsa_buf.buf = reinterpret_cast<CHAR*>(conn->GetWriteBuffer()) + conn->GetSentBytes();

                    auto send_result = WSASend(conn->GetSocket(), &overlapped->wsa_buf, 1, &bytes_transferred, 0, reinterpret_cast<LPWSAOVERLAPPED>(overlapped), NULL);
                    if (send_result != NULL && (send_result != SOCKET_ERROR || WSAGetLastError() != WSA_IO_PENDING))
                    {
                        LOG_ERROR("Failed to send data");
                    }
                }
                else
                {
                    // async write operation fully completed
                    if (m_on_write)
                        m_on_write(overlapped->connection, bytes_transferred);
                }
                continue;
            }

            if (overlapped->type == Overlapped::Type::RecvFrom)
            {
                // async recvfrom operation fihished
                if (m_on_recvfrom)
                    m_on_recvfrom(overlapped->connection, overlapped->connection->GetReadBuffer(),
                        bytes_transferred, (SOCKADDR*)&(overlapped->remote_addr));

                AsyncRecvFrom(overlapped->connection);
                continue;
            }

            if (overlapped->type == Overlapped::Type::SendTo)
            {
                // async sendto operation fihished or only the part
                auto conn = overlapped->connection;

                conn->SetSentBytes(conn->GetSentBytes() + bytes_transferred);

                if (conn->GetSentBytes() < conn->GetTotalBytes())
                {
                    // need to write more
                    overlapped->wsa_buf.len = conn->GetTotalBytes() - conn->GetSentBytes();
                    overlapped->wsa_buf.buf = reinterpret_cast<CHAR*>(conn->GetWriteBuffer()) + conn->GetSentBytes();

                    auto send_result = WSASendTo(conn->GetSocket(),
                        &overlapped->wsa_buf, 1, &bytes_transferred, 0,
                        (SOCKADDR*)&(overlapped->remote_addr), overlapped->remote_addr_len,
                        reinterpret_cast<LPWSAOVERLAPPED>(overlapped), NULL);
                    if (send_result != NULL && (send_result != SOCKET_ERROR
                        || WSAGetLastError() != WSA_IO_PENDING))
                    {
                        LOG_ERROR("Failed to send data");
                    }
                }
                else
                {
                    // async sendto operation fully completed
                    if (m_on_sendto)
                        m_on_sendto(overlapped->connection, bytes_transferred, (SOCKADDR*)&(overlapped->remote_addr));
                }
                continue;
            }
        }
        LOG_ERROR("Exit with error code: " << WSAGetLastError());
    }

    Socket& Context::GetSocket()
    {
        return m_socket;
    }

    void Context::GetAcceptExSockaddrs(Connection * conn, sockaddr & local, sockaddr & remote)
    {
        if (m_acceptor)
            return m_acceptor->GetAcceptExSockaddrs(conn, local, remote);
    }

    HANDLE Context::GetCompletionPort()
    {
        return m_completion_port;
    }
} // namespace Networking