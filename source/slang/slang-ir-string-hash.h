// slang-ir-string-hash.h
#pragma once

#include "slang-ir.h"

#include "../core/slang-string-slice-pool.h"

namespace Slang
{

struct IRModule;
struct SharedIRBuilder;

// Finds the global GlobalHashedStringLiterals instruction for the module if there is one, and then
// adds all of it's strings to ioPool.
void findGlobalHashedStringLiterals(IRModule* module, StringSlicePool& ioPool);

// Given a pool, with > 0 strings adds a GlobalHashedStringLiterals to the module. 
void addGlobalHashedStringLiterals(const StringSlicePool& pool, SharedIRBuilder& sharedBuilder);


} // namespace Slang
