// platform.cpp
#include "slang-http-session.h"

#include "common.h"

#ifdef _WIN32

#if 1
    #define WIN32_LEAN_AND_MEAN
	#define NOMINMAX
	#include <Windows.h>
	#undef WIN32_LEAN_AND_MEAN
	#undef NOMINMAX
#endif

    #include <WinInet.h>
#else
#endif

namespace Slang
{

#ifdef _WIN32

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! Win32HttpSession !!!!!!!!!!!!!!!!!!!!!!!! */

#pragma comment(lib, "Wininet.lib")

class Win32HTTPSession : public HTTPSession
{
public:

    struct SmartHandle
    {

            // assign
        void operator=(HINTERNET handle)
        {
            SLANG_ASSERT(m_handle == nullptr);
            m_handle = handle;
        }
            // Convert
        operator HINTERNET() const { return m_handle;  }

            /// Ctors
        SmartHandle(HINTERNET handle = nullptr) :
            m_handle(handle)
        {}
        ~SmartHandle()
        {
            if (m_handle)
            {
                ::InternetCloseHandle(m_handle);
            }
        }

    protected:
        HINTERNET m_handle;
    };

    virtual SlangResult request(const char* path, const Slang::UnownedStringSlice* post, List<char>* headersOut, List<char>& responseOut) SLANG_OVERRIDE;

    SlangResult initialize(const char* serverName, int port);

    Win32HTTPSession()
    {}

    SmartHandle m_hinternet;
    SmartHandle m_hconnection;
};

SlangResult Win32HTTPSession::initialize(const char* serverName, int port)
{
    ::GetLastError();

    m_hinternet = ::InternetOpenA("slang/1.0", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    if (!m_hinternet)
    {
        return SLANG_FAIL;
    }

    const char* userName = nullptr;
    const char* password = nullptr;
    DWORD dwConnectFlags = 0;
    DWORD dwConnectContext = 0;

    m_hconnection = ::InternetConnectA(m_hinternet, serverName, INTERNET_PORT(port), userName, password, 
        INTERNET_SERVICE_HTTP,
        dwConnectFlags, dwConnectContext);
    if (!m_hconnection)
    {
        return SLANG_FAIL;
    }

    return SLANG_OK;
}

SlangResult Win32HTTPSession::request(const char* path, const Slang::UnownedStringSlice* post, List<char>* headersOut, List<char>& responseOut)
{
    ::GetLastError();

    const char* verb = "GET";
    if (post)
    {
        verb = "POST";
    }

    const char* version = nullptr;
    const char* referrer = nullptr;
    const char** acceptTypes = nullptr;

    DWORD openRequestFlags = INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTP |
        INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTPS |
        INTERNET_FLAG_KEEP_CONNECTION |
        INTERNET_FLAG_NO_AUTH |
        INTERNET_FLAG_NO_AUTO_REDIRECT |
        INTERNET_FLAG_NO_COOKIES |
        INTERNET_FLAG_NO_UI |
        INTERNET_FLAG_RELOAD;

    DWORD openRequestContext = 0;
    SmartHandle hrequest = HttpOpenRequestA(m_hconnection, verb, path, version,
        referrer, acceptTypes,
        openRequestFlags, openRequestContext);

    if (!hrequest)
    {
        return SLANG_FAIL;
    }

    if (!::HttpSendRequestA(hrequest, nullptr, 0, post ? (void*)post->begin() : nullptr, post ? DWORD(post->size()) : 0))
    {
        return SLANG_FAIL;
    }

    if (headersOut)
    {
        headersOut->Clear();

        DWORD infoLevel = HTTP_QUERY_RAW_HEADERS_CRLF;

        DWORD infoBufferLength = 0;
        ::HttpQueryInfoA(hrequest, infoLevel, nullptr, &infoBufferLength, NULL);
        DWORD errorCode = ::GetLastError();
        if (errorCode != 0 && errorCode != ERROR_INSUFFICIENT_BUFFER)
        {
            return SLANG_FAIL;
        }

        headersOut->SetSize(infoBufferLength);

        char* headers = headersOut->Buffer();     
        if (!::HttpQueryInfoA(hrequest, infoLevel, headers, &infoBufferLength, NULL))
        {
            return SLANG_FAIL;
        }
        // Set the size to be sure
        headersOut->SetSize(infoBufferLength);
    }

    responseOut.Clear();

    DWORD bytesAvailable;
    while (::InternetQueryDataAvailable(hrequest, &bytesAvailable, 0, 0))
    {
        const UInt prevSize = responseOut.Count();
        responseOut.SetSize(prevSize + bytesAvailable);

        BYTE* dst = (BYTE*)(responseOut.Buffer() + prevSize);
        DWORD bytesRead;
        if (!::InternetReadFile(hrequest, dst, bytesAvailable, &bytesRead))
        {
            return SLANG_FAIL;
        }
        responseOut.SetSize(prevSize + bytesRead);

        if (bytesRead == 0)
        {
            break;
        }
    }

    return SLANG_OK;
}

/* static */RefPtr<HTTPSession> HTTPSession::create(const char* serverName, int port)
{
    RefPtr<Win32HTTPSession> session(new Win32HTTPSession);
    if (SLANG_FAILED(session->initialize(serverName, port)))
    {
        return RefPtr<HTTPSession>();
    }
    return session;
}

#else // _WIN32

/* static */RefPtr<HTTPSession> HTTPSession::create(const char* serverName, int port)
{
    return RefPtr<HTTPSession>();
}

#endif // _WIN32

}
