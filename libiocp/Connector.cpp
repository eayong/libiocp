#include "Connector.h"
#include "Context.h"
#include "Log.h"

namespace Networking
{

    Connector::Connector(Context& context)
        : m_context(context)
    {
    }

    bool Connector::Init()
    {
        m_context.GetSocket().BindForConnect();

        GUID connectex_guid = WSAID_CONNECTEX;
        DWORD bytes_returned = 0;

        // Get ConnectEx function address
        if(WSAIoctl(
                m_context.GetSocket().Native(), SIO_GET_EXTENSION_FUNCTION_POINTER,
                &connectex_guid, sizeof(connectex_guid),
                &m_connectex_func, sizeof(m_connectex_func),
                &bytes_returned, NULL, NULL) != 0)
        { 
            LOG_ERROR("Failed to get ConnectEx function address");
            return false;
        }
        return true;
    }

    bool Connector::Start()
    {
        std::unique_ptr<Connection> new_connection(new Connection(m_context.GetSocket().Native()));
        auto socket_address = m_context.GetSocket().GetAddress();

        DWORD bytes = 0;
        const int connect_ex_result = m_connectex_func
        (
            m_context.GetSocket().Native(),
            reinterpret_cast<SOCKADDR*>(&socket_address),
            sizeof(socket_address),
            NULL,
            0,
            &bytes,
            reinterpret_cast<LPOVERLAPPED>(new_connection->GetConnectOverlapped())
        );

        if (connect_ex_result != TRUE && WSAGetLastError() != WSA_IO_PENDING)
        {
            LOG_ERROR("Failed to call ConnectEx");
            return false;
        }

        // free connection's ownership
        new_connection.release();
        return true;
    }

} // namespace Networking