# NVVM Backend Capability Ledger

This ledger records what the direct NVVM experiment has demonstrated and where ordinary Slang
programs currently stop. `Pass` means the named lane ran successfully on the local CUDA/toolchain
configuration described in [the backend design](nvvm-backend.md). `Expected stop` means the test
reached a deliberate, stable boundary; it is not counted as backend support. Empty measurement
fields have not been collected yet.

## Semantic capability evidence

| Test | Bucket | Requirements | NVRTC | NVVM | First NVVM stop | Diagnostic or capability | ABI/runtime comparison | Measurements |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmCompilerCompilesEmptyKernelBitcode` | 0 | CUDA 12+, libNVVM, `compute_75` | Not applicable | Pass | — | Exact LLVM-bitcode artifact verifies and compiles | Entry symbol checked; no runtime | — |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmIRBuilderCompilesEmptyKernel` | 1 | LLVM 14 provider, libNVVM, `compute_75` | Not applicable | Pass | — | Builder-produced empty kernel compiles | Entry symbol checked; no runtime | — |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmIRBuilderDifferentialScalarPTX` | 1 | LLVM 14 provider, NVRTC, libNVVM, `compute_75` | Pass | Pass | — | AS1 `i32` load/store reference kernels | Parameter widths and entry-scoped global operations agree; no runtime | PTX/resource timing not measured |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmIRBuilderPtxasAcceptsScalarReferenceKernels` | 1 | LLVM 14 provider, CUDA `ptxas`, `sm_75` | Pass | Pass | — | Both scalar reference kernels assemble | Static PTX acceptance; no runtime | PTX/resource timing not measured |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmIRBuilderCompilesScalarBitcodeThroughRegistry` | 1 | LLVM 14 provider, libNVVM, `compute_75` | Not applicable | Pass | — | Session-registered NVVM compiler accepts exact builder bitcode | Both entry symbols checked; no runtime | — |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmSlangEmptyComputeUsesDirectPipeline` | 1 | In-process fake LLVM 14 builder and libNVVM, `cuda_sm_7_0` | Not compared | Pass | — | Ordinary Slang linked IR lowers through verified builder bitcode and the registered NVVM compiler | Builder receives `computeMain`; exact bitcode bytes and `compute_70` options checked; no runtime | Fake-only; no performance measurements |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmSlangRealEmptyCompute` | 1 | LLVM 14.0.6 provider, CUDA 12+ libNVVM, `cuda_sm_7_0` | Not compared | Pass | — | Ordinary empty compute kernel compiles through the real direct route | PTX entry `computeMain` checked; no runtime | PTX size and compile time not measured |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmSlangRealEmptyComputePtxasAccepts` | 1 | LLVM 14.0.6 provider, CUDA 12+ libNVVM and `ptxas`, `cuda_sm_7_0` | Not compared | Pass | — | Real direct-route PTX assembles successfully | Static PTX acceptance; no runtime | Resource and timing measurements not collected |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmIRBuilderBuildsScalarConditionalKernel` | 2 | LLVM 14.0.6 provider | Not applicable | Pass | Not applicable | Provider emits signed `i32` add/sub/less-than and conditional/unconditional branches | Verified LLVM assembly and bitcode; no runtime | Not measured |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmSlangScalarMemoryAndConditionalUseDirectPipeline` (write/copy cases) | 1 | In-process fake builder and libNVVM, `cuda_sm_7_0` | Not compared | Pass | Not applicable | Raw `[CUDAKernel]` signed `i32` and AS1 pointer parameters, load, and store | Exact parameter order, value producers, and memory operands checked | Fake-only; no performance measurements |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmSlangScalarMemoryAndConditionalUseDirectPipeline` (conditional case) | 2 | In-process fake builder and libNVVM, `cuda_sm_7_0` | Not compared | Pass | Not applicable | Signed `i32` add/sub/less-than and phi-free `if`/else control flow | Exact comparison operands, branch targets, and per-arm stores checked | Fake-only; no performance measurements |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmSlangRealScalarDifferentialPTX` | 1-2 | LLVM 14.0.6 provider, NVRTC, libNVVM, `cuda_sm_7_0` | Pass | Pass | Not applicable | Ordinary Slang write, copy, and conditional kernels compile through both routes | Parameter widths `[64, 32]`, `[64, 64]`, and `[64, 32, 32]` agree; expected global memory, add, and signed comparison families checked | PTX size and timing not measured |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmSlangRealScalarPtxasAccepts` | 1-2 | LLVM 14.0.6 provider, NVRTC, libNVVM, CUDA 12.9 `ptxas`, `sm_75` | Pass | Pass | Not applicable | All three ordinary-Slang scalar kernels assemble from both PTX routes | Static PTX acceptance | Resource and timing measurements not collected |
| `tools/slang-unit-test/unit-test-nvvm-compiler.cpp::nvvmSlangScalarRuntimeMatchesNVRTC` | 1-2 | LLVM 14.0.6 provider, NVRTC, libNVVM, CUDA driver and GPU with compute capability 7.0+ | Pass | Pass | Not applicable | Both routes launch the same write, copy, and conditional kernels | Both stored `37`, copied `-17`, and produced `7`, `4`, `0`, and `-1`; the `choose(-2, 1)` case distinguishes signed from unsigned comparison | Kernel timing not measured |
| `tests/cuda/nvvm-unsupported-ir.slang` | 4 | None beyond `slangc`; `cuda_sm_7_0` | Not applicable | Expected stop | `emit` | E52017: unsupported linked Slang IR instruction `call` | Not applicable | — |

## Routing and regression evidence

| Test | Contract | Result |
| --- | --- | --- |
| `slang-unit-test-tool/nvvmIRBuilderNegotiatesScalarControlFlowAPI` | The append-only control-flow block leaves an exact scalar-memory provider valid, rejects a partial or incomplete new block, accepts larger tables, and sanitizes failed outputs | Pass |
| `slang-unit-test-tool/nvvmIRBuilderRejectsInvalidScalarControlOperations` | The LLVM 14 provider rejects missing insertion points, type mismatches, cross-module/cross-function handles, non-dominating values, and post-terminator operations before mutation | Pass |
| `slang-unit-test-tool/nvvmSlangNegotiatesScalarControlFlowCapability` | An exact old scalar-memory V2 provider still compiles copy/load/store, while a conditional program reaches E52016 after discovery but before module creation | Pass |
| `slang-unit-test-tool/nvvmSlangRetainsOnlySelectedCUDAKernel` | CUDA-kernel pruning still removes an unselected kernel before direct emission while preserving the exact selected entry point | Pass |
| `slang-unit-test-tool/nvvmSlangRejectsConventionalRawKernelParameters` | Only `[CUDAKernel]` receives the raw scalar ABI; a conventional parameterized compute entry reaches E52017 before builder or libNVVM program creation | Pass |
| `slang-unit-test-tool/nvvmSlangUnsupportedIRStopsBeforeEmission` | Calls, executable integer constants, phi branch arguments, and loops reach deterministic E52017 before builder discovery or libNVVM program creation | Pass |
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

Slice 7 extends the ordinary-Slang direct-NVVM lane through the raw `[CUDAKernel]` ABI for signed
`i32` and AS1 device-pointer parameters. That completes the demonstrated raw scalar parameter part
of Bucket 1. The signed comparison and phi-free conditional cases enter Bucket 2, but that bucket
remains incomplete: executable constants, block parameters and branch arguments/phis, loops, and
direct calls are unsupported. Complex and aggregate types, additional address spaces, shared
memory, builtins, and resources also remain outside the accepted subset. The barrier row is an
expected stop and does not claim barrier support.
