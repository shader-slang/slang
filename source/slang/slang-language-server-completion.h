// slang-language-server-completion.h
#pragma once

#include "slang-workspace-version.h"
#include "slang-language-server-ast-lookup.h"

namespace Slang
{
class LanguageServer;

enum class CommitCharacterBehavior
{
    Disabled,
    MembersOnly,
    All
};

struct CompletionContext
{
    LanguageServer* server;
    Index cursorOffset;
    WorkspaceVersion* version;
    DocumentVersion* doc;
    Module* parsedModule;
    JSONValue responseId;
    UnownedStringSlice canonicalPath;
    CommitCharacterBehavior commitCharacterBehavior;
    Int line;
    Int col;

    SlangResult tryCompleteMemberAndSymbol();
    SlangResult tryCompleteHLSLSemantic();
    SlangResult tryCompleteAttributes();
    SlangResult tryCompleteImport();
    SlangResult tryCompleteInclude();
    SlangResult tryCompleteRawFileName(UnownedStringSlice lineContent, Index fileNameStartPos, bool isImportString);


    List<LanguageServerProtocol::CompletionItem> collectMembersAndSymbols();
    List<LanguageServerProtocol::CompletionItem> createSwizzleCandidates(
        Type* baseType, IntegerLiteralValue elementCount[2]);
    List<LanguageServerProtocol::CompletionItem> collectAttributes();
    LanguageServerProtocol::CompletionItem generateGUIDCompletionItem();
    List<LanguageServerProtocol::TextEditCompletionItem> gatherFileAndModuleCompletionItems(
        const String& prefixPath,
        bool translateModuleName,
        bool isImportString,
        Index lineIndex,
        Index fileNameEnd,
        Index sectionStart,
        Index sectionEnd,
        char closingChar);
};

} // namespace Slang
