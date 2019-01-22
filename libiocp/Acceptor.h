#ifndef __NETWORKING_ACCEPTOR_H__
#define __NETWORKING_ACCEPTOR_H__

#include "NonCopyable.h"
#include <WinSock2.h>
#include <mswsock.h>

namespace Networking
{

    class Context;

    // Extending 'Context' and helps to start async Accept operation
    // (used on server side)
    class Acceptor : Utils::NonCopyable
    {
    public:
        Acceptor(Context& context);
        ~Acceptor();

    public:
        // Initialize acceptor
        bool Init();

        // Start new async Accept operation
        bool Start();

    private:
        // Main networking context
        Context& m_context;

        // AcceptEx function pointer
        LPFN_ACCEPTEX m_acceptex_func;
    };

} // namespace Networking

#endif // !__NETWORKING_ACCEPTOR_H__
