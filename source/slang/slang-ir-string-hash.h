// slang-ir-string-hash.h
#pragma once

#include "slang-ir.h"

#include "../core/slang-string-slice-pool.h"

namespace Slang
{

struct IRModule;
struct SharedIRBuilder;

// Find all of the calls to 'getStringHash' and replace them with the an int literal of the value. Place all
// of the string literal values into the ioPool. 
void replaceGetStringHash(IRModule* module, SharedIRBuilder& sharedBuilder, StringSlicePool& ioPool);

// Does the same as replaceGetStringHash, but also adds a GlobalHashedStringLiterals instruction of any
// string literals referenced via 'getStringHash'. If there are none, it does nothing.
void replaceGetStringHashWithGlobal(IRModule* module, SharedIRBuilder& sharedBuilder);

// Finds the global GlobalHashedStringLiterals instruction for the module if there is one, and then
// adds all of it's strings to ioPool.
void findGlobalHashedStringLiterals(IRModule* module, StringSlicePool& ioPool);

// Given a pool, with > 0 strings adds a GlobalHashedStringLiterals to the module. 
void addGlobalHashedStringLiterals(const StringSlicePool& pool, SharedIRBuilder& sharedBuilder);


} // namespace Slang
