#ifndef __NETWORKING_CONNECTOR_H__
#define __NETWORKING_CONNECTOR_H__

#include "NonCopyable.h"
#include <WinSock2.h>
#include <mswsock.h>

namespace Networking
{

    class Context;

    // Extending 'Context' and helps to start async Connect operation
    // (used on client side)
    class Connector : Utils::NonCopyable
    {
    public:
        Connector(Context& context);

        // Initialize connector
        bool Init();

        // Start async Connect operation
        bool Start();

    private:
        // Main network context
        Context& m_context;

        // ConnectEx function pointer
        LPFN_CONNECTEX m_connectex_func;
    };

} // namespace Networking

#endif // !__NETWORKING_CONNECTOR_H__
