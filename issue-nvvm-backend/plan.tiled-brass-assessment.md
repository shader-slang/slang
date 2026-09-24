# Assess and preserve the tiled-brass material workload

This ExecPlan follows `.agent/PLANS.md`. This is a bounded corpus/assessment task requested by
the user, not a restart of the feature loop. Completed NVVM assessment plans/reports follow the
repository's durable slice-record convention; raw artifacts remain under ignored build/.

## Purpose and Observable Result

Preserve the supplied isolated MaterialX tiled-brass shader and its two compute entry points as
complex compile workloads. Record what NVRTC O3 and NVVM O0/O3 can compile today, identify the
first genuine blocker, and leave reproducible commands and an honest initial baseline for
later compile-speed and generated-code work. Stop before implementing backend features.

## Progress

- [x] 2026-09-24: Inspected clean checkout and the 7,110-line, self-contained attached shader.
- [x] 2026-09-24: Identified eval_buffer and sample_buffer; reviewed existing runtime corpora
      and compile-performance infrastructure.
- [x] 2026-09-24: Initial invocations without defines failed in both paths; shader's
      `__TARGET_CUDA__` source branch requires an explicit application macro.
- [x] 2026-09-24: Both NVRTC entries compile and assemble; both NVVM O0/O3 entries reject the canonical CastUInt64ToDescriptorHandle operation.
- [x] 2026-09-24: Added shader with the user-approved define, entry contracts, reusable runner, measurement summary, README, and report.
- [x] 2026-09-24: Verified source fidelity and six material cells, a three-mode successful control, source-hash rejection, zero-sample rejection, and Python syntax. Recorded follow-up choices; feature work has not started.

## Surprises and Discoveries

The attachment uses CRLF and carries NVIDIA Apache-2.0 provenance. It has no imports/includes.
Without `__TARGET_CUDA__`, DescriptorHandle constructors receive uint2: NVRTC rejects conversion
into CUtexObject/SamplerState and NVVM reports E52017 CastUInt2ToDescriptorHandle. This is not yet
an NVVM-only feature finding. The shader already has an explicit uint64 CUDA branch.

The existing frozen and discovery corpora require runtime inputs and expected output. This
attachment provides neither texture/LUT assets nor expected material outputs. Their denominators
must stay unchanged. The older NVVM timing helper hardcodes Windows paths and computeMain; the
compile-perf suite can represent real source corpora, but is oriented toward successful timing
samples rather than inventorying unsupported direct-NVVM shapes.

## Decision Log

- 2026-09-24: Preserve the supplied shader's semantics and license. Record SHA-256 and any newline
  normalization explicitly; do not simplify the workload merely to make it compile.
- 2026-09-24: Use the prepared Debug compiler, CUDA12.9, and SM80 for initial support assessment.
  Any timings are exploratory, not optimized compiler throughput or GPU-runtime claims.
- 2026-09-24: Test `__TARGET_CUDA__=1` for both backends, and ask the user for normal application flags.
- 2026-09-24: Establish separate compile-only complex-workload identities; do not count successful
  compilation as runtime correctness or change frozen/discovery historical denominators.

## Outcomes and Retrospective

The complex corpus contains two independently assessed entries. NVRTC O3 compiles and assembles
both; direct NVVM O0/O3 rejects CastUInt64ToDescriptorHandle before provider emission. Debug
NVRTC median wall times are 2.569 s and 2.619 s; failure latencies are not speed comparisons.
The report records PTX/cubin size, registers/stack/spills, source warnings, and the canonical
producer-to-preflight trace. Runtime correctness and optimized performance remain future gates.
No production source, historical corpus row, or slice-201 acceptance changed.

## Context and Current Pipeline

Both entries load a generated material record, construct a shading frame and a layered material
instance, then evaluate a BSDF/PDF or sample an outgoing direction, PDF, and weight. The shader
combines generic material interfaces, nested aggregates and arrays, loops, texture descriptors,
texture sampling, LUT buffers, and substantial floating-point math. Source target defines select
the descriptor producer before linking/legalization. NVRTC consumes emitted CUDA source; direct
NVVM consumes typed linked IR through preflight, LLVM14 provider, and libNVVM.

## Scope and Non-Goals

Add corpus coverage and an initial assessment. No production compiler/provider changes, shader
semantic repair, runtime asset invention, benchmark speedup claims, feature loop, or PR publication.
The longer-term aim is compile support, then lower compilation cost and better generated code,
balanced with other features; this task supplies evidence for discussing the next bounded slices.

## Architecture and Invariants

Each workload identity contains source, entry point, stage, defines, and target architecture.
Each mode must use the same workload contract. Keep failures visible, with diagnostics and raw
logs. Require fresh nonempty PTX and successful ptxas assembly before claiming compilation success.
Treat PTX size, registers and spills as descriptive proxies, not kernel-performance evidence.

## Interfaces and Dependencies

Native Linux compiler build/Debug/bin/slangc and matched adjacent provider; CUDA_PATH=/usr/local/cuda-12.9.
Existing helpers under extras/validate-nvvm-toolkit.py and issue-nvvm-backend guide artifact checks.
Runtime execution needs a separately reviewed material/texture/LUT input and correctness contract.

## Milestones

1. Inspect both source entries and normal flags, establish NVRTC reference and direct outcomes.
2. Preserve shader under a complex compile corpus with explicit entry contracts and diagnostic baseline.
3. Add repeatable assessment commands/tooling and run actual compiler/assembler validation.
4. Record measurements, first-blocker producer/consumer trace, and choices for discussion.

## Validation and Acceptance

Compile eval_buffer and sample_buffer independently at NVRTC O3 and direct NVVM O0/O3, SM80.
Capture wall time and Slang phase timers; assemble successful PTX, retain ptxas resource reports.
Validate manifest identities/hashes, runner syntax/error handling, and unchanged historical corpus
files. Use bounded timeouts. No compiler rebuild or full regression run is required for corpus data.

## Failure and Recovery

Keep missing-tool failures distinct from unsupported shaders. Do not turn a failure-to-compile
latency into a performance improvement. Leave unsupported shapes visible for a future slice.
Retain original input and log every compile-option change. Stop before choosing feature fixes.

## Artifacts and Hand-Off

Raw evidence: build/nvvm-tiled-brass/. Durable shader, compile-only manifest, assessment summary,
and report will name both identities and exact commands. Keep the existing slice-201 status intact.

## Final Decisions and Evidence

- The user confirmed `__TARGET_CUDA__=1` and approved placing it in the shader.
- `tests/cuda/complex` owns the intact workload; `complex-corpus.manifest.json` records two entries
  and source/provenance hashes. The new runner reuses existing toolkit execution/hash helpers
  rather than changing the Windows/computeMain-specific historical measurement script.
- Material assessment: six cells, two passed and four preflight-rejected, exit 1. A known-supported
  core shader passes all three modes and assembly, exit 0. Corrupt source hash and zero samples
  each reject with exit 2. Source text matches the original after the documented transformations.
- Exact commands and raw artifacts live in build/nvvm-tiled-brass; durable results are in
  assessment.tiled-brass.json and report.tiled-brass-assessment.md.
- New tooling helpers only measure/validate artifacts; no AST/IR representation or provider
  fallback is introduced. No speculative source reduction or descriptor-cast workaround was kept.

Final inventory checks preserved the exact 452 frozen identities and selected all 82 discovery
workloads with zero frozen-source overlap. New-file formatting and git diff --check passed.
