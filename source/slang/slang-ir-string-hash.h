// slang-ir-string-hash.h
#pragma once

#include "slang-ir.h"

#include "../core/slang-string-slice-pool.h"

namespace Slang
{

struct IRModule;
struct SharedIRBuilder;

void replaceGetStringHash(IRModule* module, SharedIRBuilder& sharedBuilder, StringSlicePool& ioPool);

void findGlobalHashedStringLiterals(IRModule* module, StringSlicePool& ioPool);

void addGlobalHashedStringLiterals(const StringSlicePool& pool, SharedIRBuilder& sharedBuilder);


void replaceGetStringHash(IRModule* module);


} // namespace Slang
