// slang-language-server-completion.h
#pragma once

#include "slang-workspace-version.h"
#include "slang-language-server-ast-lookup.h"

namespace Slang
{
class LanguageServer;

struct CompletionContext
{
    LanguageServer* server;
    Index cursorOffset;
    WorkspaceVersion* version;
    DocumentVersion* doc;
    Module* parsedModule;
    JSONValue responseId;
    UnownedStringSlice canonicalPath;
    Int line;
    Int col;

    SlangResult tryCompleteMemberAndSymbol();
    SlangResult tryCompleteHLSLSemantic();
    SlangResult tryCompleteAttributes();
    List<LanguageServerProtocol::CompletionItem> collectMembersAndSymbols();
    List<LanguageServerProtocol::CompletionItem> createSwizzleCandidates(
        Type* baseType, IntegerLiteralValue elementCount[2]);
    List<LanguageServerProtocol::CompletionItem> collectAttributes();
};

} // namespace Slang
