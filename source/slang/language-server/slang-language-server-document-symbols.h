#pragma once

#include "core/slang-basic.h"
#include "slang-workspace-version.h"
#include "slang.h"
#include "slang/ast/slang-ast-all.h"
#include "slang/ast/slang-syntax.h"
#include "slang/slang-compiler.h"

namespace Slang
{
List<LanguageServerProtocol::DocumentSymbol> getDocumentSymbols(
    Linkage* linkage,
    Module* module,
    UnownedStringSlice fileName,
    DocumentVersion* doc);
} // namespace Slang
