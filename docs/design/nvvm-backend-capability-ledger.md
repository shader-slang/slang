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
| `tests/cuda/nvvm-routing-not-implemented.slang` | 1 | None beyond `slangc` | Not applicable | Expected stop | `legalize` | E52014: Slang-IR-to-NVVM lowering is not implemented | Not applicable | — |

## Routing and regression evidence

| Test | Contract | Result |
| --- | --- | --- |
| `slang-unit-test-tool/parseCUDAEmissionMethods` | Both selector orders are last-wins, only the canonical option is stored, and target settings are isolated | Pass |
| `slang-unit-test-tool/cudaEmissionMethodSelectsDownstreamCompiler` | Default follows the session transition; explicit NVRTC/NVVM bypass it; invalid input selects no compiler | Pass |
| `slang-unit-test-tool/cudaEmissionMethodLinkOptionsAffectRoutingAndHash` | A canonical `linkWithOptions` override changes both effective dispatch and the shader hash | Pass |
| `slang-unit-test-tool/invalidCUDAEmissionMethodIsDiagnosed` | An unknown integer supplied through the public target-option API reaches E52015 | Pass |
| `tests/cuda/sampler-comparison-state-unused.slang` | Established default PTX and explicit NVRTC lanes produce accepted PTX | Pass |
| `tests/cuda/cuda-compile.cu` | Explicit `-pass-through nvrtc` retains precedence even with `-emit-cuda-via-nvvm` | Pass |

Work advances from the lowest incomplete bucket. Slice 6 must replace the E52014 row with a real
minimal-compute lowering lane before higher-level CUDA tests can claim direct-backend results.
