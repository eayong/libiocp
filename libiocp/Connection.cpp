#include "Connection.h"
#include "Overlapped.h"

#include <WinSock2.h>

namespace Networking
{
    // Connection's impl class declaration
    class Connection::Impl
    {
    public:
        Impl(const SOCKET& socket, Connection* owner);
        ~Impl();

    public:
        // Socket associated with connection
        SOCKET m_socket;

        // Static read buffer
        char m_read_buffer[ReadBufferSize];

        // Size of current write buffer
        std::size_t m_write_buffer_size;

        // Dynamic write buffer
        std::unique_ptr<char> m_write_buffer;

        // Bytes already sent since last AsyncWrite call
        std::size_t m_sent_bytes;

        // Bytes need to be sent since last AsyncWrite call
        std::size_t m_total_bytes;

        // Typed overlapped structs
        std::unique_ptr<Overlapped> m_connect_overlapped;
        std::unique_ptr<Overlapped> m_accept_overlapped;
        std::unique_ptr<Overlapped> m_read_overlapped;
        std::unique_ptr<Overlapped> m_write_overlapped;
        std::unique_ptr<Overlapped> m_recvfrom_overlapped;
        std::unique_ptr<Overlapped> m_sendto_overlapped;
    };

    Connection::Impl::Impl(const SOCKET& socket, Connection* owner)
        : m_socket(socket)
        , m_write_buffer_size(0)
        , m_write_buffer(nullptr)
        , m_sent_bytes(0)
        , m_total_bytes(0)
        , m_connect_overlapped(CreateOverlapped(Overlapped::Connect))
        , m_accept_overlapped(CreateOverlapped(Overlapped::Accept))
        , m_read_overlapped(CreateOverlapped(Overlapped::Read))
        , m_write_overlapped(CreateOverlapped(Overlapped::Write))
        , m_recvfrom_overlapped(CreateOverlapped(Overlapped::RecvFrom))
        , m_sendto_overlapped(CreateOverlapped(Overlapped::SendTo))
    {
        m_connect_overlapped->connection = owner;
        m_accept_overlapped->connection = owner;
        m_read_overlapped->connection = owner;
        m_write_overlapped->connection = owner;
        m_recvfrom_overlapped->connection = owner;
        m_sendto_overlapped->connection = owner;
    }

    Connection::Impl::~Impl()
    {
        if (m_socket)
            closesocket(m_socket);
        m_socket = NULL;
    }

    Connection::Connection(const SOCKET& socket)
        : m_impl(new Impl(socket, this))
    {
    }

    Connection::~Connection()
    {
    }

    Overlapped* Connection::GetWriteOverlapped() const
    {
        return m_impl->m_write_overlapped.get();
    }

    Overlapped* Connection::GetReadOverlapped() const
    {
        return m_impl->m_read_overlapped.get();
    }

    Overlapped* Connection::GetAcceptOverlapped() const
    {
        return m_impl->m_accept_overlapped.get();
    }

    Overlapped* Connection::GetConnectOverlapped() const
    {
        return m_impl->m_connect_overlapped.get();
    }

    Overlapped * Connection::GetRecvFromOverlapped() const
    {
        return m_impl->m_recvfrom_overlapped.get();
    }

    Overlapped * Connection::GetSendToOverlapped() const
    {
        return m_impl->m_sendto_overlapped.get();
    }

    void Connection::SetTotalBytes(std::size_t value)
    {
        m_impl->m_total_bytes = value;
    }

    std::size_t Connection::GetTotalBytes() const
    {
        return m_impl->m_total_bytes;
    }

    void Connection::SetSentBytes(std::size_t value)
    {
        m_impl->m_sent_bytes = value;
    }

    std::size_t Connection::GetSentBytes() const
    {
        return m_impl->m_sent_bytes;
    }

    void Connection::ResizeWriteBuffer(std::size_t new_size)
    {
        m_impl->m_write_buffer.reset(new char[new_size]);
        m_impl->m_write_buffer_size = new_size;
    }

    std::size_t Connection::GetWriteBufferSize() const
    {
        return m_impl->m_write_buffer_size;
    }

    void* Connection::GetWriteBuffer()
    {
        return m_impl->m_write_buffer.get();
    }

    void* Connection::GetReadBuffer()
    {
        return &m_impl->m_read_buffer;
    }

    SOCKET& Connection::GetSocket()
    {
        return m_impl->m_socket;
    }
}
