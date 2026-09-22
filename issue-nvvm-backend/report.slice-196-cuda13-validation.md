# Slice 196: truthful CUDA 13 baseline

## Motivation

An empty compute shader requested compute_70 from CUDA 13 and failed before testing backend behavior. A separate runtime fixture could pass its session-creation assertion, discover no CUDA device, and be counted as passing by the test server. The baseline needs a supported explicit target and honest coverage accounting.

## Proposed solution

Use test-only `SLANG_NVVM_TEST_ARCH` for real compilation, assembly, and device eligibility. It accepts exactly 70, 80, or 90; an absent setting preserves historical defaults. Preserve explicit ignored outcomes after successful assertions, with failures taking precedence and retry reconciliation unchanged.

The supported-target run also exposed an NVRTC artifact producer bug: its PTX size includes a C string terminator which was serialized into text files. Remove that byte at the artifact producer and assert the vendor contract.

## Change summary

- NVVM test support and integration fixtures share explicit real target selection. Fake fixtures retain fixed inputs; low-level fixtures retain their historical SM75 minimum.
- Both reporter paths retain explicit skips. The test-server parent prints skip reasons. Child-process regressions exercise the server protocol; direct reporter tests cover assertion precedence and existing retry behavior.
- `NVRTCDownstreamCompiler::compile` excludes the terminating NUL from PTX artifacts. `nvvmNVRTCReferencePTXExcludesTerminator` checks the returned blob.
- The plan, design, and ledger record this native Linux baseline separately from historical Windows corpus evidence. AGENTS.md records the maintainer's instruction to commit slice plans and reports.

## Concepts and vocabulary

A real target setting controls actual Slang/libNVVM/NVRTC requests, assembly, and device eligibility. Fake compiler fixtures test routing without a toolkit. Explicit skip means the intended result was not established, even after successful setup. PTX artifacts store text bytes; the vendor API's terminating NUL is not part of that text.

## Process report

The target producer was test infrastructure: `_createSlangPTXLinkedProgram` received hard-coded cuda_sm_7_0. `_compileSlangWithRealPTXMethod` now constructs that capability from the validated setting, and `_compileSlangWithRealNVVM` reuses it. Real low-level helpers and assembly preserve their pre-existing SM75 minimum. Runtime checks use the corresponding selected target. No compiler retargeting or provider ABI change was introduced.

Pass followed by Ignored is valid reporter input when device discovery follows session creation. The server discarded Ignored and inferred completion from assertion counts. It now stores that state; `_executeUnitTest` returns not-available unless an assertion failed. The parent preserves the reason. `_combineResultsWithinTest` handles the same valid sequence in-process without changing retry combination. This is the owning aggregation layer, not a CUDA-specific workaround.

[NVRTC documents](https://docs.nvidia.com/cuda/nvrtc/) that `nvrtcGetPTXSize` includes the terminator. `NVRTCDownstreamCompiler::compile` allocated that buffer and handed the full length to `ListBlob::moveCreate`. The vendor string is canonical; the artifact producer's length was wrong. A CLI reproduction produced an 8,707-byte file ending in NUL and failed ptxas with unexpected EOF; removing only that byte made it assemble. The producer now asserts the terminator and excludes it. Without this correction the supported-target suite had 58 reference-route assembly failures. The direct blob regression additionally protects the invariant. Consumers need no special handling.

Self-review inventory: the strict parser and real-only wrappers survive because tests own target selection; the SM75 floor preserves existing fixture requirements. The within-test combiner and server skip state survive because Pass/Ignore sequences are intentional input. The NVRTC assertion and byte removal survive because the vendor-string-to-artifact boundary owns length. No AST/IR representation is reconstructed and no fallback hides an invalid request.

Validation on 2026-09-22 used native Linux Debug, LLVM 14.0.6 provider, and CUDA Toolkit 13.4.2. Build: `cmake --build --preset debug --target slangc slang-test -j12`. Tests ran with `CUDA_PATH=/usr/local/cuda-13.4 SLANG_NVVM_TEST_ARCH=80`, using `build/Debug/bin/slang-test` and prefixes `slang-unit-test-tool/nvvm`, `slang-unit-test-tool/cudaEmissionMethod`, `slang-unit-test-tool/invalidCUDAEmissionMethod`, `slang-unit-test-tool/testServerIgnore`, and `slang-unit-test-tool/slangTestReporter`, followed by `-use-test-server -server-count 8 -disable-retries`.

Result: **419/419 passed, 54 ignored, zero failures**. Reporter tests passed 26/26 and child skip regressions 3/3, each both through the server and in-process. Invalid architecture text failed both empty-kernel checks. Explicit 70 failed both real Slang empty-compute checks with unsupported compute_70. Low-level defaults intentionally remain compute_75 and are not compute_70 evidence. Formatting and diff checks passed.

No GPU is exposed here. These results establish compilation, assembly, and harness behavior, not GPU correctness. Frozen-v1 423/427 and discovery 72/72 remain historical evidence with unchanged identities and classifications. Raw logs remain uncommitted.
