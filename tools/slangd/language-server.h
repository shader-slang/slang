#pragma once

#include "../../source/core/slang-io.h"
#include "../../source/core/slang-process-util.h"
#include "../../source/core/slang-string-util.h"
#include "../../source/core/slang-string.h"
#include "../../source/core/slang-writer.h"
#include "../../source/compiler-core/slang-json-rpc-connection.h"
#include "language-server-protocol.h"

namespace Slang
{
    class LanguageServer
    {
    public:
        RefPtr<JSONRPCConnection> m_connection;
        ComPtr<slang::IGlobalSession> m_session;
        bool m_quit = false;
        List<LanguageServerProtocol::WorkspaceFolder> m_workspaceFolders;

        SlangResult init(const LanguageServerProtocol::InitializeParams& args);
        SlangResult execute();
        void update();
        SlangResult didOpenTextDocument(
            const LanguageServerProtocol::DidOpenTextDocumentParams& args);
        SlangResult didCloseTextDocument(
            const LanguageServerProtocol::DidCloseTextDocumentParams& args);
        SlangResult didChangeTextDocument(
            const LanguageServerProtocol::DidChangeTextDocumentParams& args);

    private:
        SlangResult _executeSingle();
        slang::IGlobalSession* getOrCreateGlobalSession();
    };
}
