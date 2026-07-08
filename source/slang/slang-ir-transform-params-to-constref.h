// source\slang\slang-ir-transform-params-to-constref.h
#pragma once

#include "slang-ir.h"

namespace Slang
{
class DiagnosticSink;

SlangResult transformParamsToConstRef(IRModule* module, DiagnosticSink* sink);

SlangResult translateEntryPointInParamToBorrow(IRModule* module, DiagnosticSink* sink);

/// Rewrite all uses of `addrInst` from value uses into address uses, after `addrInst`
/// has been retyped from a value type to an address-like type (e.g. `borrow in T` or a
/// parameter-group type). Each direct use is first wrapped in a load; then
/// `FieldExtract(load, key)` / `GetElement(load, index)` patterns are rewritten into
/// `Load(FieldAddress(addrInst, key))` / `Load(GetElementPtr(addrInst, index))`
/// chains, and loads whose uses were all rewritten are removed.
void rewriteValueUsesToAddrUses(IRBuilder& builder, IRInst* addrInst);

} // namespace Slang
