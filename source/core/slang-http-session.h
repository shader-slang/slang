// platform.h
#ifndef SLANG_HTTP_SESSION_H_INCLUDED
#define SLANG_HTTP_SESSION_H_INCLUDED

#include "../../slang.h"
#include "../core/slang-string.h"

#include "list.h"

namespace Slang
{
    class HTTPSession: public RefObject
    {
    public:
        virtual SlangResult request(const char* path, const Slang::UnownedStringSlice* post, List<char>* headersOut, List<char>& responseOut) = 0;

            /// Creates a session where can communicate to a server through a port
        static RefPtr<HTTPSession> create(const char* serverName, int port);
    };

}

#endif
