#pragma once

#include "../../slang.h"
#include "../compiler-core/slang-json-rpc.h"
#include "../compiler-core/slang-json-rpc-connection.h"

#include "slang-workspace-version.h"
#include "slang-language-server-completion.h"

namespace Slang
{
ArrayView<const char*> getCommitChars();

struct Command
{
    PersistentJSONValue id;
    String method;

    template <typename T> struct Optional
    {
    public:
        T* value = nullptr;
        bool isValid() { return value != nullptr; }
        T& operator=(const T& val)
        {
            delete value;
            value = new T(val);
            return *value;
        }
        T& operator=(Optional&& other)
        {
            if (other.isValid())
                *this = (other.get());
            other.value = nullptr;
            return *value;
        }
        T& get()
        {
            SLANG_ASSERT(isValid());
            return *value;
        }
        Optional() = default;
        Optional(const Optional& other)
        {
            if (other.isValid())
                *this = (other.get());
        }
        Optional(Optional&& other)
        {
            if (other.isValid())
                *this = (other.get());
            other.value = nullptr;
        }

        ~Optional() { delete value; }
    };

    Optional<LanguageServerProtocol::CompletionParams> completionArgs;
    Optional<LanguageServerProtocol::CompletionItem> completionResolveArgs;
    Optional<LanguageServerProtocol::DocumentSymbolParams> documentSymbolArgs;
    Optional<LanguageServerProtocol::DidChangeConfigurationParams> changeConfigArgs;
    Optional<LanguageServerProtocol::SignatureHelpParams> signatureHelpArgs;
    Optional<LanguageServerProtocol::DefinitionParams> definitionArgs;
    Optional<LanguageServerProtocol::SemanticTokensParams> semanticTokenArgs;
    Optional<LanguageServerProtocol::HoverParams> hoverArgs;
    Optional<LanguageServerProtocol::DidOpenTextDocumentParams> openDocArgs;
    Optional<LanguageServerProtocol::DidChangeTextDocumentParams> changeDocArgs;
    Optional<LanguageServerProtocol::DidCloseTextDocumentParams> closeDocArgs;
    Optional<LanguageServerProtocol::CancelParams> cancelArgs;
};

class LanguageServer
{
private:
    static const int kConfigResponseId = 0x1213;

public:
    bool m_initialized = false;
    CommitCharacterBehavior m_commitCharacterBehavior = CommitCharacterBehavior::MembersOnly;
    RefPtr<JSONRPCConnection> m_connection;
    ComPtr<slang::IGlobalSession> m_session;
    RefPtr<Workspace> m_workspace;
    Dictionary<String, String> m_lastPublishedDiagnostics;
    time_t m_lastDiagnosticUpdateTime = 0;
    bool m_quit = false;
    List<LanguageServerProtocol::WorkspaceFolder> m_workspaceFolders;

    SlangResult init(const LanguageServerProtocol::InitializeParams& args);
    SlangResult execute();
    void update();
    SlangResult didOpenTextDocument(const LanguageServerProtocol::DidOpenTextDocumentParams& args);
    SlangResult didCloseTextDocument(
        const LanguageServerProtocol::DidCloseTextDocumentParams& args);
    SlangResult didChangeTextDocument(
        const LanguageServerProtocol::DidChangeTextDocumentParams& args);
    SlangResult didChangeConfiguration(
        const LanguageServerProtocol::DidChangeConfigurationParams& args);
    SlangResult hover(const LanguageServerProtocol::HoverParams& args, const JSONValue& responseId);
    SlangResult gotoDefinition(
        const LanguageServerProtocol::DefinitionParams& args, const JSONValue& responseId);
    SlangResult completion(
        const LanguageServerProtocol::CompletionParams& args, const JSONValue& responseId);
    SlangResult completionResolve(
        const LanguageServerProtocol::CompletionItem& args, const JSONValue& responseId);
    SlangResult semanticTokens(
        const LanguageServerProtocol::SemanticTokensParams& args, const JSONValue& responseId);
    SlangResult signatureHelp(
        const LanguageServerProtocol::SignatureHelpParams& args, const JSONValue& responseId);
    SlangResult documentSymbol(
        const LanguageServerProtocol::DocumentSymbolParams& args, const JSONValue& responseId);

private:
    SlangResult parseNextMessage();
    slang::IGlobalSession* getOrCreateGlobalSession();
    void resetDiagnosticUpdateTime();
    void publishDiagnostics();
    void updatePredefinedMacros(const JSONValue& macros);
    void updateSearchPaths(const JSONValue& value);
    void updateSearchInWorkspace(const JSONValue& value);
    void updateCommitCharacters(const JSONValue& value);

    void sendConfigRequest();
    void registerCapability(const char* methodName);
    void logMessage(int type, String message);

    SlangResult tryGetMacroHoverInfo(
        WorkspaceVersion* version,
        DocumentVersion* doc,
        Index line,
        Index col,
        JSONValue responseId);
    SlangResult tryGotoMacroDefinition(
        WorkspaceVersion* version,
        DocumentVersion* doc,
        Index line,
        Index col,
        JSONValue responseId);
    SlangResult tryGotoFileInclude(
        WorkspaceVersion* version,
        DocumentVersion* doc,
        Index line,
        JSONValue responseId);
    List<Command> commands;
    SlangResult queueJSONCall(JSONRPCCall call);
    SlangResult runCommand(Command& cmd);
    void processCommands();
};

inline bool _isIdentifierChar(char ch)
{
    return ch >= '0' && ch <= '9' || ch >= 'a' && ch <= 'z' || ch >= 'A' && ch <= 'Z' || ch == '_';
}

SLANG_API SlangResult runLanguageServer();
} // namespace Slang
