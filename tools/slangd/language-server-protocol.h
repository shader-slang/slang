#pragma once

#include "../../slang-com-helper.h"
#include "../../slang-com-ptr.h"
#include "../../slang.h"

#include "../../source/core/slang-rtti-info.h"
#include "../../source/compiler-core/slang-json-value.h"

namespace Slang
{
namespace LanguageServerProtocol
{
    struct ServerInfo
    {
        String name;
        String version;

        static const StructRttiInfo g_rttiInfo;
    };

    enum class TextDocumentSyncKind
    {
        None = 0,
        Full = 1,
        Incremental = 2
    };

    struct TextDocumentSyncOptions
    {
        bool openClose;
        int32_t change; // TextDocumentSyncKind 
        static const StructRttiInfo g_rttiInfo;
    };

    struct TextDocumentItem
    {
        String uri;
        String languageId;
        int version;
        String text;
        static const StructRttiInfo g_rttiInfo;
    };

    struct TextDocumentIdentifier
    {
        String uri;
        static const StructRttiInfo g_rttiInfo;
    };

    struct VersionedTextDocumentIdentifier
    {
        String uri;
        int version;
        static const StructRttiInfo g_rttiInfo;
    };

    struct Position
    {
        int line = -1;
        int character = -1;
        static const StructRttiInfo g_rttiInfo;
    };

    struct Range
    {
        Position start;
        Position end;
        static const StructRttiInfo g_rttiInfo;
    };

    struct DidOpenTextDocumentParams
    {
        TextDocumentItem textDocument;
        static const StructRttiInfo g_rttiInfo;
        static const UnownedStringSlice methodName;
    };

    struct TextDocumentContentChangeEvent
    {
        Range range; // optional
        String text;
        static const StructRttiInfo g_rttiInfo;
    };

    struct DidChangeTextDocumentParams
    {
        VersionedTextDocumentIdentifier textDocument;
        List<TextDocumentContentChangeEvent> contentChanges;
        static const StructRttiInfo g_rttiInfo;
        static const UnownedStringSlice methodName;

    };

    struct DidCloseTextDocumentParams
    {
        TextDocumentIdentifier textDocument;
        static const StructRttiInfo g_rttiInfo;
        static const UnownedStringSlice methodName;
    };

    struct ServerCapabilities
    {
        String positionEncoding;
        TextDocumentSyncOptions textDocumentSync;
        static const StructRttiInfo g_rttiInfo;
    };

    struct WorkspaceFolder
    {
        String uri;
        String name;
        static const StructRttiInfo g_rttiInfo;
    };

    struct InitializeParams
    {
        List<WorkspaceFolder> workspaceFolders;
        static const UnownedStringSlice methodName;
        static const StructRttiInfo g_rttiInfo;
    };

    struct NullResponse
    {
        static const StructRttiInfo g_rttiInfo;
        static NullResponse* get();
    };

    struct InitializeResult
    {
        ServerCapabilities capabilities;
        ServerInfo serverInfo;

        static const StructRttiInfo g_rttiInfo;
    };

    struct ShutdownParams
    {
        static const UnownedStringSlice methodName;
    };

    struct ExitParams
    {
        static const UnownedStringSlice methodName;
    };

}
} // namespace LanguageServerProtocol
