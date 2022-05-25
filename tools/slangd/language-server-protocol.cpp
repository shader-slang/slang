#include "language-server-protocol.h"

namespace Slang
{
namespace LanguageServerProtocol
{
static const StructRttiInfo _makeTextDocumentSyncOptionsRtti()
{
    TextDocumentSyncOptions obj;
    StructRttiBuilder builder(&obj, "LanguageServerProtocol::TextDocumentSyncOptions", nullptr);
    builder.addField("change", &obj.change);
    builder.addField("openClose", &obj.openClose);
    return builder.make();
}
const StructRttiInfo TextDocumentSyncOptions::g_rttiInfo = _makeTextDocumentSyncOptionsRtti();

static const StructRttiInfo _makeTextDocumentItemRtti()
{
    TextDocumentItem obj;
    StructRttiBuilder builder(&obj, "LanguageServerProtocol::TextDocumentItem", nullptr);
    builder.addField("uri", &obj.uri);
    builder.addField("version", &obj.version);
    builder.addField("languageId", &obj.languageId);
    builder.addField("text", &obj.text);
    return builder.make();
}
const StructRttiInfo TextDocumentItem::g_rttiInfo = _makeTextDocumentItemRtti();

static const StructRttiInfo _makeTextDocumentIdentifierRtti()
{
    TextDocumentIdentifier obj;
    StructRttiBuilder builder(&obj, "LanguageServerProtocol::TextDocumentIdentifier", nullptr);
    builder.addField("uri", &obj.uri);
    return builder.make();
}
const StructRttiInfo TextDocumentIdentifier::g_rttiInfo = _makeTextDocumentIdentifierRtti();

static const StructRttiInfo _makeVersionedTextDocumentIdentifierRtti()
{
    VersionedTextDocumentIdentifier obj;
    StructRttiBuilder builder(&obj, "LanguageServerProtocol::VersionedTextDocumentIdentifier", nullptr);
    builder.addField("uri", &obj.uri);
    builder.addField("version", &obj.version);
    return builder.make();
}
const StructRttiInfo VersionedTextDocumentIdentifier::g_rttiInfo =
    _makeVersionedTextDocumentIdentifierRtti();

static const StructRttiInfo _makePositionRtti()
{
    Position obj;
    StructRttiBuilder builder(
        &obj, "LanguageServerProtocol::Position", nullptr);
    builder.addField("line", &obj.line);
    builder.addField("character", &obj.character);
    return builder.make();
}
const StructRttiInfo Position::g_rttiInfo = _makePositionRtti();

static const StructRttiInfo _makeRangeRtti()
{
    Range obj;
    StructRttiBuilder builder(&obj, "LanguageServerProtocol::Range", nullptr);
    builder.addField("start", &obj.start);
    builder.addField("end", &obj.end);
    return builder.make();
}
const StructRttiInfo Range::g_rttiInfo = _makeRangeRtti();

static const StructRttiInfo _makeDidOpenTextDocumentRtti()
{
    DidOpenTextDocumentParams obj;
    StructRttiBuilder builder(&obj, "LanguageServerProtocol::DidOpenTextDocumentParams", nullptr);
    builder.addField("textDocument", &obj.textDocument);
    return builder.make();
}
const StructRttiInfo DidOpenTextDocumentParams::g_rttiInfo = _makeDidOpenTextDocumentRtti();
const UnownedStringSlice DidOpenTextDocumentParams::methodName =
    UnownedStringSlice::fromLiteral("textDocument/didOpen");

static const StructRttiInfo _makeTextDocumentContentChangeEventRtti()
{
    TextDocumentContentChangeEvent obj;
    StructRttiBuilder builder(&obj, "LanguageServerProtocol::TextDocumentContentChangeEvent", nullptr);
    builder.addField("range", &obj.range, StructRttiInfo::Flag::Optional);
    builder.addField("text", &obj.text);
    return builder.make();
}
const StructRttiInfo TextDocumentContentChangeEvent::g_rttiInfo =
    _makeTextDocumentContentChangeEventRtti();

static const StructRttiInfo _makeDidChangeTextDocumentParamsRtti()
{
    DidChangeTextDocumentParams obj;
    StructRttiBuilder builder(
        &obj, "LanguageServerProtocol::DidChangeTextDocumentParams", nullptr);
    builder.addField("textDocument", &obj.textDocument);
    builder.addField("contentChanges", &obj.contentChanges);
    return builder.make();
}
const StructRttiInfo DidChangeTextDocumentParams::g_rttiInfo =
    _makeDidChangeTextDocumentParamsRtti();
const UnownedStringSlice DidChangeTextDocumentParams::methodName =
    UnownedStringSlice::fromLiteral("textDocument/didChange");


static const StructRttiInfo _makeDidCloseTextDocumentParamsRtti()
{
    DidCloseTextDocumentParams obj;
    StructRttiBuilder builder(&obj, "LanguageServerProtocol::DidCloseTextDocumentParams", nullptr);
    builder.addField("textDocument", &obj.textDocument);
    return builder.make();
}
const StructRttiInfo DidCloseTextDocumentParams::g_rttiInfo = _makeDidCloseTextDocumentParamsRtti();
const UnownedStringSlice DidCloseTextDocumentParams::methodName =
    UnownedStringSlice::fromLiteral("textDocument/didClose");

static const StructRttiInfo _makeServerCapabilitiesRtti()
{
    ServerCapabilities obj;
    StructRttiBuilder builder(&obj, "LanguageServerProtocol::ServerCapabilities", nullptr);
    builder.addField("positionEncoding", &obj.positionEncoding);
    builder.addField("textDocumentSync", &obj.textDocumentSync);
    return builder.make();
}
const StructRttiInfo ServerCapabilities::g_rttiInfo = _makeServerCapabilitiesRtti();

static const StructRttiInfo _makeServerInfoRtti()
{
    ServerInfo obj;
    StructRttiBuilder builder(&obj, "LanguageServerProtocol::ServerInfo", nullptr);
    builder.addField("name", &obj.name);
    builder.addField("version", &obj.version);
    return builder.make();
}
const StructRttiInfo ServerInfo::g_rttiInfo = _makeServerInfoRtti();


static const StructRttiInfo _makeInitializeResultRtti()
{
    InitializeResult obj;
    StructRttiBuilder builder(&obj, "LanguageServerProtocol::InitializeResult", nullptr);
    builder.addField("capabilities", &obj.capabilities);
    builder.addField("serverInfo", &obj.serverInfo);
    return builder.make();
}
const StructRttiInfo InitializeResult::g_rttiInfo = _makeInitializeResultRtti();

const UnownedStringSlice InitializeParams::methodName =
    UnownedStringSlice::fromLiteral("initialize");

const UnownedStringSlice ShutdownParams::methodName = UnownedStringSlice::fromLiteral("shutdown");

const UnownedStringSlice ExitParams::methodName = UnownedStringSlice::fromLiteral("exit");

static const StructRttiInfo _makeWorkspaceFolderRtti()
{
    WorkspaceFolder obj;
    StructRttiBuilder builder(&obj, "LanguageServerProtocol::WorkspaceFolder", nullptr);
    builder.addField("uri", &obj.uri);
    builder.addField("name", &obj.name);
    return builder.make();
}
const StructRttiInfo WorkspaceFolder::g_rttiInfo = _makeWorkspaceFolderRtti();

static const StructRttiInfo _makeInitializeParamsRtti()
{
    InitializeParams obj;
    StructRttiBuilder builder(&obj, "LanguageServerProtocol::InitializeParams", nullptr);
    builder.addField("workspaceFolders", &obj.workspaceFolders, StructRttiInfo::Flag::Optional);
    return builder.make();
}
const StructRttiInfo InitializeParams::g_rttiInfo = _makeInitializeParamsRtti();

static const StructRttiInfo _makeNullResponseRtti()
{
    NullResponse obj;
    StructRttiBuilder builder(&obj, "LanguageServerProtocol::NullResponse", nullptr);
    return builder.make();
}
const StructRttiInfo NullResponse::g_rttiInfo = _makeNullResponseRtti();

NullResponse* NullResponse::get()
{
    static NullResponse result = {};
    return &result;
}

} // namespace LanguageServerProtocol

}
