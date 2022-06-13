// language-server.cpp

// This file implements the language server for Slang, conforming to the Language Server Protocol.
// https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <thread>
#include "../core/slang-secure-crt.h"
#include "../core/slang-range.h"
#include "../../slang-com-helper.h"
#include "../compiler-core/slang-json-rpc-connection.h"
#include "../compiler-core/slang-language-server-protocol.h"
#include "slang-language-server.h"
#include "slang-workspace-version.h"
#include "slang-language-server-ast-lookup.h"
#include "slang-language-server-collect-member.h"
#include "slang-language-server-semantic-tokens.h"
#include "slang-ast-print.h"
#include "slang-doc-markdown-writer.h"

namespace Slang
{
using namespace LanguageServerProtocol;

class LanguageServer
{
public:
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
    SlangResult hover(const LanguageServerProtocol::HoverParams& args, const JSONValue& responseId);
    SlangResult gotoDefinition(const LanguageServerProtocol::DefinitionParams& args, const JSONValue& responseId);
    SlangResult completion(
        const LanguageServerProtocol::CompletionParams& args, const JSONValue& responseId);
    SlangResult completionResolve(
        const LanguageServerProtocol::CompletionItem& args, const JSONValue& responseId);
    SlangResult semanticTokens(
        const LanguageServerProtocol::SemanticTokensParams& args, const JSONValue& responseId);
    SlangResult signatureHelp(
        const LanguageServerProtocol::SignatureHelpParams& args, const JSONValue& responseId);

    List<LanguageServerProtocol::CompletionItem> collectMembers(
        WorkspaceVersion* wsVersion, Module* module, Expr* baseExpr);

private:
    SlangResult _executeSingle();
    slang::IGlobalSession* getOrCreateGlobalSession();
    void resetDiagnosticUpdateTime();
    void publishDiagnostics();
};


SlangResult LanguageServer::init(const InitializeParams& args)
{
    SLANG_RETURN_ON_FAIL(m_connection->initWithStdStreams(JSONRPCConnection::CallStyle::Object));
    m_workspaceFolders = args.workspaceFolders;
    m_workspace = new Workspace();
    List<URI> rootUris;
    for (auto& wd : m_workspaceFolders)
    {
        rootUris.add(URI::fromString(wd.uri.getUnownedSlice()));
    }
    m_workspace->init(rootUris, getOrCreateGlobalSession());
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

void LanguageServer::resetDiagnosticUpdateTime() { time(&m_lastDiagnosticUpdateTime); }

String uriToCanonicalPath(const String& uri)
{
    String canonnicalPath;
    Path::getCanonical(URI::fromString(uri.getUnownedSlice()).getPath(), canonnicalPath);
    return canonnicalPath;
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
                InitializeParams args;
                m_connection->toNativeArgsOrSendError(call.params, &args, call.id);

                init(args);

                InitializeResult result;
                result.serverInfo.name = "SlangLanguageServer";
                result.serverInfo.version = "1.0";
                result.capabilities.positionEncoding = "utf-8";
                result.capabilities.textDocumentSync.openClose = true;
                result.capabilities.textDocumentSync.change = (int)TextDocumentSyncKind::Full;
                result.capabilities.hoverProvider = true;
                result.capabilities.definitionProvider = true;
                const char* commitChars[] = {",", ".", ";", ":", "(", ")", "[", "]",
                                             "<", ">", "{", "}", "*", "&", "^", "%",
                                             "!", "-", "=", "+", "|", "/", "?"};
                for (auto ch : commitChars)
                    result.capabilities.completionProvider.allCommitCharacters.add(ch);
                result.capabilities.completionProvider.triggerCharacters.add(".");
                result.capabilities.completionProvider.triggerCharacters.add(":");
                result.capabilities.completionProvider.resolveProvider = true;
                result.capabilities.completionProvider.workDoneToken = "";
                result.capabilities.semanticTokensProvider.full = true;
                result.capabilities.semanticTokensProvider.range = false;
                result.capabilities.signatureHelpProvider.triggerCharacters.add("(");
                result.capabilities.signatureHelpProvider.retriggerCharacters.add(",");
                for (auto tokenType : kSemanticTokenTypes)
                    result.capabilities.semanticTokensProvider.legend.tokenTypes.add(tokenType);
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
            else if (call.method == HoverParams::methodName)
            {
                HoverParams args;
                SLANG_RETURN_ON_FAIL(
                    m_connection->toNativeArgsOrSendError(call.params, &args, call.id));
                return hover(args, call.id);
            }
            else if (call.method == DefinitionParams::methodName)
            {
                DefinitionParams args;
                SLANG_RETURN_ON_FAIL(
                    m_connection->toNativeArgsOrSendError(call.params, &args, call.id));
                return gotoDefinition(args, call.id);
            }
            else if (call.method == CompletionParams::methodName)
            {
                CompletionParams args;
                SLANG_RETURN_ON_FAIL(
                    m_connection->toNativeArgsOrSendError(call.params, &args, call.id));
                return completion(args, call.id);
            }
            else if (call.method == SemanticTokensParams::methodName)
            {
                SemanticTokensParams args;
                SLANG_RETURN_ON_FAIL(
                    m_connection->toNativeArgsOrSendError(call.params, &args, call.id));
                return semanticTokens(args, call.id);
            }
            else if (call.method == SignatureHelpParams::methodName)
            {
                SignatureHelpParams args;
                SLANG_RETURN_ON_FAIL(
                    m_connection->toNativeArgsOrSendError(call.params, &args, call.id));
                return signatureHelp(args, call.id);
            }
            else if (call.method == "completionItem/resolve")
            {
                CompletionItem args;
                SLANG_RETURN_ON_FAIL(
                    m_connection->toNativeArgsOrSendError(call.params, &args, call.id));
                return completionResolve(args, call.id);
                
            }
            else if (call.method == "initialized")
            {
                return SLANG_OK;
            }
            else if (call.method.startsWith("$/"))
            {
                // Ignore.
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
}

SlangResult LanguageServer::didOpenTextDocument(const DidOpenTextDocumentParams& args)
{
    String canonicalPath = uriToCanonicalPath(args.textDocument.uri);
    m_workspace->openDoc(canonicalPath, args.textDocument.text);
    return SLANG_OK;
}

String getDeclSignatureString(DeclRef<Decl> declRef, ASTBuilder* astBuilder)
{
    if (declRef.getDecl())
    {
        ASTPrinter printer(
            astBuilder,
            ASTPrinter::OptionFlag::ParamNames | ASTPrinter::OptionFlag::NoInternalKeywords |
                ASTPrinter::OptionFlag::SimplifiedBuiltinType);
        printer.addDeclSignature(declRef);
        return printer.getString();
    }
    return "unknown";
}


static String _formatDocumentation(String doc)
{
    // TODO: may want to use DocMarkdownWriter in the future to format the text.
    // For now just insert line breaks before `\param` and `\returns` markups.
    List<UnownedStringSlice> lines;
    StringUtil::split(doc.getUnownedSlice(), '\n', lines);
    StringBuilder result;
    
    for (Index i = 0; i < lines.getCount(); i++)
    {
        auto trimedLine = lines[i].trimStart();
        if (i > 0)
        {
            if (trimedLine.startsWith("\\") && lines[i - 1].trim().getLength() != 0)
            {
                result << "  \n";
            }
            else
            {
                result << "\n";
            }
        }
        if (trimedLine.startsWith("\\returns "))
        {
            trimedLine = trimedLine.subString(9, trimedLine.getLength());
            result << "**returns** ";
        }
        else if (trimedLine.startsWith("\\return "))
        {
            trimedLine = trimedLine.subString(8, trimedLine.getLength());
            result << "**Returns** ";
        }
        result << trimedLine;
    }
    result << "\n";
    return result.ProduceString();
}

static void _tryGetDocumentation(StringBuilder& sb, WorkspaceVersion* workspace, Decl* decl)
{
    auto definingModule = getModuleDecl(decl);
    if (definingModule)
    {
        auto markupAST = workspace->getOrCreateMarkupAST(definingModule);
        auto markupEntry = markupAST->getEntry(decl);
        if (markupEntry)
        {
            sb << "\n";
            sb << _formatDocumentation(markupEntry->m_markup);
            sb << "\n";
        }
    }
}

SlangResult LanguageServer::hover(
    const LanguageServerProtocol::HoverParams& args, const JSONValue& responseId)
{
    String canonicalPath = uriToCanonicalPath(args.textDocument.uri);
    auto version = m_workspace->getCurrentVersion();
    Module* parsedModule = version->getOrLoadModule(canonicalPath);
    if (!parsedModule)
    {
        m_connection->sendResult(NullResponse::get(), responseId);
        return SLANG_OK;
    }
    auto findResult = findASTNodesAt(
        version->linkage->getSourceManager(),
        parsedModule->getModuleDecl(),
        ASTLookupType::Decl,
        canonicalPath.getUnownedSlice(),
        args.position.line + 1,
        args.position.character + 1);
    if (findResult.getCount() == 0 || findResult[0].path.getCount() == 0)
    {
        m_connection->sendResult(NullResponse::get(), responseId);
        return SLANG_OK;
    }
    StringBuilder sb;

    Hover hover;
    auto leafNode = findResult[0].path.getLast();
    auto fillDeclRefHoverInfo = [&](DeclRef<Decl> declRef)
    {
        if (declRef.getDecl())
        {
            sb << "```\n"
                << getDeclSignatureString(declRef, version->linkage->getASTBuilder())
                << "\n```\n";
            
            _tryGetDocumentation(sb, version, declRef.getDecl());

            auto humaneLoc = version->linkage->getSourceManager()->getHumaneLoc(
                declRef.getLoc(), SourceLocType::Actual);
            sb << "Defined in " << humaneLoc.pathInfo.foundPath << "(" << humaneLoc.line
                << ")\n";

            auto nodeHumaneLoc =
                version->linkage->getSourceManager()->getHumaneLoc(leafNode->loc);
            hover.range.start.line = int(nodeHumaneLoc.line - 1);
            hover.range.end.line = int(nodeHumaneLoc.line - 1);
            hover.range.start.character = int(nodeHumaneLoc.column - 1);
            if (declRef.getName())
            {
                hover.range.end.character =
                    int(nodeHumaneLoc.column + declRef.getName()->text.getLength() - 1);
            }
        }
    };
    if (auto declRefExpr = as<DeclRefExpr>(leafNode))
    {
        fillDeclRefHoverInfo(declRefExpr->declRef);
    }
    else if (auto overloadedExpr = as<OverloadedExpr>(leafNode))
    {
        LookupResultItem& item = overloadedExpr->lookupResult2.item;
        fillDeclRefHoverInfo(item.declRef);
    }
    else if (auto decl = as<Decl>(leafNode))
    {
        fillDeclRefHoverInfo(DeclRef<Decl>(decl, nullptr));
    }
    if (sb.getLength() == 0)
    {
        m_connection->sendResult(NullResponse::get(), responseId);
        return SLANG_OK;
    }
    else
    {
        hover.contents.kind = "markdown";
        hover.contents.value = sb.ProduceString();
        m_connection->sendResult(&hover, responseId);
        return SLANG_OK;
    }
}

SlangResult LanguageServer::gotoDefinition(
    const LanguageServerProtocol::DefinitionParams& args, const JSONValue& responseId)
{
    String canonicalPath = uriToCanonicalPath(args.textDocument.uri);
    auto version = m_workspace->getCurrentVersion();
    Module* parsedModule = version->getOrLoadModule(canonicalPath);
    if (!parsedModule)
    {
        m_connection->sendResult(NullResponse::get(), responseId);
        return SLANG_OK;
    }
    auto findResult = findASTNodesAt(
        version->linkage->getSourceManager(),
        parsedModule->getModuleDecl(),
        ASTLookupType::Decl,
        canonicalPath.getUnownedSlice(),
        args.position.line + 1,
        args.position.character + 1);
    if (findResult.getCount() == 0 || findResult[0].path.getCount() == 0)
    {
        m_connection->sendResult(NullResponse::get(), responseId);
        return SLANG_OK;
    }
    struct LocationResult
    {
        HumaneSourceLoc loc;
        int length;
    };
    List<LocationResult> locations;
    auto leafNode = findResult[0].path.getLast();
    if (auto declRefExpr = as<DeclRefExpr>(leafNode))
    {
        if (declRefExpr->declRef.getDecl())
        {
            auto location = version->linkage->getSourceManager()->getHumaneLoc(
                declRefExpr->declRef.getNameLoc(), SourceLocType::Actual);
            auto name = declRefExpr->declRef.getName();
            locations.add(LocationResult{location, name ? (int)name->text.getLength() : 0});
        }
    }
    else if (auto overloadedExpr = as<OverloadedExpr>(leafNode))
    {
        if (overloadedExpr->lookupResult2.items.getCount())
        {
            for (auto item : overloadedExpr->lookupResult2.items)
            {
                auto location = version->linkage->getSourceManager()->getHumaneLoc(
                    item.declRef.getNameLoc(), SourceLocType::Actual);
                auto name = item.declRef.getName();
                locations.add(LocationResult{location, name ? (int)name->text.getLength() : 0});
            }
        }
        else 
        {
            LookupResultItem& item = overloadedExpr->lookupResult2.item;
            if (item.declRef.getDecl() != nullptr)
            {
                auto location = version->linkage->getSourceManager()->getHumaneLoc(
                    item.declRef.getNameLoc(), SourceLocType::Actual);
                auto name = item.declRef.getName();
                locations.add(LocationResult{location, name ? (int)name->text.getLength() : 0});
            }
        }
        
    }
    if (locations.getCount() == 0)
    {
        m_connection->sendResult(NullResponse::get(), responseId);
        return SLANG_OK;
    }
    else
    {
        List<Location> results;
        for (auto loc : locations)
        {
            Location result;
            result.uri = URI::fromLocalFilePath(loc.loc.pathInfo.foundPath.getUnownedSlice()).uri;
            result.range.start.line = int(loc.loc.line - 1);
            result.range.start.character = int(loc.loc.column - 1);
            result.range.end = result.range.start;
            result.range.end.character += loc.length;
            results.add(result);
        }
        m_connection->sendResult(&results, responseId);
        return SLANG_OK;
    }
}

bool _isIdentifierChar(char ch)
{
    return ch >= '0' && ch <= '9' || ch >= 'a' && ch <= 'z' || ch >= 'A' && ch <= 'Z' || ch == '_';
}

bool _isWhitespaceChar(char ch)
{
    return ch == ' ' || ch == '\r' || ch == '\n' || ch == '\t';
}

SlangResult LanguageServer::completion(
    const LanguageServerProtocol::CompletionParams& args, const JSONValue& responseId)
{
    String canonicalPath = uriToCanonicalPath(args.textDocument.uri);

    RefPtr<DocumentVersion> doc;
    if (!m_workspace->openedDocuments.TryGetValue(canonicalPath, doc))
    {
        m_connection->sendResult(NullResponse::get(), responseId);
        return SLANG_OK;
    }
    
    auto cursorOffset = doc->getOffset(args.position.line + 1, args.position.character + 1);
    if (cursorOffset == -1 || doc->getText().getLength() == 0)
    {
        m_connection->sendResult(NullResponse::get(), responseId);
        return SLANG_OK;
    }
    // Scan backward until we locate a '.' or ':'.
    if (cursorOffset == doc->getText().getLength())
        cursorOffset--;
    while (cursorOffset > 0 && _isWhitespaceChar(doc->getText()[cursorOffset]))
    {
        cursorOffset--;
    }
    while (cursorOffset > 0 && _isIdentifierChar(doc->getText()[cursorOffset]))
    {
        cursorOffset--;
    }
    while (cursorOffset > 0 && _isWhitespaceChar(doc->getText()[cursorOffset]))
    {
        cursorOffset--;
    }
    if (cursorOffset > 0 && doc->getText()[cursorOffset] == ':')
        cursorOffset--;
    if (cursorOffset <= 0 ||
        (doc->getText()[cursorOffset] != '.' && doc->getText()[cursorOffset] != ':'))
    {
        m_connection->sendResult(NullResponse::get(), responseId);
        return SLANG_OK;
    }
    Index line = 0;
    Index col = 0;
    doc->offsetToLineCol(cursorOffset, line, col);
    auto version = m_workspace->getCurrentVersion();
    Module* parsedModule = version->getOrLoadModule(canonicalPath);
    if (!parsedModule)
    {
        m_connection->sendResult(NullResponse::get(), responseId);
        return SLANG_OK;
    }
    auto findResult = findASTNodesAt(
        version->linkage->getSourceManager(),
        parsedModule->getModuleDecl(),
        ASTLookupType::Decl,
        canonicalPath.getUnownedSlice(),
        line,
        col);
    if (findResult.getCount() != 1)
    {
        m_connection->sendResult(NullResponse::get(), responseId);
        return SLANG_OK;
    }
    if (findResult[0].path.getCount() == 0)
    {
        m_connection->sendResult(NullResponse::get(), responseId);
        return SLANG_OK;
    }
    Expr* baseExpr = nullptr;
    if (auto memberExpr = as<MemberExpr>(findResult[0].path.getLast()))
    {
        baseExpr = memberExpr->baseExpression;
    }
    else if (auto staticMemberExpr = as<StaticMemberExpr>(findResult[0].path.getLast()))
    {
        baseExpr = staticMemberExpr->baseExpression;
    }
    else if (auto swizzleExpr = as<SwizzleExpr>(findResult[0].path.getLast()))
    {
        baseExpr = swizzleExpr->base;
    }
    else if (auto matSwizzleExpr = as<MatrixSwizzleExpr>(findResult[0].path.getLast()))
    {
        baseExpr = matSwizzleExpr->base;
    }
    if (!baseExpr || !baseExpr->type.type || baseExpr->type.type->equals(version->linkage->getASTBuilder()->getErrorType()))
    {
        m_connection->sendResult(NullResponse::get(), responseId);
        return SLANG_OK;
    }

    List<LanguageServerProtocol::CompletionItem> items = collectMembers(version, parsedModule, baseExpr);
    m_connection->sendResult(&items, responseId);
    return SLANG_OK;
}

SlangResult LanguageServer::completionResolve(
    const LanguageServerProtocol::CompletionItem& args, const JSONValue& responseId)
{
    LanguageServerProtocol::CompletionItem resolvedItem = args;
    int itemId = StringToInt(args.data);
    auto version = m_workspace->getCurrentVersion();
    if (itemId >= 0 && itemId < version->currentCompletionItems.getCount())
    {
        auto decl = version->currentCompletionItems[itemId];
        resolvedItem.detail = getDeclSignatureString(
            DeclRef<Decl>(decl, nullptr), version->linkage->getASTBuilder());
        StringBuilder docSB;
        _tryGetDocumentation(docSB, version, decl);
        resolvedItem.documentation.value = docSB.ProduceString();
        resolvedItem.documentation.kind = "markdown";
    }
    m_connection->sendResult(&resolvedItem, responseId);
    return SLANG_OK;
}

SlangResult LanguageServer::semanticTokens(
    const LanguageServerProtocol::SemanticTokensParams& args, const JSONValue& responseId)
{
    String canonicalPath = uriToCanonicalPath(args.textDocument.uri);

    auto version = m_workspace->getCurrentVersion();
    Module* parsedModule = version->getOrLoadModule(canonicalPath);
    if (!parsedModule)
    {
        m_connection->sendResult(NullResponse::get(), responseId);
        return SLANG_OK;
    }

    auto tokens = getSemanticTokens(version->linkage, parsedModule, canonicalPath.getUnownedSlice());
    SemanticTokens response;
    response.resultId = "";
    response.data = getEncodedTokens(tokens);
    m_connection->sendResult(&response, responseId);
    return SLANG_OK;
}

SlangResult LanguageServer::signatureHelp(
    const LanguageServerProtocol::SignatureHelpParams& args, const JSONValue& responseId)
{
    String canonicalPath = uriToCanonicalPath(args.textDocument.uri);

    auto version = m_workspace->getCurrentVersion();
    Module* parsedModule = version->getOrLoadModule(canonicalPath);
    if (!parsedModule)
    {
        m_connection->sendResult(NullResponse::get(), responseId);
        return SLANG_OK;
    }

    auto findResult = findASTNodesAt(
        version->linkage->getSourceManager(),
        parsedModule->getModuleDecl(),
        ASTLookupType::Invoke,
        canonicalPath.getUnownedSlice(),
        args.position.line + 1,
        args.position.character + 1);

    if (findResult.getCount() == 0)
    {
        m_connection->sendResult(NullResponse::get(), responseId);
        return SLANG_OK;
    }

    AppExprBase* appExpr = nullptr;
    auto& declPath = findResult[0].path;
    for (Index i = declPath.getCount() - 1; i >= 0; i--)
    {
        if (auto expr = as<AppExprBase>(declPath[i]))
        {
            // Find the inner most invoke expr that has source token info.
            // This allows us to skip the invoke expr nodes for operators/implcit casts.
            if (expr->argumentDelimeterLocs.getCount())
            {
                appExpr = expr;
                break;
            }
        }
    }
    if (!appExpr)
    {
        m_connection->sendResult(NullResponse::get(), responseId);
        return SLANG_OK;
    }

    if (appExpr->argumentDelimeterLocs.getCount() == 0)
    {
        m_connection->sendResult(NullResponse::get(), responseId);
        return SLANG_OK;
    }

    auto funcExpr =
        appExpr->originalFunctionExpr ? appExpr->originalFunctionExpr : appExpr->functionExpr;
    if (!funcExpr)
    {
        m_connection->sendResult(NullResponse::get(), responseId);
        return SLANG_OK;
    }

    SignatureHelp response;
    auto addDeclRef = [&](DeclRef<Decl> declRef)
    {
        if (!declRef.getDecl())
            return;

        SignatureInformation sigInfo;

        List<Slang::Range<Index>> paramRanges;
        ASTPrinter printer(
            version->linkage->getASTBuilder(),
            ASTPrinter::OptionFlag::ParamNames | ASTPrinter::OptionFlag::NoInternalKeywords |
                ASTPrinter::OptionFlag::SimplifiedBuiltinType);
        
        printer.addDeclKindPrefix(declRef.getDecl());
        printer.addDeclPath(declRef);
        printer.addDeclParams(declRef, &paramRanges);
        printer.addDeclResultType(declRef);

        sigInfo.label = printer.getString();

        StringBuilder docSB;
        auto humaneLoc = version->linkage->getSourceManager()->getHumaneLoc(declRef.getLoc(), SourceLocType::Actual);
        _tryGetDocumentation(docSB, version, declRef.getDecl());

        docSB << "Defined in " << humaneLoc.pathInfo.foundPath << "(" << humaneLoc.line << ")\n";
        sigInfo.documentation.value = docSB.ProduceString();
        sigInfo.documentation.kind = "markdown";

        for (auto& range : paramRanges)
        {
            ParameterInformation paramInfo;
            paramInfo.label[0] = (uint32_t)range.begin;
            paramInfo.label[1] = (uint32_t)range.end;
            sigInfo.parameters.add(paramInfo);
        }
        response.signatures.add(sigInfo);
    };
    if (auto declRefExpr = as<DeclRefExpr>(funcExpr))
    {
        addDeclRef(declRefExpr->declRef);
    }
    else if (auto overloadedExpr = as<OverloadedExpr>(funcExpr))
    {
        for (auto item : overloadedExpr->lookupResult2)
        {
            addDeclRef(item.declRef);
        }
    }
    response.activeSignature = 0;
    response.activeParameter = 0;
    for (int i = 1; i < appExpr->argumentDelimeterLocs.getCount(); i++)
    {
        auto delimLoc = version->linkage->getSourceManager()->getHumaneLoc(
            appExpr->argumentDelimeterLocs[i], SourceLocType::Actual);
        if (delimLoc.line > args.position.line + 1 ||
            delimLoc.line == args.position.line + 1 && delimLoc.column >= args.position.character + 1)
        {
            response.activeParameter = i - 1;
            break;
        }
    }

    m_connection->sendResult(&response, responseId);
    return SLANG_OK;
}


List<LanguageServerProtocol::CompletionItem> LanguageServer::collectMembers(WorkspaceVersion* version, Module* module, Expr* baseExpr)
{
    List<LanguageServerProtocol::CompletionItem> result;
    auto linkage = version->linkage;
    Type* type = baseExpr->type.type;
    if (auto typeType = as<TypeType>(type))
    {
        type = typeType->type;
    }
    version->currentCompletionItems.clear();
    if (type)
    {
        if (as<ArithmeticExpressionType>(type))
        {
            // Hard code members for vector and matrix types.
            result.clear();
            version->currentCompletionItems.clear();
            int elementCount = 0;
            Type* elementType = nullptr;
            const char* memberNames[4] = {"x", "y", "z", "w"};
            if (auto vectorType = as<VectorExpressionType>(type))
            {
                if (auto elementCountVal = as<ConstantIntVal>(vectorType->elementCount))
                {
                    elementCount = (int)elementCountVal->value;
                    elementType = vectorType->elementType;
                }
            }
            else if (auto matrixType = as<MatrixExpressionType>(type))
            {
                if (auto elementCountVal = as<ConstantIntVal>(matrixType->getRowCount()))
                {
                    elementCount = (int)elementCountVal->value;
                    elementType = matrixType->getRowType();
                }
            }
            String typeStr;
            if (elementType)
                typeStr = elementType->toString();
            for (int i = 0; i < elementCount; i++)
            {
                CompletionItem item;
                item.data = 0;
                item.detail = typeStr;
                item.kind = LanguageServerProtocol::kCompletionItemKindVariable;
                item.label = memberNames[i];
                result.add(item);
            }
        }
        else
        {
            DiagnosticSink sink;
            MemberCollectingContext context(linkage, module, &sink);
            context.astBuilder = linkage->getASTBuilder();
            collectMembersInType(&context, type);
            HashSet<String> deduplicateSet;
            for (auto member : context.members)
            {
                CompletionItem item;
                item.label = member->getName()->text;
                item.kind = 0;
                if (as<TypeConstraintDecl>(member))
                {
                    continue;
                }
                if (as<ConstructorDecl>(member))
                {
                    continue;
                }
                if (as<SubscriptDecl>(member))
                {
                    continue;
                }

                if (item.label.startsWith("$"))
                    continue;
                if (!deduplicateSet.Add(item.label))
                    continue;

                if (as<StructDecl>(member))
                {
                    item.kind = LanguageServerProtocol::kCompletionItemKindStruct;
                }
                else if (as<ClassDecl>(member))
                {
                    item.kind = LanguageServerProtocol::kCompletionItemKindClass;
                }
                else if (as<InterfaceDecl>(member))
                {
                    item.kind = LanguageServerProtocol::kCompletionItemKindInterface;
                }
                else if (as<SimpleTypeDecl>(member))
                {
                    item.kind = LanguageServerProtocol::kCompletionItemKindClass;
                }
                else if (as<PropertyDecl>(member))
                {
                    item.kind = LanguageServerProtocol::kCompletionItemKindProperty;
                }
                else if (as<EnumDecl>(member))
                {
                    item.kind = LanguageServerProtocol::kCompletionItemKindEnum;
                }
                else if (as<VarDeclBase>(member))
                {
                    item.kind = LanguageServerProtocol::kCompletionItemKindVariable;
                }
                else if (as<EnumCaseDecl>(member))
                {
                    item.kind = LanguageServerProtocol::kCompletionItemKindEnumMember;
                }
                else if (as<CallableDecl>(member))
                {
                    item.kind = LanguageServerProtocol::kCompletionItemKindMethod;
                }
                else if (as<AssocTypeDecl>(member))
                {
                    item.kind = LanguageServerProtocol::kCompletionItemKindClass;
                }
                item.data = String(version->currentCompletionItems.getCount());
                result.add(item);
                version->currentCompletionItems.add(member);
            }
        }

        for (auto& item : result)
        {
            switch (item.kind)
            {
            case LanguageServerProtocol::kCompletionItemKindMethod:
                item.commitCharacters.add("(");
                item.commitCharacters.add("[");
                item.commitCharacters.add(" ");
                break;
            default:
                item.commitCharacters.add("(");
                item.commitCharacters.add(")");
                item.commitCharacters.add(".");
                item.commitCharacters.add(";");
                item.commitCharacters.add(":");
                item.commitCharacters.add(",");
                item.commitCharacters.add("<");
                item.commitCharacters.add(">");
                item.commitCharacters.add("[");
                item.commitCharacters.add("]");
                item.commitCharacters.add("{");
                item.commitCharacters.add("}");
                item.commitCharacters.add("-");
                item.commitCharacters.add("*");
                item.commitCharacters.add("/");
                item.commitCharacters.add("%");
                item.commitCharacters.add("+");
                item.commitCharacters.add("=");
                item.commitCharacters.add("&");
                item.commitCharacters.add("|");
                item.commitCharacters.add("!");
                item.commitCharacters.add(" ");
                break;
            }
        }
    }
    return result;
}

void LanguageServer::publishDiagnostics()
{
    time_t timeNow = 0;
    time(&timeNow);

    if (timeNow - m_lastDiagnosticUpdateTime < 3)
    {
        return;
    }
    m_lastDiagnosticUpdateTime = timeNow;

    auto version = m_workspace->getCurrentVersion();
    // Send updates to clear diagnostics for files that no longer have any messages.
    List<String> filesToRemove;
    for (auto& file : m_lastPublishedDiagnostics)
    {
        if (!version->diagnostics.ContainsKey(file.Key))
        {
            PublishDiagnosticsParams args;
            args.uri = URI::fromLocalFilePath(file.Key.getUnownedSlice()).uri;
            m_connection->sendCall(UnownedStringSlice("textDocument/publishDiagnostics"), &args);
            filesToRemove.add(file.Key);
        }
    }
    for (auto& toRemove : filesToRemove)
    {
        m_lastPublishedDiagnostics.Remove(toRemove);
    }
    // Send updates for any files whose diagnostic messages has changed since last update.
    for (auto& list : version->diagnostics)
    {
        auto lastPublished = m_lastPublishedDiagnostics.TryGetValue(list.Key);
        if (!lastPublished || *lastPublished != list.Value.originalOutput)
        {
            PublishDiagnosticsParams args;
            args.uri = URI::fromLocalFilePath(list.Key.getUnownedSlice()).uri;
            for (auto& d : list.Value.messages)
                args.diagnostics.add(d);
            m_connection->sendCall(UnownedStringSlice("textDocument/publishDiagnostics"), &args);
            m_lastPublishedDiagnostics[list.Key] = list.Value.originalOutput;
        }
    }
}

SlangResult LanguageServer::didCloseTextDocument(const DidCloseTextDocumentParams& args)
{
    String canonicalPath = uriToCanonicalPath(args.textDocument.uri);
    m_workspace->openedDocuments.Remove(canonicalPath);
    m_workspace->invalidate();
    resetDiagnosticUpdateTime();
    return SLANG_OK;
}
SlangResult LanguageServer::didChangeTextDocument(const DidChangeTextDocumentParams& args)
{
    String canonicalPath = uriToCanonicalPath(args.textDocument.uri);

    RefPtr<DocumentVersion> doc;
    if (m_workspace->openedDocuments.TryGetValue(canonicalPath, doc))
    {
        doc->setText(args.contentChanges[0].text.getUnownedSlice());
    }
    m_workspace->invalidate();
    resetDiagnosticUpdateTime();
    return SLANG_OK;
}

void LanguageServer::update()
{
    if (!m_workspace)
        return;
    publishDiagnostics();
}

SlangResult LanguageServer::execute()
{

    m_connection = new JSONRPCConnection();
    m_connection->initWithStdStreams();
    
    while (m_connection->isActive() && !m_quit)
    {
        // Consume all messages first.
        while (true)
        {
            m_connection->tryReadMessage();
            if (!m_connection->hasMessage())
                break;
            const SlangResult res = _executeSingle();

        }

        // Now we can use this time to reparse user's code, report diagnostics, etc.
        update();
    }

    return SLANG_OK;
}

SLANG_API SlangResult runLanguageServer()
{
    Slang::LanguageServer server;
    SLANG_RETURN_ON_FAIL(server.execute());
    return SLANG_OK;
}

} // namespace Slang
