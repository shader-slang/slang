// slang-ir-dce.h
#pragma once

#include "slang-ir-insts.h"

namespace Slang
{
struct IRModule;

struct IRDeadCodeEliminationOptions
{
    bool keepExportsAlive = false;
    bool keepLayoutsAlive = false;
    bool useFastAnalysis = false;
    bool keepGlobalParamsAlive = true;
};

/// Eliminate "dead" code from the given IR module.
///
/// This pass is primarily designed for flow-insensitive
/// "global" dead code elimination (DCE), such as removing
/// types that are unused, functions that are never called,
/// etc.
/// Returns true if changed.
bool eliminateDeadCode(
    IRModule* module,
    IRDeadCodeEliminationOptions const& options = IRDeadCodeEliminationOptions());

bool eliminateDeadCode(
    IRInst* root,
    IRDeadCodeEliminationOptions const& options = IRDeadCodeEliminationOptions());

bool shouldInstBeLiveIfParentIsLive(IRInst* inst, IRDeadCodeEliminationOptions options);

bool isWeakReferenceOperand(IRInst* inst, UInt operandIndex);

bool trimOptimizableTypes(IRModule* module);

/// Return true if `type` is a struct that contributes no storage, i.e. it has no fields
/// other than `void` fields or (recursively) other empty structs. Returns false for any
/// non-struct type.
bool isStructEmpty(IRType* type);

/// Remove every empty-struct field from every struct in `module`, rewriting all of the
/// field's uses so nothing references the removed member.
///
/// The type-layout rules give an empty struct size 0 and skip it when assigning struct
/// offsets (so reflection reports the following field at the empty member's offset; see the
/// empty-aggregate handling in `slang-type-layout.cpp`), but an empty struct still occupies
/// 1 byte as a C++/CUDA member. Leaving the member in the emitted struct therefore pushes
/// subsequent fields past their reflected offsets, which produces a host/device layout
/// mismatch on the CPU/CUDA targets (see shader-slang#8125).
///
/// Removing only the declaration is not enough: a surviving `FieldExtract`, `FieldAddress`,
/// or `MakeStruct` operand would then reference a member that no longer exists. Because an
/// empty value carries no data, this pass replaces each such use with a fresh empty value
/// (`FieldExtract` -> a fresh `MakeStruct`, `FieldAddress` -> a fresh local variable) and
/// trims the corresponding `MakeStruct` operand. Any load or store that targeted the field
/// address is thereby redirected to the fresh local, where it is harmless (an empty value
/// has no bytes to read or write), so declaration, construction, and access stay consistent.
///
/// Intended only for the C-like *source* emit path (C++ / CUDA), where an empty struct is a
/// 1-byte member. The direct-LLVM CPU path represents an empty struct as a zero-size type that
/// already matches reflection, so the caller must not run this pass there.
void removeEmptyStructFields(IRModule* module);

} // namespace Slang
