// slang-ir-string-hash.h
#pragma once

#include "slang-ir.h"

namespace Slang
{

struct IRModule;

SlangResult replaceGetStringHash(IRModule* module);

void addGlobalHashedStringLiterals(const HashSet<IRStringLit*>& set, IRBuilder& ioBuilder);
  
} // namespace Slang
