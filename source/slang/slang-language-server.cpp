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
#include "../compiler-core/slang-json-native.h"
#include "../compiler-core/slang-json-rpc-connection.h"
#include "../compiler-core/slang-language-server-protocol.h"
#include "slang-language-server.h"
#include "slang-workspace-version.h"
#include "slang-language-server-ast-lookup.h"
#include "slang-language-server-completion.h"
#include "slang-language-server-semantic-tokens.h"
#include "slang-language-server-document-symbols.h"
#include "slang-ast-print.h"
#include "slang-doc-markdown-writer.h"
#include "slang-mangle.h"
#include "../../tools/platform/performance-counter.h"
namespace Slang
{
using namespace LanguageServerProtocol;

ArrayView<const char*> getCommitChars()
{
    static const char* _commitCharsArray[] = {",", ".", ";", ":", "(", ")", "[", "]",
                                              "<", ">", "{", "}", "*", "&", "^", "%",
                                              "!", "-", "=", "+", "|", "/", "?", " "};
    return makeArrayView(_commitCharsArray, SLANG_COUNT_OF(_commitCharsArray));
}

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

SlangResult LanguageServer::parseNextMessage()
{
    const JSONRPCMessageType msgType = m_connection->getMessageType();

    switch (msgType)
    {
    case JSONRPCMessageType::Call:
        {
            JSONRPCCall call;
            SLANG_RETURN_ON_FAIL(m_connection->getRPCOrSendError(&call));
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
                result.capabilities.positionEncoding = "utf-16";
                result.capabilities.textDocumentSync.openClose = true;
                result.capabilities.textDocumentSync.change = (int)TextDocumentSyncKind::Incremental;
                result.capabilities.workspace.workspaceFolders.supported = true;
                result.capabilities.workspace.workspaceFolders.changeNotifications = false;
                result.capabilities.hoverProvider = true;
                result.capabilities.definitionProvider = true;
                result.capabilities.documentSymbolProvider = true;
                result.capabilities.completionProvider.triggerCharacters.add(".");
                result.capabilities.completionProvider.triggerCharacters.add(":");
                result.capabilities.completionProvider.triggerCharacters.add("[");
                result.capabilities.completionProvider.resolveProvider = true;
                result.capabilities.completionProvider.workDoneToken = "";
                for (auto ch : getCommitChars())
                    result.capabilities.completionProvider.allCommitCharacters.add(ch);
                result.capabilities.semanticTokensProvider.full = true;
                result.capabilities.semanticTokensProvider.range = false;
                result.capabilities.signatureHelpProvider.triggerCharacters.add("(");
                result.capabilities.signatureHelpProvider.retriggerCharacters.add(",");
                for (auto tokenType : kSemanticTokenTypes)
                    result.capabilities.semanticTokensProvider.legend.tokenTypes.add(tokenType);
                m_connection->sendResult(&result, call.id);
                return SLANG_OK;
            }
            else if (call.method == "initialized")
            {
                sendConfigRequest();
                registerCapability("workspace/didChangeConfiguration");
                m_initialized = true;
                return SLANG_OK;
            }
            else
            {
                queueJSONCall(call);
                return SLANG_OK;
            }

        }
    case JSONRPCMessageType::Result:
        {
            JSONResultResponse response;
            SLANG_RETURN_ON_FAIL(m_connection->getRPCOrSendError(&response));
            auto responseId = (int)m_connection->getContainer()->asInteger(response.id);
            switch (responseId)
            {
            case kConfigResponseId:
                if (response.result.getKind() == JSONValue::Kind::Array)
                {
                    auto arr = m_connection->getContainer()->getArray(response.result);
                    if (arr.getCount() == 3)
                    {
                        updatePredefinedMacros(arr[0]);
                        updateSearchPaths(arr[1]);
                        updateSearchInWorkspace(arr[2]);
                    }
                }
                break;
            }
            return SLANG_OK;
        }
    case JSONRPCMessageType::Error:
        {
#if 0 // Enable for debug only
            JSONRPCErrorResponse error;
            SLANG_RETURN_ON_FAIL(m_connection->getRPCOrSendError(&error));
#endif
            return SLANG_OK;
        }
        break;
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
        if (auto varDecl = as<VarDeclBase>(declRef.getDecl()))
        {
            auto& sb = printer.getStringBuilder();
            if (auto litExpr = as<LiteralExpr>(varDecl->initExpr))
            {
                sb << " = " << litExpr->token.getContent();
            }
        }
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
            if ((trimedLine.startsWith("\\") || trimedLine.startsWith("@")) && lines[i - 1].trim().getLength() != 0)
            {
                result << "  \n";
            }
            else
            {
                result << "\n";
            }
        }
        if (trimedLine.startsWith("\\") || trimedLine.startsWith("@"))
        {
            trimedLine = trimedLine.tail(1);
            if (trimedLine.startsWith("returns "))
            {
                trimedLine = trimedLine.subString(8, trimedLine.getLength());
                result << "**returns** ";
            }
            else if (trimedLine.startsWith("return "))
            {
                trimedLine = trimedLine.subString(7, trimedLine.getLength());
                result << "**Returns** ";
            }
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
    RefPtr<DocumentVersion> doc;
    if (!m_workspace->openedDocuments.TryGetValue(canonicalPath, doc))
    {
        m_connection->sendResult(NullResponse::get(), responseId);
        return SLANG_OK;
    }
    Index line, col;
    doc->zeroBasedUTF16LocToOneBasedUTF8Loc(args.position.line, args.position.character, line, col);

    auto version = m_workspace->getCurrentVersion();
    Module* parsedModule = version->getOrLoadModule(canonicalPath);
    if (!parsedModule)
    {
        m_connection->sendResult(NullResponse::get(), responseId);
        return SLANG_OK;
    }
    auto findResult = findASTNodesAt(
        doc.Ptr(),
        version->linkage->getSourceManager(),
        parsedModule->getModuleDecl(),
        ASTLookupType::Decl,
        canonicalPath.getUnownedSlice(),
        line,
        col);
    if (findResult.getCount() == 0 || findResult[0].path.getCount() == 0)
    {
        if (SLANG_SUCCEEDED(tryGetMacroHoverInfo(version, doc, line, col, responseId)))
            return SLANG_OK;
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
            auto name = declRef.getName();
            if (auto ctorDecl = declRef.as<ConstructorDecl>())
            {
                auto parent = ctorDecl.getDecl()->parentDecl;
                if (parent)
                {
                    name = parent->getName();
                }
            }
            if (name)
            {
                hover.range.end.character = int(nodeHumaneLoc.column + name->text.getLength() - 1);
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
    RefPtr<DocumentVersion> doc;
    if (!m_workspace->openedDocuments.TryGetValue(canonicalPath, doc))
    {
        m_connection->sendResult(NullResponse::get(), responseId);
        return SLANG_OK;
    }
    Index line, col;
    doc->zeroBasedUTF16LocToOneBasedUTF8Loc(args.position.line, args.position.character, line, col);

    auto version = m_workspace->getCurrentVersion();
    Module* parsedModule = version->getOrLoadModule(canonicalPath);
    if (!parsedModule)
    {
        m_connection->sendResult(NullResponse::get(), responseId);
        return SLANG_OK;
    }
    auto findResult = findASTNodesAt(
        doc.Ptr(),
        version->linkage->getSourceManager(),
        parsedModule->getModuleDecl(),
        ASTLookupType::Decl,
        canonicalPath.getUnownedSlice(),
        line,
        col);
    if (findResult.getCount() == 0 || findResult[0].path.getCount() == 0)
    {
        if (SLANG_SUCCEEDED(tryGotoMacroDefinition(version, doc, line, col, responseId)))
            return SLANG_OK;
        if (SLANG_SUCCEEDED(tryGotoFileInclude(version, doc, line, responseId)))
            return SLANG_OK;
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
                    declRefExpr->declRef.getNameLoc().isValid() ? declRefExpr->declRef.getNameLoc()
                                                                : declRefExpr->declRef.getLoc(),
                    SourceLocType::Actual);
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
    else if (auto importDecl = as<ImportDecl>(leafNode))
    {
        if (importDecl->importedModuleDecl)
        {
            if (importDecl->importedModuleDecl->members.getCount() &&
                importDecl->importedModuleDecl->members[0])
            {
                auto loc = importDecl->importedModuleDecl->members[0]->loc;
                if (loc.isValid())
                {
                    auto location = version->linkage->getSourceManager()->getHumaneLoc(loc, SourceLocType::Actual);
                    locations.add(LocationResult{location, 0});
                }
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
            if (File::exists(loc.loc.pathInfo.foundPath))
            {
                result.uri =
                    URI::fromLocalFilePath(loc.loc.pathInfo.foundPath.getUnownedSlice()).uri;
                result.range.start.line = int(loc.loc.line - 1);
                result.range.start.character = int(loc.loc.column - 1);
                result.range.end = result.range.start;
                result.range.end.character += loc.length;
                results.add(result);
            }
        }
        m_connection->sendResult(&results, responseId);
        return SLANG_OK;
    }
}

static bool _isIdentifierChar(char ch)
{
    return ch >= '0' && ch <= '9' || ch >= 'a' && ch <= 'z' || ch >= 'A' && ch <= 'Z' || ch == '_';
}

template <typename Func> struct Deferred
{
    Func f;
    Deferred(const Func& func)
        : f(func)
    {}
    ~Deferred() { f(); }
};
template <typename Func> Deferred<Func> makeDeferred(const Func& f) { return Deferred<Func>(f); }

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
    Index utf8Line, utf8Col;
    doc->zeroBasedUTF16LocToOneBasedUTF8Loc(
        args.position.line, args.position.character, utf8Line, utf8Col);
    auto cursorOffset = doc->getOffset(utf8Line, utf8Col);
    if (cursorOffset == -1 || doc->getText().getLength() == 0)
    {
        m_connection->sendResult(NullResponse::get(), responseId);
        return SLANG_OK;
    }

    // Ajust cursor position to the beginning of the current/last identifier.
    cursorOffset--;
    while (cursorOffset > 0 && _isIdentifierChar(doc->getText()[cursorOffset]))
    {
        cursorOffset--;
    }

    // Insert a completion request token at cursor position.
    auto originalText = doc->getText();
    StringBuilder newText;
    newText << originalText.getUnownedSlice().head(cursorOffset + 1) << "#?"
            << originalText.getUnownedSlice().tail(cursorOffset + 1);
    doc->setText(newText.ProduceString());
    auto restoreDocText = makeDeferred([&]() { doc->setText(originalText); });

    // Always create a new workspace version for the completion request since we
    // will use a modified source.
    auto version = m_workspace->createVersionForCompletion();
    auto moduleName = getMangledNameFromNameString(canonicalPath.getUnownedSlice());
    version->linkage->contentAssistInfo.cursorLine = utf8Line;
    version->linkage->contentAssistInfo.cursorCol = utf8Col;
    Module* parsedModule = version->getOrLoadModule(canonicalPath);
    if (!parsedModule)
    {
        m_connection->sendResult(NullResponse::get(), responseId);
        return SLANG_OK;
    }

    CompletionContext context;
    context.server = this;
    context.cursorOffset = cursorOffset;
    context.version = version;
    context.doc = doc.Ptr();
    context.parsedModule = parsedModule;
    context.responseId = responseId;
    context.canonicalPath = canonicalPath.getUnownedSlice();
    context.line = utf8Line;
    context.col = utf8Col;
    if (SLANG_SUCCEEDED(context.tryCompleteAttributes()))
    {
        return SLANG_OK;
    }
    if (SLANG_SUCCEEDED(context.tryCompleteHLSLSemantic()))
    {
        return SLANG_OK;
    }
    if (SLANG_SUCCEEDED(context.tryCompleteMember()))
    {
        return SLANG_OK;
    }
    m_connection->sendResult(NullResponse::get(), responseId);
    return SLANG_OK;
}

SlangResult LanguageServer::completionResolve(
    const LanguageServerProtocol::CompletionItem& args, const JSONValue& responseId)
{
    LanguageServerProtocol::CompletionItem resolvedItem = args;
    int itemId = StringToInt(args.data);
    auto version = m_workspace->getCurrentCompletionVersion();
    if (!version || !version->linkage)
    {
        m_connection->sendResult(&resolvedItem, responseId);
        return SLANG_OK;
    }
    auto& candidateItems = version->linkage->contentAssistInfo.completionSuggestions.candidateItems;
    if (itemId >= 0 && itemId < candidateItems.getCount())
    {
        auto declRef = candidateItems[itemId].declRef;
        resolvedItem.detail = getDeclSignatureString(declRef, version->linkage->getASTBuilder());
        StringBuilder docSB;
        _tryGetDocumentation(docSB, version, declRef.getDecl());
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

    RefPtr<DocumentVersion> doc;
    if (!m_workspace->openedDocuments.TryGetValue(canonicalPath, doc))
    {
        m_connection->sendResult(NullResponse::get(), responseId);
        return SLANG_OK;
    }

    auto version = m_workspace->getCurrentVersion();
    Module* parsedModule = version->getOrLoadModule(canonicalPath);
    if (!parsedModule)
    {
        m_connection->sendResult(NullResponse::get(), responseId);
        return SLANG_OK;
    }

    auto tokens = getSemanticTokens(version->linkage, parsedModule, canonicalPath.getUnownedSlice(), doc.Ptr());
    for (auto& token : tokens)
    {
        Index line, col;
        doc->oneBasedUTF8LocToZeroBasedUTF16Loc(token.line, token.col, line, col);
        Index lineEnd, colEnd;
        doc->oneBasedUTF8LocToZeroBasedUTF16Loc(
            token.line, token.col + token.length, lineEnd, colEnd);
        token.line = (int)line;
        token.col = (int)col;
        token.length = (int)(colEnd - col);
    }
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
    RefPtr<DocumentVersion> doc;
    if (!m_workspace->openedDocuments.TryGetValue(canonicalPath, doc))
    {
        m_connection->sendResult(NullResponse::get(), responseId);
        return SLANG_OK;
    }
    Index line, col;
    doc->zeroBasedUTF16LocToOneBasedUTF8Loc(args.position.line, args.position.character, line, col);

    auto version = m_workspace->getCurrentVersion();
    Module* parsedModule = version->getOrLoadModule(canonicalPath);
    if (!parsedModule)
    {
        m_connection->sendResult(NullResponse::get(), responseId);
        return SLANG_OK;
    }

    auto findResult = findASTNodesAt(
        doc.Ptr(),
        version->linkage->getSourceManager(),
        parsedModule->getModuleDecl(),
        ASTLookupType::Invoke,
        canonicalPath.getUnownedSlice(),
        line,
        col);

    if (findResult.getCount() == 0)
    {
        m_connection->sendResult(NullResponse::get(), responseId);
        return SLANG_OK;
    }

    AppExprBase* appExpr = nullptr;
    auto& declPath = findResult[0].path;
    Loc currentLoc = {args.position.line + 1, args.position.character + 1};
    for (Index i = declPath.getCount() - 1; i >= 0; i--)
    {
        if (auto expr = as<AppExprBase>(declPath[i]))
        {
            // Find the inner most invoke expr that has source token info.
            // This allows us to skip the invoke expr nodes for operators/implcit casts.
            if (expr->argumentDelimeterLocs.getCount())
            {
                auto start = Loc::fromSourceLoc(version->linkage->getSourceManager(), expr->argumentDelimeterLocs.getFirst());
                auto end = Loc::fromSourceLoc(
                    version->linkage->getSourceManager(), expr->argumentDelimeterLocs.getLast());
                if (start < currentLoc && currentLoc <= end)
                {
                    appExpr = expr;
                    break;
                }
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
        if (auto aggDecl = as<AggTypeDecl>(declRefExpr->declRef.getDecl()))
        {
            // Look for initializers
            for (auto member : aggDecl->getMembersOfType<ConstructorDecl>())
            {
                addDeclRef(DeclRef<Decl>(member, declRefExpr->declRef.substitutions));
            }
        }
        else
        {
            addDeclRef(declRefExpr->declRef);
        }
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

SlangResult LanguageServer::documentSymbol(
    const LanguageServerProtocol::DocumentSymbolParams& args, const JSONValue& responseId)
{
    String canonicalPath = uriToCanonicalPath(args.textDocument.uri);
    RefPtr<DocumentVersion> doc;
    if (!m_workspace->openedDocuments.TryGetValue(canonicalPath, doc))
    {
        m_connection->sendResult(NullResponse::get(), responseId);
        return SLANG_OK;
    }
    auto version = m_workspace->getCurrentVersion();
    Module* parsedModule = version->getOrLoadModule(canonicalPath);
    if (!parsedModule)
    {
        m_connection->sendResult(NullResponse::get(), responseId);
        return SLANG_OK;
    }
    List<DocumentSymbol> symbols = getDocumentSymbols(version->linkage, parsedModule, canonicalPath.getUnownedSlice(), doc.Ptr());
    m_connection->sendResult(&symbols, responseId);
    return SLANG_OK;
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

void LanguageServer::updatePredefinedMacros(const JSONValue& macros)
{
    if (macros.isValid())
    {
        auto container = m_connection->getContainer();
        JSONToNativeConverter converter(container, m_connection->getSink());
        List<String> predefinedMacros;
        if (SLANG_SUCCEEDED(converter.convert(macros, &predefinedMacros)))
        {
            if (m_workspace->updatePredefinedMacros(predefinedMacros))
            {
                m_connection->sendCall(
                    UnownedStringSlice("workspace/semanticTokens/refresh"), JSONValue::makeInt(0));
            }
        }
    }
}

void LanguageServer::updateSearchPaths(const JSONValue& macros)
{
    if (macros.isValid())
    {
        auto container = m_connection->getContainer();
        JSONToNativeConverter converter(container, m_connection->getSink());
        List<String> searchPaths;
        if (SLANG_SUCCEEDED(converter.convert(macros, &searchPaths)))
        {
            if (m_workspace->updateSearchPaths(searchPaths))
            {
                m_connection->sendCall(
                    UnownedStringSlice("workspace/semanticTokens/refresh"), JSONValue::makeInt(0));
            }
        }
    }
}

void LanguageServer::updateSearchInWorkspace(const JSONValue& macros)
{
    if (macros.isValid())
    {
        auto container = m_connection->getContainer();
        JSONToNativeConverter converter(container, m_connection->getSink());
        bool searchPaths;
        if (SLANG_SUCCEEDED(converter.convert(macros, &searchPaths)))
        {
            if (m_workspace->updateSearchInWorkspace(searchPaths))
            {
                m_connection->sendCall(
                    UnownedStringSlice("workspace/semanticTokens/refresh"), JSONValue::makeInt(0));
            }
        }
    }
}

void LanguageServer::sendConfigRequest()
{
    ConfigurationParams args;
    ConfigurationItem item;
    item.section = "slang.predefinedMacros";
    args.items.add(item);
    item.section = "slang.additionalSearchPaths";
    args.items.add(item);
    item.section = "slang.searchInAllWorkspaceDirectories";
    args.items.add(item);
    m_connection->sendCall(
        ConfigurationParams::methodName,
        &args,
        JSONValue::makeInt(kConfigResponseId));
}

void LanguageServer::registerCapability(const char* methodName)
{
    RegistrationParams args;
    Registration reg;
    reg.method = methodName;
    reg.id = reg.method;
    args.registrations.add(reg);
    m_connection->sendCall(
        UnownedStringSlice("client/registerCapability"), &args, JSONValue::makeInt(999));
}

void LanguageServer::logMessage(int type, String message)
{
    LanguageServerProtocol::LogMessageParams args;
    args.type = type;
    args.message = message;
    m_connection->sendCall(LanguageServerProtocol::LogMessageParams::methodName, &args);
}

SlangResult LanguageServer::tryGetMacroHoverInfo(
    WorkspaceVersion* version, DocumentVersion* doc, Index line, Index col, JSONValue responseId)
{
    Index startOffset = 0;
    auto identifier = doc->peekIdentifier(line, col, startOffset);
    if (identifier.getLength() == 0)
        return SLANG_FAIL;
    auto def = version->tryGetMacroDefinition(identifier);
    if (!def)
        return SLANG_FAIL;
    LanguageServerProtocol::Hover hover;
    doc->offsetToLineCol(startOffset, line, col);
    Index outLine, outCol;
    doc->oneBasedUTF8LocToZeroBasedUTF16Loc(line, col, outLine, outCol);
    hover.range.start.line = (int)outLine;
    hover.range.start.character = (int)outCol;
    hover.range.end.line = (int)outLine;
    hover.range.end.character = (int)(outCol + identifier.getLength());
    StringBuilder sb;
    sb << "```\n#define " << identifier;
    if (def->params.getCount())
    {
        sb << "(";
        bool isFirst = true;
        for (auto param : def->params)
        {
            if (!isFirst)
                sb << ", ";
            if (param.isVariadic)
                sb << "...";
            else if (param.name)
                sb << param.name->text;
            isFirst = false;
        }
        sb << ")";
    }
    for (auto& token : def->tokenList)
    {
        sb << " ";
        sb << token.getContent();
    }
    sb << "\n```\n\n";
    auto humaneLoc =
        version->linkage->getSourceManager()->getHumaneLoc(def->loc, SourceLocType::Actual);
    sb << "Defined in " << humaneLoc.pathInfo.foundPath << "(" << humaneLoc.line << ")\n";
    hover.contents.kind = "markdown";
    hover.contents.value = sb.ProduceString();
    m_connection->sendResult(&hover, responseId);
    return SLANG_OK;
}

SlangResult LanguageServer::tryGotoMacroDefinition(
    WorkspaceVersion* version, DocumentVersion* doc, Index line, Index col, JSONValue responseId)
{
    Index startOffset = 0;
    auto identifier = doc->peekIdentifier(line, col, startOffset);
    if (identifier.getLength() == 0)
        return SLANG_FAIL;
    auto def = version->tryGetMacroDefinition(identifier);
    if (!def)
        return SLANG_FAIL;
    auto humaneLoc =
        version->linkage->getSourceManager()->getHumaneLoc(def->loc, SourceLocType::Actual);
    LanguageServerProtocol::Location result;
    result.uri = URI::fromLocalFilePath(humaneLoc.pathInfo.foundPath.getUnownedSlice()).uri;
    Index outLine, outCol;
    doc->oneBasedUTF8LocToZeroBasedUTF16Loc(humaneLoc.line, humaneLoc.column, outLine, outCol);
    result.range.start.line = (int)outLine;
    result.range.start.character = (int)outCol;
    result.range.end.line = (int)outLine;
    result.range.end.character = (int)(outCol + identifier.getLength());
    m_connection->sendResult(&result, responseId);
    return SLANG_OK;
}

SlangResult LanguageServer::tryGotoFileInclude(
    WorkspaceVersion* version, DocumentVersion* doc, Index line, JSONValue responseId)
{
    auto lineContent = doc->getLine(line).trim();
    if (!lineContent.startsWith("#") || lineContent.indexOf(UnownedStringSlice("include")) == -1)
        return SLANG_FAIL;
    for (auto& include : version->linkage->contentAssistInfo.preprocessorInfo.fileIncludes)
    {
        auto includeLoc =
            version->linkage->getSourceManager()->getHumaneLoc(include.loc, SourceLocType::Actual);
        if (includeLoc.line == line && includeLoc.pathInfo.foundPath == doc->getPath())
        {
            LanguageServerProtocol::Location result;
            result.uri = URI::fromLocalFilePath(include.path.getUnownedSlice()).uri;
            result.range.start.line = 0;
            result.range.start.character = 0;
            result.range.end.line = 0;
            result.range.end.character = 0;
            m_connection->sendResult(&result, responseId);
            return SLANG_OK;
        }
    }
    return SLANG_FAIL;
}

SlangResult LanguageServer::queueJSONCall(JSONRPCCall call)
{
    Command cmd;
    cmd.id = PersistentJSONValue(call.id, m_connection->getContainer());
    cmd.method = call.method;
    if (call.method == DidOpenTextDocumentParams::methodName)
    {
        DidOpenTextDocumentParams args;
        SLANG_RETURN_ON_FAIL(m_connection->toNativeArgsOrSendError(call.params, &args, call.id));
        cmd.openDocArgs = args;
    }
    else if (call.method == DidCloseTextDocumentParams::methodName)
    {
        DidCloseTextDocumentParams args;
        SLANG_RETURN_ON_FAIL(m_connection->toNativeArgsOrSendError(call.params, &args, call.id));
        cmd.closeDocArgs = args;
    }
    else if (call.method == DidChangeTextDocumentParams::methodName)
    {
        DidChangeTextDocumentParams args;
        SLANG_RETURN_ON_FAIL(m_connection->toNativeArgsOrSendError(call.params, &args, call.id));
        cmd.changeDocArgs = args;
    }
    else if (call.method == HoverParams::methodName)
    {
        HoverParams args;
        SLANG_RETURN_ON_FAIL(m_connection->toNativeArgsOrSendError(call.params, &args, call.id));
        cmd.hoverArgs = args;
    }
    else if (call.method == DefinitionParams::methodName)
    {
        DefinitionParams args;
        SLANG_RETURN_ON_FAIL(m_connection->toNativeArgsOrSendError(call.params, &args, call.id));
        cmd.definitionArgs = args;
    }
    else if (call.method == CompletionParams::methodName)
    {
        CompletionParams args;
        SLANG_RETURN_ON_FAIL(m_connection->toNativeArgsOrSendError(call.params, &args, call.id));
        cmd.completionArgs = args;
    }
    else if (call.method == SemanticTokensParams::methodName)
    {
        SemanticTokensParams args;
        SLANG_RETURN_ON_FAIL(m_connection->toNativeArgsOrSendError(call.params, &args, call.id));
        cmd.semanticTokenArgs = args;
    }
    else if (call.method == SignatureHelpParams::methodName)
    {
        SignatureHelpParams args;
        SLANG_RETURN_ON_FAIL(m_connection->toNativeArgsOrSendError(call.params, &args, call.id));
        cmd.signatureHelpArgs = args;
    }
    else if (call.method == "completionItem/resolve")
    {
        Slang::LanguageServerProtocol::CompletionItem args;
        SLANG_RETURN_ON_FAIL(m_connection->toNativeArgsOrSendError(call.params, &args, call.id));
        cmd.completionResolveArgs = args;
    }
    else if (call.method == DocumentSymbolParams::methodName)
    {
        DocumentSymbolParams args;
        SLANG_RETURN_ON_FAIL(m_connection->toNativeArgsOrSendError(call.params, &args, call.id));
        cmd.documentSymbolArgs = args;
    }
    else if (call.method == DidChangeConfigurationParams::methodName)
    {
        DidChangeConfigurationParams args;
        SLANG_RETURN_ON_FAIL(m_connection->toNativeArgsOrSendError(call.params, &args, call.id));
        // We need to process it now instead of sending to queue.
        // This is because there is reference to JSONValue that is only available here.
        return didChangeConfiguration(args);
    }
    else if (call.method == "$/cancelRequest")
    {
        CancelParams args;
        SLANG_RETURN_ON_FAIL(m_connection->toNativeArgsOrSendError(call.params, &args, call.id));
        cmd.cancelArgs = args;
    }
    commands.add(_Move(cmd));
    return SLANG_OK;
}

SlangResult LanguageServer::runCommand(Command& call)
{
    // Do different things
    if (call.method == DidOpenTextDocumentParams::methodName)
    {
        return didOpenTextDocument(call.openDocArgs.get());
    }
    else if (call.method == DidCloseTextDocumentParams::methodName)
    {
        return didCloseTextDocument(call.closeDocArgs.get());
    }
    else if (call.method == DidChangeTextDocumentParams::methodName)
    {
        return didChangeTextDocument(call.changeDocArgs.get());
    }
    else if (call.method == HoverParams::methodName)
    {
        return hover(call.hoverArgs.get(), call.id);
    }
    else if (call.method == DefinitionParams::methodName)
    {
        return gotoDefinition(call.definitionArgs.get(), call.id);
    }
    else if (call.method == CompletionParams::methodName)
    {
        return completion(call.completionArgs.get(), call.id);
    }
    else if (call.method == SemanticTokensParams::methodName)
    {
        return semanticTokens(call.semanticTokenArgs.get(), call.id);
    }
    else if (call.method == SignatureHelpParams::methodName)
    {
        return signatureHelp(call.signatureHelpArgs.get(), call.id);
    }
    else if (call.method == "completionItem/resolve")
    {
        return completionResolve(call.completionResolveArgs.get(), call.id);
    }
    else if (call.method == DocumentSymbolParams::methodName)
    {
        return documentSymbol(call.documentSymbolArgs.get(), call.id);
    }
    else if (call.method == DidChangeConfigurationParams::methodName)
    {
        return didChangeConfiguration(call.changeConfigArgs.get());
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

void LanguageServer::processCommands()
{
    HashSet<int64_t> canceledIDs;
    for (auto& cmd : commands)
    {
        if (cmd.method == "$/cancelRequest")
        {
            auto id = cmd.cancelArgs.get().id;
            if (id > 0)
            {
                canceledIDs.Add(id);
            }
        }
    }
    const int kErrorRequestCanceled = -32800;
    for (auto& cmd : commands)
    {
        if (cmd.id.getKind() == JSONValue::Kind::Integer && canceledIDs.Contains(cmd.id.asInteger()))
        {
            m_connection->sendError((JSONRPC::ErrorCode)kErrorRequestCanceled, cmd.id);
        }
        else
        {
            runCommand(cmd);
        }
    }
}

SlangResult LanguageServer::didCloseTextDocument(const DidCloseTextDocumentParams& args)
{
    String canonicalPath = uriToCanonicalPath(args.textDocument.uri);
    m_workspace->closeDoc(canonicalPath);
    resetDiagnosticUpdateTime();
    return SLANG_OK;
}
SlangResult LanguageServer::didChangeTextDocument(const DidChangeTextDocumentParams& args)
{
    String canonicalPath = uriToCanonicalPath(args.textDocument.uri);
    for (auto change : args.contentChanges)
        m_workspace->changeDoc(canonicalPath, change.range, change.text);
    resetDiagnosticUpdateTime();
    return SLANG_OK;
}

SlangResult LanguageServer::didChangeConfiguration(
    const LanguageServerProtocol::DidChangeConfigurationParams& args)
{
    SLANG_UNUSED(args);
    sendConfigRequest();
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
        commands.clear();
        auto start = platform::PerformanceCounter::now();
        while (true)
        {
            m_connection->tryReadMessage();
            if (!m_connection->hasMessage())
                break;
            parseNextMessage();
        }
        auto parseTime = platform::PerformanceCounter::getElapsedTimeInSeconds(start);
        auto parseEnd = platform::PerformanceCounter::now();
        processCommands();
        // Now we can use this time to reparse user's code, report diagnostics, etc.
        update();
        auto workTime = platform::PerformanceCounter::getElapsedTimeInSeconds(parseEnd);

        if (commands.getCount() > 0 && m_initialized)
        {
            StringBuilder msgBuilder;
            msgBuilder << "Server processed " << commands.getCount() << " commands, parsed in "
                       << String(int(parseTime * 1000)) << "ms, executed in "
                       << String(int(workTime * 1000)) << "ms";
            logMessage(3, msgBuilder.ProduceString());
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
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
