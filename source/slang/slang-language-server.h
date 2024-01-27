#pragma once
#include <chrono>
#include "../../slang.h"
#include "../core/slang-range.h"
#include "../compiler-core/slang-json-rpc.h"
#include "../compiler-core/slang-json-rpc-connection.h"
#include "slang-workspace-version.h"
#include "slang-language-server-completion.h"
#include "slang-language-server-auto-format.h"
#include "slang-language-server-inlay-hints.h"

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
    Optional<LanguageServerProtocol::TextEditCompletionItem> textEditCompletionResolveArgs;
    Optional<LanguageServerProtocol::DocumentSymbolParams> documentSymbolArgs;
    Optional<LanguageServerProtocol::InlayHintParams> inlayHintArgs;
    Optional<LanguageServerProtocol::DocumentFormattingParams> formattingArgs;
    Optional<LanguageServerProtocol::DocumentRangeFormattingParams> rangeFormattingArgs;
    Optional<LanguageServerProtocol::DocumentOnTypeFormattingParams> onTypeFormattingArgs;
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

struct LanguageServerStartupOptions
{
    // Are we working with Visual Studio client?
    bool isVisualStudio = false;

    SLANG_API void parse(int argc, const char* const* argv);
};

class LanguageServer
{
private:
    static const int kConfigResponseId = 0x1213;
    
public:
    enum class TraceOptions
    {
        Off,
        Messages,
        Verbose
    };
    bool m_initialized = false;
    TraceOptions m_traceOptions = TraceOptions::Off;
    CommitCharacterBehavior m_commitCharacterBehavior = CommitCharacterBehavior::MembersOnly;
    RefPtr<JSONRPCConnection> m_connection;
    ComPtr<slang::IGlobalSession> m_session;
    RefPtr<Workspace> m_workspace;
    Dictionary<String, String> m_lastPublishedDiagnostics;
    std::chrono::time_point<std::chrono::system_clock> m_lastDiagnosticUpdateTime;
    FormatOptions m_formatOptions;
    Slang::InlayHintOptions m_inlayHintOptions;
    bool m_quit = false;
    List<LanguageServerProtocol::WorkspaceFolder> m_workspaceFolders;
    RttiTypeFuncsMap m_typeMap;
    LanguageServerStartupOptions m_options;

    LanguageServer(LanguageServerStartupOptions options)
        : m_options(options)
    {}

    SlangResult init(const LanguageServerProtocol::InitializeParams& args);
    SlangResult execute();
    void update();
    void updateConfigFromJSON(const JSONValue& jsonVal);
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
        const LanguageServerProtocol::CompletionItem& args, const LanguageServerProtocol::TextEditCompletionItem& editItem, const JSONValue& responseId);
    SlangResult semanticTokens(
        const LanguageServerProtocol::SemanticTokensParams& args, const JSONValue& responseId);
    SlangResult signatureHelp(
        const LanguageServerProtocol::SignatureHelpParams& args, const JSONValue& responseId);
    SlangResult documentSymbol(
        const LanguageServerProtocol::DocumentSymbolParams& args, const JSONValue& responseId);
    SlangResult inlayHint(
        const LanguageServerProtocol::InlayHintParams& args, const JSONValue& responseId);
    SlangResult formatting(
        const LanguageServerProtocol::DocumentFormattingParams& args, const JSONValue& responseId);
    SlangResult rangeFormatting(
        const LanguageServerProtocol::DocumentRangeFormattingParams& args, const JSONValue& responseId);
    SlangResult onTypeFormatting(
        const LanguageServerProtocol::DocumentOnTypeFormattingParams& args, const JSONValue& responseId);
    String getExprDeclSignature(Expr* expr, String* outDocumentation, List<Slang::Range<Index>>* outParamRanges);
    String getDeclRefSignature(DeclRef<Decl> declRef, String* outDocumentation, List<Slang::Range<Index>>* outParamRanges);
private:
    SlangResult parseNextMessage();
    slang::IGlobalSession* getOrCreateGlobalSession();
    void resetDiagnosticUpdateTime();
    void publishDiagnostics();
    void updatePredefinedMacros(const JSONValue& macros);
    void updateSearchPaths(const JSONValue& value);
    void updateSearchInWorkspace(const JSONValue& value);
    void updateCommitCharacters(const JSONValue& value);
    void updateFormattingOptions(const JSONValue& clangFormatLoc, const JSONValue& clangFormatStyle, const JSONValue& clangFormatFallbackStyle, const JSONValue& allowLineBreakOnType, const JSONValue& allowLineBreakInRange);
    void updateInlayHintOptions(const JSONValue& deducedTypes, const JSONValue& parameterNames);
    void updateTraceOptions(const JSONValue& value);

    void sendConfigRequest();
    void registerCapability(const char* methodName);
    void logMessage(int type, String message);

    FormatOptions getFormatOptions(Workspace* workspace, FormatOptions inOptions);
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

SLANG_API SlangResult runLanguageServer(LanguageServerStartupOptions options);
} // namespace Slang
