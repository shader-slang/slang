// slang-ir-cuda-byref-entry-point-params.h
#pragma once

namespace Slang
{

struct IRModule;
class DiagnosticSink;

/// Reconcile CUDA entry-point parameters to the by-reference layout decision made at
/// parameter binding.
///
/// On CUDA compute targets, parameter binding lays out an entry-point `uniform`
/// parameter that carries a descriptor table (a fixed-size array of resources or
/// pointer-backed structs) as an implicit `ParameterBlock<T>`: one 8-byte device
/// pointer in kernel-argument space, payload in device global memory (see
/// `shouldPassCUDAEntryPointUniformParamByRef` in slang-parameter-binding.cpp for the
/// rationale and the predicate — that is the single site where the decision is made).
///
/// This pass makes the IR match what the recorded layout already declares: for each
/// entry-point parameter whose `IRVarLayout` reports a parameter-group type layout
/// while the parameter's IR type is still the by-value element type, it retypes the
/// parameter to `ParameterBlock<T>` and rewrites the body's value uses into loads
/// through addresses. It never evaluates the predicate itself — it consumes the
/// layout decision — so the emitted kernel signature and the reflected layout cannot
/// disagree (the failure mode that sank earlier emit-stage attempts at this fix).
///
/// For example, `void main(uniform TensorList list)` whose layout says
/// parameter-group emits as `__global__ void main(TensorList_0* list_0)`, and a
/// runtime index `list.tensors[i]` becomes an ordinary dynamically-addressed global
/// load instead of a serial `.param`-space load chain (issue #11774).
void reconcileCUDAByRefEntryPointParams(IRModule* module, DiagnosticSink* sink);

} // namespace Slang
