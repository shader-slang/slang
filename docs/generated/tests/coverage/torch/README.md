---
generated: true
model: claude-opus-4-8[1m]
generated_at: 2026-06-11T16:00:00+00:00
source_commit: ef1068b5485e09b3a7afadba2e25f9541e29af42
watched_paths_digest: 72169107a8604a0a9f04384d6eea2b34191649206aecf28b8f167f23edfb7780
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Tests for coverage/torch

## Intent

White-box characterization tests for the PyTorch C++ binding generator
`source/slang/slang-ir-pytorch-cpp-binding.cpp` (~15% covered). These pin the
**current observed behaviour** of the `-target torch` emission pipeline driven
by `[TorchEntryPoint]`, `[AutoPyBindCUDA]`, `[CudaKernel]`, and `TorchTensor` /
`TensorView`. The strategy: one test per composite-type lowering branch of
`translateToTupleType` / `makeTargetTuple` / `makeValueFromTargetTuple`
(struct / vector / matrix / array) pinned on the emitted C++ host signature, one
test each for the struct and array reflection paths
(`markTypeForPyExport` -> `generateReflectionForType` -> `__typeinfo__`), and
two diagnostic tests for the two clean reject paths in
`generateCppBindingForFunc` (E55101 / E55102). The third reject path
(`UnableToAutoMapCudaTypeToHostType`, E56001) is followed by a SIGSEGV, so it is
filed as a finding rather than pinned as a test.

All emission CHECKs are pinned verbatim from real `slangc -target torch` output;
the emitted host shapes corroborate the hand-written sibling tests under
`tests/autodiff/` (e.g. `cuda-kernel-export.slang`, `autopybind-basic.slang`),
so they are not tagged `characterization-unverified`.

## Functional coverage

| Test                                                                                 | Pins                                                                                                                                                 | covers=                                       |
| ------------------------------------------------------------------------------------ | ---------------------------------------------------------------------------------------------------------------------------------------------------- | --------------------------------------------- |
| [`torch-entrypoint-struct-return.slang`](torch-entrypoint-struct-return.slang)       | A `[TorchEntryPoint]` struct `{float, TorchTensor<float>}` return flattens to `std::tuple<float, torch::Tensor>`; pybind `m.def` exports it.         | source/slang/slang-ir-pytorch-cpp-binding.cpp |
| [`torch-entrypoint-vector-roundtrip.slang`](torch-entrypoint-vector-roundtrip.slang) | A `float3` parameter and return both map to `std::tuple<float, float, float>` (vector branch).                                                       | source/slang/slang-ir-pytorch-cpp-binding.cpp |
| [`torch-entrypoint-matrix-return.slang`](torch-entrypoint-matrix-return.slang)       | A `float2x2` maps to a nested `std::tuple<std::tuple<float,float>, std::tuple<float,float>>` (matrix branch).                                        | source/slang/slang-ir-pytorch-cpp-binding.cpp |
| [`torch-entrypoint-array-roundtrip.slang`](torch-entrypoint-array-roundtrip.slang)   | A fixed `float[2]` parameter and return both map to `std::tuple<float, float>` (array branch).                                                       | source/slang/slang-ir-pytorch-cpp-binding.cpp |
| [`autobind-cuda-struct-reflection.slang`](autobind-cuda-struct-reflection.slang)     | An `[AutoPyBindCUDA]` kernel with a struct-of-builtins param emits a host wrapper (uint3 block/grid + flattened struct) plus `__typeinfo__MyStruct`. | source/slang/slang-ir-pytorch-cpp-binding.cpp |
| [`autobind-cuda-array-reflection.slang`](autobind-cuda-array-reflection.slang)       | An `[AutoPyBindCUDA]` kernel with a `float3[4]` param emits a `__typeinfo__Array__4` reflection function named by element count.                     | source/slang/slang-ir-pytorch-cpp-binding.cpp |
| [`torch-return-string-rejected.slang`](torch-return-string-rejected.slang)           | A `[TorchEntryPoint]` returning `String` is rejected with E55101 (invalid pytorch kernel return type).                                               | source/slang/slang-ir-pytorch-cpp-binding.cpp |
| [`torch-param-string-rejected.slang`](torch-param-string-rejected.slang)             | A `[TorchEntryPoint]` taking a `String` parameter is rejected with E55102 (invalid pytorch kernel parameter type).                                   | source/slang/slang-ir-pytorch-cpp-binding.cpp |

## Unreachable gaps

| Area                                                                                                                                              | Why not targeted                                                                                                                                                                                                     |
| ------------------------------------------------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `UnableToAutoMapCudaTypeToHostType` (E56001) path in `generateCUDAWrapperForFunc`                                                                 | Reachable (e.g. an `[AutoPyBindCUDA]` kernel with a `String` param), but the diagnostic is followed by a SIGSEGV — filed as finding `torch-autobind-unmappable-param-sigsegv`, not a passing test.                   |
| `nullptr` early-returns inside `translateToTupleType` / `makeTargetTuple` for non-literal matrix/vector/array extents (e.g. `as<IRIntLit>` fails) | Defensive: a `TorchEntryPoint` signature with a non-constant array/vector/matrix extent does not type-check earlier in the frontend, so these null guards are not reachable from a clean `-target torch` invocation. |
| `SLANG_UNEXPECTED("struct marked for export has no name")` in `markTypeForPyExport`                                                               | Defensive: a struct reaching this code always carries a name hint from the frontend; an anonymous-struct param is not expressible in a CLI-reachable kernel signature.                                               |
| `generateDerivativeWrappers` fwd/bwd-diff wrapper synthesis                                                                                       | Reachable via `[AutoPyBindCUDA][Differentiable][CudaKernel]` but already covered by the hand-written `tests/autodiff/autopybind-differentiable.slang`; duplicating it here adds no coverage signal this bundle owns. |
| `lowerBuiltinTypesForKernelEntryPoints` builtin-type substitution                                                                                 | Reachable via bfloat16/half tensor kernels but already exercised by `tests/autodiff/autopybind-bfloat16-tensor.slang`; out of this bundle's scope.                                                                   |

## Doc gaps observed

NA
