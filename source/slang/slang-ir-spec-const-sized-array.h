#pragma once

namespace Slang
{
struct IRModule;

/// Replace each `constexpr*` instruction in `module` with the ordinary instruction for the same
/// operation, for example `constexprMul(kTile, 2)` with `mul(kTile, 2)`.
///
/// IR lowering builds compile-time integer values such as the size in
/// `groupshared float wide[kTile * 2 + 1]` from the hoistable `constexpr*` instructions so that
/// equal sizes give the same array type. After specialization the only ones left depend on a
/// specialization constant. The SPIR-V emitter turns them into `OpSpecConstantOp`; a source
/// target prints the size as an expression, and with ordinary instructions the target's existing
/// legalization and emit logic for those operations apply unchanged (WGSL, for example, requires
/// an unsigned shift amount and spells a select as `select(...)`).
void lowerConstexprInstsToOrdinaryInsts(IRModule* module);

} // namespace Slang
