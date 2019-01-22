#include "Acceptor.h"
#include "Context.h"
#include "Log.h"

namespace Networking
{

    Acceptor::Acceptor(Context& context)
        : m_context(context)
        , m_acceptex_func(nullptr)
    {
    }

    Acceptor::~Acceptor()
    {
    }

    bool Acceptor::Init()
    {
        // bind Socket to address and port
        if (!m_context.GetSocket().BindForListen())
            return false;

        //  Place a socket in a state in which it is listening for an incoming connection.
        if (listen(m_context.GetSocket().Native(), 5) != 0)
        {
            LOG_ERROR("Failed to switch socket to listening mode");
            return false;
        }
            

        GUID acceptex_guid = WSAID_ACCEPTEX;
        DWORD bytes_returned = 0;

        // Get AcceptEx function address
        if(WSAIoctl(m_context.GetSocket().Native(), SIO_GET_EXTENSION_FUNCTION_POINTER,
                &acceptex_guid, sizeof(acceptex_guid),
                &m_acceptex_func, sizeof(m_acceptex_func),
                &bytes_returned, NULL, NULL) != 0)
        {
            LOG_ERROR("Failed to get AcceptEx function address");
            return false;
        }
        acceptex_guid = WSAID_GETACCEPTEXSOCKADDRS;
        if (WSAIoctl(m_context.GetSocket().Native(), SIO_GET_EXTENSION_FUNCTION_POINTER,
            &acceptex_guid, sizeof(acceptex_guid),
            &m_get_acceptex_sockaddrs_func, sizeof(m_get_acceptex_sockaddrs_func),
            &bytes_returned, NULL, NULL) != 0)
        {
            LOG_ERROR("Failed to get GetAcceptExSockaddrs function address");
            return false;
        }
        m_context.m_acceptor = this;
        return true;
    }

    bool Acceptor::Start()
    {
        SOCKET accepted_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (accepted_socket == INVALID_SOCKET)
        {
            LOG_ERROR("Failed to initialize the socket for connection accept");
            return false;
        }
        std::unique_ptr<Connection> new_connection(new Connection(accepted_socket));

        DWORD bytes = 0;
        const int accept_ex_result = m_acceptex_func
        (
            m_context.GetSocket().Native(),
            accepted_socket,
            new_connection->GetReadBuffer(),
            0,
            sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16,
            &bytes, reinterpret_cast<LPOVERLAPPED>(new_connection->GetAcceptOverlapped())
        );

        if (accept_ex_result != TRUE && WSAGetLastError() != WSA_IO_PENDING)
        {
            LOG_ERROR("Failed to call AcceptEx");
            return false;
        }

        // Associate the accept socket with the completion port
        CreateIoCompletionPort(reinterpret_cast<HANDLE>(accepted_socket), m_context.GetCompletionPort(), 0, 0);

        // free connection's ownership
        new_connection.release();

        return  true;
    }

    void Acceptor::GetAcceptExSockaddrs(Connection * conn, sockaddr & local, sockaddr & remote)
    {
        int local_len = sizeof(local);
        int remote_len = sizeof(remote);
        m_get_acceptex_sockaddrs_func(
            conn->GetReadBuffer(),
            0,
            sizeof(sockaddr_in) + 16,
            sizeof(sockaddr_in) + 16,
            (SOCKADDR **)&local,
            &local_len,
            (SOCKADDR **)&remote,
            &remote_len);
    }

} // namespace Networking