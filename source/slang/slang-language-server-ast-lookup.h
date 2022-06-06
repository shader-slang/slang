#pragma once

#include "slang-ast-all.h"

namespace Slang
{
struct ASTLookupResult
{
    List<SyntaxNode*> path;
};
enum class ASTLookupType
{
    Decl,
    Invoke,
};
List<ASTLookupResult> findASTNodesAt(
    SourceManager* sourceManager,
    ModuleDecl* moduleDecl,
    ASTLookupType findType,
    UnownedStringSlice fileName,
    Int line,
    Int col);

} // namespace LanguageServerProtocol
