# Lower canonical wave-mask match operations

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires each completed experimental-backend slice plan to be committed with its implementation;
the completed plan therefore follows that established exception to the active working-log policy.

## Purpose and Observable Result

After this slice, direct NVVM lowers the canonical `IRWaveMaskMatch` shape used by the active-mask
switch and divergent-switch compute workloads. The implementation must preserve mask semantics at
O0 and O3, reuse the existing generic builder interface unless final IR proves an unexpressible
operation, and unlock all three healthy frozen-v1 rows sharing this exact producer.

## Progress

- [x] (2026-08-31) Completed and committed Slice 162 as `2105c51`; preserved both corpus identities
  and raised frozen/discovery both-mode correctness to 391/427 and 64/72.
- [x] (2026-08-31) Ranked remaining healthy failures and selected the exact three-row
  `waveMaskMatch` cluster over heterogeneous helper/preflight and harness-layout buckets.
- [x] (2026-08-31) Dumped and audited final IR, source semantics, existing wave recipes, and LLVM/libNVVM capability.
- [x] (2026-08-31) Defined the exact producer/type contract and proved one new generic value-operation ID is required.
- [x] (2026-08-31) Implemented focused coverage, probed all three targets at O0/O3, and promoted the stable correct rows.
- [x] (2026-08-31) Ran the selected prefix, exact frozen/discovery corpora, measurements, formatting, integrity
  checks, and the unprincipled-change self-review.
- [x] (2026-08-31) Completed durable documentation and the five-part report; commit Slice 163 after the final staged audit.

## Surprises and Discoveries

- Frozen-v1's six-row `preflight-other` and five-row `helper-abi-type-contract` groups contain
  unrelated canonical operations and types; neither is one reusable next invariant.
- Three descriptor-handle rows fail after compilation in the compare harness with `Unsupported
  value size`. They are not evidence for an emitter widening until the runtime producer is audited.
- `IRWaveMaskMatch` is the first unsupported canonical shape for three healthy frozen workloads at
  both optimization levels, making it the largest exact remaining operation cluster.
- Final linked IR is identical across the three targets: `UInt = waveMaskMatch(UInt mask, Int
  selector)`. `synthesizeActiveMask` creates it before the switch so every lane receives the mask
  of active peers taking the same case value.
- The existing `WAVE_MASK_ALL_EQUAL` semantic cannot express this operation. LLVM 14 exposes
  `llvm.nvvm.match.any.sync.i32`, which returns the required peer mask directly, while match-all
  returns only a `{mask, predicate}` pair whose predicate discards the needed partition identity.
- CUDA/PTX uses bitwise comparison for the b32 operation. The shared semantic catalog therefore
  admits signed i32, unsigned i32, and float32 rows, with float32 bitcast to i32 exactly as the
  existing match-all provider path does.
- The repository formatting driver ran but could not format because gersemi, clang-format,
  prettier, and shfmt are not installed on this machine. Manual diff review and
  `git diff --check` remain clean.

## Decision Log

- Decision: make Slice 163 the exact wave-mask-match cluster.
  Rationale: it has one named producer/operation, blocks three real workloads in both modes, and
  exercises the MVP's common-wave area without conflating unrelated failures.
  Date/author: 2026-08-31, Codex.
- Decision: advance the forward-only provider ABI to revision 31 with one appended
  `SLANG_NVVM_VALUE_OP_WAVE_MASK_MATCH` ID and no new callback.
  Rationale: the generic value-operation callback is already the economical typed interface, but
  revision 30 has no semantic ID that can request LLVM's match-any result without approximation.
  Date/author: 2026-08-31, Codex.
- Decision: expose the same bounded i32/u32/f32 scalar family already established for match-all.
  Rationale: all three map to the exact b32 intrinsic; signedness does not alter bit matching and
  float32 uses an explicit bitcast. Wider values and aggregates remain separate contracts.
  Date/author: 2026-08-31, Codex.

## Outcomes and Retrospective

Direct NVVM now maps canonical scalar `IRWaveMaskMatch` to one typed revision-31 value operation.
The LLVM 14 provider emits `llvm.nvvm.match.any.sync.i32`, and generated PTX contains
`match.any.sync.b32` at both O0 and O3. Validation proves an unsigned-i32 result and mask plus one
selected signed-i32, unsigned-i32, or float32 scalar; adjacent wave shapes remain rejected.

Frozen corpus v1 remains exactly 452 workloads/427 healthy references and improves from
391/395/391 to 394/398/394 O0/O3/both-mode correct. The three selected rows are the only gains and
there are zero old-correct losses. Discovery remains exactly 82/72 at 64/64/64 with zero loss. The
selected prefix passes 432/432.

All 24 representative direct-O3 gates assemble with CUDA 12.9 for SM70, SM80, and SM90. The active
mask switch gate measures 247.9 ms and 1924-byte PTX at direct O3 SM70 versus 353.9 ms and 9999
bytes through NVRTC O3; direct O0 measures 247.7 ms and emits 5420-byte PTX. The no-default variant
measures 236.3 ms and 1707 bytes versus 351.3 ms and 9787 bytes. The functional reconvergence gate
measures 243.1 ms and 1937 bytes versus 363.2 ms and 10831 bytes. These timings remain exploratory.

## Context and Current Pipeline

The direct emitter already lowers lane index/count, active mask, lane reads, first-lane queries,
any/all/equality votes, masked scalar helpers, and recursive aggregate wave recipes. Preflight
currently rejects `IRWaveMaskMatch` in `_validateNVVMFunction` before provider mutation. The three
selected rows are:

- `hlsl-intrinsic/active-mask/switch-no-default.slang#cuda-1`;
- `hlsl-intrinsic/active-mask/switch.slang#cuda-1`;
- `language-feature/execution-model/wave-switch-divergence-functional.slang#cuda-1`.

The audit must determine the operand/result type families and whether the operation means a native
match-any mask, a match-all result, or a compiler-generated reconvergence primitive. That semantic
answer controls whether an existing ballot/read/compare recipe is correct.

## Scope and Non-Goals

In scope are exact final `IRWaveMaskMatch` instructions used by the three selected workloads;
supported scalar input and mask result types proved by final IR; preflight capability collection;
typed emission; focused fake/provider coverage; permanent O0/O3 lanes after differential
correctness; both exact corpus snapshots; representative measurements; and durable documentation.

Out of scope are generic assembly parsing, source-text reconstruction, arbitrary advanced wave
operations, assumptions about fixture names, divergent-control-flow repair, new provider callbacks
without a concrete expressibility gap, compatibility fallbacks, malformed upstream IR patches,
corpus-v2 activation, and changes to the frozen-v1 denominator.

## Architecture and Invariants

- The exact canonical producer and operand/result type relation own admission.
- Any software recipe must have the same active-lane and reconvergence semantics as the canonical
  operation; ordinary full-warp assumptions are invalid.
- Preflight queries every generic builder operation used by a recipe before provider creation.
- Unsupported adjacent wave shapes retain deterministic diagnostics.
- Provider ABI revision 31 carries one appended semantic operation ID through the existing generic
  value-operation callback; no interface-table layout or callback changes.

## Interfaces and Dependencies

Expected compiler work is in `source/slang/slang-emit-nvvm.cpp` plus focused fake-emitter sources
and tests. A provider or ABI change is conditional on the final semantic/expressibility audit, not
assumed. Real validation uses the existing CUDA 12.9/libNVVM installation and Release test tools.

## Milestones

1. Trace source intrinsic through the final linked `IRWaveMaskMatch` instruction and record exact
   operand/result types and control-flow context for all three rows.
2. Compare that contract with existing direct wave resolvers and generic typed operations; select
   the smallest reusable semantic recipe or prove a provider interface gap.
3. Add exact validation/emission and focused positive/negative coverage, then probe all targets at
   O0/O3 and record any independent cascade.
4. Promote only correct rows, run the full validation matrix, refresh artifacts, and complete the
   input-shape audit.

## Validation and Acceptance

All CMake builds and tests run outside the sandbox with Windows-native tools. Acceptance requires
focused fake coverage; O0/O3 differential correctness for every promoted target; zero old-correct
regression; the selected NVVM prefix; frozen identity 452/427; discovery identity 82/72;
direct-O3 PTX assembly for all established and any new gates at SM70, SM80, and SM90; explicit
provider ABI revision-31 evidence; formatting attempt; `git diff --check`; JSON/TSV integrity; and an exact
staged-file audit excluding `external/slang-binaries/`.

## Failure and Recovery

If the first admitted operation exposes a distinct wave, control-flow, or ABI failure, retain only
independently proved support and do not count that workload as unlocked. If the only correct
lowering requires an absent provider primitive, stop at that concrete boundary and document the
required typed contract rather than parsing generic assembly or approximating active-lane
semantics. Raw IR/PTX and probe logs remain under ignored `build/nvvm-census` paths.

## Artifacts and Hand-Off

Retain the completed plan with implementation under the user's established experimental-workflow
exception. Keep refreshed frozen/discovery TSV and Pareto JSON, measurement manifest, five-part
report, promoted lanes, and design/ledger updates. Raw dumps and logs stay under `build/`.
