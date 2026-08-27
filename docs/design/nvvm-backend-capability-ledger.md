# NVVM Backend Capability Ledger

This ledger records what the direct NVVM experiment has demonstrated and where ordinary Slang
programs currently stop. `Pass` means the named lane ran successfully on the local CUDA/toolchain
configuration described in [the backend design](nvvm-backend.md). `Expected stop` means the test
reached a deliberate, stable boundary; it is not counted as backend support. Empty measurement
fields have not been collected yet. `Pending` describes planned evidence that has not run and does
not establish backend support.

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
| `slang-unit-test-tool/nvvmSlangUnsupportedIRStopsBeforeEmission` | Signed-i32 left/right shifts, division, and remainder stop at `'shl'`, `'shr'`, `'div'`, and `'irem'`; logical NOT and unsigned/i64/float Neg stop at `'entry-point parameter'`; unsigned or i64 bitwise AND/OR/XOR/NOT, unsigned/wide multiplication and equality, pointer equality, void helper calls, and unsigned pointer offsets retain deterministic E52017 boundaries before builder discovery or libNVVM program creation. Float multiplication is accepted, so its old negative fixture now stops at the following unsupported `'castFloatToInt'`. The Slice 18 f32-sine case stops at the float-returning target helper's unsupported `'helper function result type'`, before provider discovery and without exercising `GenericAsm` matching | Pass |
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
| `slang-unit-test-tool/nvvmSlangUnsupportedIRStopsBeforeEmission` | A retained barrier call emits E52017 before builder-module or libNVVM-program creation | Pass |
| `slang-unit-test-tool/nvvmSlangMissingBuilderDoesNotFallback` | An unavailable builder emits E52016 and never falls back to NVRTC | Pass |
| `tests/cuda/sampler-comparison-state-unused.slang` | Established default PTX and explicit NVRTC lanes produce accepted PTX | Pass |
| `tests/cuda/cuda-compile.cu` | Explicit `-pass-through nvrtc` retains precedence even with `-emit-cuda-via-nvvm` | Pass |

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
