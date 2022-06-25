#pragma once

#include "../../slang.h"
#include "../core/slang-basic.h"
#include "slang-ast-all.h"
#include "slang-syntax.h"
#include "slang-compiler.h"
#include "slang-workspace-version.h"

namespace Slang
{
List<LanguageServerProtocol::InlayHint> getInlayHints(
    Linkage* linkage, Module* module, UnownedStringSlice fileName, DocumentVersion* doc, LanguageServerProtocol::Range range);
} // namespace Slang
