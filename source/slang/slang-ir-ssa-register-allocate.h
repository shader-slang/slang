// slang-ir-ssa-register-allocate.h
#pragma once

#include "slang-ir.h"

namespace Slang
{
struct IRDominatorTree;
struct PhiEliminationOptions;

struct RegisterInfo : RefObject
{
    IRType* type;
    List<IRInst*> insts;
};

struct RegisterAllocationResult
{
    OrderedDictionary<IRType*, List<RefPtr<RegisterInfo>>> mapTypeToRegisterList;
    Dictionary<IRInst*, RefPtr<RegisterInfo>> mapInstToRegister;
};

// Allocate virtual storage for selected block parameters and UpdateElement instructions.
// phiOptions controls parameter selection; the caller decides whether to enable allocation.
RegisterAllocationResult allocateRegistersForFunc(
    IRGlobalValueWithCode* func,
    RefPtr<IRDominatorTree>& inOutDom,
    const PhiEliminationOptions& phiOptions);

} // namespace Slang
