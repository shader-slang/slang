# Establish a truthful CUDA 13 validation baseline

This ExecPlan follows `.agent/PLANS.md`. The maintainer explicitly requires committing the completed plan and report with each slice; AGENTS.md records this exception.

## Purpose and Observable Result

Run the existing direct-NVVM unit suite against CUDA 13 with an explicit supported target and preserve ignored GPU tests as ignored. A selected architecture must govern real Slang compilation, assembly, and runtime eligibility. Fake compiler tests keep their deterministic historical target inputs. The compiler must never silently retarget user shaders.

## Progress

- [x] (2026-09-22) Reproduced the baseline: CUDA 13 rejects compute_70; 13 sm_80 shaders compile and assemble.
- [x] (2026-09-22) Identified that test-server ignores explicit Ignored results after successful assertions.
- [x] Implement explicit real-test target selection and truthful skip aggregation.
- [x] Build and run focused regression and full NVVM unit checks.
- [x] Document exact evidence, self-review, format, and commit.

## Surprises and Discoveries

The prior server run reported 325 passing, 116 failing, and one ignored test. GPU tests with successful preflight assertions could be counted as passing when they subsequently requested a skip. `TestServer::TestReporter` discards Ignored and `_executeUnitTest` infers a skip only from zero assertions. This is a harness result-aggregation defect, not NVVM runtime evidence.

The first supported-target run exposed a producer bug in the NVRTC reference route: nvrtcGetPTXSize includes a trailing NUL, and NVRTCDownstreamCompiler::compile stored that byte in the PTX artifact. Linux ptxas rejected the file; removing only that byte made the exact output assemble. The slice therefore includes the producer-side correction and a direct blob-content regression. NVIDIA documents the size contract at https://docs.nvidia.com/cuda/nvrtc/.

## Decision Log

- 2026-09-22, Codex: keep explicit target selection in test infrastructure and preserve the frozen corpus; do not alter compiler architecture selection or classify unsupported architectures as success.
- 2026-09-22, Codex: missing local NVIDIA devices limit acceptance to compilation/assembly and correct skip reporting. GPU correctness evidence remains a separate slice requiring hardware.

## Outcomes and Retrospective

Completed on 2026-09-22: native Linux Debug with CUDA 13.4.2 passed 419/419 selected tests, with 54 explicit skips and zero failures. Both reporter paths passed focused regression checks. Invalid architecture text and a real compute_70 request failed explicitly. No new GPU runtime evidence is claimed.

## Context and Current Pipeline

Real integration fixtures call `_compileSlangWithDirectNVVM` and `_compileSlangWithPTXMethod`, which currently select cuda_sm_7_0. CUDA 13 libNVVM rejects the resulting compute_70 option. `_assemblePTX` separately selects sm_75, and runtime tests assume a minimum device of sm_70. These test-owned selections need one explicit real-test target contract.

`SLANG_IGNORE_TEST` records Ignored and throws AbortTestException. The test server must preserve that outcome through its ToolReturnCode so the parent runner does not count absent GPU coverage as passing.

## Scope and Non-Goals

Test target selection, test-server result propagation, focused regressions, durable validation instructions, and baseline evidence. No compiler IR changes, provider ABI changes, frozen corpus edits, GPU driver installation, or unsupported-feature expansion.

## Architecture and Invariants

Requested architecture remains explicit. Real compiler, assembler, and device checks agree. Fake tests remain independent of CUDA installation. Failures take precedence over skips, and explicit skips survive successful setup assertions. Infrastructure absence is recorded separately from correctness.

## Interfaces and Dependencies

The existing Debug binaries, compiler-matched LLVM 14.0.6 provider, and CUDA_PATH=/usr/local/cuda-13.4. A documented test-only architecture selection will support the existing SM70/80/90 validation families and reject invalid selections. CUDA 13 baseline uses SM80.

## Milestones

1. Add shared real-test architecture selection in the NVVM test support and integration fixtures.
2. Preserve explicit ignored outcomes in tools/test-server/test-server-main.cpp and add regression evidence.
3. Rebuild slang-test and its modules; run the affected harness checks and full NVVM prefix.
4. Record the exact pass/fail/ignored counts and PTX assembly results, then commit the plan, report, and durable instructions.

## Validation and Acceptance

Use the slang-build skill's native Linux Debug preset. Required gates: formatting and git diff --check; focused target validation; skip-after-success and failure-before-skip behavior; the complete NVVM/cudaEmissionMethod/invalidCUDAEmissionMethod prefixes with CUDA 13 and explicit SM80. Reuse the previously established 13 shader sm_80 compilation/assembly baseline unless changes affect it. GPU-dependent tests must be visibly ignored on this machine, not reported as passed. Unsupported requests must fail clearly.

## Failure and Recovery

Keep raw logs and binaries under build/. Investigate each remaining failure without weakening assertions or hiding unsupported combinations. Test-only changes can be reverted independently of compiler/provider behavior. Preserve the existing clean commit and do not touch unrelated work.

## Artifacts and Hand-Off

Commit this completed plan, a five-part report, code/tests, and durable design/capability notes. Preserve exact commands and counts. Later slices consume this baseline for reproducible provider builds, packaging, CI, and physical GPU checks.
