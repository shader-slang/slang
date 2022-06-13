// slang-language-server-completion.h
#pragma once

#include "slang-workspace-version.h"

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

    SlangResult tryCompleteMember();
    SlangResult tryCompleteHLSLSemantic();
    List<LanguageServerProtocol::CompletionItem> collectMembers(Expr* baseExpr);
};

} // namespace Slang
