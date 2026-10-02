// slang-ir-sccp.h
#pragma once
#include "slang-ir-insts-enum.h"

namespace Slang
{
struct IRModule;
struct IRInst;
struct TranslationContext;
class DiagnosticSink;
class TargetProgram;

/// Apply Sparse Conditional Constant Propagation (SCCP) to a module.
///
/// This optimization replaces instructions that can only ever evaluate
/// to a single (well-defined) value with that constant value, and
/// also eliminates conditional branches where the condition will
/// always evaluate to a constant (which can lead to entire blocks
/// becoming dead code)
/// Returns true if IR is changed.
///
/// If `diagnoseConstantEvaluation` is true, the pass also reports problems that constant
/// evaluation reveals, such as integer division by zero and implicit integer conversions of
/// constants that change the value. The front end enables this once per user module, right
/// after lowering; later runs over the same code leave it off so that nothing is reported twice.
bool applySparseConditionalConstantPropagation(
    IRModule* module,
    TargetProgram* targetProgram,
    DiagnosticSink* sink);
bool applySparseConditionalConstantPropagationForGlobalScope(
    IRModule* module,
    TargetProgram* targetProgram,
    DiagnosticSink* sink);

bool applySparseConditionalConstantPropagation(
    IRInst* func,
    TargetProgram* targetProgram,
    DiagnosticSink* sink,
    TranslationContext* translationContext = nullptr);

IRInst* tryConstantFoldInst(IRModule* module, TargetProgram* targetProgram, IRInst* inst);

bool isEvaluableOpCode(IROp op);

} // namespace Slang
