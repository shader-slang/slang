# NVVM Backend Capability Ledger

This ledger records what the direct NVVM experiment has demonstrated and where ordinary Slang
programs currently stop. `Pass` means the named lane ran successfully on the local CUDA/toolchain
configuration described in [the backend design](nvvm-backend.md). `Expected stop` means the test
reached a deliberate, stable boundary; it is not counted as backend support. Empty measurement
fields have not been collected yet. `Pending` describes planned evidence that has not run and does
not establish backend support.

## File-backed compatibility gate

The ordinary shader corpus now provides the prioritization signal in addition to the deeper unit
and runtime matrix. Only passing direct-NVVM files are registered; probes that stop at an
unsupported shape remain planning evidence rather than expected failures.

| Shader | Route | Status | Evidence or first stop |
| --- | --- | --- | --- |
| `tests/cuda/nvvm-raw-scalar.slang` | Direct NVVM PTX | Pass | Helper, branch/phi, finite loop, and global signed-i32 store; FileCheck observes the entry and store |
| `tests/cuda/nvvm-core-execution.slang` | Direct NVVM PTX | Pass | `%tid.x`, shared storage, group barrier, and global store are present |
| `tests/cuda/nvvm-mixed-numeric.slang` | Direct NVVM PTX | Pass | Narrow/wide integer, float32, and fixed-vector workload emits the expected typed global stores |
| `tests/cuda/nvvm-unsupported-ir.slang` | Direct NVVM diagnostic | Expected stop | Exact `CUDA kernel decoration` boundary for executable conventional compute |
| `tests/cuda/compile-to-cuda.slang` | NVRTC + direct NVVM runtime | Pass | Ordinary `SV_DispatchThreadID`, one conventional `RWStructuredBuffer<int>`, host-populated `SLANG_globalParams`, and zero-parameter kernel produce identical results |
| `tests/cuda/cuda-layout.slang` | NVRTC + direct NVVM runtime | Pass | All 28 scalar/vector CUDA layout queries agree; direct PTX passes `ptxas` |
| `tests/cuda/cuda-array-layout.slang` | NVRTC + direct NVVM runtime | Pass | Aggregate/array/matrix/pointer layout queries fold before runtime emission and produce identical results; direct PTX passes `ptxas` |
| `tests/cuda/wave-lane-index-multidim.slang` | NVRTC + direct NVVM + Vulkan runtime | Pass | A two-dimensional group combines `SV_GroupIndex`, float resource stores, and wave operations with identical comparison output; direct PTX passes `ptxas` |
| `tests/cuda/sampler-comparison-state-unused.slang` | CUDA source + NVRTC PTX + direct NVVM PTX | Pass | Multi-field sampler/resource storage has one shared 40-byte ABI; direct PTX passes `ptxas` |
| `tests/cuda/param-block-alignment.slang` | NVRTC + direct NVVM runtime | Pass | A scalar uniform, flat scalar parameter block, resource view, and folded layout queries produce identical results; direct PTX passes `ptxas` |
| `tests/cuda/cuda-kernel-param-layout.slang` | NVRTC + direct NVVM runtime | Pass | A flat by-value scalar struct, read-only and read-write float resource views, and scalar count produce `11, 12, 13, 14`; direct PTX preserves the launch layout and passes `ptxas` |
| `tests/cuda/get-buffer-ptr.slang` | NVRTC + direct NVVM runtime | Pass | Structured and byte-address data-pointer extraction composes through generic field extraction and scalar pointer offsets; all eight values agree and direct PTX passes `ptxas` |
| `tests/compute/byte-address-buffer.slang` | CUDA/NVRTC + direct NVVM runtime | Pass | Core UInt/UInt2-4 byte-address loads and UInt store pass for read-only and read-write inputs; direct PTX passes `ptxas` |
| `tests/compute/byte-address-buffer-aligned.slang` | CUDA/NVRTC + direct NVVM runtime | Pass | Float/Float4 aligned and scalarized byte-address copies pass; direct PTX exposes four Float loads/stores and passes `ptxas` |
| `tests/compute/byte-address-buffer-singlearg-float3-11592.slang` | CPU + direct NVVM runtime | Pass | A Float3 constant round-trips through a byte buffer into typed Float output; direct PTX passes `ptxas` |
| `tests/compute/byte-address-buffer-64bit.slang` | CPU + direct NVVM runtime | Pass | UInt widens and adds as UInt64 before byte storage; direct PTX lowers the four-byte-aligned wide store to two ordered UInt stores and passes `ptxas` |
| `tests/compute/byte-address-buffer-array.slang` | CUDA/NVRTC + direct NVVM runtime | Pass | One-level `Array<Float4, 2>` byte pass-through and the scalarized Float copy compile and run; direct PTX passes `ptxas` |
| `tests/cuda/cuda-vector-binary-ops.slang` | CPU + CUDA/NVRTC + direct NVVM runtime | Pass | Selected integer/float vectors exercise arithmetic, shifts, signed narrow division/remainder/comparison, Float32 comparison, Boolean extraction, and floating remainder across 40 asymmetric results; direct PTX passes `ptxas` |
| `tests/compute/structured-buffer-load.slang` | CUDA + direct NVVM runtime/PTX | Pass | Scalar UInt and vector Int4 read-only/read-write resource loads produce `0x40, 0x40, 0x37`; direct PTX contains `ld.global.nc.v4.u32` and passes `ptxas` |
| `tests/compute/structured-buffer-swizzle-store.slang` | CUDA + direct NVVM runtime/PTX | Pass | Four Float4 destination permutations preserve independent component stores and produce `4`; direct PTX reloads `v4.f32` after scalar lane stores and passes `ptxas` |
| `tests/cuda/vector-dot-unroll.slang` | CUDA source + direct NVVM PTX | Pass | Float2/3/4 and Int3 fallback helpers cross the selected vector ABI; PTX retains scalarized dot arithmetic and passes `ptxas` |
| `tests/hlsl-intrinsic/vector-dot-int.slang` | CUDA/NVRTC + direct NVVM runtime/PTX | Pass | Int3, UInt3, UInt64x2, and Int16x4 dot helpers produce `-14, 28, 20, 5`; direct PTX passes `ptxas` |
| `tests/compute/vector-scalar-compare.slang` | CUDA/NVRTC + direct NVVM runtime/PTX | Pass | Integer vector/scalar bitwise and comparison operations feed `all(bool2)` through a dynamic Boolean extract; all 16 results agree and unoptimized PTX passes `ptxas` |
| `tests/language-feature/operator-overload/builtin-operator-fastpath.slang` | CPU + Vulkan + direct NVVM runtime/PTX | Pass | Signed scalar and vector arithmetic, comparisons, bitwise operations, Boolean lane logic, and scalar broadcast produce all 16 expected values; direct PTX passes `ptxas` |
| `tests/language-feature/operator-overload/builtin-operator-fastpath-uint.slang` | CPU + Vulkan + direct NVVM runtime/PTX | Pass | Unsigned vector/scalar shifts accept signless physical shift counts and preserve logical right-shift results; all ten values agree and direct PTX passes `ptxas` |
| `tests/language-feature/operator-overload/builtin-operator-fastpath-bool.slang` | CPU + Vulkan + direct NVVM runtime/PTX | Pass | Scalar and Bool4 equality/inequality, explicit Boolean-vector construction, negation, and extraction produce all eight expected values; direct PTX passes `ptxas` |
| `tests/cuda/nvvm-float-matrix-values.slang` | Direct NVVM runtime/PTX | Pass | Float2x2 construction, matrix/scalar and matrix/matrix addition, branch/aggregate-phi transport, and constant row/column extraction produce `8, 15`; direct PTX passes `ptxas` |
| `tests/compute/row-major.slang` | CUDA + direct NVVM runtime/PTX | Pass | A legalized Float4x4 constant-buffer value crosses generated matrix/vector helpers and local row/lane addresses; direct output is `11, 22, 33, 1` and PTX passes `ptxas` |
| `tests/compute/column-major.slang` | CUDA + direct NVVM runtime/PTX | Pass | Early LLVM storage lowering preserves the column-major Float4x4 representation and emits an explicit unpack/transpose graph; direct output is the unchanged expected `1` and PTX passes `ptxas` |
| `tests/compute/non-square-row-major.slang` | CUDA + direct NVVM runtime/PTX | Pass | The existing packed Float3x2 CUDA contract produces `12, 16`; the 881-byte direct PTX passes `ptxas` |
| `tests/compute/non-square-column-major.slang` | Direct NVVM diagnostic | Expected stop | Its established physical form is `Array<Float3, 2, stride=12>`, which cannot be represented by LLVM's naturally 16-byte-strided `<3 x float>` array without a distinct padded-storage contract |
| `tests/compute/groupshared.slang` | CUDA + direct NVVM runtime/PTX | Pass | The established helper-based Int shared-array workload returns `1, 0, 3, 2`; direct PTX preserves shared load/store and synchronization and passes `ptxas` |
| `tests/language-feature/execution-model/groupshared-barrier-functional.slang` | CUDA + direct NVVM runtime/PTX | Pass | An unsigned execution index writes shared Int storage, synchronizes, and reads its neighbor with results `10, 20, 30, 0`; direct PTX passes `ptxas` |
| `tests/language-feature/execution-model/groupshared-multi-barrier-functional.slang` | CUDA + direct NVVM runtime/PTX | Pass | Three barrier calls preserve two rounds of shared communication with results `2, 3, 0, 1`; direct PTX passes `ptxas` |

Slices 69 and 70 consolidated the implementation onto one exact forward-only builder ABI and one
typed-descriptor capability system. Older rows below retain the interface names that described the
historical evidence when it was collected; they are not claims that those compatibility surfaces
remain in the current implementation.

## Semantic capability evidence

| Test | Bucket | Requirements | NVRTC | NVVM | First NVVM stop | Diagnostic or capability | ABI/runtime comparison | Measurements |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmCompilerCompilesEmptyKernelBitcode` | 0 | CUDA 12+, libNVVM, `compute_75` | Not applicable | Pass | — | Exact LLVM-bitcode artifact verifies and compiles | Entry symbol checked; no runtime | — |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmIRBuilderCompilesEmptyKernel` | 1 | LLVM 14 provider, libNVVM, `compute_75` | Not applicable | Pass | — | Builder-produced empty kernel compiles | Entry symbol checked; no runtime | — |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmIRBuilderDifferentialScalarPTX` | 1 | LLVM 14 provider, NVRTC, libNVVM, `compute_75` | Pass | Pass | — | AS1 `i32` load/store reference kernels | Parameter widths and entry-scoped global operations agree; no runtime | PTX/resource timing not measured |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmIRBuilderPtxasAcceptsScalarReferenceKernels` | 1 | LLVM 14 provider, CUDA `ptxas`, `sm_75` | Pass | Pass | — | Both scalar reference kernels assemble | Static PTX acceptance; no runtime | PTX/resource timing not measured |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmIRBuilderCompilesScalarBitcodeThroughRegistry` | 1 | LLVM 14 provider, libNVVM, `compute_75` | Not applicable | Pass | — | Session-registered NVVM compiler accepts exact builder bitcode | Both entry symbols checked; no runtime | — |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmSlangEmptyComputeUsesDirectPipeline` | 1 | In-process fake LLVM 14 builder and libNVVM, `cuda_sm_7_0` | Not compared | Pass | — | Ordinary Slang linked IR lowers through the provider-negotiated verified wire dialect and the registered NVVM compiler | Builder receives `computeMain`; exact NVVM-2.0 assembly bytes and `compute_70` options checked; no runtime | Fake-only; no performance measurements |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmSlangRealEmptyCompute` | 1 | LLVM 14.0.6 provider, CUDA 12+ libNVVM, `cuda_sm_7_0` | Not compared | Pass | — | Ordinary empty compute kernel compiles through the real direct route | PTX entry `computeMain` checked; no runtime | PTX size and compile time not measured |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmSlangRealEmptyComputePtxasAccepts` | 1 | LLVM 14.0.6 provider, CUDA 12+ libNVVM and `ptxas`, `cuda_sm_7_0` | Not compared | Pass | — | Real direct-route PTX assembles successfully | Static PTX acceptance; no runtime | Resource and timing measurements not collected |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmIRBuilderBuildsScalarConditionalKernel` | 2 | LLVM 14.0.6 provider | Not applicable | Pass | Not applicable | Provider emits signed `i32` add/sub/less-than and conditional/unconditional branches | Verified LLVM assembly and bitcode; no runtime | Not measured |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmIRBuilderBuildsScalarSSALoopKernel` | 2 | LLVM 14.0.6 provider | Not applicable | Pass | Not applicable | Provider emits constants, two signed-`i32` phis, and a finite loop with delayed incoming edges | Verified LLVM assembly and bitcode; no runtime | Not measured |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmIRBuilderBuildsScalarFunctionKernel` | 2 | LLVM 14.0.6 provider | Not applicable | Pass | Not applicable | Provider emits one signed-`i32` helper, direct call, valued return, and kernel-only annotation | Verified LLVM assembly and bitcode; no runtime | Not measured |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmIRBuilderBuildsPointerOffsetKernel` | 3 | LLVM 14.0.6 provider | Not applicable | Pass | Not applicable | Provider emits two plain address-space-1 `getelementptr i32` values without `inbounds` | Verified LLVM assembly and bitcode; no runtime | Not measured |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmIRBuilderBuildsArrayElementKernel` | 3 | LLVM 14.0.6 provider | Not applicable | Pass | Not applicable | Provider emits a fixed `[4 x i32]` type and two plain address-space-1 array-element GEPs with `{i32 0, index}` | Verified LLVM assembly and bitcode; no runtime | Not measured |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmIRBuilderBuildsIntegerMultiplyKernel` | 2 | LLVM 14.0.6 provider | Not applicable | Pass | Not applicable | Provider emits one `mul i32` whose result feeds the address-space-1 store | Verified LLVM assembly and bitcode; no runtime | Not measured |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmIRBuilderBuildsIntegerBitAndKernel` | 2 | LLVM 14.0.6 provider | Not applicable | Pass | Not applicable | Provider emits one `and i32` whose result feeds the address-space-1 store | Verified LLVM assembly and bitcode; no runtime | Not measured |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmIRBuilderBuildsIntegerBitOrKernel` | 2 | LLVM 14.0.6 provider | Not applicable | Pass | Not applicable | Provider emits one `or i32` whose result feeds the address-space-1 store | Verified LLVM assembly and bitcode; no runtime | Not measured |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmIRBuilderBuildsIntegerBitXorKernel` | 2 | LLVM 14.0.6 provider | Not applicable | Pass | Not applicable | Provider emits one `xor i32` whose result feeds the address-space-1 store | Verified LLVM assembly and bitcode; no runtime | Not measured |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmIRBuilderBuildsIntegerBitNotKernel` | 2 | LLVM 14.0.6 provider | Not applicable | Pass | Not applicable | Provider emits exactly one `xor i32` with an all-ones operand whose result feeds the address-space-1 store and no `xor i64` | Verified LLVM assembly and bitcode; no runtime | Not measured |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmIRBuilderBuildsIntegerNegateKernel` | 2 | LLVM 14.0.6 provider | Not applicable | Pass | Not applicable | Provider emits exactly one unflagged `sub i32 0, value` whose result feeds the address-space-1 store, with no i64/`nsw`/`nuw` variant | Verified LLVM assembly and bitcode; no runtime | Not measured |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmIRBuilderBuildsIntegerEqualKernel` | 2 | LLVM 14.0.6 provider | Not applicable | Pass | Not applicable | Provider emits exactly one `icmp eq i32` whose `i1` result controls the established conditional branch | Verified LLVM assembly and bitcode; no runtime | Not measured |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmIRBuilderBuildsIntegerNotEqualKernel` | 2 | LLVM 14.0.6 provider | Not applicable | Pass | Not applicable | Provider emits exactly one `icmp ne i32` whose `i1` result controls the established conditional branch | Verified LLVM assembly and bitcode; no runtime | Not measured |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmIRBuilderBuildsIntegerSignedGreaterThanKernel` | 2 | LLVM 14.0.6 provider | Not applicable | Pass | Not applicable | Provider emits exactly one `icmp sgt i32` whose `i1` result controls the established conditional branch | Verified LLVM assembly and bitcode; no runtime | Not measured |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmIRBuilderBuildsIntegerSignedLessEqualKernel` | 2 | LLVM 14.0.6 provider | Not applicable | Pass | Not applicable | Provider emits exactly one `icmp sle i32` whose `i1` result controls the established conditional branch | Verified LLVM assembly and bitcode; no runtime | Not measured |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmIRBuilderBuildsIntegerSignedGreaterEqualKernel` | 2 | LLVM 14.0.6 provider | Not applicable | Pass | Not applicable | Provider emits exactly one `icmp sge i32` whose `i1` result controls the established conditional branch | Verified LLVM assembly and bitcode; no runtime | Not measured |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmSlangScalarMemoryAndConditionalUseDirectPipeline` (write/copy cases) | 1 | In-process fake builder and libNVVM, `cuda_sm_7_0` | Not compared | Pass | Not applicable | Raw `[CUDAKernel]` signed `i32` and AS1 pointer parameters, load, and store | Exact parameter order, value producers, and memory operands checked | Fake-only; no performance measurements |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmSlangScalarMemoryAndConditionalUseDirectPipeline` (conditional case) | 2 | In-process fake builder and libNVVM, `cuda_sm_7_0` | Not compared | Pass | Not applicable | Signed `i32` add/sub/less-than and phi-free `if`/else control flow | Exact comparison operands, branch targets, and per-arm stores checked | Fake-only; no performance measurements |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmSlangScalarSSAUsesDirectPipeline` | 2 | In-process fake builder and libNVVM, `cuda_sm_7_0` | Not compared | Pass | Not applicable | Signed-i32 constants, a merge phi, and the two-phi `sumToLimit` loop lower from canonical Slang IR | Exact constant identity, blocks, values, predecessor edges, phi pairs, and store consumers checked | Fake-only; no performance measurements |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmSlangScalarFunctionsUseDirectPipeline` | 2 | In-process fake builder and libNVVM, `cuda_sm_7_0` | Not compared | Pass | Not applicable | A two-helper transitive signed-i32 DAG lowers through direct calls and valued returns; an unreachable multiplication helper is pruned | Exact function/caller/callee/argument/result/store topology and kernel-only annotation checked | Fake-only; no performance measurements |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmSlangPointerOffsetUsesDirectPipeline` | 3 | In-process fake builder and libNVVM, `cuda_sm_7_0` | Not compared | Pass | Not applicable | Two canonical signed-i32 device-pointer offsets lower without byte arithmetic or access reconstruction | Exact block/base/offset/result topology proves source-offset load and destination-offset store consumers | Fake-only; no performance measurements |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmSlangFixedDeviceArrayUsesDirectPipeline` | 3 | In-process fake builder and libNVVM, `cuda_sm_7_0` | Not compared | Pass | Not applicable | Exact fixed-i32 device-array pointer parameters and two canonical `IRGetElementPtr` values lower through the array capability | Exact array/type sharing and base/index/result/load/store topology prove the source and destination consumers | Fake-only; no performance measurements |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmSlangIntegerMultiplyUsesDirectPipeline` | 2 | In-process fake builder and libNVVM, `cuda_sm_7_0` | Not compared | Pass | Not applicable | One canonical signed-i32 `kIROp_Mul` lowers through the dedicated multiply capability | Exact left/right parameter identities and multiply-result store consumer checked | Fake-only; no performance measurements |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmSlangIntegerBitAndUsesDirectPipeline` | 2 | In-process fake builder and libNVVM, `cuda_sm_7_0` | Not compared | Pass | Not applicable | One canonical signed-i32 `kIROp_BitAnd` lowers through the dedicated bitwise-AND capability | Exact left/right parameter identities and bitwise-AND-result store consumer checked | Fake-only; no performance measurements |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmSlangIntegerBitOrUsesDirectPipeline` | 2 | In-process fake builder and libNVVM, `cuda_sm_7_0` | Not compared | Pass | Not applicable | One canonical signed-i32 `kIROp_BitOr` lowers through the dedicated bitwise-OR capability | Exact left/right parameter identities and bitwise-OR-result store consumer checked | Fake-only; no performance measurements |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmSlangIntegerBitXorUsesDirectPipeline` | 2 | In-process fake builder and libNVVM, `cuda_sm_7_0` | Not compared | Pass | Not applicable | One canonical signed-i32 `kIROp_BitXor` lowers through the dedicated bitwise-XOR capability | Exact left/right parameter identities and bitwise-XOR-result store consumer checked | Fake-only; no performance measurements |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmSlangIntegerBitNotUsesDirectPipeline` | 2 | In-process fake builder and libNVVM, `cuda_sm_7_0` | Not compared | Pass | Not applicable | One canonical signed-i32 `kIROp_BitNot` lowers through the dedicated bitwise-NOT capability | Exact parameter-1 operand and bitwise-NOT-result store consumer checked; unrelated callbacks remain unused | Fake-only; no performance measurements |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmSlangIntegerNegateUsesDirectPipeline` | 2 | In-process fake builder and libNVVM, `cuda_sm_7_0` | Not compared | Pass | Not applicable | One canonical signed-i32 `kIROp_Neg` lowers through the dedicated integer-negate capability | Exact parameter-1 operand and negate-result store consumer checked; unrelated callbacks including BitNot remain unused | Fake-only; no performance measurements |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmSlangIntegerEqualUsesDirectPipeline` | 2 | In-process fake builder and libNVVM, `cuda_sm_7_0` | Not compared | Pass | Not applicable | One canonical signed-i32 `kIROp_Eql` lowers through the dedicated equality capability | Exact left/right parameters feed equality, whose Boolean result controls the two-arm constant/phi/store graph; signed less-than remains unused | Fake-only; no performance measurements |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmSlangIntegerNotEqualUsesDirectPipeline` | 2 | In-process fake builder and libNVVM, `cuda_sm_7_0` | Not compared | Pass | Not applicable | One canonical signed-i32 `kIROp_Neq` lowers through the dedicated inequality capability | Exact left/right parameters feed inequality, whose Boolean result controls the two-arm constant/phi/store graph; equality and signed less-than remain unused | Fake-only; no performance measurements |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmSlangIntegerSignedGreaterThanUsesDirectPipeline` | 2 | In-process fake builder and libNVVM, `cuda_sm_7_0` | Not compared | Pass | Not applicable | One canonical signed-i32 `kIROp_Greater` lowers through the dedicated signed-greater-than capability | Exact left/right parameters feed greater-than, whose Boolean result controls the two-arm constant/phi/store graph; less-than, equality, and inequality callbacks remain unused | Fake-only; no performance measurements |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmSlangIntegerSignedLessEqualUsesDirectPipeline` | 2 | In-process fake builder and libNVVM, `cuda_sm_7_0` | Not compared | Pass | Not applicable | One canonical signed-i32 `kIROp_Leq` lowers through the dedicated signed-less-equal capability | Exact left/right parameters feed less-equal, whose Boolean result controls the two-arm constant/phi/store graph; strict less-than, equality, inequality, and greater-than callbacks remain unused | Fake-only; no performance measurements |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmSlangIntegerSignedGreaterEqualUsesDirectPipeline` | 2 | In-process fake builder and libNVVM, `cuda_sm_7_0` | Not compared | Pass | Not applicable | One canonical signed-i32 `kIROp_Geq` lowers through the dedicated signed-greater-equal capability | Exact left/right parameters feed greater-equal, whose Boolean result controls the two-arm constant/phi/store graph; all earlier comparison callbacks remain unused | Fake-only; no performance measurements |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmSlangRealScalarDifferentialPTX` | 1-2 | LLVM 14.0.6 provider, NVRTC, libNVVM, `cuda_sm_7_0` | Pass | Pass | Not applicable | Ordinary Slang write, copy, conditional, constant, merge-phi, loop, and helper kernels compile through both routes | Existing widths plus `[64, 32]` for the helper case agree; expected global-memory semantics checked | PTX size and timing not measured |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmSlangRealScalarPtxasAccepts` | 1-2 | LLVM 14.0.6 provider, NVRTC, libNVVM, CUDA 12.9 `ptxas`, `sm_75` | Pass | Pass | Not applicable | All seven ordinary-Slang scalar/SSA/helper kernels assemble from both PTX routes | Static PTX acceptance | Resource and timing measurements not collected |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmSlangScalarRuntimeMatchesNVRTC` | 1-2 | LLVM 14.0.6 provider, NVRTC, libNVVM, CUDA driver and GPU with compute capability 7.0+ | Pass | Pass | Not applicable | Both routes launch the same scalar, merge, finite-loop, and transitive-helper kernels | Existing values still match; helper graph maps `5 -> 13` and `-2 -> -1` | Kernel timing not measured |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmSlangRealPointerOffsetDifferentialPTX` | 3 | LLVM 14.0.6 provider, NVRTC, libNVVM, `cuda_sm_7_0` | Pass | Pass | Not applicable | Signed-i32 offsets on raw read/read-write device pointers compile through both routes | Parameter widths `[64, 64, 32]` and entry-scoped global-memory behavior agree | PTX size and timing not measured |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmSlangRealPointerOffsetPtxasAccepts` | 3 | LLVM 14.0.6 provider, NVRTC, libNVVM, CUDA 12.9 `ptxas`, `sm_75` | Pass | Pass | Not applicable | Both pointer-offset PTX outputs assemble | Static PTX acceptance | Resource and timing measurements not collected |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmSlangPointerOffsetRuntimeMatchesNVRTC` | 3 | LLVM 14.0.6 provider, NVRTC, libNVVM, CUDA driver and GPU with compute capability 7.0+ | Pass | Pass | Not applicable | Both routes launch positive-index and negative interior-base pointer-offset cases | Intended elements are copied and neighboring sentinels remain unchanged | Kernel timing not measured |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmSlangRealFixedDeviceArrayDifferentialPTX` | 3 | LLVM 14.0.6 provider, NVRTC, libNVVM, `cuda_sm_7_0` | Pass | Pass | Not applicable | Fixed `int[4]` device-array pointers and signed-i32 element indexing compile through both routes | Parameter widths `[64, 64, 32]` and entry-scoped global i32 load/store behavior agree | PTX size and timing not measured |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmSlangRealFixedDeviceArrayPtxasAccepts` | 3 | LLVM 14.0.6 provider, NVRTC, libNVVM, CUDA 12.9 `ptxas`, `sm_75` | Pass | Pass | Not applicable | Both fixed-device-array PTX outputs assemble | Static PTX acceptance | Resource and timing measurements not collected |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmSlangFixedDeviceArrayRuntimeMatchesNVRTC` | 3 | LLVM 14.0.6 provider, NVRTC, libNVVM, CUDA driver and GPU with compute capability 7.0+ | Pass | Pass | Not applicable | Both routes launch the same fixed-device-array kernel at endpoint indices `0` and `3` | Intended elements are copied and every neighboring sentinel remains unchanged | Kernel timing not measured |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmSlangRealIntegerMultiplyDifferentialPTX` | 2 | LLVM 14.0.6 provider, NVRTC, libNVVM, `cuda_sm_7_0` | Pass | Pass | Not applicable | Exact signed-i32 multiplication compiles through both routes | Parameter widths `[64, 32, 32]`, 32-bit multiply, and global i32 store semantics agree | PTX size and timing not measured |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmSlangRealIntegerMultiplyPtxasAccepts` | 2 | LLVM 14.0.6 provider, NVRTC, libNVVM, CUDA 12.9 `ptxas`, `sm_75` | Pass | Pass | Not applicable | Both signed-i32-multiply PTX outputs assemble | Static PTX acceptance | Resource and timing measurements not collected |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmSlangIntegerMultiplyRuntimeMatchesNVRTC` | 2 | LLVM 14.0.6 provider, NVRTC, libNVVM, CUDA driver and GPU with compute capability 7.0+ | Pass | Pass | Not applicable | Both routes launch the same signed-i32-multiply kernel for positive, negative, and zero cases | On the RTX 5090, products `42`, `-42`, and `0` agree | Kernel timing not measured |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmSlangRealIntegerBitAndDifferentialPTX` | 2 | LLVM 14.0.6 provider, NVRTC, libNVVM, `cuda_sm_7_0` | Pass | Pass | Not applicable | Exact signed-i32 bitwise AND compiles through both routes | Parameter widths `[64, 32, 32]`, `and.b32`, and global i32 store semantics agree | PTX size and timing not measured |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmSlangRealIntegerBitAndPtxasAccepts` | 2 | LLVM 14.0.6 provider, NVRTC, libNVVM, CUDA 12.9 `ptxas`, `sm_75` | Pass | Pass | Not applicable | Both signed-i32-bitwise-AND PTX outputs assemble | Static PTX acceptance | Resource and timing measurements not collected |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmSlangIntegerBitAndRuntimeMatchesNVRTC` | 2 | LLVM 14.0.6 provider, NVRTC, libNVVM, CUDA driver and GPU with compute capability 7.0+ | Pass | Pass | Not applicable | Both routes launch the same signed-i32-bitwise-AND kernel for four representative bit patterns | On the RTX 5090, results are `0x18`, `0x12345678`, `-4`, and `0` | Kernel timing not measured |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmSlangRealIntegerBitOrDifferentialPTX` | 2 | LLVM 14.0.6 provider, NVRTC, libNVVM, `cuda_sm_7_0` | Pass | Pass | Not applicable | Exact signed-i32 bitwise OR compiles through both routes | Parameter widths `[64, 32, 32]`, token-safe `or.b32`, and global i32 store semantics agree | PTX size and timing not measured |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmSlangRealIntegerBitOrPtxasAccepts` | 2 | LLVM 14.0.6 provider, NVRTC, libNVVM, CUDA 12.9 `ptxas`, `sm_75` | Pass | Pass | Not applicable | Both signed-i32-bitwise-OR PTX outputs assemble | Static PTX acceptance | Resource and timing measurements not collected |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmSlangIntegerBitOrRuntimeMatchesNVRTC` | 2 | LLVM 14.0.6 provider, NVRTC, libNVVM, CUDA driver and GPU with compute capability 7.0+ | Pass | Pass | Not applicable | Both routes launch the same signed-i32-bitwise-OR kernel for four representative bit patterns | On the RTX 5090, results are `0x7e`, `-13`, `-1`, and `0x5f5f5f5f` | Kernel timing not measured |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmSlangRealIntegerBitXorDifferentialPTX` | 2 | LLVM 14.0.6 provider, NVRTC, libNVVM, `cuda_sm_7_0` | Pass | Pass | Not applicable | Exact signed-i32 bitwise XOR compiles through both routes | Parameter widths `[64, 32, 32]`, token-safe `xor.b32`, and global u32 store semantics agree | PTX size and timing not measured |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmSlangRealIntegerBitXorPtxasAccepts` | 2 | LLVM 14.0.6 provider, NVRTC, libNVVM, CUDA 12.9 `ptxas`, `sm_75` | Pass | Pass | Not applicable | Both signed-i32-bitwise-XOR PTX outputs assemble | Static PTX acceptance | Resource and timing measurements not collected |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmSlangIntegerBitXorRuntimeMatchesNVRTC` | 2 | LLVM 14.0.6 provider, NVRTC, libNVVM, CUDA driver and GPU with compute capability 7.0+ | Pass | Pass | Not applicable | Both routes launch the same signed-i32-bitwise-XOR kernel for four representative bit patterns | On the RTX 5090, results are `0x66`, `-305419897`, `15`, and `-1` | Kernel timing not measured |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmSlangRealIntegerBitNotDifferentialPTX` | 2 | LLVM 14.0.6 provider, NVRTC, libNVVM, `cuda_sm_7_0` | Pass | Pass | Not applicable | Exact signed-i32 bitwise NOT compiles through both routes | Parameter widths `[64, 32]`, token-safe `not.b32`, and global u32 store semantics agree; NVRTC uses address conversion while direct uses the raw pointer | PTX size and timing not measured |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmSlangRealIntegerBitNotPtxasAccepts` | 2 | LLVM 14.0.6 provider, NVRTC, libNVVM, CUDA 12.9 `ptxas`, `sm_75` | Pass | Pass | Not applicable | Both signed-i32-bitwise-NOT PTX outputs assemble | Static PTX acceptance | Resource and timing measurements not collected |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmSlangIntegerBitNotRuntimeMatchesNVRTC` | 2 | LLVM 14.0.6 provider, NVRTC, libNVVM, CUDA driver and GPU with compute capability 7.0+ | Pass | Pass | Not applicable | Both routes launch the same signed-i32-bitwise-NOT kernel for four representative bit patterns | On the RTX 5090, results are `-1`, `0`, `-1431655766`, and `15` | Kernel timing not measured |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmSlangRealIntegerNegateDifferentialPTX` | 2 | LLVM 14.0.6 provider, NVRTC, libNVVM, `cuda_sm_7_0` | Pass | Pass | Not applicable | Exact signed-i32 arithmetic negation compiles through both routes | Widths `[64, 32]`, token-safe `neg.s32`, and global u32 stores agree; NVRTC uses address conversion while direct uses the raw pointer, with no `sub.s32` or `not.b32` | PTX size and timing not measured |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmSlangRealIntegerNegatePtxasAccepts` | 2 | LLVM 14.0.6 provider, NVRTC, libNVVM, CUDA 12.9 `ptxas`, `sm_75` | Pass | Pass | Not applicable | Both signed-i32-negate PTX outputs assemble | Static PTX acceptance | Resource and timing measurements not collected |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmSlangIntegerNegateRuntimeMatchesNVRTC` | 2 | LLVM 14.0.6 provider, NVRTC, libNVVM, CUDA driver and GPU with compute capability 7.0+ | Pass | Pass | Not applicable | Both routes launch the same signed-i32-negate kernel for zero, positive, negative, and wrapping-minimum inputs | On the RTX 5090, inputs `0`, `1`, `-7`, and `INT_MIN` produce `0`, `-1`, `7`, and `-2147483648` | Kernel timing not measured |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmSlangRealIntegerEqualDifferentialPTX` | 2 | LLVM 14.0.6 provider, audited NVVM-2.0 text writer, NVRTC, CUDA 12.9 libNVVM, `cuda_sm_7_0` | Pass | Pass | Not applicable | Exact signed-i32 equality compiles through both routes | Parameter widths `[64, 32, 32]`, token-safe 32-bit equality comparison, and global 32-bit store semantics agree | PTX size and timing not measured |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmSlangRealIntegerEqualPtxasAccepts` | 2 | LLVM 14.0.6 provider, audited NVVM-2.0 text writer, NVRTC, CUDA 12.9 libNVVM and matching-root `ptxas`, `sm_75` | Pass | Pass | Not applicable | Both signed-i32-equality PTX outputs assemble | Static PTX acceptance | Resource and timing measurements not collected |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmSlangIntegerEqualRuntimeMatchesNVRTC` | 2 | LLVM 14.0.6 provider, audited NVVM-2.0 text writer, NVRTC, CUDA driver, and GPU with compute capability 7.0+ | Pass | Pass | Not applicable | Both routes launch equal and unequal signed-i32 cases | On the RTX 5090, both routes produce one for equal zero/negative values and zero for unequal signs/extremes | Kernel timing not measured |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmSlangRealIntegerNotEqualDifferentialPTX` | 2 | LLVM 14.0.6 provider, audited NVVM-2.0 text writer, NVRTC, CUDA 12.9 libNVVM, `cuda_sm_7_0` | Pass | Pass | Not applicable | Exact signed-i32 inequality compiles through both routes | Parameter widths `[64, 32, 32]`, token-safe 32-bit equality-predicate comparison, and global 32-bit store semantics agree | PTX size and timing not measured |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmSlangRealIntegerNotEqualPtxasAccepts` | 2 | LLVM 14.0.6 provider, audited NVVM-2.0 text writer, NVRTC, CUDA 12.9 libNVVM and matching-root `ptxas`, `sm_75` | Pass | Pass | Not applicable | Both signed-i32-inequality PTX outputs assemble | Static PTX acceptance | Resource and timing measurements not collected |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmSlangIntegerNotEqualRuntimeMatchesNVRTC` | 2 | LLVM 14.0.6 provider, audited NVVM-2.0 text writer, NVRTC, CUDA driver, and GPU with compute capability 7.0+ | Pass | Pass | Not applicable | Both routes launch equal and unequal signed-i32 cases | On the RTX 5090, both routes produce zero for equal zero/negative values and one for unequal signs/extremes | Kernel timing not measured |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmSlangRealIntegerSignedGreaterThanDifferentialPTX` | 2 | LLVM 14.0.6 provider, audited NVVM-2.0 text writer, NVRTC, CUDA 12.9 libNVVM, `cuda_sm_7_0` | Pass | Pass | Not applicable | Exact signed-i32 greater-than compiles through both routes | Parameter widths `[64, 32, 32]`, token-safe 32-bit signed ordered comparison, and global 32-bit store semantics agree | PTX size and timing not measured |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmSlangRealIntegerSignedGreaterThanPtxasAccepts` | 2 | LLVM 14.0.6 provider, audited NVVM-2.0 text writer, NVRTC, CUDA 12.9 libNVVM and matching-root `ptxas`, `sm_75` | Pass | Pass | Not applicable | Both signed-i32-greater-than PTX outputs assemble | Static PTX acceptance | Resource and timing measurements not collected |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmSlangIntegerSignedGreaterThanRuntimeMatchesNVRTC` | 2 | LLVM 14.0.6 provider, audited NVVM-2.0 text writer, NVRTC, CUDA driver, and GPU with compute capability 7.0+ | Pass | Pass | Not applicable | Both routes launch equal, less-than, greater-than, and signed-extreme cases | On the RTX 5090, both routes produce the expected zero/one truth table including `INT_MAX > INT_MIN` | Kernel timing not measured |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmSlangRealIntegerSignedLessEqualDifferentialPTX` | 2 | LLVM 14.0.6 provider, audited NVVM-2.0 text writer, NVRTC, CUDA 12.9 libNVVM, `cuda_sm_7_0` | Pass | Pass | Not applicable | Exact signed-i32 less-than-or-equal compiles through both routes | Parameter widths `[64, 32, 32]`, token-safe 32-bit signed ordered comparison, and global 32-bit store semantics agree | PTX size and timing not measured |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmSlangRealIntegerSignedLessEqualPtxasAccepts` | 2 | LLVM 14.0.6 provider, audited NVVM-2.0 text writer, NVRTC, CUDA 12.9 libNVVM and matching-root `ptxas`, `sm_75` | Pass | Pass | Not applicable | Both signed-i32-less-equal PTX outputs assemble | Static PTX acceptance | Resource and timing measurements not collected |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmSlangIntegerSignedLessEqualRuntimeMatchesNVRTC` | 2 | LLVM 14.0.6 provider, audited NVVM-2.0 text writer, NVRTC, CUDA driver, and GPU with compute capability 7.0+ | Pass | Pass | Not applicable | Both routes launch equal, less-than, greater-than, and signed-extreme cases | On the RTX 5090, both routes produce the expected zero/one truth table including equality and `INT_MIN <= INT_MAX` | Kernel timing not measured |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmSlangRealIntegerSignedGreaterEqualDifferentialPTX` | 2 | LLVM 14.0.6 provider, audited NVVM-2.0 text writer, NVRTC, CUDA 12.9 libNVVM, `cuda_sm_7_0` | Pass | Pass | Not applicable | Exact signed-i32 greater-than-or-equal compiles through both routes | Parameter widths `[64, 32, 32]`, token-safe 32-bit signed ordered comparison, and global 32-bit store semantics agree | PTX size and timing not measured |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmSlangRealIntegerSignedGreaterEqualPtxasAccepts` | 2 | LLVM 14.0.6 provider, audited NVVM-2.0 text writer, NVRTC, CUDA 12.9 libNVVM and matching-root `ptxas`, `sm_75` | Pass | Pass | Not applicable | Both signed-i32-greater-equal PTX outputs assemble | Static PTX acceptance | Resource and timing measurements not collected |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmSlangIntegerSignedGreaterEqualRuntimeMatchesNVRTC` | 2 | LLVM 14.0.6 provider, audited NVVM-2.0 text writer, NVRTC, CUDA driver, and GPU with compute capability 7.0+ | Pass | Pass | Not applicable | Both routes launch equal, less-than, greater-than, and signed-extreme cases | On the RTX 5090, both routes produce the expected zero/one truth table including equality and `INT_MAX >= INT_MIN` | Kernel timing not measured |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmCompilerCompilesSelfContainedLibdeviceSine` | 5 | Selected CUDA 12.9 toolkit libNVVM and `nvvm/libdevice/libdevice.10.bc`, `compute_75` | Not compared | Pass | Not applicable | Explicit device-library demand resolves one compiler-level `__nv_sinf` call through toolkit-matched libdevice | PTX contains the named entry and global store with no unresolved `.extern .func` | Libdevice is 486,144 bytes, UTC `2025-05-27 09:50:51`, SHA-256 `CD2824F8DD3F862B6B9259086F49F6CB56CA2547E14C61DE889C1C0D4A7DB175` |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmCompilerLibdeviceSinePtxasAccepts` | 5 | Selected CUDA toolkit libNVVM, libdevice, and matching-root `ptxas`, `sm_75` | Not applicable | Pass | Not applicable | Compiler-level libdevice sine PTX assembles without an unresolved libdevice external | Static PTX acceptance through `ptxas` from the same CUDA 12.9 root | Resource and timing measurements not collected |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmCompilerLibdeviceSineRuns` | 5 | Selected CUDA toolkit libNVVM/libdevice, CUDA driver, and GPU with compute capability 7.5+ | Not compared | Pass | Not applicable | The compiler-level libdevice sine kernel launches for zero, finite positive/negative, and range-reduction inputs | On the RTX 5090, inputs `0`, `0.5`, `-1.25`, and `20` match host `sinf` within `2e-6` | Kernel timing not measured |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmSlangRealRelaxedGlobalI32AtomicAddDifferentialPTX` | 4 | LLVM 14.0.6 provider, audited NVVM-2.0 text writer, NVRTC, CUDA 12.9 libNVVM, `cuda_sm_7_0` | Pass | Pass | Not applicable | Exact canonical Relaxed signed-i32 atomic add compiles through both routes | Parameter width `[64]`, token-safe `atom.global.add.u32`, relaxed/device semantics, and the old-value store fixture agree | Synthetic wire benchmark only; kernel timing not measured |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmSlangRealRelaxedGlobalI32AtomicAddPtxasAccepts` | 4 | LLVM 14.0.6 provider, audited NVVM-2.0 text writer, NVRTC, CUDA 12.9 libNVVM and matching-root `ptxas`, `sm_75` | Pass | Pass | Not applicable | Both signed-i32 atomic-add PTX outputs assemble | Static PTX acceptance | Resource and timing measurements not collected |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmSlangRelaxedGlobalI32AtomicAddRuntimeMatchesNVRTC` | 4 | LLVM 14.0.6 provider, audited NVVM-2.0 text writer, NVRTC, CUDA driver, and GPU with compute capability 7.0+ | Pass | Pass | Not applicable | Both routes launch 2,048 threads adding one to one initialized device integer | On the RTX 5090, both final values equal the launch width; the separate old-value fixture preserves its store consumer | Kernel timing not measured |
| `tests/cuda/nvvm-unsupported-ir.slang` | 4 | None beyond `slangc`; `cuda_sm_7_0` | Not applicable | Expected stop | `emit` | E52017: barrier resolves to a void helper outside the signed-i32 helper ABI | Not applicable | — |

| `tools/slang-unit-test/unit-test-nvvm-builder.cpp::nvvmIRBuilderBuildsFloat32AddKernel` | 2 | LLVM 14.0.6 provider and audited NVVM-2.0 text writer | Not applicable | Pass | Not applicable | Provider constructs exact LLVM `float`, AS1 pointer, unflagged `fadd`, and four-byte-aligned store | Verified LLVM and NVVM-2.0 assembly; no runtime | Not measured |
| `tools/slang-unit-test/unit-test-nvvm-emitter.cpp::nvvmSlangFloat32AddUsesDirectPipeline` | 2-3 | In-process fake V3 builder and libNVVM, `cuda_sm_7_0` | Not compared | Pass | Not applicable | Canonical float32 entry values, AS1 float pointer, add, and store lower through one negotiated floating family | Exact `[FloatPointer, Float, Float]` parameters, ordered add operands, type reuse, and result-store consumer checked | Fake-only; no performance measurements |
| `tools/slang-unit-test/unit-test-nvvm-integration.cpp::nvvmSlangRealFloat32AddDifferentialPTX` | 2-3 | LLVM 14.0.6 provider, audited NVVM-2.0 text writer, NVRTC, CUDA 12.9 libNVVM, `cuda_sm_7_0` | Pass | Pass | Not applicable | Exact scalar float32 addition compiles through both routes | Parameter widths `[64, 32, 32]`, token-safe `add.f32`, one global 32-bit store, and no global load agree | PTX size and timing not measured |
| `tools/slang-unit-test/unit-test-nvvm-integration.cpp::nvvmSlangRealFloat32AddPtxasAccepts` | 2-3 | LLVM 14.0.6 provider, audited NVVM-2.0 text writer, NVRTC, CUDA 12.9 libNVVM and matching-root `ptxas`, `sm_75` | Pass | Pass | Not applicable | Both scalar-float32-add PTX outputs assemble | Static PTX acceptance | Resource and timing measurements not collected |
| `tools/slang-unit-test/unit-test-nvvm-integration.cpp::nvvmSlangFloat32AddRuntimeMatchesNVRTC` | 2-3 | LLVM 14.0.6 provider, audited NVVM-2.0 text writer, NVRTC, CUDA driver, and GPU with compute capability 7.0+ | Pass | Pass | Not applicable | Both routes launch exactly representable finite normal addition cases | On the RTX 5090, both routes produce `3.75`, `-7.5`, and `768` | Kernel timing not measured |

| `tools/slang-unit-test/unit-test-nvvm-builder.cpp::nvvmIRBuilderBuildsFloat32CopyKernel` | 2-3 | LLVM 14.0.6 provider and audited NVVM-2.0 text writer | Not applicable | Pass | Not applicable | Existing generic callbacks construct exact aligned LLVM float32 load/store | Verified LLVM and NVVM-2.0 assembly; no runtime | Not measured |
| `tools/slang-unit-test/unit-test-nvvm-emitter.cpp::nvvmSlangFloat32CopyUsesDirectPipeline` | 2-3 | In-process fake V3 builder and libNVVM, `cuda_sm_7_0` | Not compared | Pass | Not applicable | Canonical AS1 float pointer load and store lower without a new provider operation | Two shared FloatPointer parameters, typed generic load result, and exact result-store topology checked | Fake-only; no performance measurements |
| `tools/slang-unit-test/unit-test-nvvm-integration.cpp::nvvmSlangRealFloat32CopyDifferentialPTX` | 2-3 | LLVM 14.0.6 provider, audited NVVM-2.0 text writer, NVRTC, CUDA 12.9 libNVVM, `cuda_sm_7_0` | Pass | Pass | Not applicable | Exact scalar float32 device copy compiles through both routes | Parameter widths `[64, 64]`, one global 32-bit load/store, and no float add agree | PTX size and timing not measured |
| `tools/slang-unit-test/unit-test-nvvm-integration.cpp::nvvmSlangRealFloat32CopyPtxasAccepts` | 2-3 | LLVM 14.0.6 provider, audited NVVM-2.0 text writer, NVRTC, CUDA 12.9 libNVVM and matching-root `ptxas`, `sm_75` | Pass | Pass | Not applicable | Both scalar-float32-copy PTX outputs assemble | Static PTX acceptance | Resource and timing measurements not collected |
| `tools/slang-unit-test/unit-test-nvvm-integration.cpp::nvvmSlangFloat32CopyRuntimeMatchesNVRTC` | 2-3 | LLVM 14.0.6 provider, audited NVVM-2.0 text writer, NVRTC, CUDA driver, and GPU with compute capability 7.0+ | Pass | Pass | Not applicable | Both routes launch exact finite float32 copy cases | On the RTX 5090, both routes copy `3.75`, `-7.5`, `0`, and `1024` | Kernel timing not measured |

| `tools/slang-unit-test/unit-test-nvvm-builder.cpp::nvvmIRBuilderBuildsFloat32SubtractKernel` | 2 | LLVM 14.0.6 provider and audited NVVM-2.0 text writer | Not applicable | Pass | Not applicable | Generic floating callback constructs exact unflagged LLVM `fsub` and aligned store | Verified LLVM/NVVM-2.0 assembly; no runtime | Not measured |
| `tools/slang-unit-test/unit-test-nvvm-emitter.cpp::nvvmSlangFloat32SubtractUsesDirectPipeline` | 2 | In-process fake V3 builder and libNVVM, `cuda_sm_7_0` | Not compared | Pass | Not applicable | Canonical float32 `kIROp_Sub` lowers through floating-binary operation 1 | Exact ordered parameters and subtraction-result store topology checked | Fake-only |
| `tools/slang-unit-test/unit-test-nvvm-integration.cpp::nvvmSlangRealFloat32SubtractDifferentialPTX` | 2 | LLVM 14.0.6 provider, NVRTC, CUDA 12.9 libNVVM | Pass | Pass | Not applicable | Exact scalar float32 subtraction compiles through both routes | `[64, 32, 32]`, `sub.f32`, store, no load/add agree | Not measured |
| `tools/slang-unit-test/unit-test-nvvm-integration.cpp::nvvmSlangRealFloat32SubtractPtxasAccepts` | 2 | Matching-root CUDA 12.9 `ptxas`, `sm_75` | Pass | Pass | Not applicable | Both outputs assemble | Static acceptance | Not measured |
| `tools/slang-unit-test/unit-test-nvvm-integration.cpp::nvvmSlangFloat32SubtractRuntimeMatchesNVRTC` | 2 | CUDA driver/GPU compute 7.0+ | Pass | Pass | Not applicable | Both routes launch exact finite subtraction cases | RTX 5090 results `7.5`, `-8.5`, `1280` | Not measured |

| `tools/slang-unit-test/unit-test-nvvm-builder.cpp::nvvmIRBuilderBuildsFloat32MultiplyKernel` | 2 | LLVM 14.0.6 provider and audited NVVM-2.0 text writer | Not applicable | Pass | Not applicable | Generic floating callback constructs exact unflagged LLVM `fmul` and aligned store | Verified LLVM/NVVM-2.0 assembly with no other floating-binary opcode | Not measured |
| `tools/slang-unit-test/unit-test-nvvm-emitter.cpp::nvvmSlangFloat32MultiplyUsesDirectPipeline` | 2 | In-process fake V3 builder and libNVVM, `cuda_sm_7_0` | Not compared | Pass | Not applicable | Canonical float32 `kIROp_Mul` lowers through floating-binary operation 2 while signed-i32 MUL remains unchanged | Exact ordered parameters and multiplication-result store topology checked | Fake-only |
| `tools/slang-unit-test/unit-test-nvvm-integration.cpp::nvvmSlangRealFloat32MultiplyDifferentialPTX` | 2 | LLVM 14.0.6 provider, NVRTC, CUDA 12.9 libNVVM | Pass | Pass | Not applicable | Exact scalar float32 multiplication compiles through both routes | `[64, 32, 32]`, token-safe `mul.f32`, store, and no load/add/sub agree | Not measured |
| `tools/slang-unit-test/unit-test-nvvm-integration.cpp::nvvmSlangRealFloat32MultiplyPtxasAccepts` | 2 | Matching-root CUDA 12.9 `ptxas`, `sm_75` | Pass | Pass | Not applicable | Both outputs assemble | Static acceptance | Not measured |
| `tools/slang-unit-test/unit-test-nvvm-integration.cpp::nvvmSlangFloat32MultiplyRuntimeMatchesNVRTC` | 2 | CUDA driver/GPU compute 7.0+ | Pass | Pass | Not applicable | Both routes launch exact finite multiplication cases | RTX 5090 results `3`, `-4`, `-256` | Not measured |

| `tools/slang-unit-test/unit-test-nvvm-builder.cpp::nvvmIRBuilderBuildsFloat32DivideKernel` | 2 | LLVM 14.0.6 provider and audited NVVM-2.0 text writer | Not applicable | Pass | Not applicable | Generic floating callback constructs exact unflagged LLVM `fdiv` and aligned store | Verified LLVM/NVVM-2.0 assembly with no other floating-binary opcode | Not measured |
| `tools/slang-unit-test/unit-test-nvvm-emitter.cpp::nvvmSlangFloat32DivideUsesDirectPipeline` | 2 | In-process fake V3 builder and libNVVM, `cuda_sm_7_0` | Not compared | Pass | Not applicable | Canonical float32 `kIROp_Div` lowers through floating-binary operation 3 while integer DIV remains unsupported | Exact ordered parameters and division-result store topology checked | Fake-only |
| `tools/slang-unit-test/unit-test-nvvm-integration.cpp::nvvmSlangRealFloat32DivideDifferentialPTX` | 2 | LLVM 14.0.6 provider, NVRTC, CUDA 12.9 libNVVM | Pass | Pass | Not applicable | Exact scalar float32 division compiles through both routes | `[64, 32, 32]`, token-safe 32-bit division, store, and no load/add/sub/mul agree | Not measured |
| `tools/slang-unit-test/unit-test-nvvm-integration.cpp::nvvmSlangRealFloat32DividePtxasAccepts` | 2 | Matching-root CUDA 12.9 `ptxas`, `sm_75` | Pass | Pass | Not applicable | Both outputs assemble | Static acceptance | Not measured |
| `tools/slang-unit-test/unit-test-nvvm-integration.cpp::nvvmSlangFloat32DivideRuntimeMatchesNVRTC` | 2 | CUDA driver/GPU compute 7.0+ | Pass | Pass | Not applicable | Both routes launch exact finite nonzero-denominator division cases | RTX 5090 results `4`, `-16`, `-4` | Not measured |

| `tools/slang-unit-test/unit-test-nvvm-builder.cpp::nvvmIRBuilderBuildsFloat32NegateKernel` | 2 | LLVM 14.0.6 provider and audited NVVM-2.0 text writer | Not applicable | Pass | Not applicable | Generic floating-unary callback constructs exact unflagged LLVM `fneg`; audited wire text lowers it to legacy `fsub -0.0` | Both forms, aligned store, metadata, and absence of other float arithmetic are checked | Not measured |
| `tools/slang-unit-test/unit-test-nvvm-emitter.cpp::nvvmSlangFloat32NegateUsesDirectPipeline` | 2 | In-process fake V3 builder and libNVVM, `cuda_sm_7_0` | Not compared | Pass | Not applicable | Canonical float32 `kIROp_Neg` lowers through floating-unary operation 0 while signed-i32 NEG remains unchanged | Exact parameter operand and unary-result store topology checked | Fake-only |
| `tools/slang-unit-test/unit-test-nvvm-integration.cpp::nvvmSlangRealFloat32NegateDifferentialPTX` | 2 | LLVM 14.0.6 provider, NVRTC, CUDA 12.9 libNVVM | Pass | Pass | Not applicable | Exact finite scalar float32 negation compiles through both routes | `[64, 32]`, token-safe `neg.f32`, store, and no load/binary-float operation agree | Not measured |
| `tools/slang-unit-test/unit-test-nvvm-integration.cpp::nvvmSlangRealFloat32NegatePtxasAccepts` | 2 | Matching-root CUDA 12.9 `ptxas`, `sm_75` | Pass | Pass | Not applicable | Both outputs assemble | Static acceptance | Not measured |
| `tools/slang-unit-test/unit-test-nvvm-integration.cpp::nvvmSlangFloat32NegateRuntimeMatchesNVRTC` | 2 | CUDA driver/GPU compute 7.0+ | Pass | Pass | Not applicable | Both routes launch exact finite negation cases | RTX 5090 results `-1.5`, `8`, `-1024` | Not measured |

| `tools/slang-unit-test/unit-test-nvvm-builder.cpp::nvvmIRBuilderBuildsFloat32EqualKernel` | 2 | LLVM 14.0.6 provider and audited NVVM-2.0 text writer | Not applicable | Pass | Not applicable | Generic floating-compare callback constructs exact unflagged LLVM `fcmp oeq` whose `i1` result selects zero/one stores | Both text dialects contain one ordered float comparison, two aligned i32 stores, and no fast flag | Not measured |
| `tools/slang-unit-test/unit-test-nvvm-emitter.cpp::nvvmSlangFloat32EqualUsesDirectPipeline` | 2 | In-process fake V3 builder and libNVVM, `cuda_sm_7_0` | Not compared | Pass | Not applicable | Canonical Bool `kIROp_Eql` with Float operands lowers through floating-compare operation 0 while signed-i32 equality remains unchanged | Exact Float parameter operands feed the comparison, whose Boolean result controls the existing constant/phi/i32-store graph | Fake-only |
| `tools/slang-unit-test/unit-test-nvvm-integration.cpp::nvvmSlangRealFloat32EqualDifferentialPTX` | 2 | LLVM 14.0.6 provider, NVRTC, CUDA 12.9 libNVVM | Pass | Pass | Not applicable | Exact scalar float32 ordered equality compiles through both routes | `[64, 32, 32]`, token-safe float32 equality predicate, one global i32 store, no load or float arithmetic, and no integer predicate agree | Not measured |
| `tools/slang-unit-test/unit-test-nvvm-integration.cpp::nvvmSlangRealFloat32EqualPtxasAccepts` | 2 | Matching-root CUDA 12.9 `ptxas`, `sm_75` | Pass | Pass | Not applicable | Both outputs assemble | Static acceptance | Not measured |
| `tools/slang-unit-test/unit-test-nvvm-integration.cpp::nvvmSlangFloat32EqualRuntimeMatchesNVRTC` | 2 | CUDA driver/GPU compute 7.0+ | Pass | Pass | Not applicable | Both routes launch finite, signed-zero, and quiet-NaN ordered-equality cases | RTX 5090 results are `1` for `3.75 == 3.75` and `+0 == -0`, and `0` for `-8 != 0.5` and `NaN == NaN` | Not measured |

| `tools/slang-unit-test/unit-test-nvvm-builder.cpp::nvvmIRBuilderBuildsFloat32NotEqualKernel` | 2 | LLVM 14.0.6 provider and audited NVVM-2.0 text writer | Not applicable | Pass | Not applicable | Existing generic floating-compare callback constructs exact unflagged LLVM `fcmp une` whose `i1` result selects zero/one stores | Both text dialects contain one unordered float comparison, two aligned i32 stores, and no fast flag | Not measured |
| `tools/slang-unit-test/unit-test-nvvm-emitter.cpp::nvvmSlangFloat32NotEqualUsesDirectPipeline` | 2 | In-process fake V3 builder and libNVVM, `cuda_sm_7_0` | Not compared | Pass | Not applicable | Canonical Bool `kIROp_Neq` with Float operands lowers through floating-compare operation 1 while signed-i32 inequality remains unchanged | Exact Float parameter operands feed the comparison, whose Boolean result controls the existing constant/phi/i32-store graph | Fake-only |
| `tools/slang-unit-test/unit-test-nvvm-integration.cpp::nvvmSlangRealFloat32NotEqualDifferentialPTX` | 2 | LLVM 14.0.6 provider, NVRTC, CUDA 12.9 libNVVM | Pass | Pass | Not applicable | Exact scalar float32 unordered inequality compiles through both routes | `[64, 32, 32]`, token-safe float32 comparison predicate, one global i32 store, no load or float arithmetic, and no integer predicate agree | Not measured |
| `tools/slang-unit-test/unit-test-nvvm-integration.cpp::nvvmSlangRealFloat32NotEqualPtxasAccepts` | 2 | Matching-root CUDA 12.9 `ptxas`, `sm_75` | Pass | Pass | Not applicable | Both outputs assemble | Static acceptance | Not measured |
| `tools/slang-unit-test/unit-test-nvvm-integration.cpp::nvvmSlangFloat32NotEqualRuntimeMatchesNVRTC` | 2 | CUDA driver/GPU compute 7.0+ | Pass | Pass | Not applicable | Both routes launch finite, signed-zero, and quiet-NaN unordered-inequality cases | RTX 5090 results are `0` for `3.75 != 3.75` and `+0 != -0`, and `1` for `-8 != 0.5` and `NaN != NaN` | Not measured |

| `tools/slang-unit-test/unit-test-nvvm-builder.cpp::nvvmIRBuilderBuildsFloat32GreaterThanKernel` | 2 | LLVM 14.0.6 provider and audited NVVM-2.0 text writer | Not applicable | Pass | Not applicable | Existing generic floating-compare callback constructs exact unflagged LLVM `fcmp ogt` whose `i1` result selects zero/one stores | Both text dialects contain one ordered greater-than comparison, two aligned i32 stores, and no fast flag | Not measured |
| `tools/slang-unit-test/unit-test-nvvm-emitter.cpp::nvvmSlangFloat32GreaterThanUsesDirectPipeline` | 2 | In-process fake V3 builder and libNVVM, `cuda_sm_7_0` | Not compared | Pass | Not applicable | Canonical Bool `kIROp_Greater` with Float operands lowers through floating-compare operation 2 while signed-i32 greater-than remains unchanged | Exact original-order Float parameters feed the comparison and existing constant/phi/i32-store graph | Fake-only |
| `tools/slang-unit-test/unit-test-nvvm-integration.cpp::nvvmSlangRealFloat32GreaterThanDifferentialPTX` | 2 | LLVM 14.0.6 provider, NVRTC, CUDA 12.9 libNVVM | Pass | Pass | Not applicable | Exact scalar float32 ordered greater-than compiles through both routes | `[64, 32, 32]`, token-safe float32 relation predicate, one global i32 store, no load/float arithmetic/integer predicate agree | Not measured |
| `tools/slang-unit-test/unit-test-nvvm-integration.cpp::nvvmSlangRealFloat32GreaterThanPtxasAccepts` | 2 | Matching-root CUDA 12.9 `ptxas`, `sm_75` | Pass | Pass | Not applicable | Both outputs assemble | Static acceptance | Not measured |
| `tools/slang-unit-test/unit-test-nvvm-integration.cpp::nvvmSlangFloat32GreaterThanRuntimeMatchesNVRTC` | 2 | CUDA driver/GPU compute 7.0+ | Pass | Pass | Not applicable | Both routes launch finite, signed-zero, and quiet-NaN ordered greater-than cases | RTX 5090 results are `1` for `3.75 > 1.5`, and `0` for `-8 > 0.5`, `+0 > -0`, and `NaN > -1` | Not measured |

| `tools/slang-unit-test/unit-test-nvvm-builder.cpp::nvvmIRBuilderBuildsFloat32LessEqualKernel` | 2 | LLVM 14.0.6 provider and audited NVVM-2.0 text writer | Not applicable | Pass | Not applicable | Existing generic floating-compare callback constructs exact unflagged LLVM `fcmp ole` whose `i1` result selects zero/one stores | Both text dialects contain one ordered less-equal comparison, two aligned i32 stores, and no fast flag | Not measured |
| `tools/slang-unit-test/unit-test-nvvm-emitter.cpp::nvvmSlangFloat32LessEqualUsesDirectPipeline` | 2 | In-process fake V3 builder and libNVVM, `cuda_sm_7_0` | Not compared | Pass | Not applicable | Canonical Bool `kIROp_Leq` with Float operands lowers through floating-compare operation 3 while signed-i32 less-equal remains unchanged | Exact original-order Float parameters feed the comparison and existing constant/phi/i32-store graph | Fake-only |
| `tools/slang-unit-test/unit-test-nvvm-integration.cpp::nvvmSlangRealFloat32LessEqualDifferentialPTX` | 2 | LLVM 14.0.6 provider, NVRTC, CUDA 12.9 libNVVM | Pass | Pass | Not applicable | Exact scalar float32 ordered less-than-or-equal compiles through both routes | `[64, 32, 32]`, token-safe float32 relation predicate, one global i32 store, no load/float arithmetic/integer predicate agree | Not measured |
| `tools/slang-unit-test/unit-test-nvvm-integration.cpp::nvvmSlangRealFloat32LessEqualPtxasAccepts` | 2 | Matching-root CUDA 12.9 `ptxas`, `sm_75` | Pass | Pass | Not applicable | Both outputs assemble | Static acceptance | Not measured |
| `tools/slang-unit-test/unit-test-nvvm-integration.cpp::nvvmSlangFloat32LessEqualRuntimeMatchesNVRTC` | 2 | CUDA driver/GPU compute 7.0+ | Pass | Pass | Not applicable | Both routes launch finite, signed-zero, and quiet-NaN ordered less-equal cases | RTX 5090 results are `1` for `1.5 <= 3.75` and `+0 <= -0`, and `0` for `0.5 <= -8` and `NaN <= 1` | Not measured |

| `tools/slang-unit-test/unit-test-nvvm-builder.cpp::nvvmIRBuilderBuildsFloat32GreaterEqualKernel` | 2 | LLVM 14.0.6 provider and audited NVVM-2.0 text writer | Not applicable | Pass | Not applicable | Existing generic floating-compare callback constructs exact unflagged LLVM `fcmp oge` whose `i1` result selects zero/one stores | Both text dialects contain one ordered greater-equal comparison, two aligned i32 stores, and no fast flag | Not measured |
| `tools/slang-unit-test/unit-test-nvvm-emitter.cpp::nvvmSlangFloat32GreaterEqualUsesDirectPipeline` | 2 | In-process fake V3 builder and libNVVM, `cuda_sm_7_0` | Not compared | Pass | Not applicable | Canonical Bool `kIROp_Geq` with Float operands lowers through floating-compare operation 4 while signed-i32 greater-equal remains unchanged | Exact original-order Float parameters feed the comparison and existing constant/phi/i32-store graph | Fake-only |
| `tools/slang-unit-test/unit-test-nvvm-integration.cpp::nvvmSlangRealFloat32GreaterEqualDifferentialPTX` | 2 | LLVM 14.0.6 provider, NVRTC, CUDA 12.9 libNVVM | Pass | Pass | Not applicable | Exact scalar float32 ordered greater-than-or-equal compiles through both routes | `[64, 32, 32]`, token-safe float32 relation predicate, one global i32 store, no load/float arithmetic/integer predicate agree | Not measured |
| `tools/slang-unit-test/unit-test-nvvm-integration.cpp::nvvmSlangRealFloat32GreaterEqualPtxasAccepts` | 2 | Matching-root CUDA 12.9 `ptxas`, `sm_75` | Pass | Pass | Not applicable | Both outputs assemble | Static acceptance | Not measured |
| `tools/slang-unit-test/unit-test-nvvm-integration.cpp::nvvmSlangFloat32GreaterEqualRuntimeMatchesNVRTC` | 2 | CUDA driver/GPU compute 7.0+ | Pass | Pass | Not applicable | Both routes launch finite, signed-zero, and quiet-NaN ordered greater-equal cases | RTX 5090 results are `1` for `3.75 >= 1.5` and `+0 >= -0`, and `0` for `-8 >= 0.5` and `NaN >= -1` | Not measured |

| `tools/slang-unit-test/unit-test-nvvm-builder.cpp::nvvmIRBuilderBuildsFloat32LessThanKernel` | 2 | LLVM 14.0.6 provider and audited NVVM-2.0 text writer | Not applicable | Pass | Not applicable | Existing generic floating-compare callback constructs exact unflagged LLVM `fcmp olt` whose `i1` result selects zero/one stores | Both text dialects contain one ordered less-than comparison, two aligned i32 stores, and no fast flag | Not measured |
| `tools/slang-unit-test/unit-test-nvvm-emitter.cpp::nvvmSlangFloat32LessThanUsesDirectPipeline` | 2 | In-process fake V3 builder and libNVVM, `cuda_sm_7_0` | Not compared | Pass | Not applicable | Canonical Bool `kIROp_Less` with Float operands lowers through floating-compare operation 5 while signed-i32 less-than remains on `SCALAR_CONTROL_FLOW` | Exact original-order Float parameters feed the comparison and existing constant/phi/i32-store graph | Fake-only |
| `tools/slang-unit-test/unit-test-nvvm-integration.cpp::nvvmSlangRealFloat32LessThanDifferentialPTX` | 2 | LLVM 14.0.6 provider, NVRTC, CUDA 12.9 libNVVM | Pass | Pass | Not applicable | Exact scalar float32 ordered less-than compiles through both routes | `[64, 32, 32]`, token-safe float32 relation predicate, one global i32 store, no load/float arithmetic/integer predicate agree | Not measured |
| `tools/slang-unit-test/unit-test-nvvm-integration.cpp::nvvmSlangRealFloat32LessThanPtxasAccepts` | 2 | Matching-root CUDA 12.9 `ptxas`, `sm_75` | Pass | Pass | Not applicable | Both outputs assemble | Static acceptance | Not measured |
| `tools/slang-unit-test/unit-test-nvvm-integration.cpp::nvvmSlangFloat32LessThanRuntimeMatchesNVRTC` | 2 | CUDA driver/GPU compute 7.0+ | Pass | Pass | Not applicable | Both routes launch finite, signed-zero, and quiet-NaN ordered less-than cases | RTX 5090 results are `1` for `1.5 < 3.75`, and `0` for `0.5 < -8`, `+0 < -0`, and `NaN < 1` | Not measured |

| `tools/slang-unit-test/unit-test-nvvm-builder.cpp::nvvmIRBuilderBuildsFloat32ConstantKernel` | 2 | LLVM 14.0.6 provider and audited NVVM-2.0 text writer | Not applicable | Pass | Not applicable | Exact-bit callback constructs scalar float32 `1.5` without decimal transport or synthetic arithmetic | Both text dialects contain one aligned `store float 1.500000e+00` | Not measured |
| `tools/slang-unit-test/unit-test-nvvm-emitter.cpp::nvvmSlangFloat32ConstantUsesDirectPipeline` | 2 | In-process fake V3 builder and libNVVM, `cuda_sm_7_0` | Not compared | Pass | Not applicable | Canonical Float literal rounds once to semantic float32 and requests feature 31 | One width-32 `FloatingPointConstant` node with payload `0x3fc00000` feeds the sole Float-pointer store | Fake-only |
| `tools/slang-unit-test/unit-test-nvvm-integration.cpp::nvvmSlangRealFloat32ConstantDifferentialPTX` | 2 | LLVM 14.0.6 provider, NVRTC, CUDA 12.9 libNVVM | Pass | Pass | Not applicable | Exact scalar float32 constant compiles through both routes | `[64]`, one global 32-bit store, and no load, Float arithmetic, or predicate agree | Not measured |
| `tools/slang-unit-test/unit-test-nvvm-integration.cpp::nvvmSlangRealFloat32ConstantPtxasAccepts` | 2 | Matching-root CUDA 12.9 `ptxas`, `sm_75` | Pass | Pass | Not applicable | Both constant-store outputs assemble | Static acceptance | Not measured |
| `tools/slang-unit-test/unit-test-nvvm-integration.cpp::nvvmSlangFloat32ConstantRuntimeMatchesNVRTC` | 2 | CUDA driver/GPU compute 7.0+ | Pass | Pass | Not applicable | Both routes launch the exact constant store | RTX 5090 writes float32 `1.5` through both routes | Not measured |

| `tools/slang-unit-test/unit-test-nvvm-builder.cpp::nvvmIRBuilderBuildsFloat32PhiKernel` | 2 | LLVM 14.0.6 provider and audited NVVM-2.0 text writer | Not applicable | Pass | Not applicable | Generic typed-phi callbacks construct the canonical Float merge while frozen V2 integer phis remain unchanged | Both text dialects contain exactly one `phi float` with the two actual predecessor values | Not measured |
| `tools/slang-unit-test/unit-test-nvvm-emitter.cpp::nvvmSlangFloat32PhiUsesDirectPipeline` | 2 | In-process fake V3 builder and libNVVM, `cuda_sm_7_0` | Not compared | Pass | Not applicable | Canonical Float block parameter and positional branch arguments request feature 32 and use the generic scalar-phi pair | `[FloatPointer, Integer, Float, Float]`; parameters 2 and 3 feed one typed `ScalarPhi`, then the sole store | Fake-only |
| `tools/slang-unit-test/unit-test-nvvm-integration.cpp::nvvmSlangRealFloat32PhiDifferentialPTX` | 2 | LLVM 14.0.6 provider, NVRTC, CUDA 12.9 libNVVM | Pass | Pass | Not applicable | Exact scalar float32 conditional merge compiles through both routes | `[64, 32, 32, 32]`, one global 32-bit store, and no load, Float arithmetic, or Float predicate agree | Not measured |
| `tools/slang-unit-test/unit-test-nvvm-integration.cpp::nvvmSlangRealFloat32PhiPtxasAccepts` | 2 | Matching-root CUDA 12.9 `ptxas`, `sm_75` | Pass | Pass | Not applicable | Both conditional-merge outputs assemble | Static acceptance | Not measured |
| `tools/slang-unit-test/unit-test-nvvm-integration.cpp::nvvmSlangFloat32PhiRuntimeMatchesNVRTC` | 2 | CUDA driver/GPU compute 7.0+ | Pass | Pass | Not applicable | Both routes launch finite and signed-zero choices through the typed callable runtime harness | RTX 5090 selects the requested finite operand and preserves selected `-0.0` versus `+0.0` bits | Not measured |

| `tools/slang-unit-test/unit-test-nvvm-builder.cpp::nvvmIRBuilderBuildsFloat32FunctionKernel` | 2 | LLVM 14.0.6 provider and audited NVVM-2.0 text writer | Not applicable | Pass | Not applicable | Generic call/valued-return callbacks construct a two-argument Float helper while frozen V2 integer functions remain unchanged | Both text dialects contain one Float helper definition, `call float`, `ret float`, and `fadd float` | Not measured |
| `tools/slang-unit-test/unit-test-nvvm-emitter.cpp::nvvmSlangFloat32FunctionsUseDirectPipeline` | 2 | In-process fake V3 builder and libNVVM, `cuda_sm_7_0` | Not compared | Pass | Not applicable | Canonical helper signature and call arguments request feature 33 and choose the generic scalar-function pair | Void `[FloatPointer, Float, Float]` kernel calls Float `[Float, Float]` helper; addition returns to the sole store | Fake-only |
| `tools/slang-unit-test/unit-test-nvvm-integration.cpp::nvvmSlangRealFloat32FunctionDifferentialPTX` | 2 | LLVM 14.0.6 provider, NVRTC, CUDA 12.9 libNVVM | Pass | Pass | Not applicable | Exact scalar float32 helper call compiles through both routes | `[64, 32, 32]`, Float add, one global 32-bit store, and no load or Float predicate agree | Not measured |
| `tools/slang-unit-test/unit-test-nvvm-integration.cpp::nvvmSlangRealFloat32FunctionPtxasAccepts` | 2 | Matching-root CUDA 12.9 `ptxas`, `sm_75` | Pass | Pass | Not applicable | Both Float-helper outputs assemble | Static acceptance | Not measured |
| `tools/slang-unit-test/unit-test-nvvm-integration.cpp::nvvmSlangFloat32FunctionRuntimeMatchesNVRTC` | 2 | CUDA driver/GPU compute 7.0+ | Pass | Pass | Not applicable | Both routes launch finite and signed-zero additions through the typed callable runtime harness | RTX 5090 results agree for finite values and preserve exact `-0 + -0` and `+0 + -0` bits | Not measured |
| `tools/slang-unit-test/unit-test-nvvm-emitter.cpp::nvvmSlangCUDAExecutionUsesDirectPipeline` | 2-3 | V4 fake provider and fake libNVVM | Not compared | Pass | Not applicable | Four canonical UInt3 execution helpers, twelve scalar extracts, and one void group-sync helper lower through typed V4 operations | Exact helper/call/operation/extract/store topology; no runtime | Fake-only; not measured |
| `tools/slang-unit-test/unit-test-nvvm-integration.cpp::nvvmSlangCUDAExecutionDifferentialPTX` | 2-3 | LLVM 14.0.6 provider, audited NVVM-2.0 text writer, NVRTC, CUDA 12.9 libNVVM | Pass | Pass | Not applicable | Both routes emit all `tid`, `ctaid`, `ntid`, and `nctaid` x/y/z components plus a group barrier | Matching `[64, 64]` ABI and complete special-register/barrier inventory | Not measured |
| `tools/slang-unit-test/unit-test-nvvm-integration.cpp::nvvmSlangCUDAExecutionPtxasAccepts` | 2-3 | LLVM 14.0.6 provider, audited NVVM-2.0 text writer, NVRTC, CUDA 12.9 `ptxas` | Pass | Pass | Not applicable | Both execution-family PTX outputs assemble | Static PTX acceptance | Resource and timing measurements not collected |
| `tools/slang-unit-test/unit-test-nvvm-integration.cpp::nvvmSlangCUDAExecutionRuntimeMatchesNVRTC` | 2-3 | LLVM 14.0.6 provider, audited NVVM-2.0 text writer, NVRTC, CUDA driver, RTX 5090 | Pass | Pass | Not applicable | A 288-invocation multi-block launch records all twelve execution components after group synchronization | Every thread/block coordinate occurs exactly once and all block/grid dimensions agree | Kernel timing not measured |
| `tools/slang-unit-test/unit-test-nvvm-emitter.cpp::nvvmSlangSharedMemoryUsesDirectPipeline` | 3-4 | V4 construction-version-3 fake provider and fake libNVVM | Not compared | Pass | Not applicable | One canonical module-owned `groupshared int[64]` lowers through generic fixed-array, global-storage, GEP, load/store, atomic, and barrier operations | Exact address-space-3 storage and two element-pointer relations; no runtime | Fake-only; not measured |
| `tools/slang-unit-test/unit-test-nvvm-emitter.cpp::nvvmSlangUnsignedSharedArrayIndexUsesDirectPipeline` | 3-4 | Current fake provider and fake libNVVM | Not compared | Pass | Not applicable | Two available UInt32 parameters index one canonical module-owned `groupshared int[4]` through the generic array-element operation | Exact write/read parameter identities feed shared store/load around one barrier | Fake-only; not measured |
| `tools/slang-unit-test/unit-test-nvvm-emitter.cpp::nvvmSlangUnsignedConstantPointerIndicesUseDirectPipeline` | 3 | Current fake provider and fake libNVVM | Not compared | Pass | Not applicable | Exact UInt32 literal indices address ordinary selected-scalar pointers and fixed i32 device arrays | Both source/destination pointer families consume the same exact 32-bit constant; no cast or byte reconstruction | Fake-only; not measured |
| `tools/slang-unit-test/unit-test-nvvm-integration.cpp::nvvmSlangSharedMemoryDifferentialPTX` | 3-4 | LLVM 14.0.6 provider, audited NVVM-2.0 text writer, NVRTC, CUDA 12.9 libNVVM | Pass | Pass | Not applicable | Both routes compile the 64-element shared-memory reverse-read workload | Matching `[64, 64]` ABI, 256-byte shared object, shared load/store, global atomic, and group barrier inventory | PTX size/timing not measured |
| `tools/slang-unit-test/unit-test-nvvm-integration.cpp::nvvmSlangSharedMemoryPtxasAccepts` | 3-4 | Matching-root CUDA 12.9 `ptxas`, `sm_75` | Pass | Pass | Not applicable | Both shared-memory outputs assemble | Static acceptance; direct `sm_70 -v` probe reports 14 registers, one barrier, 256 bytes shared memory, and no stack or spills | Compile time reported as 0.000 ms |
| `tools/slang-unit-test/unit-test-nvvm-integration.cpp::nvvmSlangSharedMemoryRuntimeMatchesNVRTC` | 3-4 | LLVM 14.0.6 provider, audited NVVM-2.0 text writer, NVRTC, CUDA driver, RTX 5090 | Pass | Pass | Not applicable | One 64-thread block writes, synchronizes, and reverse-reads peer slots across two warps | All 64 results equal `(63 - ticket) * 3 + 1` through both routes | Kernel timing not measured |

## Slice 19 atomic and wire-compatibility evidence

The probe measured final linked Slang IR containing exact
`atomicAdd(destination, delta, 0)`, where the destination is the established read-write device
`Ptr<int>`, the value and result are signed `i32`, and zero is Relaxed. CUDA 12.9 NVRTC measured
`atom.global.add.u32` for the corresponding unsuffixed CUDA atomic; omitted PTX semantic and scope
qualifiers encode relaxed/device behavior. The pre-change direct route stopped at E52017
`atomicAdd`. LLVM 14 bitcode then failed at the LLVM 7 reader's atomic record, so the completed
slice negotiates a provider-owned NVVM-2.0 text dialect while keeping old-provider bitcode intact.

| Evidence | Bucket | Contract | Status |
| --- | --- | --- | --- |
| Provider ABI negotiation and invalid-operation matrix | 4 | Append exact `emitRelaxedGlobalI32AtomicAdd(module, pointer, value, outOriginalValue)` and `serializeNVVMIR20AssemblyWithDiagnostics` after the frozen 312-byte prefix; require the complete 328-byte x64 block and reject every partial/null shape | Pass |
| Verified LLVM provider construction | 4 | Accept only an available same-module typed AS1 `i32*` and `i32`, then emit one `atomicrmw add` with `Align(4)`, LLVM `monotonic`, and default System sync scope; return the original value | Pass |
| Audited wire dialect | 4 | Preserve generic LLVM 14 assembly, name parameters at construction, and remove only the semantically validated natural atomic alignment in the dedicated NVVM-2.0 writer; require semantic/rewrite counts to match | Pass |
| Old-provider compatibility | 4 | Keep an exact 312-byte provider usable, serialize its accepted negate program as bitcode, and stop an atomic program at E52016 before module creation | Pass |
| Fake direct pipeline and capability boundary | 4 | Validate exact `atomicAdd(destination, delta, 0)`, map its original-value result to the established store consumer, and use only the dedicated callback | Pass |
| Negative direct boundaries | 4 | Reject other atomic opcodes, orders, value/pointee types, access qualifiers, and address spaces before provider discovery without source-name matching or load/add/store reconstruction | Pass |
| Direct/NVRTC PTX and matching-root `ptxas` | 4 | Require token-safe global 32-bit atomic add with relaxed/device semantics from both routes and static acceptance by the selected CUDA toolkit | Pass |
| Runtime differential | 4 | Prove a 2,048-thread increment reaches the launch count on both routes and separately preserve the returned original-value result | Pass |

## Slice 21 signed-i32 equality evidence

The probe measured final linked Slang IR containing exact `cmpEQ(left, right) : Bool`. The result
feeds the established conditional branch, each arm contributes zero or one, the merge selects the
value through an integer phi, and the existing global store consumes that phi. The pre-change
direct route stopped at E52017 `cmpEQ`, while CUDA 12.9 NVRTC accepted the exact source. The
completed slice appends one equality callback after the frozen Slice 19 prefix and preserves the
same canonical graph through LLVM `icmp eq`.

| Evidence | Bucket | Contract | Status |
| --- | --- | --- | --- |
| Provider ABI negotiation and invalid-operation matrix | 2 | Append exact `emitIntegerEqual(module, left, right, outValue)` after the frozen 328-byte x64 prefix; require the complete 336-byte table and reject every partial/null shape | Pass |
| Verified LLVM provider construction | 2 | Accept only available same-module, same-function, same-type scalar LLVM integers at an unterminated insertion point, then emit one `ICMP_EQ` with an `i1` result | Pass |
| Old-provider compatibility | 2 | Keep an exact Slice 19 provider usable for its atomic/text programs and stop equality at E52016 before module creation | Pass |
| Fake direct pipeline | 2 | Preserve exact left/right parameter identities and route the equality result through conditional branches, zero/one constants, one integer phi, and one store without invoking signed less-than | Pass |
| Negative direct boundaries | 2 | Reject unsigned, wide, floating-point, and pointer equality before provider discovery while retaining the exact signed-i32/Boolean shape | Pass |
| Direct/NVRTC PTX and matching-root `ptxas` | 2 | Require matching `[64, 32, 32]` ABI, token-safe 32-bit equality comparison, global 32-bit store, and static acceptance from both routes | Pass |
| Runtime differential | 2 | Produce one for equal zero/negative pairs and zero for unequal sign/extreme pairs on both routes | Pass |

## Routing and regression evidence

| Test | Contract | Result |
| --- | --- | --- |
| `slang-unit-test-tool/nvvmIRBuilderNegotiatesScalarControlFlowAPI` | The append-only control-flow block leaves an exact scalar-memory provider valid, rejects a partial or incomplete new block, accepts larger tables, and sanitizes failed outputs | Pass |
| `slang-unit-test-tool/nvvmIRBuilderRejectsInvalidScalarControlOperations` | The LLVM 14 provider rejects missing insertion points, type mismatches, cross-module/cross-function handles, non-dominating values, and post-terminator operations before mutation | Pass |
| `slang-unit-test-tool/nvvmIRBuilderNegotiatesScalarSSAAPI` | The append-only scalar-SSA block leaves an exact Slice 7 provider valid, rejects every partial/incomplete new block, accepts larger tables, and sanitizes failed outputs | Pass |
| `slang-unit-test-tool/nvvmIRBuilderRejectsInvalidScalarSSAOperations` | The LLVM 14 provider rejects unrepresentable constants and invalid phi placement/type/module/function/CFG/predecessor/dominance/duplicate inputs before mutation | Pass |
| `slang-unit-test-tool/nvvmIRBuilderNegotiatesScalarFunctionAPI` | The append-only scalar-function block leaves an exact Slice 8 provider valid, rejects partial/incomplete tables, accepts larger tables, and sanitizes failed call outputs | Pass |
| `slang-unit-test-tool/nvvmIRBuilderRejectsInvalidScalarFunctionOperations` | The LLVM 14 provider rejects missing/post-terminator insertion points and invalid call/return callee/arity/type/module/function/dominance shapes before mutation | Pass |
| `slang-unit-test-tool/nvvmIRBuilderNegotiatesScalarPointerArithmeticAPI` | The append-only scalar-pointer-arithmetic field leaves an exact Slice 9 provider valid, rejects partial/incomplete tables, accepts larger tables, and sanitizes failed pointer outputs | Pass |
| `slang-unit-test-tool/nvvmIRBuilderRejectsInvalidPointerOffsetOperations` | The LLVM 14 provider rejects missing/post-terminator insertion points and invalid pointer/offset/module/function/dominance shapes before mutation; public constructors separately reject void pointees and expose no safe unsized pointer handle for exercising the provider guard | Pass |
| `slang-unit-test-tool/nvvmIRBuilderNegotiatesScalarArrayAddressingAPI` | The coherent two-member array prefix leaves an exact Slice 10 provider valid, rejects both partial sizes and either null operation, accepts larger tables, forwards exact type/value identities, and sanitizes failed outputs | Pass |
| `slang-unit-test-tool/nvvmIRBuilderRejectsInvalidArrayAddressingOperations` | The LLVM 14 provider rejects invalid array element/count/context shapes and missing/post-terminator insertion points, scalar bases, cross-module/cross-function values, and non-dominating indices before mutation | Pass |
| `slang-unit-test-tool/nvvmIRBuilderNegotiatesScalarIntegerMultiplyAPI` | The dedicated multiply field leaves an exact 264-byte Slice 11 provider valid, rejects a partial or null suffix, accepts and clamps larger tables, forwards exact operand/result identities, and sanitizes failed outputs | Pass |
| `slang-unit-test-tool/nvvmIRBuilderRejectsInvalidIntegerMultiplyOperations` | The LLVM 14 provider rejects missing/post-terminator insertion points, null outputs, non-integer or mismatched operands, cross-module/cross-function values, and unavailable or non-dominating operands before mutation | Pass |
| `slang-unit-test-tool/nvvmIRBuilderNegotiatesScalarIntegerBitAndAPI` | The dedicated bitwise-AND field leaves an exact 272-byte Slice 12 provider valid, rejects every partial size from 273 through 279 bytes and a null complete operation, accepts and clamps larger tables, forwards exact operand/result identities, and sanitizes failed outputs | Pass |
| `slang-unit-test-tool/nvvmIRBuilderRejectsInvalidIntegerBitAndOperations` | The LLVM 14 provider rejects missing/post-terminator insertion points, null outputs, non-integer or mismatched operands, cross-module/cross-function values, and unavailable or non-dominating operands before mutation | Pass |
| `slang-unit-test-tool/nvvmIRBuilderNegotiatesScalarIntegerBitOrAPI` | The dedicated bitwise-OR field leaves an exact 280-byte Slice 13 provider valid, rejects every partial size from 281 through 287 bytes and a null complete operation, accepts and clamps larger tables, forwards exact operand/result identities, and sanitizes failed outputs | Pass |
| `slang-unit-test-tool/nvvmIRBuilderRejectsInvalidIntegerBitOrOperations` | The LLVM 14 provider rejects missing/post-terminator insertion points, null outputs, non-integer or mismatched operands, cross-module/cross-function values, and unavailable or non-dominating operands before mutation | Pass |
| `slang-unit-test-tool/nvvmIRBuilderNegotiatesScalarIntegerBitXorAPI` | The dedicated bitwise-XOR field leaves an exact 288-byte Slice 14 provider valid, rejects every partial size from 289 through 295 bytes and a null complete operation, accepts and clamps larger tables, forwards exact operand/result identities, and sanitizes failed outputs | Pass |
| `slang-unit-test-tool/nvvmIRBuilderRejectsInvalidIntegerBitXorOperations` | The LLVM 14 provider rejects missing/post-terminator insertion points, null outputs, non-integer or mismatched operands, cross-module/cross-function values, and unavailable or non-dominating operands before mutation | Pass |
| `slang-unit-test-tool/nvvmIRBuilderNegotiatesScalarIntegerBitNotAPI` | The dedicated bitwise-NOT field leaves an exact 296-byte Slice 15 provider valid, rejects every partial size from 297 through 303 bytes and a null complete operation, accepts and clamps larger tables, forwards exact operand/result identities, and sanitizes failed outputs | Pass |
| `slang-unit-test-tool/nvvmIRBuilderRejectsInvalidIntegerBitNotOperations` | The LLVM 14 provider rejects missing/post-terminator insertion points, null outputs, non-integer operands, cross-module/cross-function values, and unavailable or non-dominating operands before mutation while preserving prior binary validation | Pass |
| `slang-unit-test-tool/nvvmIRBuilderNegotiatesScalarIntegerNegateAPI` | The dedicated integer-negate field leaves an exact 304-byte Slice 16 provider valid, rejects every partial size from 305 through 311 bytes and a null complete operation, accepts and clamps larger tables, forwards exact operand/result identities, reports stable capability identity, and sanitizes failed outputs | Pass |
| `slang-unit-test-tool/nvvmIRBuilderRejectsInvalidIntegerNegateOperations` | The LLVM 14 provider rejects missing/post-terminator insertion points, null outputs, non-integer operands, cross-module/cross-function values, and unavailable or non-dominating operands before mutation through the shared unary validator while BitNot predecessor coverage stays green | Pass |
| `slang-unit-test-tool/nvvmIRBuilderNegotiatesRelaxedGlobalI32AtomicAddAPI` | The coherent atomic/text suffix leaves an exact 312-byte Slice 17 provider valid, rejects every partial size from 313 through 327 bytes and either null complete operation, accepts and clamps larger tables, reports both capabilities in identity, forwards exact operands/results, and sanitizes failures | Pass |
| `slang-unit-test-tool/nvvmIRBuilderRejectsInvalidRelaxedGlobalI32AtomicAddOperations` | The LLVM 14 provider rejects missing/post-terminator insertion points, null outputs, non-i32/mismatched values, wrong pointer address spaces and pointees, cross-module/cross-function handles, and unavailable or non-dominating operands before mutation; raw assembly preserves LLVM 14 alignment while the dedicated dialect omits it | Pass |
| `slang-unit-test-tool/nvvmIRBuilderNegotiatesScalarIntegerEqualAPI` | The dedicated equality field leaves an exact 328-byte Slice 19 provider valid, rejects every partial size from 329 through 335 bytes and a null complete operation, accepts and clamps larger tables, reports stable capability identity, forwards exact operands/results, and sanitizes failed outputs | Pass |
| `slang-unit-test-tool/nvvmIRBuilderRejectsInvalidIntegerEqualOperations` | The LLVM 14 provider rejects missing/post-terminator insertion points, null outputs, non-integer or mismatched operands, cross-module/cross-function values, and unavailable or non-dominating operands before mutation through the shared binary comparison validator | Pass |
| `slang-unit-test-tool/nvvmSlangNegotiatesScalarControlFlowCapability` | An exact old scalar-memory V2 provider still compiles copy/load/store, while a conditional program reaches E52016 after discovery but before module creation | Pass |
| `slang-unit-test-tool/nvvmSlangNegotiatesScalarSSACapability` | An exact Slice 7 V2 provider retains its old programs, while constant, merge-phi, and loop programs reach E52016 after discovery but before module creation | Pass |
| `slang-unit-test-tool/nvvmSlangNegotiatesScalarFunctionCapability` | An exact Slice 8 V2 provider retains the finite-loop program, while a helper program reaches E52016 after discovery but before module creation | Pass |
| `slang-unit-test-tool/nvvmSlangNegotiatesScalarPointerArithmeticCapability` | An exact Slice 9 V2 provider retains the helper program, while a pointer-offset program reaches E52016 after discovery but before module creation | Pass |
| `slang-unit-test-tool/nvvmSlangNegotiatesScalarArrayAddressingCapability` | An exact Slice 10 V2 provider retains the pointer-offset program, while a fixed-device-array program reaches E52016 after discovery but before module creation | Pass |
| `slang-unit-test-tool/nvvmSlangNegotiatesScalarIntegerMultiplyCapability` | An exact Slice 11 V2 provider retains fixed-array addressing, while a signed-i32-multiply program reaches E52016 after discovery but before module creation | Pass |
| `slang-unit-test-tool/nvvmSlangNegotiatesScalarIntegerBitAndCapability` | An exact Slice 12 V2 provider retains signed-i32 multiplication, while a signed-i32-bitwise-AND program reaches E52016 after discovery but before module creation | Pass |
| `slang-unit-test-tool/nvvmSlangNegotiatesScalarIntegerBitOrCapability` | An exact Slice 13 V2 provider retains signed-i32 bitwise AND, while a signed-i32-bitwise-OR program reaches E52016 after discovery but before module creation | Pass |
| `slang-unit-test-tool/nvvmSlangNegotiatesScalarIntegerBitXorCapability` | An exact Slice 14 V2 provider retains signed-i32 bitwise OR, while a signed-i32-bitwise-XOR program reaches E52016 after discovery but before module creation | Pass |
| `slang-unit-test-tool/nvvmSlangNegotiatesScalarIntegerBitNotCapability` | An exact Slice 15 V2 provider retains signed-i32 bitwise XOR, while a signed-i32-bitwise-NOT program reaches E52016 after discovery but before module creation | Pass |
| `slang-unit-test-tool/nvvmSlangNegotiatesScalarIntegerNegateCapability` | An exact Slice 16 V2 provider retains signed-i32 bitwise NOT, while a signed-i32-negate program reaches E52016 after one discovery but before module creation | Pass |
| `slang-unit-test-tool/nvvmSlangNegotiatesRelaxedGlobalI32AtomicAddCapability` | An exact 312-byte Slice 17 provider still compiles signed-i32 negate through captured LLVM bitcode, while an atomic-add program reaches E52016 after discovery but before module creation | Pass |
| `slang-unit-test-tool/nvvmSlangNegotiatesScalarIntegerEqualCapability` | An exact 328-byte Slice 19 provider still compiles relaxed global signed-i32 atomic add through its negotiated text writer, while an equality program reaches E52016 after discovery but before module creation | Pass |
| `slang-unit-test-tool/nvvmSlangNegotiatesScalarIntegerNotEqualCapability` | An exact 336-byte Slice 21 provider still compiles signed-i32 equality, while an inequality program reaches E52016 after discovery but before module creation | Pass |
| `slang-unit-test-tool/nvvmSlangNegotiatesScalarIntegerSignedGreaterThanCapability` | An exact 344-byte Slice 22 provider still compiles signed-i32 inequality, while a signed-greater-than program reaches E52016 after discovery but before module creation | Pass |
| `slang-unit-test-tool/nvvmSlangNegotiatesScalarIntegerSignedLessEqualCapability` | An exact 352-byte Slice 23 provider still compiles signed-i32 greater-than, while a signed-less-equal program reaches E52016 after discovery but before module creation | Pass |
| `slang-unit-test-tool/nvvmSlangNegotiatesScalarIntegerSignedGreaterEqualCapability` | An exact 360-byte Slice 24 provider still compiles signed-i32 less-than-or-equal, while a signed-greater-equal program reaches E52016 after discovery but before module creation | Pass |
| `slang-unit-test-tool/nvvmSlangRetainsOnlySelectedCUDAKernel` | CUDA-kernel pruning still removes an unselected kernel before direct emission while preserving the exact selected entry point | Pass |
| `slang-unit-test-tool/nvvmSlangRejectsConventionalRawKernelParameters` | Only `[CUDAKernel]` receives the raw scalar ABI; a conventional parameterized compute entry reaches E52017 before builder or libNVVM program creation | Pass |
| `slang-unit-test-tool/nvvmSlangUnsupportedIRStopsBeforeEmission` | Logical NOT, half/double parameters, libdevice sine, pointer comparisons, void/pointer helpers, unsupported arrays/aggregates/storage, and independent atomic/resource/shared-memory variants retain deterministic E52017 boundaries before builder discovery or libNVVM program creation. Selected integer shifts/division/remainder now use typed operation families and are no longer listed as unsupported | Pass |
| `slang-unit-test-tool/nvvmCompilerNegotiatesCUDADeviceLibraryOption` | The terminal naturally aligned pointer-sized `requiresCUDADeviceLibrary` storage does not reuse prior tail padding; zero means false, nonzero means true, and an older compatible options prefix leaves established libdevice-free compiles independent of a toolkit root | Pass |
| `slang-unit-test-tool/nvvmCompilerUsesSelectedToolkitLibdevice` | Filesystem discovery carries the canonical successful decorated libNVVM candidate into compiler construction, never retries a conflicting environment root, reads exact `nvvm/libdevice/libdevice.10.bc` only on demand, and includes a coherent libdevice timestamp in compiler identity | Pass |
| `slang-unit-test-tool/nvvmCompilerRejectsUnavailableRequestedLibdevice` | Zero demand compiles without requiring, reading, or adding libdevice; requested missing-root or missing-file/read failures stop before program creation | Pass |
| `slang-unit-test-tool/nvvmCompilerHandlesLibdeviceModuleAddition` | Explicit demand reads opaque bytes before program creation, adds user module `slang-nvvm-input` first and `libdevice.10.bc` second, uses ordinary addition only when the lazy API is absent, surfaces a failed lazy add without retry, and destroys each created program exactly once | Pass |
| `slang-unit-test-tool/nvvmCompilerEnforcesFloatingPointPolicy` | Default/Precise/Fast compose with fp32 Any/Preserve/FlushToZero into the exact managed option matrix used identically for verification and compilation; non-Any fp16/fp64 modes, malformed enum values, and compiler-specific overrides fail before program creation | Pass |
| `slang-unit-test-tool/parseCUDAEmissionMethods` | Both selector orders are last-wins, only the canonical option is stored, and target settings are isolated | Pass |
| `slang-unit-test-tool/cudaEmissionMethodSelectsDownstreamCompiler` | Default follows the session transition; explicit NVRTC/NVVM bypass it; invalid input selects no compiler | Pass |
| `slang-unit-test-tool/cudaEmissionMethodLinkOptionsAffectRoutingAndHash` | A canonical `linkWithOptions` override changes both effective dispatch and the shader hash | Pass |
| `slang-unit-test-tool/invalidCUDAEmissionMethodIsDiagnosed` | An unknown integer supplied through the public target-option API reaches E52015 | Pass |
| `slang-unit-test-tool/nvvmSlangBuilderIdentityAffectsHashAndIsSessionCached` | Builder availability changes the direct-NVVM shader hash, and hash/codegen reuse one session load result | Pass |
| `slang-unit-test-tool/nvvmSlangBuilderDiagnosticsStopBeforeLibNVVM` | Invalid builder verification emits E52018, preserves verifier text, destroys the module, and creates no libNVVM program | Pass |
| `slang-unit-test-tool/nvvmSlangUnsupportedIRStopsBeforeEmission` | The canonical barrier helper is accepted, but a conventional compute entry retaining that helper stops at the established `'CUDA kernel decoration'` boundary before builder-module or libNVVM-program creation | Pass |
| `slang-unit-test-tool/nvvmSlangMissingBuilderDoesNotFallback` | An unavailable builder emits E52016 and never falls back to NVRTC | Pass |
| `tests/cuda/sampler-comparison-state-unused.slang` | Established default PTX and explicit NVRTC lanes produce accepted PTX | Pass |
| `tests/cuda/cuda-compile.cu` | Explicit `-pass-through nvrtc` retains precedence even with `-emit-cuda-via-nvvm` | Pass |
| `slang-unit-test-tool/nvvmIRBuilderBuildsAndValidatesCUDAExecutionOperations` | V4 construction version 2 preserves version 1, validates vector construction/extraction and all execution/barrier operations before mutation, and emits exact normal and LLVM-7-compatible intrinsic forms | Pass |
| `slang-unit-test-tool/nvvmSlangNegotiatesCUDAExecutionCapability` | An exact V4 construction-version-1 provider is discovered but reports E52018 for required extended construction before module creation or operation emission | Pass |
| `slang-unit-test-tool/nvvmIRBuilderBuildsAndValidatesSharedGlobalStorage` | V4 construction version 3 preserves versions 1 and 2, rejects invalid type/address-space/alignment/name/output and duplicate-name cases without mutation, then emits exact normal and LLVM-7-compatible address-space-3 storage/GEP/load/store forms | Pass |
| `slang-unit-test-tool/nvvmSlangNegotiatesSharedGlobalStorageCapability` | An exact V4 construction-version-2 provider retains Slice 66 programs but reports E52018 for shared storage before module creation | Pass |
| `slang-unit-test-tool/nvvmIRBuilderBuildsNumericTypeFamilies` | Dimensioned descriptors emit selected scalar/vector integer arithmetic/comparison, Float16/Float32 arithmetic, all six floating comparisons, Boolean equality/inequality, lane-preserving integer/floating conversions, and Float16/Float32 width conversions in normal and compatible assembly; mixed signedness, i24, float64, same-width conversion, Boolean arithmetic/ordering, mismatched lanes, and five-lane vectors remain unsupported | Pass |
| `slang-unit-test-tool/nvvmSlangFloat16ValuesUseGenericTypedPipeline` | The fake boundary records exact Half/Half2 constants, construction/extraction, arithmetic/comparison/conversion descriptors, helper signatures, calls/returns, and phi transport through the generic APIs | Pass |
| `tests/cuda/nvvm-half-values.slang` | Optimized direct CUDA and PTX lanes exercise Half/Half2 arithmetic, comparison, integer and Float32 conversion, helper/phi transport, and dynamic extraction with outputs `-8, -5, 1, -5`; CUDA 12.9 `ptxas` accepts the module | Pass |
| `slang-unit-test-tool/nvvmSlangLocalVectorSwizzlePromotesToGenericValues` | A local Half4 partial assignment reaches the fake boundary only as generic vector extractions/construction; the interface has no local-allocation callback, and a dynamic local-element store remains rejected before builder discovery | Pass |
| `tests/compute/half-vector-calc.slang.3/.4` | Optimized direct runtime/PTX lanes exercise the existing Half2/Half3/Half4 calculation including `.xyz` partial assignment, with outputs `75, 220.5, 565, 1108` | Pass |
| `slang-unit-test-tool/nvvmSlangVectorOperationFamiliesUseTypedDescriptors` | Narrow/32-bit integer, Float32-comparison, and Boolean-operation producers retain exact kind/width/lane descriptors through generic operations, Boolean construction, and scalar extraction | Pass |
| `slang-unit-test-tool/nvvmSlangScalarShiftDivideRemainderUseTypedOperations` | The four former scalar E52017 controls now compile independently through exact signed-i32 left/right shift, division, and remainder descriptors | Pass |
| `slang-unit-test-tool/nvvmIRBuilderBuildsConventionalGlobalParameterStorage` | Exact ABI revision 3 structurally constructs an unpacked resource view and one-field global block, extracts the resource pointer value, applies a typed pointer offset, and emits verified normal and NVVM-2.0-compatible assembly | Pass |
| `slang-unit-test-tool/nvvmIRBuilderBuildsGenericAggregateValues` | Exact ABI revision 9 constructs and extracts fixed arrays through `insertvalue`/`extractvalue` in normal and compatible assembly; null, foreign, wrong-type/count, unavailable, out-of-range, and post-termination shapes fail without mutation | Pass |
| `slang-unit-test-tool/nvvmIRBuilderRejectsInvalidAggregateElementOperations` | Generic aggregate extraction preserves the established resource-view struct path while rejecting null, foreign, non-aggregate, out-of-range, unavailable, and post-termination operands without module mutation | Pass |
| `slang-unit-test-tool/nvvmSlangFloatMatrixValuesUseLegalizedAggregates` | The fake boundary observes Float2 row arrays, ordered aggregate construction, exact array phis/incoming values, constant row extraction, vector column extraction, and the final Float store | Pass |
| `slang-unit-test-tool/nvvmSlangConventionalComputeUsesDirectPipeline` | The fake provider observes a zero-parameter kernel, external `SLANG_globalParams`, UInt3 block/thread arithmetic, structural field/resource loads, pointer extraction/offset, and a structured-buffer store from ordinary source | Pass |
| `slang-unit-test-tool/nvvmSlangMultidimensionalWaveUsesDirectPipeline` | The exact existing shader graph performs five canonical UInt3 component extractions, two float resource pointer extractions/offsets/stores, and the established wave operations without resource-specific builder callbacks | Pass |
| `slang-unit-test-tool/nvvmSlangNegotiatesNumericFamilyCapability` | A static-catalog-only V4 provider is discovered once but reports E52018 for the mixed numeric family before module creation or libNVVM program creation | Pass |
| `slang-unit-test-tool/nvvmSlangMixedNumericDifferentialPTX` | Raw signed/unsigned 8/16/64-bit scalars, float32, eight numeric pointers, explicit conversions, signed/unsigned branches, and signed-i32x2 load/add/store compile through both routes | Pass |
| `slang-unit-test-tool/nvvmSlangMixedNumericPtxasAccepts` | CUDA 12.9 `ptxas` accepts direct NVVM and NVRTC PTX for the representative mixed-width/vector workload | Pass |
| `slang-unit-test-tool/nvvmSlangMixedNumericRuntimeMatchesNVRTC` | One RTX 5090 thread exercises narrow wrapping/bitwise results, i64 arithmetic, signed/unsigned comparisons, float/integer conversions, and both int2 lanes identically through direct NVVM and NVRTC | Pass |
| `slang-unit-test-tool/nvvmSlangRawBufferDataPointersUseGenericPipeline` | Three structured/byte-address views expose field zero, three ordinary scalar pointer offsets feed two loads and one store, and no fixed-array or resource-specific provider operation is used | Pass |
| `slang-unit-test-tool/nvvmSlangReadOnlyByteAddressDataPointerIsInvariant` | A read-only byte-address producer retains immutable-root semantics through its ordinary pointer result, producing one invariant load and one mutable destination store | Pass |
| `slang-unit-test-tool/nvvmSlangRejectsReadOnlyByteAddressDataPointerStoreBeforeProviderMutation` | A store rooted in a read-only byte-address producer reaches E52017 before builder discovery or mutation even though the canonical escaped pointer type is ordinarily read-write-qualified | Pass |
| `slang-unit-test-tool/nvvmIRBuilderBuildsByteOffsetPointerKernel` | Exact ABI revision 8 applies generic typed byte offsets to scalar and vector pointees, preserves address space and alignment, and emits verified normal and NVVM-2.0 assembly | Pass |
| `slang-unit-test-tool/nvvmSlangCoreByteAddressAccessUsesGenericByteOffsets` | Read-only UInt4 and mutable UInt loads plus one UInt store use three generic byte-offset pointers; immutable flags and canonical alignment reach the fake provider exactly | Pass |
| `slang-unit-test-tool/nvvmSlangFloatVectorByteAddressAccessUsesGenericOperations` | Float4 construction/extraction and exact Int/Float/Float4 byte pointers compose through generic vector and memory callbacks with scalar/vector identity preserved | Pass |
| `slang-unit-test-tool/nvvmSlangWideIntegerByteAddressAccessUsesGenericOperations` | Read-only Int64 and mutable UInt64 loads plus matching wide stores use four generic byte-offset pointers, exact explicit-eight/default-four-byte alignment, and the correct invariant policy | Pass |
| `slang-unit-test-tool/nvvmSlangNumericArrayByteAddressAccessUsesGenericOperations` | `Array<Float4, 2>` preserves exact element/count identity through generic array construction, two byte-offset pointers, one invariant load, and one store | Pass |
| `slang-unit-test-tool/nvvmSlangRejectsNestedArrayByteAddressAccessBeforeProviderMutation` | A nested numeric array remains outside the nonrecursive byte payload family and reaches E52017 before builder discovery or mutation | Pass |
| `slang-unit-test-tool/nvvmSlangVectorStructuredBuffersUseGenericTransport` | Exact Int4/Float4 resource-view identity reaches whole-vector loads; one canonical `wzyx` store composes four existing vector extracts, byte offsets `12, 8, 4, 0`, and scalar stores without a new callback | Pass |
| `slang-unit-test-tool/nvvmSlangRejectsDoubleVectorStructuredBufferBeforeProviderMutation` | Double2 remains outside the selected 32-bit vector resource family and reaches E52017 at the entry-parameter boundary before builder discovery or mutation | Pass |
| `slang-unit-test-tool/nvvmSlangVectorFunctionsUseExactGenericTypes` | Exact Int4, Float3, and comparison-produced Bool2 types cross helper parameters/results, calls/returns, and an Int4 block-parameter phi; Double2 and invalid five-lane sources stop before builder discovery | Pass |

Slice 9 extends Bucket 2 through a finite DAG of canonical direct `IRFunc`
callees with signed-i32 parameters/results and valued returns. Complex and aggregate types, pointer
helper ABI, additional address spaces, shared memory, builtins, external/indirect calls, recursion,
and resources remain outside the accepted subset. The barrier row is an expected stop and does not
claim barrier support.

Slice 11 extends Bucket 3 from signed-i32 scalar-pointer offsets to one exact aggregate-addressing
shape: a nonempty fixed `IRArrayType` of signed `i32` behind a read or read-write device entry-point
pointer, consumed by a canonical two-operand `IRGetElementPtr` with a signed-i32 index. The result
preserves the base address space and access while changing from the array pointee's `DefaultLayout`
to the scalar pointee's `ScalarLayout`, and the provider emits ordinary non-`inbounds`
`{i32 0, index}` GEP. Unsized, empty, nested, non-i32, local, global, constant, or shared arrays;
array values; other aggregates; unsigned or wider indices; helper array pointers; and additional
address spaces remain outside the accepted subset. The barrier row remains an expected stop and
does not claim barrier support.

Slice 12 extends Bucket 2 with exact two-operand signed-i32 `kIROp_Mul`. Slang preflight owns the
signed-i32 restriction and canonical operand availability, while the dedicated provider operation
requires same-typed scalar LLVM integers with valid ownership and availability at the current
unterminated insertion point before emitting `mul`. Unsigned, narrow, wide, floating-point, vector,
and matrix multiplication; overflow or fused variants; division, remainder, shifts, bitwise
operations other than the Slice 13 signed-i32 AND, and casts remain outside the accepted subset. The
complete Slice 12 focused NVVM prefix passed 84/84 after its Slice 11 test was updated to
distinguish that slice's frozen prefix from the full 272-byte table.

Slice 13 extends Bucket 2 with exact two-operand signed-i32 `kIROp_BitAnd`. Slang preflight owns the
exact opcode, signed-i32 result, and canonical operand availability, while the dedicated provider
operation requires same-typed scalar LLVM integers with valid ownership and availability at the
current unterminated insertion point before emitting `and`. Slice 14 adds signed-i32 OR; signed-i32
XOR remains the E52017 `'xor'` opcode boundary, while unsigned and i64 AND remain at the raw
entry-point parameter boundary; other bitwise operations and value types remain outside the
accepted subset. The complete focused NVVM prefix passed 92/92 with the full 280-byte table.

Slice 14 extends Bucket 2 with exact two-operand signed-i32 `kIROp_BitOr`. Slang preflight owns the
exact ordinary opcode, signed-i32 result, and canonical operand availability, while the dedicated
provider operation requires same-typed scalar LLVM integers with valid ownership and availability
at the current unterminated insertion point before emitting `or`. Signed-i32 XOR, NOT, shifts,
division, and remainder remain E52017 opcode boundaries; unsigned and i64 OR remain at the raw
entry-point parameter boundary. The complete x64 provider prefix is 288 bytes; the exact 280-byte
Slice 13 prefix stays compatible, sizes 281 through 287 and a null complete operation are rejected,
and future-larger tables are accepted and clamped. Direct NVVM and NVRTC both expose
`[64, 32, 32]`, token-safe `or.b32`, and the expected global i32 store; CUDA 12.9 `ptxas` accepts
both outputs, and the RTX 5090 runtime results are `0x7e`, `-13`, `-1`, and `0x5f5f5f5f`. The
complete focused NVVM prefix passed 100/100 with the full 288-byte table.

Slice 15 extends Bucket 2 with exact two-operand signed-i32 `kIROp_BitXor`. Slang preflight owns
the exact ordinary opcode, signed-i32 result, and canonical operand availability, while the
dedicated provider operation requires same-typed scalar LLVM integers with valid ownership and
availability at the current unterminated insertion point before emitting `xor`. Signed-i32 BitNot,
shifts, division, and remainder remain E52017 opcode boundaries; unsigned and i64 XOR remain at the
raw entry-point parameter boundary. The complete x64 provider prefix is 296 bytes; the exact
288-byte Slice 14 prefix stays compatible, sizes 289 through 295 and a null complete operation are
rejected, and future-larger tables are accepted and clamped. Direct NVVM and NVRTC both expose
`[64, 32, 32]`, token-safe `xor.b32`, and the expected global u32 store; CUDA 12.9 `ptxas` accepts
both outputs, and the RTX 5090 runtime results are `0x66`, `-305419897`, `15`, and `-1`. The first
complete focused NVVM run passed 108/108 with the full 296-byte table; the preservation runs passed
1/1, 2/2, 1/1, 3/3, 2/2, and 1/1.

Slice 16 extends Bucket 2 with exact one-operand signed-i32 `kIROp_BitNot`. Slang preflight owns
the exact ordinary opcode, signed-i32 result, and canonical operand availability, while the
dedicated provider operation requires one scalar LLVM integer with valid ownership and availability
at the current unterminated insertion point before emitting `CreateNot` as an all-ones `xor i32`.
One shared unary integer validator owns that per-value contract; the established binary validator
composes two unary checks with exact type equality and preserves prior behavior. Signed-i32 shifts,
division, and remainder remain E52017 opcode boundaries; logical NOT and unsigned or i64 BitNot
remain at the raw entry-point parameter boundary. The complete
x64 provider prefix is 304 bytes; the exact 296-byte Slice 15 prefix stays compatible, sizes 297
through 303 and a null complete operation are rejected, and future-larger tables are accepted and
clamped. Direct NVVM and NVRTC both expose `[64, 32]`, token-safe `not.b32`, and the expected global
u32 store; NVRTC uses address conversion while direct uses the raw pointer. CUDA 12.9 `ptxas`
accepts both outputs, and the RTX 5090 runtime results are `-1`, `0`, `-1431655766`, and `15`. The
first complete focused NVVM run passed 116/116 with the full 304-byte table. Preservation passed
1/1 parser, 2/2 routing/hash, 1/1 unsupported boundary, 3/3 sampler, 2/2 CUDA
compile/pass-through, and 1/1 runtime dispatch.

Slice 17 extends Bucket 2 with exact one-operand signed-i32 `kIROp_Neg`. Slang preflight owns the
exact ordinary opcode, signed-i32 result, canonical operand availability, and wrapping policy,
including `-INT_MIN == INT_MIN`. The dedicated provider operation reuses the shared unary integer
validator before plain unflagged `CreateNeg`, represented as `sub i32 0, value`. The complete x64
provider prefix is 312 bytes; the exact 304-byte Slice 16 prefix stays compatible, sizes 305 through
311 and a null complete operation are rejected, and future-larger tables are accepted and clamped.
The pre-change direct route stopped at E52017 `'neg'`. Integrated direct NVVM and NVRTC both expose
`[64, 32]`, token-safe `neg.s32`, and a global u32 store; NVRTC uses `cvta.to.global.u64` while
direct uses the raw pointer, and neither uses `sub.s32` or `not.b32`. CUDA 12.9 `ptxas` accepts both
outputs. On the RTX 5090, inputs `0`, `1`, `-7`, and `INT_MIN` produce `0`, `-1`, `7`, and
`-2147483648` on both routes. The first complete focused NVVM run passed 124/124 with the full
312-byte table. Preservation passed 1/1 parser, 2/2 routing/hash, 1/1 unsupported boundary, 3/3
sampler, 2/2 CUDA compile/pass-through, and 1/1 runtime dispatch.

Slice 18 extends Bucket 5 at the downstream compiler boundary without adding a direct-Slang float
or builder capability. The terminal naturally aligned pointer-sized `requiresCUDADeviceLibrary`
field uses zero for false and any nonzero value for true; older compatible option prefixes receive
zero. A true request uses only the root retained from the selected libNVVM library, reads exact
`nvvm/libdevice/libdevice.10.bc` bytes before program creation, adds the user module normally and
libdevice lazily, and uses ordinary addition only when the optional lazy symbol is absent. The
Default/Precise/Fast and fp32 Any/Preserve/FlushToZero matrices compose independently; non-default
fp16/fp64 denormal policy and compiler-specific overrides of the managed families are rejected
before program creation. The selected CUDA 12.9 libdevice is 486,144 bytes, UTC
`2025-05-27 09:50:51`, with SHA-256
`CD2824F8DD3F862B6B9259086F49F6CB56CA2547E14C61DE889C1C0D4A7DB175`. The direct Slang f32
arithmetic negative case stops during raw parameter validation, while the sine case stops at its
float-returning target helper's unsupported result type. Both occur before provider discovery and
neither tests GenericAsm matching. The complete focused Slice 18 NVVM suite passed 132/132. The
real PTX, same-root `ptxas`, and RTX 5090 runtime lanes passed; preservation passed 1/1 parser, 2/2
routing/hash, 1/1 unsupported boundary, 3/3 sampler, 2/2 CUDA compile/pass-through, and 1/1 runtime
dispatch.

Slice 19 extends Bucket 4 with exact canonical Relaxed signed-i32 atomic add through the established
raw read-write device `Ptr<int>`, returning the original value. The terminal provider operation is
deliberately exact: AS1 typed `i32`, four-byte alignment, LLVM `monotonic`, and default System sync
scope, with no configurable policy parameters. It shares one 328-byte x64 capability block with the
audited NVVM-2.0 text serializer; the exact 312-byte Slice 17 provider remains compatible and
receives bitcode. Stable parameter naming and semantic removal of the LLVM-14-only atomic alignment
spelling bridge the proven LLVM 7 text dialect. Direct and NVRTC PTX expose
`atom.global.add.u32`; matching-root `ptxas` and RTX 5090 runtime lanes pass. Waves and all other
atomic operations, orders, value/pointee types, access qualifiers, and address spaces remain
unsupported. This slice adds no new pointer producer, but it does not reject an already-supported
canonical read-write device-i32 pointer merely because that pointer is derived.

The final Release NVVM prefix passed 140/140. Preservation passed 1/1 parser, 2/2 routing/hash,
1/1 unsupported boundary, 3/3 sampler, 2/2 CUDA compile/pass-through, and 1/1 runtime dispatch.

Slice 21 extends Bucket 2 with exact two-operand signed-i32 `kIROp_Eql` producing the canonical
Boolean type. Slang preflight owns the signed-i32 operand restriction, Boolean result, and existing
value availability; the provider reuses the shared binary integer ownership/type/dominance
validation before exact `ICMP_EQ`. The `i1` result may feed established conditional control flow,
but this adds no Boolean entry ABI, storage, return, or phi capability. The complete x64 provider
prefix is 336 bytes; the exact 328-byte Slice 19 prefix stays compatible, sizes 329 through 335 and
a null complete operation are rejected, and future-larger tables are accepted and clamped.
Unsigned, wide, floating-point, and pointer equality remain deterministic E52017 boundaries before
provider discovery.

Direct NVVM and NVRTC expose matching `[64, 32, 32]`, token-safe 32-bit equality, and a global
32-bit store. CUDA 12.9 `ptxas` accepts both outputs, and the RTX 5090 runtime produces the expected
one/zero results for equal and unequal representative values. The final Release NVVM prefix passes
148/148. Preservation passes 1/1 parser, 2/2 routing/hash, 1/1 unsupported boundary, 3/3 sampler,
2/2 CUDA compile/pass-through, and 1/1 runtime dispatch.

Slice 22 extends Bucket 2 with exact two-operand signed-i32 `kIROp_Neq` producing the canonical
Boolean type. Slang preflight owns the signed-i32 operand restriction, Boolean result, and existing
value availability; the provider reuses the shared binary integer ownership/type/dominance
validation before exact `ICMP_NE`. The `i1` result may feed established conditional control flow,
but this adds no Boolean entry ABI, storage, return, or phi capability. The complete x64 provider
prefix is 344 bytes; the exact 336-byte Slice 21 prefix stays compatible, sizes 337 through 343 and
a null complete operation are rejected, and future-larger tables are accepted and clamped.
Unsigned, wide, floating-point, and pointer inequality remain deterministic E52017 boundaries
before provider discovery.

Direct NVVM and NVRTC expose matching `[64, 32, 32]`, token-safe 32-bit equality predicates, and a
global 32-bit store. CUDA 12.9 `ptxas` accepts both outputs, and the RTX 5090 runtime produces the
expected zero/one results for equal and unequal representative values. The final Release NVVM
prefix passes 156/156. Preservation passes 1/1 parser, 2/2 routing/hash, 1/1 unsupported boundary,
3/3 sampler, 2/2 CUDA compile/pass-through, and 1/1 runtime dispatch.

Slice 23 extends Bucket 2 with exact two-operand signed-i32 `kIROp_Greater` producing the canonical
Boolean type. Slang preflight owns the signed-i32 operand restriction, Boolean result, and existing
value availability; the provider reuses the shared binary integer ownership/type/dominance
validation before exact `ICMP_SGT`. The `i1` result may feed established conditional control flow,
but this adds no Boolean entry ABI, storage, return, or phi capability. The complete x64 provider
prefix is 352 bytes; the exact 344-byte Slice 22 prefix stays compatible, sizes 345 through 351 and
a null complete operation are rejected, and future-larger tables are accepted and clamped.
Unsigned, wide, floating-point, and pointer greater-than remain deterministic E52017 boundaries
before provider discovery.

Direct NVVM and NVRTC expose matching `[64, 32, 32]`, token-safe 32-bit signed ordered comparison,
and a global 32-bit store. CUDA 12.9 `ptxas` accepts both outputs, and the RTX 5090 runtime produces
the expected zero/one results for equal, less, greater, and signed-extreme pairs. The final Release
NVVM prefix passes 164/164. Preservation passes 1/1 parser, 2/2 routing/hash, 1/1 unsupported
boundary, 3/3 sampler, 2/2 CUDA compile/pass-through, and 1/1 runtime dispatch.

Slice 24 extends Bucket 2 with exact two-operand signed-i32 `kIROp_Leq` producing the canonical
Boolean type. Slang preflight owns the signed-i32 operand restriction, Boolean result, and existing
value availability; the provider reuses the shared binary integer ownership/type/dominance
validation before exact `ICMP_SLE`. The `i1` result may feed established conditional control flow,
but this adds no Boolean entry ABI, storage, return, or phi capability. The complete x64 provider
prefix is 360 bytes; the exact 352-byte Slice 23 prefix stays compatible, sizes 353 through 359 and
a null complete operation are rejected, and future-larger tables are accepted and clamped.
Unsigned, wide, floating-point, and pointer less-equal remain deterministic E52017 boundaries
before provider discovery.

Direct NVVM and NVRTC expose matching `[64, 32, 32]`, token-safe 32-bit signed ordered comparison,
and a global 32-bit store. CUDA 12.9 `ptxas` accepts both outputs, and the RTX 5090 runtime produces
the expected zero/one results for equal, less, greater, and signed-extreme pairs. The final Release
NVVM prefix passes 172/172. Preservation passes 1/1 parser, 2/2 routing/hash, 1/1 unsupported
boundary, 3/3 sampler, 2/2 CUDA compile/pass-through, and 1/1 runtime dispatch.

Slice 25 completes the ordinary signed-i32 comparison family with exact two-operand `kIROp_Geq`
producing the canonical Boolean type. Slang preflight owns the signed-i32 operand restriction,
Boolean result, and existing value availability; the provider reuses the shared binary integer
ownership/type/dominance validation before exact `ICMP_SGE`. The `i1` result may feed established
conditional control flow, but this adds no Boolean entry ABI, storage, return, or phi capability.
The complete x64 provider prefix is 368 bytes; the exact 360-byte Slice 24 prefix stays compatible,
sizes 361 through 367 and a null complete operation are rejected, and future-larger tables are
accepted and clamped. Unsigned, wide, floating-point, and pointer greater-equal remain deterministic
E52017 boundaries before provider discovery.

Direct NVVM and NVRTC expose matching `[64, 32, 32]`, token-safe 32-bit signed ordered comparison,
and a global 32-bit store. CUDA 12.9 `ptxas` accepts both outputs, and the RTX 5090 runtime produces
the expected zero/one results for equal, less, greater, and signed-extreme pairs. The final Release
NVVM prefix passes 180/180. Preservation passes 1/1 parser, 2/2 routing/hash, 1/1 unsupported
boundary, 3/3 sampler, 2/2 CUDA compile/pass-through, and 1/1 runtime dispatch.

Slice 26 begins Bucket 6 with the exact raw CUDA `RWStructuredBuffer<int, DefaultLayout>` launch
value and canonical `kIROp_RWStructuredBufferGetElementPtr` addressing producer. Slang preflight
owns the exact resource type, signed-i32 index and result-pointee shape, availability, and producer
relationship. The provider owns one `{ i32 addrspace(1)*, i64 }` ABI type and validates exact type,
ownership, availability, dominance, and insertion state before extracting the data-pointer field
and emitting one non-`inbounds` GEP. The complete x64 provider prefix is 384 bytes; the exact
368-byte Slice 25 prefix stays compatible for all established programs, sizes 369 through 383 and
a complete table missing either new callback are rejected, and future-larger tables are accepted
and clamped. The identity records `raw-rw-structured-buffer-i32`.

Conventional global parameter blocks, read-only structured buffers, unsigned/floating-point
resource elements, raw read-write resource loads/atomics, and neighboring resource operations
remain deterministic E52017 boundaries before provider discovery. Direct NVVM and NVRTC expose
the same `.align 8 .b8[16]` raw resource
parameter followed by i32 index, first-u64 data-pointer load, signed index scaling, and global u32
store. CUDA 12.9 `ptxas` accepts both with four registers and no stack, spills, or barriers. On the
RTX 5090, both routes store 42 through the exact one-element `{device pointer, count}` launch
value. The final Release NVVM prefix passes 188/188. Preservation passes 1/1 parser, 2/2
routing/hash, 1/1 unsupported boundary, 3/3 sampler, 2/2 CUDA compile/pass-through, and 1/1 runtime
dispatch.

Slice 27 changes test ownership without changing capability evidence. The former 25,941-line NVVM
unit-test source is separated into provider-builder (66 registered tests), direct-emitter (44),
real integration/runtime (52), and downstream compiler/loader (26) owners plus a support header
that registers no tests. The exact sorted 188-name set has the same pre/post SHA-256,
`c197159202001f39765394b2399146398d0c4534803864b3ea44cc694827ac78`, so every row above retains
its established selector and semantic claim. The final Release prefix passes 188/188 and Debug
preservation passes 1/1 parser, 2/2 routing/hash, 1/1 unsupported boundary, 3/3 sampler, 2/2 CUDA
compile/pass-through, and 1/1 runtime dispatch. No production source or provider export changes.

Slice 28 changes provider-boundary mechanics without widening any capability bucket. The 384-byte
x64/212-byte x86 V2 table is frozen and embedded as V3's compatibility core. V3 adds four 64-bit
semantic feature words plus one generic unary, binary, and comparison callback, producing a
448-byte x64 table and a 272-byte x86 table (268-byte terminal callback minimum). The 20 established
feature bits replace the linear highest-slice requirement, while exact V2 prefixes synthesize the
same semantics for old-provider fallback. Present malformed V3 tables fail; only an absent V3
symbol permits V2 discovery.

All established scalar IR now crosses the generic family facade, but the accepted Slang IR, LLVM
construction helpers, libNVVM text, PTX classifications, and runtime expectations in every ledger
row remain unchanged. Negotiation tests prove an independent feature hole, unknown-required-bit
rejection, future-size clamping, malformed-present failure, all three unknown-op no-mutation paths,
and exact V2 absence fallback. The final Release prefix passes 192/192, including real differential
PTX, every `ptxas` lane, and the RTX 5090 runtime matrix. Debug preservation passes 1/1 parser, 2/2
routing/hash, 1/1 unsupported boundary, 3/3 sampler, 2/2 CUDA compile/pass-through, and 1/1 runtime
dispatch. The provider exports only V1/V2/V3 getters and has no process-visible LLVM dependency.

Slice 29 changes type-lowering ownership without widening any capability bucket. One module-local
`NVVMTypeLoweringContext` now maps exact canonical linked-IR types for entry/helper signatures,
constants, phis, device pointers, fixed arrays, and the raw resource ABI. Exact `IRType*` entries
reuse their provider handle. A separate representation key of canonical pointee plus LLVM address
space lets read and read-write Slang pointers share one LLVM type while their qualifiers continue to
govern preflight legality. No custom structural equivalence, alternate type spelling, or parallel
semantic type tree is introduced.

`nvvmSlangScalarFunctionsUseDirectPipeline` proves one `i32` construction across three function
signatures, four parameters, a constant, operations, calls, and returns. Existing scalar-copy and
fixed-array tests prove one AS1 pointer construction across distinct read/read-write qualifiers and
one exact array construction/count. `nvvmSlangTypeCacheIsModuleLocal` compiles the same kernel into
two provider modules and observes one void/i32/pointer construction per module, while the adjacent
unsigned, wide, floating, aggregate, resource, layout, and conventional-global matrix retains
E52017 before provider discovery. All semantic, PTX, `ptxas`, and runtime expectations in the rows
above remain unchanged.

The final Release NVVM prefix passes 193/193, including every real differential PTX, `ptxas`, and
RTX 5090 runtime lane. Debug preservation passes 1/1 parser, 2/2 routing/hash, 1/1 unsupported
boundary, 3/3 sampler, 2/2 CUDA compile/pass-through, and 1/1 runtime dispatch. The provider ABI and
export surface are unchanged.

Slice 30 changes only how the established scalar evidence is represented. The eleven repeated
unary, binary, and comparison cases share descriptors and layer-specific runners while retaining
all 88 operation/layer wrapper names referenced above. The exact sorted 193-name set keeps SHA-256
`1f35f717b93e1cb62c3f872e99b819386ab9c5474b203256e58ee1bdb41c97b7`, so no row, selector,
bucket, or semantic claim changes. Release remains 193/193, including real differential PTX,
`ptxas`, and RTX 5090 runtime evidence; Debug preservation remains 10/10.

Slice 31 opens the exact scalar float32 portion of Buckets 2 and 3. The final linked source graph
has three raw entry parameters—AS1 `Ptr<float>`, `float`, `float`—followed by one floating add and
one aligned store. One V3 feature requires the appended type and floating-binary callbacks. The x64
table grows from the exact 448-byte core to 464 bytes; x86 retains the old 268-byte core minimum,
uses 276 bytes as the complete appended minimum, and pads `sizeof` to 280. Exact old cores remain
compatible without the bit; all partial advertised sizes and either null callback are rejected.

The real provider admits only bit width 32 and validates LLVM-float operand type, ownership,
function, availability, dominance, and insertion state before one unflagged `fadd`. Direct topology
proves one cached float type, one cached AS1 float pointer, ordered parameter-1/parameter-2 operands,
and the add-result store consumer. Float loads, half, double, and the newly reachable float
multiply/negate/comparison operations retain deterministic pre-provider boundaries.

Direct NVVM and NVRTC agree on widths `[64, 32, 32]`, `add.f32`, a global 32-bit store, and no
global load. Matching-root CUDA 12.9 `ptxas` accepts both outputs. On the RTX 5090, both routes
produce `3.75`, `-7.5`, and `768` for the exact finite cases. Release passes 201/201; the sorted
LF-terminated name set has SHA-256
`73434ac732eccaf42c9fad54ad2956b13aa5e2371e9e2e72d5fbbc2aaaf6e2e2`. Debug preservation passes
10/10.

Slice 32 admits exact scalar float32 device-pointer loads without changing the provider boundary.
The final linked graph has read-write and read AS1 `Ptr<float>` parameters, one canonical float
load, and one established aligned float store. Preflight requires the existing float32 and scalar
memory features and uses the shared pointer/pointee/availability validator. Emission continues
through the original generic load/store callbacks, so the x64/x86 V3 sizes remain 464/280 bytes,
V1/V2 are unchanged, and no provider export or text rewrite is added.

The fake now indexes generic load handles and records `Integer` or `Float` from their typed pointer
producer. The direct topology proves one float type, one shared AS1 pointer type for both access
qualifiers, source parameter 1 feeding the float load, and load result 0 feeding the store through
destination parameter 0. Established integer, pointer-offset, array, and resource load identities
remain integer, while non-direct float memory shapes remain deterministic boundaries.

Verified LLVM/NVVM text contains exactly one aligned `load float` and `store float` and no `fadd`.
Direct NVVM and NVRTC agree on widths `[64, 64]`, one global 32-bit load/store, and no float add;
CUDA 12.9 `ptxas` accepts both. On the RTX 5090, both routes copy `3.75`, `-7.5`, `0`, and `1024`.
Release passes 207/207; the sorted LF-terminated name set has SHA-256
`5e9c007c59d45c4db5bf9724e6b76c039455d342330f06b8aa68cd2e5eb2316b`. Debug preservation passes
10/10.

Slice 33 adds floating-binary SUBTRACT and semantic feature 21 without growing the 464-byte x64 or
280-byte x86 V3 table. Exact older feature sets remain valid; advertising subtraction requires the
existing complete float prefix. Generic facade, provider, and fake dispatch select operation 1,
while canonical result type keeps integer and float `kIROp_Sub` paths distinct.

Direct topology proves ordered parameter 1/2 operands and the subtraction-result store consumer.
LLVM/NVVM text has one unflagged `fsub float`, aligned store, no `fadd`, and kernel metadata. NVVM
and NVRTC agree on `[64, 32, 32]`, `sub.f32`, store/no-load/add; CUDA 12.9 `ptxas` accepts both; and
RTX 5090 results are `7.5`, `-8.5`, `1280`. Release passes 214/214 with sorted-name SHA-256
`6ba1df40ff963723a866c61cbf8518aba7596e23213d5743015397547c90af9d`; Debug preservation passes
10/10.

Slice 34 adds floating-binary MULTIPLY and semantic feature 22 without growing the 464-byte x64 or
280-byte x86 V3 table. The emitter's closed ADD/SUBTRACT/MULTIPLY mapping supplies the feature,
wire operation, and diagnostic to both preflight and emission; canonical result type keeps float
and signed-i32 `kIROp_Mul` paths distinct. The provider emits unflagged `CreateFMul` through the
existing generic callback.

The same-shape floating-binary tests now share one descriptor and layer-specific runners while
retaining every ADD/SUBTRACT registered name. The five measured test/support files shrink from
19,608 to 19,512 physical lines even after seven MULTIPLY names are added. Direct topology proves
ordered parameter 1/2 operands and the result-store consumer. LLVM/NVVM text has exactly one
`fmul float`; NVVM and NVRTC agree on `[64, 32, 32]`, token-safe `mul.f32`, store/no-load/add/sub;
CUDA 12.9 `ptxas` accepts both; and RTX 5090 results are `3`, `-4`, `-256`. The old negative
float-multiply-plus-cast fixture now reaches its next honest E52017 boundary, `castFloatToInt`.

Release passes 221/221 with sorted-name SHA-256
`c24e6b4e82e289c2533444b0b0c0dab6cc44064a1df02d75a79928de94c2afa8`; Debug preservation passes
10/10.

Slice 35 adds floating-binary DIVIDE and semantic feature 23 through the unchanged 464-byte x64/
280-byte x86 V3 table. The closed emitter mapping, generic facade/provider/fake switches, and
floating test descriptor each gain one enum row; no callback or copied runner is added. Canonical
Float `kIROp_Div` lowers to unflagged `CreateFDiv`, while integer DIV retains E52017 `div`.

Direct topology proves ordered parameter 1/2 operands and the division-result store consumer.
LLVM/NVVM text has exactly one `fdiv float`; NVVM and NVRTC agree on `[64, 32, 32]`, token-safe
32-bit division, store/no-load/add/sub/mul; CUDA 12.9 `ptxas` accepts both; and RTX 5090 results are
`4`, `-16`, `-4`. Seven names add only 48 lines to the five measured test/support files, from
19,512 to 19,560. Release passes 228/228 with sorted-name SHA-256
`99dec82e0909050b0dc909113dad988369dfe9b2666e5385faaec947c6c29bc7`; Debug preservation passes
10/10.

Slice 36 adds floating-unary NEGATE as feature 24/operation 0 and appends one V3 callback. The x64
table grows from 464 to 472 bytes; the x86 callback occupies existing tail padding, leaving its
size at 280 bytes. Exact Slice 35 providers remain valid without the new feature. Canonical Float
`kIROp_Neg` uses the new family, signed-i32 NEG remains on its old path, and the former
float-negate-plus-cast negative reaches E52017 `castFloatToInt`.

The provider module has exactly one unflagged `fneg float`. CUDA 12.9 libNVVM rejects that LLVM-14
opcode in NVVM-2.0 text, so the audited writer validates and count-matches semantic fneg
instructions before emitting the exact legacy `fsub float -0.0, value` spelling. NVVM and NVRTC
then agree on `[64, 32]`, token-safe `neg.f32`, store/no-load/binary-float; `ptxas` accepts both;
and RTX 5090 results are `-1.5`, `8`, `-1024`.

One five-row float-arithmetic descriptor now drives binary and unary kernel, topology, PTX,
assembler, and runtime runners. The five measured test/support files grow from 19,560 to 19,841
physical lines for the new ABI suffix, writer audit, fake family, and seven names. Release passes
235/235 with sorted-name SHA-256
`2b79918702a9b21110af8251944e4428001a4ea69a2ff79b7a18e488cd13b4ba`; Debug preservation passes
10/10.

Slice 37 adds ordered float32 equality as feature 25/floating-compare operation 0 and appends one
generic callback. The V3 table grows from 472 to 480 bytes on x64 and from 280 to 288 bytes on x86;
exact Slice 36 providers remain valid without the bit. Canonical Bool `kIROp_Eql` is classified by
its Float operands and becomes unflagged LLVM `fcmp oeq`, while signed-i32 equality and adjacent
float/pointer comparisons retain their established behavior.

The first `NVVMFloat32ComparisonTestCase` row shares a Boolean-to-i32 provider consumer and the
generic float `ptxas` runner while keeping float runtime arguments separate from integer comparison
descriptors. The five measured test/support files grow from 19,841 to 20,503 physical lines; later
floating predicates reuse this family. NVVM and NVRTC agree on `[64, 32, 32]`, a token-safe
float32 equality predicate, one global i32 store, and no load/float arithmetic/integer predicate;
`ptxas` accepts both. RTX 5090 returns true for equal finite values and opposite signed zeros, and
false for unequal finite values and quiet NaNs. Focused tests pass 12/12 and Release passes 242/242
with sorted-name SHA-256
`7bdb7df316f95767ad79c76e2f802dc08504dfd06fbdfd5208a9c0eafd4ca670`; Debug preservation passes
10/10.

Slice 38 adds unordered float32 inequality as feature 26/floating-compare operation 1 through the
unchanged 480-byte x64/288-byte x86 V3 table. A Slice 37 provider remains valid without the bit.
Canonical Bool `kIROp_Neq` is classified by its Float operands and becomes unflagged LLVM
`fcmp une`; signed-i32 inequality and adjacent float/pointer comparisons remain unchanged.

The second `NVVMFloat32ComparisonTestCase` row drives descriptor-based provider, direct, PTX,
assembler, and runtime helpers. Seven names add 185 measured test/support lines, from 20,503 to
20,688, versus Slice 37's 662-line family scaffolding. NVVM and NVRTC agree on `[64, 32, 32]`, a
token-safe float32 comparison predicate, one global i32 store, and no load/float arithmetic/integer
predicate; `ptxas` accepts both. RTX 5090 returns false for equal finite values and opposite signed
zeros, and true for unequal finite values and quiet NaNs. Focused tests pass 13/13 and Release passes
249/249 with sorted-name SHA-256
`529af4d3eba39ba0aabd6ca881ca3ac66b5f30c5f272c75a54a3b5cdc15156ea`; Debug preservation passes
10/10.

Slice 39 adds ordered float32 greater-than as feature 27/floating-compare operation 2 through the
unchanged V3 table. Canonical Bool `kIROp_Greater` is classified by its Float operands and becomes
unflagged LLVM `fcmp ogt` with original operand order; signed-i32 greater-than and adjacent
float/pointer relations remain unchanged.

The third comparison descriptor row and descriptor-driven feature negotiation add seven registered
names with only 60 measured test/support lines, from 20,688 to 20,748. NVVM and NVRTC agree on
`[64, 32, 32]`, token-safe float32 relation evidence, store/no-load/arithmetic/integer-predicate;
`ptxas` accepts both. RTX 5090 returns true for `3.75 > 1.5` and false for unequal descending,
signed-zero-equal, and quiet-NaN cases. Focused tests pass 14/14 and Release passes 256/256 with
sorted-name SHA-256
`f8b9a58433982e2583a7310c3e2bc43c82767adee115d121a13147783a8a6fcf`; Debug preservation passes
10/10.

Slice 40 adds ordered float32 less-than-or-equal as feature 28/floating-compare operation 3 through
the unchanged V3 table. Canonical Bool `kIROp_Leq` is classified by its Float operands and becomes
unflagged LLVM `fcmp ole` with original operand order; signed-i32 less-equal and adjacent
float/pointer relations remain unchanged.

The fourth comparison descriptor row adds seven registered names with only 41 measured
test/support lines, from 20,748 to 20,789. NVVM and NVRTC agree on `[64, 32, 32]`, token-safe
float32 relation evidence, store/no-load/arithmetic/integer-predicate; `ptxas` accepts both. RTX
5090 returns true for `1.5 <= 3.75` and signed-zero equality, and false for `0.5 <= -8` and quiet
NaN. Focused tests pass 14/14 and Release passes 263/263 with sorted-name SHA-256
`f93467f3b27def96040db05fca0fec79c5e22a5010ae6a3226fab4d249d860a1`; Debug preservation passes
10/10.

Slice 41 adds ordered float32 greater-than-or-equal as feature 29/floating-compare operation 4
through the unchanged V3 table. Canonical Bool `kIROp_Geq` is classified by its Float operands and
becomes unflagged LLVM `fcmp oge` with original operand order; signed-i32 greater-equal and
adjacent float/pointer relations remain unchanged.

The fifth comparison descriptor row adds seven registered names with only 52 measured test/support
lines, from 20,789 to 20,841. NVVM and NVRTC agree on `[64, 32, 32]`, token-safe float32 relation
evidence, store/no-load/arithmetic/integer-predicate; `ptxas` accepts both. RTX 5090 returns true
for `3.75 >= 1.5` and signed-zero equality, and false for `-8 >= 0.5` and quiet NaN. Focused tests
pass 14/14 and Release passes 270/270 with sorted-name SHA-256
`5358536da56531d08b93bd3e2f55d25d3d8cc42a21e461b3a905b1425a1f1fc4`; Debug preservation passes
10/10.

Slice 42 adds ordered float32 less-than as feature 30/floating-compare operation 5 through the
unchanged V3 table. Canonical Bool `kIROp_Less` is classified by its Float operands and becomes
unflagged LLVM `fcmp olt` with original operand order; signed-i32 less-than retains its original
`SCALAR_CONTROL_FLOW` feature and adjacent float/pointer relations remain unchanged.

The sixth comparison descriptor row adds seven registered names with only 50 measured test/support
lines, from 20,841 to 20,891. NVVM and NVRTC agree on `[64, 32, 32]`, token-safe float32 relation
evidence, store/no-load/arithmetic/integer-predicate; `ptxas` accepts both. RTX 5090 returns true
for `1.5 < 3.75`, and false for `0.5 < -8`, signed-zero equality, and quiet NaN. Focused tests pass
14/14 and Release passes 277/277 with sorted-name SHA-256
`a34a5cdb1532603a18290777a75fe23ea9407f5d294e1d9a1a739ea6b9187ae6`; Debug preservation passes
10/10.

Slice 43 adds canonical scalar float32 literals as feature 31 through an appended exact-bit
constant callback. The direct emitter rounds Slang's double-backed Float literal storage once to
semantic float32 and transports payload `0x3fc00000`; V2 signed-i32 constants remain unchanged.
The x64 V3 table grows from 480 to 488 bytes, while the x86 callback consumes tail padding and the
complete table remains 288 bytes. Exact Slice 42 tables remain loadable without feature 31.

The new value-family infrastructure plus seven evidence names adds 554 measured test/support lines,
from 20,891 to 21,445. LLVM and negotiated NVVM text each contain one exact constant store. NVVM
and NVRTC agree on `[64]`, store/no-load/arithmetic/predicate; `ptxas` accepts both, and the RTX 5090
writes float32 `1.5` through both routes. Focused tests pass 14/14 and Release passes 284/284 with
sorted-name SHA-256 `3e78b6b3069dd0a12cbde4d78e4d804e5eeace161cdbf86d620262b5e9d9a72d`;
Debug preservation passes 10/10.

Slice 44 adds canonical Float block-parameter SSA merging as feature 32 through an appended generic
typed-phi callback pair. Slang's existing Float block parameter and positional branch arguments
remain the source of truth; direct emission chooses the generic pair from that semantic type while
frozen V2 signed-i32 phis retain their original callbacks. Shared provider validation preserves
type/module/function ownership, insertion order, complete CFG edges, uniqueness, and dominance.
The x64 V3 table grows from 488 to 504 bytes and the x86 table from 288 to 296 bytes; exact Slice 43
tables remain loadable without feature 32.

The generic phi/fake/runtime-family base plus seven evidence names adds 709 measured test/support
lines, from 21,445 to 22,154. LLVM and negotiated NVVM text each contain one `phi float`. NVVM and
NVRTC agree on `[64, 32, 32, 32]`, store/no-load/Float-arithmetic/Float-predicate; `ptxas` accepts
both, and the RTX 5090 selects finite values and preserves selected signed-zero bits through both
routes. Focused tests pass 14/14 and Release passes 291/291 with sorted-name SHA-256
`c18462cd303630788566c59409f369ef57a46614652571a97663acf0ffb01690`; Debug preservation passes
10/10.

Slice 45 adds canonical Float helper signatures, calls, results, and returns as feature 33 through
an appended generic call/valued-return pair. The complete semantic helper signature chooses generic
V3 when any result or parameter is Float; all-i32 helpers retain frozen V2. Shared provider
validation preserves same-module ownership, non-variadic exact scalar signatures, insertion-point
availability, return type, and dominance. The x64 V3 table grows from 504 to 520 bytes and x86 from
296 to 304 bytes; exact Slice 44 tables remain loadable without feature 33.

The generic call/fake/result/return base plus seven evidence names adds 683 measured test/support
lines, from 22,154 to 22,837. LLVM and negotiated NVVM text each contain one Float helper,
`call float`, `ret float`, and `fadd float`. NVVM and NVRTC agree on `[64, 32, 32]`, Float add,
store/no-load/predicate; `ptxas` accepts both, and the RTX 5090 agrees for finite and signed-zero
results. Focused tests pass 14/14 and Release passes 298/298 with sorted-name SHA-256
`71658634899192b09f2d12461c25a5efb9d85c3c4f2db7c285ba35ef35d44066`; Debug preservation passes
10/10.

Slice 46 adds canonical UInt transport plus exact `WaveGetLaneIndex()` as feature 34 through one
appended generic intrinsic callback and operation 0. CUDA target selection produces a retained
zero-parameter UInt helper ending in `GenericAsm("_getLaneId()")`; direct preflight recognizes only
that exact semantic shape. Signed and unsigned canonical 32-bit values share provider `i32` for
sign-independent parameter/helper/pointer/call/return/offset/store roles, while UInt constants and
signedness-sensitive operations remain unsupported. The x64 V3 table grows from 520 to 528 bytes
and x86 from 304 to 308 bytes; exact Slice 45 and older feature/table prefixes remain loadable.

LLVM 14 emits `llvm.nvvm.read.ptx.sreg.laneid` with six optimization attributes. The audited NVVM
2.0 text writer verifies that exact declaration and attribute set, then retains only LLVM-7-era
`nounwind readnone`; semantic and rewritten counts agree. Both serialized dialects contain one
lane intrinsic call, one UInt helper call/return, and one store. NVVM and NVRTC agree on `[64]`, a
global 32-bit store, and no load; direct NVVM uses `%laneid`, while NVRTC flattens the thread ID and
masks with 31. CUDA 12.9 `ptxas` accepts both, and one 32-thread RTX 5090 warp writes 0 through 31
through each route.

Seven evidence names add 531 measured test/support lines, from 22,837 to 23,368. The focused
Slice 45/46 matrix passes 14/14 and the Release NVVM prefix passes 305/305 with sorted-name SHA-256
`a5d99d25f4218d69bf938e171083e49c3826150873a58506c42e2b8bcbf98dbb`; removing the seven
Slice 46 names reproduces Slice 45's count and hash exactly. Debug preservation passes 10/10.

Slice 47 adds exact `WaveGetLaneCount()` as feature 35/intrinsic operation 1 through the unchanged
generic callback. CUDA target selection produces a zero-parameter UInt helper ending in
`GenericAsm("(warpSize)")`; one descriptor maps exact assembly text to operation/feature for both
lane index and lane count. The composed kernel stores lane count at the lane-indexed UInt pointer.
V3 remains 528 bytes on x64 and 308 bytes on x86, and exact Slice 46 tables remain loadable without
feature 35.

The provider maps operation 1 to `llvm.nvvm.read.ptx.sreg.warpsize`. LLVM shares one six-attribute
group between the lane-id and warp-size declarations; the audited writer validates both semantic
declarations, counts their unique attribute set, and emits one LLVM-7-compatible
`nounwind readnone` group. Both dialects contain one call to each intrinsic, two UInt helper
calls/returns, one pointer offset, and one store. NVVM and NVRTC agree on `[64]`, one global 32-bit
store, and no load; direct PTX uses `%laneid` and `WARP_SZ`. CUDA 12.9 `ptxas` accepts both, and one
32-thread RTX 5090 warp writes 32 in every lane through each route.

Seven evidence names add 364 measured test/support lines, from 23,368 to 23,732. The focused
Slice 46/47 matrix passes 14/14 and the Release NVVM prefix passes 312/312 with sorted-name SHA-256
`dbd8d587f633ab06ac2daaf086690a14fa3b9f4cab8c22332d0a75e562d65ab7`; removing the seven
Slice 47 names reproduces Slice 46's count and hash exactly. Debug preservation passes 10/10.

Slice 48 adds canonical UInt `WaveMaskReadLaneAt` as feature 36/intrinsic operation 2 through the
unchanged generic callback. CUDA target selection produces an exact one-block
`Func(UInt, UInt, UInt, Int)` helper ending in `GenericAsm("__shfl_sync($0, $1, $2)")`. A structural
descriptor admits only that result/parameter shape, and direct emission passes the helper's three
existing parameter values to the provider rather than parsing placeholders. V3 remains 528 bytes
on x64 and 308 bytes on x86, and exact Slice 47 tables remain loadable without feature 36.

The provider validates three available i32 arguments and emits
`llvm.nvvm.shfl.sync.idx.i32(mask, value, lane, 31)`. Its exact LLVM 14 declaration has
`convergent inaccessiblememonly nounwind` attributes, which the audited NVVM 2.0 writer validates
and preserves unchanged. Both dialects contain one lane-id and one shuffle intrinsic call, two
UInt helper calls/returns, one pointer offset, and one store. NVVM and NVRTC agree on
`[64, 32, 32]`, one global 32-bit store, no load, and `shfl.sync.idx.b32`; `ptxas` accepts both.
One 32-thread RTX 5090 warp selects source lanes 0 and 7 correctly through both routes.

Seven evidence names add 479 measured test/support lines, from 23,732 to 24,211. The focused
Slice 47/48 matrix passes 14/14 and Release passes 319/319 with sorted-name SHA-256
`6c97ed4746f5a67237d642f180e69984ec4bdc0f5ae23e5eecb540bd7d51d83c`; removing the seven
Slice 48 names reproduces Slice 47's count and hash exactly. Debug preservation passes 10/10.

Slice 49 adds canonical Int `WaveMaskReadLaneAt` as feature 37/intrinsic operation 3. Its exact
`Func(Int, UInt, Int, Int)` helper has the same
`GenericAsm("__shfl_sync($0, $1, $2)")` text as the Slice 48 UInt helper, so descriptor lookup now
matches both exact text and the complete canonical function signature. It examines every
same-text row until one signature matches; declaration order cannot choose the semantic.

The facade keeps signed and unsigned support independently negotiable, while the provider maps
both operations to the same signless `llvm.nvvm.shfl.sync.idx.i32(mask, value, lane, 31)` after
validation. V3 remains 528 bytes on x64 and 308 bytes on x86, and exact Slice 48 tables remain
loadable without feature 37. The signed kernel contains two pointer offsets, one source load, the
lane-id and shuffle calls, and one destination store. NVVM and NVRTC agree on
`[64, 64, 32, 32]`, one global 32-bit load and store, and `shfl.sync.idx.b32`; `ptxas` accepts
both. One 32-thread RTX 5090 warp selects source lanes 0 and 7 from a varying signed buffer through
both routes.

Seven evidence names add 476 measured test/support lines, from 24,211 to 24,687. The focused
Slice 48/49 matrix passes 14/14 and Release passes 326/326 with sorted-name SHA-256
`64c930268a8edb87cf2cfba3d12991e4ac66c2a4c9336399d1f03e54f5eda8f0`; removing the seven
Slice 49 names reproduces Slice 48's count and hash exactly. Debug preservation passes 10/10.

Slice 50 adds canonical Float `WaveMaskReadLaneAt` as feature 38/intrinsic operation 4. Its exact
`Func(Float, UInt, Float, Int)` helper is the third row with
`GenericAsm("__shfl_sync($0, $1, $2)")`; complete canonical signature matching selects it without
making descriptor order semantic. V3 remains 528/308 bytes, and exact Slice 49 tables remain
loadable without feature 38.

The provider validates the exact mixed `(i32, float, i32)` argument vector and emits native
`llvm.nvvm.shfl.sync.idx.f32(mask, value, lane, 31)`. The legacy writer validates its exact
LLVM-7-compatible result, arguments, and convergent/inaccessible-memory/nounwind attributes. The
end-to-end kernel also generalizes pointer-offset preflight from the old integer-only classifier to
the centralized established scalar-pointer classifier. The fake propagates that same pointee kind
through derived pointers for typed loads and stores.

Int and Float share one loaded-scalar fixture and 32-bit one-warp launcher. NVVM and NVRTC agree on
`[64, 64, 32, 32]`, a global 32-bit load/store, and `shfl.sync.idx.b32`; `ptxas` accepts both. One
RTX 5090 warp selects Float source lanes 0 and 7 bit-exactly through both routes.

Seven evidence names add 433 measured test/support lines, from 24,687 to 25,120. The focused
Slice 48/49/50 matrix passes 21/21 and Release passes 333/333 with sorted-name SHA-256
`57f52bd80e15eefb8a35bc51821d99a4b70c858f111535fde1fea3f90b2bb367`; removing the seven
Slice 50 names reproduces Slice 49's count and hash exactly. Debug preservation passes 10/10.

Slice 51 adds synchronized wave-mask ballot as feature 39/intrinsic operation 5 through the
unchanged generic callback. Existing CUDA active-mask synthesis turns `WaveGetActiveMask()` into
canonical `waveMaskBallot(0xffffffff, true)` and threads the result through a helper. The pass had
appended the helper's mask parameter and every call argument without repairing the function value's
declared type; calling established `fixUpFuncType()` after each transformation now keeps the
definition and calls canonical for all downstream consumers. V3 remains 528/308 bytes, and exact
Slice 50 tables remain loadable without feature 39.

The provider validates `(i32, i1)` and emits
`llvm.nvvm.vote.ballot.sync(mask, predicate)`. Its exact LLVM 7/14 declaration is
`i32(i32, i1)` with convergent/inaccessible-memory/nounwind attributes, which the legacy writer
validates and preserves. Generic constant construction adds only canonical i1 true acceptance;
the emitter admits UInt literals specifically as wave masks, so the previous general unsigned
pointer-offset boundary remains unsupported. The fake records each integer constant's bit width
and enforces exact i32/i1 ballot operands.

NVVM and NVRTC agree on `[64]`, one global 32-bit store, no load, and
`vote.sync.ballot.b32`; CUDA 12.9 `ptxas` accepts both. One full RTX 5090 warp stores
`0xffffffff` in every lane through both routes.

Seven evidence names add 392 measured test/support lines, from 25,120 to 25,512. The complete
Slice 46-51 wave matrix passes 42/42 and Release passes 340/340 with sorted-name SHA-256
`7abb718be35a0e9ad61202e3c8776c718f22c43a790c22df960140164cf5ce2b`; removing the seven
Slice 51 names reproduces Slice 50's count and hash exactly. Debug preservation passes 10/10.

Slice 52 proves the public unmasked UInt `WaveReadLaneAt(value, lane)` wrapper without adding a
provider operation. CUDA target selection composes `WaveGetActiveMask()` with
`WaveMaskReadLaneAt(mask, value, lane)`. Active-mask synthesis produces one ballot in the entry and
threads its result into the public `Func(UInt, UInt, Int, UInt)` helper, whose ordinary calls reuse
the established active-mask identity and UInt masked-shuffle paths. V3 remains 528/308 bytes and
the production/provider diff is empty.

The fake records five functions, three intrinsic emissions, and four calls, including exact ballot
result flow from the entry through the synthesized public-helper argument and active-mask identity
to the masked shuffle. Removing lane-index feature 35, UInt shuffle feature 36, or ballot feature
39 independently produces E52016 before provider module construction.

NVVM and NVRTC agree on `[64, 32]`, one global 32-bit store, no load, and exactly one
`vote.sync.ballot.b32` plus one `shfl.sync.idx.b32` in the entry. CUDA 12.9 `ptxas` accepts both,
and one RTX 5090 warp selects lanes 0 and 7 correctly through both routes.

Five evidence names add 241 measured test/support lines, from 25,512 to 25,753. The complete
Slice 46-52 wave matrix passes 47/47 and Release passes 345/345 with sorted-name SHA-256
`d112ef187a1ff7999b55ed3222b51f0c5ad01416f04a63b46a70a9d25ccb1029`; removing the five
Slice 52 names reproduces Slice 51's count and hash exactly. Debug preservation passes 10/10.

Slice 53 proves public unmasked Int `WaveReadLaneAt()` through the same source-owned composition.
The entry adds one signed device load and calls the public helper with value, lane, and synthesized
mask; the helper routes that mask through the active-mask identity and into the established signed
masked shuffle. Features 35, 37, and 39 are each required before provider module construction. V3
remains 528/308 bytes and no production/provider file changes.

Composition evidence is parameterized at its second scalar row. Shared runners preserve exact
five-function/three-intrinsic/four-call topology, constituent capability failures, and the common
ballot-plus-shuffle PTX mechanism while registered UInt and Int wrappers retain layer-local names.
The measured marginal growth is 121 lines, half Slice 52's 241-line first-row cost.

NVVM and NVRTC agree on `[64, 64, 32]`, one global 32-bit load/store pair, and exactly one ballot
plus one shuffle in the entry. CUDA 12.9 `ptxas` accepts both; one RTX 5090 warp selects negative
lane-0 and lane-7 Int values bit-exactly through both routes.

Five names raise the measured files from 25,753 to 25,874 lines. The complete Slice 46-53 wave
matrix passes 52/52 and Release passes 350/350 with sorted-name SHA-256
`003afec34f28ad32e84961b91f1c87fff1fa006f1da535cb10ab00d29cc727c7`; removing the five Slice 53
names reproduces Slice 52's count and hash exactly. Debug preservation passes 10/10.

Slice 54 adds public unmasked Float `WaveReadLaneAt()` as a thin third composition row. The entry
loads one Float value and passes it with lane and synthesized mask to the public helper; that helper
routes the mask through the active-mask identity and into the established Float masked shuffle.
Features 35, 38, and 39 are independently required before module construction. V3 remains 528/308
bytes and no production/provider file changes.

The Slice 53 direct/capability/PTX runners need no new field or branch. The Float row supplies
operation 4, feature 38, loaded value origin, two pointer offsets, one load, `[64, 64, 32]`, and
expected global load. Exact five-function/three-intrinsic/four-call mask flow remains common to all
three scalar rows.

NVVM and NVRTC each emit exactly one ballot and one shuffle in the entry plus one
`ld.global.f32`/`st.global.f32` pair. CUDA 12.9 `ptxas` accepts both, and one RTX 5090 warp selects
source lanes 0 and 7 bit-exactly through both routes.

Five names add 83 measured lines, from 25,874 to 25,957. The complete Slice 46-54 wave matrix
passes 57/57 and Release passes 355/355 with sorted-name SHA-256
`492d3838278e789e6b0ebabc8798653ba6fdccefc90dbe946b46f8d224453f9e`; removing the five Slice 54
names reproduces Slice 53's count and hash exactly. Debug preservation passes 10/10.

Slice 55 adds public UInt `WaveReadLaneFirst()` as feature 40/intrinsic operation 6. CUDA source
selection composes the public wrapper from `WaveGetActiveMask()` and the exact
`WaveMaskReadLaneFirst(mask, value)` helper, whose canonical UInt terminator is
`GenericAsm("_waveReadFirst($0, $1)")`. Complete assembly text and function-signature matching
select the new semantic, while active-mask synthesis remains the only producer of its mask. V3
remains 528/308 bytes, and exact Slice 54 providers remain loadable with feature 40 clear.

The provider validates `(i32 mask, i32 value)` before mutation, emits
`llvm.cttz.i32(mask, true)` to derive the first participating lane, and feeds that result to the
established `llvm.nvvm.shfl.sync.idx.i32(mask, value, lane, 31)`. LLVM 14 gives `cttz` six newer
optimization attributes and an `immarg` marker that LLVM 7 rejects. The legacy writer validates
the exact declaration, parameter attributes, and types, then removes only the newer attributes and
marker. The nonzero-mask contract follows synchronized-shuffle participation and matches NVRTC's
branch-free CUDA-prelude implementation.

NVVM and NVRTC agree on `[64]`, one global 32-bit store, no load, and exactly one ballot plus one
shuffle in the entry. NVVM lowers `cttz` to a `popc` sequence; NVRTC uses `brev` plus
`bfind.shiftamt`. CUDA 12.9 `ptxas` accepts both, and one full RTX 5090 warp reads lane zero's UInt
value in every lane through both routes.

Seven names add 347 measured lines, from 25,957 to 26,304. The complete Slice 46-55 wave matrix
passes 64/64 and Release passes 362/362 with sorted-name SHA-256
`652de9ad6905f2e885264851e4245cdc88e9119414a920111ee081b557ff786f`; removing the seven Slice 55
names reproduces Slice 54's count and hash exactly. Debug preservation passes 10/10.

Slice 56 adds public Int `WaveReadLaneFirst()` as feature 41/intrinsic operation 7. The exact
signed helper has `Func(Int, UInt, Int)` shape and shares `_waveReadFirst($0, $1)` with UInt, so
complete signature matching preserves the source semantic before both payloads become signless
i32. V3 remains 528/308 bytes, and exact Slice 55 providers remain loadable with feature 41 clear.

The provider validates the same two i32 operands before mutation and shares Slice 55's
`llvm.cttz.i32(mask, true)` plus `llvm.nvvm.shfl.sync.idx.i32` implementation. It adds no LLVM
declaration, compatibility rewrite, callback, or table field. The signed public entry instead adds
one ordinary source-pointer load to the established ballot/public-helper/active-mask/helper graph.

NVVM and NVRTC agree on `[64, 64]`, one global 32-bit load/store pair, and exactly one ballot plus
one shuffle in the entry. NVVM uses `popc`; NVRTC uses `brev` plus `bfind.shiftamt`. CUDA 12.9
`ptxas` accepts both, and one full RTX 5090 warp reads lane zero's value `-40` bit-exactly through
both routes.

Seven names add 178 measured lines, from 26,304 to 26,482. The complete Slice 46-56 wave matrix
passes 71/71 and Release passes 369/369 with sorted-name SHA-256
`b8e9cc1b10ae6094dd3771696bc8ffa9f8c9a4fde60837c7b259c904097a8366`; removing the seven Slice 56
names reproduces Slice 55's count and hash exactly. Debug preservation passes 10/10.

Slice 57 adds public Float `WaveReadLaneFirst()` as feature 42/intrinsic operation 8. Exact
`Func(Float, UInt, Float)` signature matching distinguishes the third helper sharing
`_waveReadFirst($0, $1)`. V3 remains 528/308 bytes, and exact Slice 56 providers remain loadable
with feature 42 clear.

The provider validates `(i32 mask, float value)` before mutation, shares
`llvm.cttz.i32(mask, true)`, and selects the already-established
`llvm.nvvm.shfl.sync.idx.f32` with lane and clamp 31. Both declarations already have exact LLVM
14-to-7 validation and normalization, so this row adds no compatibility rewrite or callback field.

NVVM and NVRTC agree on `[64, 64]`, one global 32-bit Float load/store pair, and one ballot plus
one shuffle in the entry. NVVM uses `popc`; NVRTC uses `brev` plus `bfind.shiftamt`; CUDA 12.9
`ptxas` accepts both, and every RTX 5090 lane reads lane zero's `-11.5f` bits through both routes.

Seven names add 151 measured lines, from 26,482 to 26,633. The complete Slice 46-57 wave matrix
passes 78/78 and Release passes 376/376 with sorted-name SHA-256
`e345e4b4ef33f3a7fe6426c95d461fd46cfb6de8e183be59c2db77ecfa78b4e9`; removing the seven Slice 57
names reproduces Slice 56's count and hash exactly. Debug preservation passes 10/10.

Slice 58 adds public `WaveIsFirstLane()` through feature 43/intrinsic operation 9 at the canonical
masked-helper boundary. Exact assembly plus `Func(Bool, UInt)` matching distinguishes
`WaveMaskIsFirstLane(mask)`. Direct helper signatures now preserve Bool specifically as a result;
Bool parameters and phis remain unsupported. V3 stays 528/308 bytes, and exact Slice 57 providers
remain loadable with feature 43 clear.

The provider validates one i32 mask before mutation and emits the source predicate
`(mask & -mask) == (1 << laneId)` with ordinary LLVM integer instructions plus the established
lane-id intrinsic. It returns native i1 through the generic function path and adds no callback,
text marker, declaration normalization, or compatibility rewrite.

NVVM and NVRTC agree on a `[64]` entry, two synchronized ballots, `neg`, `and`, `shl`, equality,
and one global 32-bit store. CUDA 12.9 `ptxas` accepts both, and one RTX 5090 warp stores one in
lane zero and zero in lanes 1-31 through both routes.

Seven names add 349 measured lines, from 26,633 to 26,982. The complete Slice 46-58 wave matrix
passes 85/85 and Release passes 383/383 with sorted-name SHA-256
`ddb9139c2d89bafd5be199f9d299f3c85b6ca8cca82146b9466ddbaf7fb84335`; removing the seven Slice 58
names reproduces Slice 57's count and hash exactly. Debug preservation passes 10/10.

Slice 59 adds public `WaveActiveAnyTrue(condition)` through feature 44/intrinsic operation 10 at the
canonical masked-helper boundary. Exact assembly plus `Func(Bool, UInt, Bool)` matching
distinguishes `WaveMaskAnyTrue(mask, condition)`. Direct helper signatures and calls now preserve
Bool parameters as native i1; Bool entry-point parameters and block phis remain unsupported. V3
stays 528/308 bytes, and exact Slice 58 providers remain loadable with feature 44 clear.

The provider validates i32 mask plus i1 condition before mutation and calls the exact
`llvm.nvvm.vote.any.sync(i32, i1) -> i1` intrinsic. LLVM 7 and the provider LLVM use compatible
declarations and semantic attributes, so the legacy writer audits the declaration but adds no
callback, text marker, normalization, or compatibility rewrite.

NVVM and NVRTC agree on `[64, 64]`, one global 32-bit load/store pair, two synchronized ballots,
one synchronized any-vote, and signed inequality. CUDA 12.9 `ptxas` accepts both. With only lane
seven's input nonzero, every lane of one RTX 5090 warp stores one through both routes.

Seven names add 368 measured lines, from 26,982 to 27,350. The complete Slice 46-59 wave matrix
passes 92/92 and Release passes 390/390 with sorted-name SHA-256
`eaa8420ddbba56d34cb047211d872acd6ad2dc0dcdd0209059e307e9879e3186`; removing the seven Slice 59
names reproduces Slice 58's 383-name hash
`ddb9139c2d89bafd5be199f9d299f3c85b6ca8cca82146b9466ddbaf7fb84335` exactly. Debug preservation
passes 10/10.

Slice 60 adds public `WaveActiveAllTrue(condition)` through feature 45/intrinsic operation 11 at the
canonical masked-helper boundary. Exact assembly plus `Func(Bool, UInt, Bool)` matching
distinguishes `WaveMaskAllTrue(mask, condition)` from any-true. V3 stays 528/308 bytes, and exact
Slice 59 providers remain loadable with feature 45 clear.

The provider validates i32 mask plus i1 condition before mutation and calls the exact
`llvm.nvvm.vote.all.sync(i32, i1) -> i1` intrinsic. LLVM 7 and the provider LLVM use compatible
declarations and semantic attributes. One synchronized-vote audit now covers ballot, any, and all;
this row adds no callback, text marker, normalization, or compatibility rewrite.

Any/all provider fixtures and negotiation/assembly/direct/PTX evidence share setup by canonical
graph shape while retaining separate feature, operation, name, declaration, source, and mnemonic
assertions. NVVM and NVRTC agree on `[64, 64]`, one global 32-bit load/store pair, two synchronized
ballots, one synchronized all-vote, and signed inequality. CUDA 12.9 `ptxas` accepts both. With only
lane seven's input zero, every lane of one RTX 5090 warp stores zero through both routes.

Seven names add 150 measured lines, from 27,350 to 27,500. The complete Slice 46-60 wave matrix
passes 99/99 and Release passes 397/397 with sorted-name SHA-256
`d5daef5d6db4caa82e5dd8039a8b0f5e095d13cdb819a81f7ea69a30ab873b0d`; removing the seven Slice 60
names reproduces Slice 59's 390-name hash
`eaa8420ddbba56d34cb047211d872acd6ad2dc0dcdd0209059e307e9879e3186` exactly. Debug preservation
passes 10/10.

Slice 61 adds public signed-i32 `WaveActiveAllEqual(value)` through feature 46/intrinsic operation
12 at the canonical masked-helper boundary. Exact assembly plus `Func(Bool, UInt, Int)` matching
distinguishes `_waveAllEqual(mask, value)` without helper names. V3 stays 528/308 bytes, and exact
Slice 60 providers remain loadable with feature 46 clear. UInt, Float, wider, vector, and matrix
overloads remain unsupported.

The provider validates two i32 operands before mutation, calls
`llvm.nvvm.match.all.sync.i32p(i32, i32) -> {i32, i1}`, and returns only extracted element 1 as the
semantic Bool. The native match mask remains provider-private. LLVM 7 and the provider LLVM use
compatible declarations and semantic attributes, so the legacy writer audits the exact aggregate
signature without rewriting text or changing the provider ABI.

Predicate-intrinsic scaffolding shares only the genuine Boolean-result/two-operand graph while each
operation retains type, feature, operation, declaration, PTX, and runtime assertions. NVVM and
NVRTC agree on `[64, 64]`, one global 32-bit load/store pair, two synchronized ballots, and one
`match.all.sync.b32`. CUDA 12.9 `ptxas` accepts both. On an RTX 5090, distinct signed lane values
produce zero in every lane and uniform `-17` produces one through both routes.

Seven names add 169 measured lines, from 27,500 to 27,669. The complete Slice 46-61 wave/ABI
matrix passes 106/106 and Release passes 404/404 with sorted-name SHA-256
`40f3eba7cfb2602716a16b54d942cf09e34e9f2171835889a1dea43cb1e10d0a`; removing the seven Slice 61
names reproduces Slice 60's 397-name hash
`d5daef5d6db4caa82e5dd8039a8b0f5e095d13cdb819a81f7ea69a30ab873b0d` exactly. Debug preservation
passes 10/10.

Slice 62 adds public unsigned-i32 `WaveActiveAllEqual(value)` through feature 47/intrinsic
operation 13 at the canonical masked-helper boundary. Exact assembly plus
`Func(Bool, UInt, UInt)` matching distinguishes `_waveAllEqual(mask, value)` from the signed row.
V3 stays 528/308 bytes, and exact Slice 61 providers remain loadable with feature 47 clear.

The source entry's canonical `Ptr<uint, Read, Device>` load exposed the only inconsistent scalar
type gate: pointer, value, parameter, and provider roles already accept UInt as signless LLVM i32,
but load preflight accepted only signed i32. The load now uses the common 32-bit integer
classification while retaining exact pointee/result identity and all ownership, availability, and
dominance validation. No coercion or alternate representation is added.

The provider reuses `llvm.nvvm.match.all.sync.i32p` because its operands and PTX b32 semantics are
sign-independent, while the facade retains independent source-semantic negotiation. NVVM and
NVRTC agree on `[64, 64]`, one global 32-bit load/store pair, two synchronized ballots, and one
`match.all.sync.b32`. CUDA 12.9 `ptxas` accepts both. On an RTX 5090, distinct unsigned lane values
produce zero in every lane and uniform `23` produces one through both routes.

Seven names add 126 measured lines, from 27,669 to 27,795. The complete Slice 46-62 wave/ABI
matrix passes 113/113 and Release passes 411/411 with sorted-name SHA-256
`bea39cafc76c97ab6cb2d31fcc12aa42f41fe9d3d4d324ca296e115cd5d4d3a4`; removing the seven Slice 62
names reproduces Slice 61's 404-name hash
`40f3eba7cfb2602716a16b54d942cf09e34e9f2171835889a1dea43cb1e10d0a` exactly. Debug preservation
passes 10/10.

Slice 63 adds public Float `WaveActiveAllEqual(value)` through feature 48/intrinsic operation 14
at the canonical masked-helper boundary. Exact assembly plus `Func(Bool, UInt, Float)` matching
preserves the semantic Float through the emitter and facade. V3 stays 528/308 bytes, and exact
Slice 62 providers remain loadable with feature 48 clear.

LLVM 7 and the provider LLVM expose only integer match-all intrinsics, and NVRTC emits
`match.all.sync.b32` for the Float CUDA template. After validating the i32 mask and f32 value
without mutation, the provider creates one `bitcast float to i32`, calls the established aggregate
i32 match-all intrinsic, and exposes only its Bool predicate. The ordinary bitcast requires no
new callback, type role, legacy text rewrite, or duplicate intrinsic audit.

The predicate fixture now uses an explicit Boolean/Integer/Float payload kind, and one
real-provider verifier asserts the shared graph plus a Float-only bitcast. NVVM and NVRTC agree on
`[64, 64]`, one global 32-bit load/store pair, two synchronized ballots, and one
`match.all.sync.b32`. CUDA 12.9 `ptxas` accepts both. On an RTX 5090, distinct ordinary Float lane
values produce zero in every lane and uniform `3.25` produces one through both routes; NaN and
encoded-equality policy remain outside this row.

Seven names add 155 measured lines, from 27,795 to 27,950. The complete Slice 46-63 wave/ABI
matrix passes 120/120 and Release passes 418/418 with sorted-name SHA-256
`33720ee2997610b2d1823858e1e80641d44efce3d6b09b37d0271c70ec54c929`; removing the seven Slice 63
names reproduces Slice 62's 411-name hash
`bea39cafc76c97ab6cb2d31fcc12aa42f41fe9d3d4d324ca296e115cd5d4d3a4` exactly. Debug preservation
passes 10/10.

Slice 66 adds four canonical zero-parameter UInt3 execution operations and one canonical
zero-parameter Void group barrier through typed V4 catalog rows. Construction interface version 2
appends vector construction/extraction and extended call/return callbacks while its inherited
scalar callbacks and exact version-1 table remain frozen. The provider maps whole-vector semantics
to all twelve `tid`, `ctaid`, `ntid`, and `nctaid` components and maps synchronization to
`llvm.nvvm.barrier0`; the LLVM-7-compatible writer audits and normalizes only the differing
special-register declaration attributes.

Direct NVVM and NVRTC both expose all twelve special registers plus `bar.sync`, and CUDA 12.9
`ptxas` accepts both. A `3 x 2 x 2` grid of `4 x 3 x 2` blocks records all 288 unique coordinate
tuples with exact block/grid dimensions through both routes on an RTX 5090. The complete Release
prefix passes 425/425 with sorted-name SHA-256
`641fcaf6a0da63e30a6146beb3e46e261d58297299aa33d180a1f86d73e4f0e5`; removing the six Slice 66
names reproduces Slice 65's 419-name hash
`c634caa999f2b191c85b37cc7885d39462bcef55406ef64dc04bd1a1d02590c9` exactly. Debug preservation
passes 11/11.

Slice 67 adds the exact linked-IR shape for `groupshared int[64]`: a module-owned `IRGlobalVar`
with `GroupShared` rate and fixed-array pointer type, consumed by ordinary shared-address-space
element pointers, loads, and stores. V4 construction version 3 appends one generic typed global
storage declaration; the existing fixed-array, pointer, GEP, load, and store surface expresses the
rest without shared-specific callbacks. Versions 1 and 2 remain frozen and queryable.

One 64-thread block uses signed atomic tickets to write every shared slot, synchronizes, and reads
the reverse slot. Direct NVVM and NVRTC produce identical 64-element results on an RTX 5090, and
CUDA 12.9 `ptxas` accepts both. A separate direct `sm_70 -v` assembly reports 14 registers, one
barrier, 256 bytes of shared memory, and no stack or spills. Unsupported global-storage shapes and
the adjacent floating-point shared-element relation stop before provider discovery.

Six names increase the five measured test/support files from 29,241 to 29,707 physical lines. The
complete Release prefix passes 431/431 with sorted-name SHA-256
`3d3e5effec15efd6d8eec74752802df83fe21ffb89e9d9037b3abf0803d25c0b`; removing the six Slice 67
names reproduces Slice 66's 425-name hash
`641fcaf6a0da63e30a6146beb3e46e261d58297299aa33d180a1f86d73e4f0e5` exactly. Debug preservation
passes 8/8.

Slice 68 selects signed and unsigned integer scalars at 8, 16, 32, and 64 bits for raw ABI,
constants, scalar helper/SSA transport, naturally aligned device memory, and the established
wrapping arithmetic, bitwise, negate, equality, and ordered-comparison families. It also adds
explicit integer narrowing/widening/signedness changes, float32/integer conversion, and the bounded
signed-i32x2 device load/add/store proof. The canonical IR carries `intCast`, `castIntToFloat`, and
`castFloatToInt`; signedness remains in the semantic descriptor after LLVM integer types become
signless.

The exact catalog remains authoritative for all legacy adapters. Parameterized V4 family
resolution adds three conversion operation IDs but no callback, facade method, feature bit,
construction version, or whole-signature enum. Normal and compatible provider text demonstrate
`add i8`, signed and unsigned comparisons, `sext`, `zext`, `sitofp`, `fptoui`, and
`add <2 x i32>`. Mixed-sign arithmetic, i24 descriptors, and vector multiplication remain exact
negative boundaries, and an older static-only V4 provider stops before module creation.

Direct NVVM and NVRTC agree on all fifteen launch-parameter widths and produce identical mixed
results on the RTX 5090. CUDA 12.9 `ptxas` accepts both routes; a representative direct `sm_70 -v`
assembly uses 32 registers, no barriers, 444 bytes constant memory, no stack, and no spills.
Float64/low precision, arbitrary vectors/matrices, shifts/division/remainder, and non-i32
resource/shared/atomic breadth remain unclaimed.

Five names add 468 measured physical lines, from 29,707 to 30,175. Release passes 436/436 with
sorted-name SHA-256 `38cc59e5a3488f84cdb4e5c26cc11f3afbb59e10dcae97036ab64c7e7148054d`;
removing the five Slice 68 names reproduces Slice 67's hash
`3d3e5effec15efd6d8eec74752802df83fe21ffb89e9d9037b3abf0803d25c0b` exactly. The rebuilt Debug
host fake-provider sample passes 8/8; real-provider validation uses Release because the local LLVM
dependency build does not provide Debug libraries.

Slice 73 replaces the current builder's two `RWStructuredBuffer<int>`-specific callbacks with one
generic struct-value extraction operation. Selected scalar resource views are now assembled as an
unpacked global-pointer/count struct, and element addressing composes field extraction with the
existing typed pointer-offset operation. Exact ABI revision 3 is forward-only; the host, facade,
real provider, and fake provider expose no legacy resource-specific surface.

The direct consumer accepts both canonical producer spellings for a CUDA execution-vector
component: one-lane `swizzle` and constant-index `getElement`. The existing
`wave-lane-index-multidim.slang` runtime comparison passes all five registered CUDA/Vulkan lanes.
Its direct PTX contains the 16-byte conventional global block, multidimensional execution-register
reads, synchronized wave voting, and two float global stores; CUDA 12.9 `ptxas` accepts the PTX for
`sm_70`.

The full Release NVVM prefix passes 335/335 after restoring the established sign-independent
ordinary pointer-offset contract and keeping the new sign-independent resource index contract.
The next file-backed probes remain `GenericAsm` in `cuda-layout.slang` and the multi-field
conventional-global address in `sampler-comparison-state-unused.slang`.

Slice 74 admits exact compile-time CUDA `__alignOf`/`__sizeOf` helpers without widening the runtime
builder contract. A zero-parameter signed-i32 helper with sole terminator
`GenericAsm("alignof($[0])", T)` or `GenericAsm("sizeof($[0])", T)` is folded through the shared IR
CUDA layout rules when `T` is a selected integer or half/float/double scalar, or a two- through
four-lane vector of one. The result uses the existing i32 constant and generic value-return
operations; queried types, assembly text, and new semantic operation descriptors do not cross the
provider boundary. Aggregate queries remain E52017 before provider discovery.

The real CUDA comparison found and corrected the shared IR rule for Slang's prelude-defined
`__half3` and `__half4`: both are four-byte aligned and half3 has eight-byte padded size, matching
the existing AST CUDA layout rule and generated CUDA. The complete 28-alignment NVRTC/direct
comparison passes 2/2, the direct PTX contains the 16-byte conventional block and 28 integer global
stores, and CUDA 12.9 `ptxas` accepts it for `sm_70`. Release host and standalone provider builds
pass, as does the full NVVM prefix at 336/336. Builder ABI revision 3 is unchanged. The next
measured conventional boundary is the multi-field global-parameter graph in
`sampler-comparison-state-unused.slang`.

Slice 75 generalizes the compiler-synthesized conventional CUDA global block from one resource to
multiple supported fields. Selected-scalar read-write structured buffers are executable; sampler
placeholders and unsized arrays of sampler placeholders are storage-only. Field addressing maps an
IR struct key to its actual collected field index and separately requires an exact supported
resource pointee, so preserving sampler ABI bytes does not claim sampler operations.

The CUDA collector's existing unsized-array-last rule now uses `isCUDATarget` instead of matching
only CUDASource, giving direct PTX the same field order as CUDA/NVRTC. Shared IR CUDA layout now
models an unsized array as the prelude's fixed pointer-plus-count `Array<T>` representation: size
16, alignment 8. The direct storage role lowers a sampler slot to opaque i64 and its unsized array
to `{ i64 addrspace(1)*, i64 }` through the existing generic builder surface. ABI revision 3 is
unchanged.

The existing sampler fixture passes 4/4 and emits a 40-byte constant block with the used float
resource at byte offset 8. A two-resource CUDA/NVVM GPU comparison passes 2/2 with
`11, 21, 31, 41`, while a fixed sampler array remains E52017 before provider discovery. CUDA 12.9
`ptxas` accepts the sampler PTX for `sm_70`; Release host and standalone provider builds pass; and
the complete NVVM prefix passes 337/337.

Slice 76 moves CUDA layout queries out of the runtime helper ABI. One pre-validation pass recognizes
the exact type/value `__sizeOf` and `__alignOf` helpers and the exact `__offsetOf` helper, computes
their results with the shared CUDA IR layout rules, replaces every call with signed-i32 constants,
and runs ordinary DCE. The Slice 74 scalar/vector family now uses this same path instead of
declaring zero-argument provider functions.

Layout-determinate nested structs, arrays, matrices, selected scalars/vectors, and pointers can be
queried without admitting those types as runtime direct-NVVM values. Struct offsets additionally
require `field_extract(base, key)` where `base` is the query's exact first argument and `key` names
an exact field; a mixed-base control remains E52017 before provider discovery. The fake provider
sees one entry function, no query calls or value returns, and only integer-constant stores for the
representative aggregate graph.

The existing `cuda-array-layout.slang` comparison passes both CUDA/NVRTC and direct libNVVM with
`48, 0, 16, 20, 44, 4, 0, 0`. Direct PTX contains six literal global stores and no helper
definitions, and CUDA 12.9 `ptxas` accepts it for `sm_70`. Builder ABI revision 3 is unchanged.
The Release host and isolated provider builds pass, and the complete NVVM prefix passes 338/338.

Slice 77 adds selected integer/float32 scalar fields and nonempty flat selected-scalar
`ParameterBlock<T>` fields to the compiler-synthesized conventional CUDA global block. A parameter
block is structurally an address-space-1 pointer to its unpacked element struct. Exact keyed field
addressing composes across the address-space-4 outer block and the loaded parameter-block pointer;
ordinary generic loads recover the scalar, pointer, and resource-view values. ABI revision 3 is
unchanged.

The fake provider observes outer `{ i32, { i32 } addrspace(1)*, resource-view }` storage, keyed
indices 2/0/1 for resource/scalar/block, inner index 0, pointer-aligned outer loads, and a naturally
aligned inner scalar load. An element containing a nested struct remains E52017 before provider
discovery. Layout folding removes the exact dead synthesized read-none zero initializer only when
its non-value inputs are null pointer literals; no runtime aggregate construction is admitted.

The existing `param-block-alignment.slang` comparison passes CUDA/NVRTC and direct libNVVM 2/2
with `0, 8, 16, 8, 0, 0, 0, 0`. Direct PTX contains the 32-byte constant block, loads at byte
offsets 0, 8, and 16, and the inner global scalar read. CUDA 12.9 `ptxas` accepts it for `sm_70`.
The Release host and isolated provider builds pass, and the complete NVVM prefix passes 339/339.

Slice 78 generalizes the flat scalar `ParameterBlock<T>` representation to an exact parameter-group
family shared with `ConstantBuffer<T>`. Both lower to an address-space-1 pointer to the generic
unpacked scalar element struct and reuse exact keyed field addressing. Nested element shapes remain
E52017 before builder discovery.

Exact builder ABI revision 4 adds load flags to the single generic `emitLoad` construction
operation. The real provider maps the invariant bit to LLVM `!invariant.load` metadata and rejects
unknown bits; no CUDA `ldg` intrinsic or group-specific callback crosses the ABI. Direct emission
reuses `isPointerToImmutableLocation(getRootAddr(ptr))`, the same canonical semantic classifier as
CUDA-source lowering, while ordinary mutable device-pointer loads remain untagged.

The existing constant-buffer test passes CUDA-source and direct PTX 2/2. Direct PTX contains the
eight-byte conventional block, `ld.const.u64`, and `ld.global.nc.u32`. The 3D wave shader passes
CUDA/NVRTC, direct libNVVM, and Vulkan 3/3. CUDA 12.9 `ptxas` accepts both new direct modules for
`sm_70`; Release host and standalone-provider builds pass; and the complete NVVM prefix passes
340/340. `noinline.slang` is not registered: its probe compiles, but direct emission does not yet
preserve `IRNoInlineDecoration`, so retained unoptimized helpers would be false semantic evidence.

Slice 79 preserves function contracts through exact builder ABI revision 5. One generic linkage
enum now serves both function declarations and global storage; independent function flags
currently carry no-inline. The provider validates both inputs before mutation and maps them to
LLVM internal/external linkage and `noinline`. The direct emitter makes the selected entry and
CUDA device exports external, ordinary reachable helpers internal, and non-entry no-inline helpers
constrained. A device export uses the exact source name stored in its canonical IR decoration.

The existing no-inline fixture passes CUDA-source, ordinary PTX, and direct libNVVM 3/3. Its direct
PTX distinguishes internal no-inline and plain helpers, `.visible .func exportedFunc`, and
`.visible .entry computeMain`; CUDA 12.9 `ptxas` accepts it for `sm_70`. Normal LLVM 14 text, LLVM
7-compatible text, and the fake boundary independently cover the declaration matrix.

An audit corrected five existing static direct-PTX lanes to request whole-target stdout with
`-o -`; without that option the harness could route an entry-point output request through
CUDA/NVRTC and produce false-positive direct coverage. All audited lanes pass against actual
direct output. Release host and standalone-provider builds pass, the complete NVVM prefix passes
342/342, and the combined unit/file-backed run passes 354/354.

Slice 80 admits nonempty flat entry structs containing selected integer/float32 fields and exact
default-layout read-only structured-buffer views. The physical aggregate kernel parameter is a
generic typed pointer carrying `byval` and natural CUDA alignment; keyed field reads use generic
struct GEP and invariant load operations. Read-only and read-write resource types retain distinct
semantic access while sharing the generic pointer/count LLVM representation. The read-only load is
composed from struct-value extraction, pointer offset, and invariant load, with no resource-shaped
builder callback.

Exact ABI revision 6 adds the generic parameter-attribute setter. The provider validates LLVM 14
typed `byval(T)` and rewrites only that type payload to LLVM 7's `byval` spelling for libNVVM. The
fake boundary records the aggregate pointer role, by-value pointee/alignment, keyed field indices,
resource data extraction, offsetting, and immutable loads. Nested aggregate parameters and
unsupported resource element/atomic shapes stop before provider discovery.

The existing CUDA parameter-layout fixture passes CUDA/NVRTC, direct-libNVVM GPU comparison,
direct PTX checking, and reflection 4/4 with `11, 12, 13, 14`. Direct PTX exposes one aligned
16-byte aggregate parameter, two aligned 16-byte resource views, `ld.global.nc.f32`, and the
read-write global store. CUDA 12.9 `ptxas` accepts the module for `sm_70`. Release host and isolated
provider builds pass, and the complete NVVM prefix passes 344/344.

Slice 81 replaces the historical unsigned-i32x3 and signed-i32x2 value special cases with one
exact signed/unsigned 32-bit integer-vector family for two through four lanes. Canonical
`makeVector`, scalar splat, constant extraction, and swizzle graphs use generic construction and
extraction; established wrapping operations and same-lane integer conversions preserve semantic
signedness over LLVM's signless physical vector type. Dynamic indexing, vector phis/comparisons,
Boolean/float vector values, richer helper ABI, and broader vector memory roles remain unclaimed.

Exact ABI revision 7 adds one generic vector constructor. Real and fake providers reject null,
foreign, count-mismatched, element-mismatched, or unavailable values before mutation. Normal LLVM
14 and LLVM 7-compatible assembly contain two-lane `insertelement`/`extractelement` graphs rooted
in `undef`; no execution-semantic operation is present. The focused fake emitter records the
ordinary uint3 arithmetic, exact x/y extracts, one signed-i32x2 construction, and the subsequent
same-lane conversion. A dynamic-index control stops before provider discovery.

`tests/cuda/dispatch-thread-id-extraction.slang` now passes 3/3: its existing CUDA-source check and
direct PTX checks for the unsigned and signed two-lane entries. The unsigned module reads
`%ctaid`, `%ntid`, and `%tid` in both x and y; both modules retain immutable global loads and global
stores. CUDA 12.9 `ptxas -arch=sm_70` accepts both. Release host and isolated-provider builds pass,
and the complete NVVM prefix passes 347/347.

Slice 82 consolidates exact default-layout structured and byte-address resource views under one
raw-buffer descriptor. Structured resources select an admitted scalar `T`; byte-address resources
select `uint`; read-only and read-write access remains attached to the source view. All four use the
existing unpacked global-pointer/count provider struct, so builder ABI revision 7 is unchanged.

The canonical `getStructuredBufferPtr` and `getUntypedBufferPtr` producers return ordinary
read-write-qualified pointers to unsized arrays. Direct lowering does not infer resource access
from that pointer spelling: it validates the exact producer/resource/element relation, extracts
view field zero with generic aggregate-element extraction, represents the result physically as the
existing scalar global pointer, and lowers its direct `getElementPtr` consumer with generic pointer
offsetting. Fixed-array GEP correctly rejected that scalar base during development and is not used.

The shared immutable-location classifier now forwards through `getUntypedBufferPtr`. A read-only
byte-address load consequently receives invariant metadata and emits `ld.global.nc.u32`, while a
store rooted in the same read-only resource stops before provider discovery. Direct
`ByteAddressBuffer.Load/Store` remain a separate E52017 operation family.

`tests/cuda/get-buffer-ptr.slang` passes all three registered lanes with
`11, 21, 31, 41, 102, 202, 302, 402`. Direct PTX contains the 48-byte global parameter block,
loads view pointers at byte offsets 0, 16, and 32, then performs two global scalar loads and two
stores. CUDA 12.9 `ptxas -arch=sm_70` accepts that module and a separate read-only probe. Release
host and isolated-provider builds pass, and the complete NVVM prefix passes 351/351.

Slice 93 lowers every matrix type on the direct-libNVVM route through Slang's existing matrix
legalizer. The canonical physical value is a fixed array of row vectors, so matrix construction,
componentwise arithmetic, constant row/column extraction, and control-flow transport reuse the
ordinary vector operations plus one generic aggregate-value path. CUDA-source/NVRTC emission keeps
its established matrix policy.

Exact builder ABI revision 9 replaces the struct-only value-extraction callback with generic
aggregate construction and element extraction. The real provider maps arrays and structs to
`insertvalue`/`extractvalue`, recursively accepts bounded aggregate function values for phi
transport, and rejects null, foreign, mismatched, unavailable, out-of-range, and post-termination
inputs before mutating its module. The compiler currently admits only canonical nonempty fixed
numeric arrays as first-class aggregate SSA values; dynamic indexing, nested aggregate values, and
broader helper signatures remain outside this slice.

`tests/cuda/nvvm-float-matrix-values.slang` passes direct CUDA execution and direct PTX checking
with `8, 15`. The branch-sensitive case proves that the row array crosses an aggregate phi instead
of being optimized entirely into local row vectors. CUDA 12.9 `ptxas -arch=sm_70` accepts the
module. Release host and isolated-provider builds pass, and the complete NVVM prefix passes
365/365.

Slice 94 admits native Float16 as a selected first-class SSA value under CUDA 12.9's NVVM IR 2.0
contract. Exact forward-only builder ABI revision 10 adds only the `FLOAT_CONVERT` semantic
operation ID; the existing descriptor and generic operation callback carry floating kind, 16/32
bit width, signedness where relevant, and one through four lanes. Integer-to-floating,
floating-to-integer, and Float16/Float32 conversions preserve lane count. Same-width, mixed-lane,
Float64, BFloat16, and FP8 descriptors remain unsupported.

The real provider maps Half to LLVM `half`, exact 16-bit constants, native floating arithmetic and
comparison, `fptrunc`/`fpext`, and the existing vector/helper/phi construction paths. Generic
floating negation is built as typed negative-zero subtraction in the LLVM graph because LLVM 14's
`fneg` token cannot be consumed by libNVVM's LLVM 7 reader; the compatibility writer does not
infer Half or vector types from text. Half remains a value/helper role only: entry parameters,
pointers, resource storage, conventional global fields, matrices, and memory-backed local
aggregates are not admitted by this slice.

The complete Float builtin fixture now passes direct runtime and PTX lanes, and the existing scalar
`half-calc.slang` passes its new direct runtime lane at the suite's default optimization level. The
focused `nvvm-half-values.slang` produces `-8, -5, 1, -5`; its 1,608-byte optimized PTX contains
native `f16`/`f16x2` conversions and arithmetic, and CUDA 12.9.86 `ptxas -arch=sm_70` emits a
3,176-byte cubin. The same Half2-heavy module is rejected by libNVVM at `-O0` with its generic
"unsupported operation" diagnostic, while scalar Half succeeds at `-O0`; the registered focused
lanes therefore request `-O3` and preserve this toolkit limitation explicitly. The broader
`half-vector-calc.slang` and `half-vector-compare.slang` fixtures next stop at their independent
mutable local `var` and aggregate helper-result boundaries.

Slice 95 closes the local-vector half of that boundary at its producer. `constructSSA` now treats a
direct local-vector `swizzledStore` as a partial assignment to the complete tracked value and emits
canonical `swizzleSet`. It does not promote a variable whose pointer escapes, whose partial store
uses an address chain or dynamic l-value index, whose type is noncopyable, or whose lifetime crosses
switch fallthrough. The motivating final IR therefore has no local Half4 pointer or memory
operations; its one retained `swizzleSet(%old, %replacement, 0, 1, 2)` is the exact semantic value.

Direct emission flattens that selected two- through four-lane value using its existing generic
vector extraction/construction callbacks. Exact base/source/result types, source width, unique
bounded constant destinations, dominance, and availability are checked before provider mutation.
Builder ABI stays at revision 10, and neither the facade nor real LLVM provider adds an alloca,
local-variable, swizzle, or Half-specific operation.

The focused fake test proves the positive path uses only generic vector values and that a dynamic
local element store remains E52017 `var` before builder discovery. The existing
`half-vector-calc.slang` direct runtime and PTX lanes produce `75, 220.5, 565, 1108` and native
f16/f16x2 PTX. The PTX is 3,495 bytes, `ptxas -arch=sm_70` emits a 3,688-byte cubin, and the full
NVVM prefix passes 367/367. Those direct lanes request O3 for CUDA 12.9 libNVVM. The file's older
NVRTC lane has an independent pre-existing CUDA-source failure because CUDA 12.9 `__half4` does
not expose `.xyz`; it is not evidence about this direct path. The remaining adjacent direct
boundary is the scalar-struct result plus `BorrowInOutParam<Values>` stateful helper in
`half-vector-compare.slang`.

Slice 110 carries the canonical Float32 array-of-vector matrix representation through generated
helpers and memory. Numeric-array constant-buffer fields use the established conventional global
parameter block, immutable array-element addresses, and whole-array loads. Local numeric arrays
and vectors share one exact sequential element-pointer relation; the provider's ABI revision 17
accepts only a typed pointer to a nonempty fixed array or vector and an available integer index.
The same generic operation closes the adjacent dynamic local Half-vector lane store.

Reachable anonymous generated helpers receive deterministic internal physical names. Entry,
export, and mangled names remain authoritative, and collision validation happens before module
creation. Focused fake coverage observes two generated helpers, the numeric-array parameter-group
field, whole-array and immutable loads, array-row and vector-lane addresses, and the Half lane
store. The real `row-major.slang` runtime/PTX lanes pass, while `column-major.slang` is registered
for PTX only because its direct runtime probe returns `0`. CUDA 12.9.86 `ptxas -arch=sm_70`
accepts the 1,435-byte row-major and 2,951-byte column-major modules and emits 3,048-byte and
3,688-byte cubins. Release host/provider builds pass and the complete NVVM prefix passes 380/380.

Slice 111 moves the established LLVM buffer-storage lowering before matrix legalization on the
direct route. Column-major intent is therefore preserved as a canonical `[PhysicalType]` wrapper
around a fixed numeric array, and the existing unpack helper explicitly performs the conversion to
logical row vectors. Builder ABI revision 18 generalizes selected-value extraction from vectors to
fixed arrays; a dynamic LLVM array index becomes bounded `extractvalue` plus typed `select`
operations because LLVM has no dynamic `extractvalue` instruction.

The existing `column-major.slang` runtime expectation now passes unchanged with output `1`, while
`row-major.slang` remains `11, 22, 33, 1`. Their 2,951-byte and 1,433-byte PTX modules assemble to
3,688-byte and 3,048-byte cubins. The neighboring packed non-square row-major fixture also passes
direct runtime with `12, 16`; its 881-byte PTX assembles to a 2,920-byte cubin. Non-square
column-major remains an exact adjacent stop because its physical array has a 12-byte Float3 stride,
whereas LLVM's `<3 x float>` array element has 16-byte natural alignment and allocation size.
Release host/provider builds pass and the complete NVVM unit-test prefix remains 380/380.

Slice 112 admits the same canonical sole-array `[PhysicalType]` wrapper as a selected raw
`RWStructuredBuffer` element for read-only logical matrix access. The existing early LLVM storage
pass remains the producer. Resource element, physical field, dynamic fixed-array row, and dynamic
vector-lane pointers use the generic struct and recursive sequential-pointer contracts; immutable
ownership follows the complete chain and rejects writes.

Forward-only builder ABI revision 19 adds typed bit reinterpretation for the first exposed
GenericAsm stop. Exact scalar `asint`, `asuint`, and `asfloat` catalog entries lower through the
generic family to LLVM `bitcast` or a signless integer identity. The existing
`structured-buffer-of-matrices.slang` and `matrix-layout-structured-buffer.slang` fixtures pass
direct CUDA runtime and PTX lanes with unchanged 64-byte and 48-byte strides. Their 971-byte and
2,017-byte PTX modules assemble with CUDA 12.9.86 to 2,920-byte and 3,304-byte cubins.
Release host/provider builds pass, and the complete NVVM unit-test prefix passes 381/381.

Slice 113 promotes fourteen existing specialized compute fixtures without changing production
code or builder ABI. Generic dot, nine static/interface/existential dispatch cases, basic dynamic
generics, kernel-context threading, nested generic structs, and transitive interfaces all reduce
before direct preflight to the established generic NVVM contracts. Their 28 exact runtime/PTX
lanes pass unchanged.

The PTX modules are 478, 688, 685, 688, 645, 680, 645, 688, 688, 712, 685, 1,233, 674, and
645 bytes in plan order. CUDA 12.9.86 `ptxas -arch=sm_70` emits one 2,664-byte cubin, twelve
2,792-byte cubins, and one 3,048-byte cubin. The adjacent zero-index bounds fixture compiles but
reproduces its documented CUDA runtime mismatch, so no direct lane is registered. Release
compiler/provider builds pass, and the complete NVVM unit-test prefix remains 381/381.

Slice 114 adds canonical typed value selection through forward-only builder ABI revision 20. One
generic operation descriptor covers scalar and two- through four-lane Boolean, selected integer,
Float16, and Float32 results. The condition must be Boolean with the result's exact lane count, and
both alternatives must exactly match the result. Scalar-condition vector selection, broadcast,
aggregate selection, and wider floating kinds remain unsupported.

The real provider emits ordinary LLVM `select` for both LLVM 14 assembly and the LLVM 7-era NVVM
IR 2.0 dialect. Focused builder coverage observes Boolean-vector, integer-vector, and scalar-Half
selection and rejects mismatched condition lanes or alternative signedness. The fake-emitter path
records the exact three typed operands, while the direct emitter maps only canonical
`kIROp_Select`.

`tests/cuda/nvvm-typed-select.slang` passes direct runtime and PTX lanes with `0, 0, 1, 0`; its
1,193-byte PTX assembles with CUDA 12.9.86 to a 3,048-byte cubin. The existing
`logic-no-short-circuit-evaluation.slang` probe advances through select and stops at its independent
mutable module-scope `static int` pointer, which requires an initializer-preserving global-storage
contract. Release host/provider builds pass, and the complete NVVM unit-test prefix passes 382/382.

Slice 121 replaces the construction API's bespoke signed-i32 atomic-add callback with one queried,
descriptor-driven atomic-operation interface in forward-only builder ABI revision 21. The
descriptor's independent operation, value-type, physical-address-space, and memory-order fields
are resolved once and checked by the compiler, shared semantic catalog, real/fake providers, and
emission. No compatibility callback or duplicate support table remains.

The initial exact family is relaxed global scalar Int32/UInt32 add. Accepted pointers are
established writable device-pointer parameters/globals and direct structured-buffer element
pointers with an exactly matching pointee. The provider emits naturally aligned monotonic
system-scope LLVM `atomicrmw add i32`; LLVM's signless integer type covers both catalog rows, and
the existing semantic LLVM 7 compatibility serializer validates the instruction unchanged.
Other operations, orders, value types, pointer producers, and address spaces remain unsupported.

The unchanged `tests/compute/atomics.slang` result passes direct CUDA execution. Its 880-byte PTX
contains exactly three `atom.global.add.u32` instructions and CUDA 12.9.86
`ptxas -arch=sm_70` emits a 2,920-byte cubin. Release host/provider builds pass, focused tests cover
both admitted signedness rows and adjacent invalid dimensions, and the complete NVVM prefix passes
388/388.
