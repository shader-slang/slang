// slang-ir-ssa.h
#pragma once

namespace Slang
{
struct IRModule;
struct IRGlobalValueWithCode;
struct IRInst;

bool constructSSA(IRModule* module, IRGlobalValueWithCode* globalVal);
bool constructSSA(IRModule* module);
bool constructSSA(IRInst* globalVal);

/// Do all uses of `inst` lead to a `load`? True when every use is a `load`, or a
/// `getElementPtr`/`getFieldAddress` off `inst` whose own uses also lead to
/// loads (recursively). I.e. `inst` (an address) is only ever read from, never
/// stored through.
bool allUsesLeadToLoads(IRInst* inst);
} // namespace Slang
