#pragma once

#include "slang-target.h"

namespace Slang
{
struct IRModule;
class DiagnosticSink;

/// Report an error at each declaration that owns an array sized by a specialization constant when
/// `target` cannot express that array.
///
/// Consider this example:
///
///     [SpecializationConstant] const int kTile = 128;
///     groupshared float tile[kTile];
///
/// SPIR-V and GLSL size `tile` with the specialization constant itself. WGSL sizes it with an
/// `override`, but only when the spec-constant dimension is the outermost dimension of a
/// `var<workgroup>` and the element type has a fixed size. HLSL, CUDA, Metal and the CPU targets
/// have no array whose size is fixed at pipeline creation. The declarations checked are global
/// variables and parameters, local variables, function parameters and struct fields; a variable
/// of a struct type is not reported again, because the struct field that holds the array already
/// is. A value of such a type that no declaration holds, such as the result of
/// `ByteAddressBuffer.Load<float[kTile]>`, is reported once per type.
///
/// The check runs after specialization and dead-code elimination, so an array that a generic
/// argument introduces is reported and one in removed code is not. On targets without
/// specialization constants `collectGlobalUniformParameters` has by then turned `kTile` into a
/// load from the global uniform block; the load keeps the specialization-constant rate, which is
/// what the check looks for.
void checkSpecializationConstantSizedArrays(
    IRModule* module,
    CodeGenTarget target,
    DiagnosticSink* sink);

/// Report each list initializer, such as the one in `float a[kTile] = {};`, that still builds an
/// array sized by a specialization constant when optimization has finished. No target can spell
/// that value: a GLSL or HLSL array constructor needs one argument per element, and SPIR-V's
/// `OpCompositeConstruct` needs one operand per element. The check runs after the last
/// optimization pass, because an initializer whose array the optimizer removes, as in
/// `float a[kTile] = {}; a[0] = 1; return a[0];`, compiles to SPIR-V and GLSL.
void checkSpecializationConstantSizedArrayInitializers(IRModule* module, DiagnosticSink* sink);

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
