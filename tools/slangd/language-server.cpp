// language-server.cpp

// This file implements the language server for Slang, conforming to the Language Server Protocol.
// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <thread>

#include "../../source/core/slang-secure-crt.h"
#include "../../slang-com-helper.h"

#include "language-server-protocol.h"
#include "language-server.h"

namespace Slang
{
using namespace LanguageServerProtocol;

SlangResult LanguageServer::init(const InitializeParams& args)
{
    SLANG_RETURN_ON_FAIL(m_connection->initWithStdStreams());
    m_workspaceFolders = args.workspaceFolders;
    return SLANG_OK;
}

slang::IGlobalSession* LanguageServer::getOrCreateGlobalSession()
{
    if (!m_session)
    {
        // Just create the global session in the regular way if there isn't one set
        if (SLANG_FAILED(slang_createGlobalSession(SLANG_API_VERSION, m_session.writeRef())))
        {
            return nullptr;
        }
    }

    return m_session;
}

SlangResult LanguageServer::_executeSingle()
{
    // If we don't have a message, we can quit for now
    if (!m_connection->hasMessage())
    {
        return SLANG_OK;
    }

    const JSONRPCMessageType msgType = m_connection->getMessageType();

    switch (msgType)
    {
    case JSONRPCMessageType::Call:
        {
            JSONRPCCall call;
            SLANG_RETURN_ON_FAIL(m_connection->getRPCOrSendError(&call));

            // Do different things
            if (call.method == ExitParams::methodName)
            {
                m_quit = true;
                return SLANG_OK;
            }
            else if (call.method == ShutdownParams::methodName)
            {
                m_connection->sendResult(NullResponse::get(), call.id);
                return SLANG_OK;
            }
            else if (call.method == InitializeParams::methodName)
            {
                InitializeParams args = {};
                SLANG_RETURN_ON_FAIL(
                    m_connection->toNativeArgsOrSendError(call.params, &args, call.id));

                init(args);

                InitializeResult result = {};
                result.serverInfo.name = "SlangLanguageServer";
                result.serverInfo.version = "1.0";
                result.capabilities.positionEncoding = "utf-8";
                result.capabilities.textDocumentSync.openClose = true;
                result.capabilities.textDocumentSync.change = (int)TextDocumentSyncKind::Full;
                m_connection->sendResult(&result, call.id);
                return SLANG_OK;
            }
            else if (call.method == DidOpenTextDocumentParams::methodName)
            {
                DidOpenTextDocumentParams args;
                SLANG_RETURN_ON_FAIL(
                    m_connection->toNativeArgsOrSendError(call.params, &args, call.id));
                return didOpenTextDocument(args);
            }
            else if (call.method == DidCloseTextDocumentParams::methodName)
            {
                DidCloseTextDocumentParams args;
                SLANG_RETURN_ON_FAIL(
                    m_connection->toNativeArgsOrSendError(call.params, &args, call.id));
                return didCloseTextDocument(args);
            }
            else if (call.method == DidChangeTextDocumentParams::methodName)
            {
                DidChangeTextDocumentParams args;
                SLANG_RETURN_ON_FAIL(
                    m_connection->toNativeArgsOrSendError(call.params, &args, call.id));
                return didChangeTextDocument(args);
            }
            else if (call.method == "initialized")
            {
                return SLANG_OK;
            }
            else
            {
                return m_connection->sendError(JSONRPC::ErrorCode::MethodNotFound, call.id);
            }
        }
    default:
        {
            return m_connection->sendError(
                JSONRPC::ErrorCode::InvalidRequest, m_connection->getCurrentMessageId());
        }
    }

    return SLANG_OK;
}

SlangResult LanguageServer::didOpenTextDocument(const DidOpenTextDocumentParams& args)
{
    return SLANG_OK;
}
SlangResult LanguageServer::didCloseTextDocument(const DidCloseTextDocumentParams& args)
{
    return SLANG_OK;
}
SlangResult LanguageServer::didChangeTextDocument(const DidChangeTextDocumentParams& args)
{
    return SLANG_OK;
}

void LanguageServer::update()
{

}

SlangResult LanguageServer::execute()
{
    m_connection = new JSONRPCConnection();
    m_connection->initWithStdStreams();
    while (m_connection->isActive() && !m_quit)
    {
        // Consume all messages first.
        while (m_connection->tryReadMessage() == SLANG_OK)
        {
            const SlangResult res = _executeSingle();
        }

        // Now we can use this time to reparse user's code, report diagnostics, etc.
        update();
    }

    return SLANG_OK;
}

} // namespace LanguageServer

int main(int argc, const char* const* argv)
{
    bool isDebug = false;
    for (auto i = 1; i < argc; i++)
    {
        if (Slang::UnownedStringSlice(argv[i]) == "--debug")
        {
            isDebug = true;
        }
    }
    if (isDebug)
    {
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }
    Slang::LanguageServer server;
    SLANG_RETURN_ON_FAIL(server.execute());
    return 0;
}
