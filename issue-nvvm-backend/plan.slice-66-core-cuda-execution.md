# Slice 66: Add the core CUDA execution bundle

This ExecPlan follows `.agent/PLANS.md`. Keep it current. The user requires the completed plan to
ship with the slice, overriding the repository's default working-log policy for this experiment.

## Purpose and Observable Result

After this slice, direct NVVM supports the complete ordinary compute-index bundle needed by useful
multi-block kernels: thread, block, block-dimension, and grid-dimension components plus the
established synchronization primitive selected by canonical Slang CUDA lowering. A representative
grid-stride kernel agrees with NVRTC through compile, `ptxas`, and RTX execution.

## Progress

- [x] (2026-08-28) Prioritized core execution ahead of additional isolated wave operations because
  it unlocks representative workloads and shared-memory work.
- [x] (2026-08-28) Audited canonical final IR and NVVM intrinsic contracts for all
  index/dimension components and synchronization.
- [x] (2026-08-28) Extended construction V4 to version 2 and added the complete typed execution
  and barrier catalog/provider/fake family while preserving V4 version 1 and frozen V3.
- [x] (2026-08-28) Added family-level invalid/capability/topology, differential PTX, `ptxas`, and
  288-invocation RTX runtime evidence.
- [x] (2026-08-28) Validated Release 425/425, Debug preservation 11/11, compatible
  assembly/`ptxas`/RTX lanes, formatting, and the final input-shape audit.

## Surprises and Discoveries

- Canonical CUDA lowering uses zero-parameter `uint3` helper functions with exact GenericAsm
  `(threadIdx)`, `(blockIdx)`, `(blockDim)`, and `(gridDim)`. Consumers are ordinary one-element
  `kIROp_Swizzle` with a constant axis 0 through 2. `GroupMemoryBarrierWithGroupSync` is a
  zero-parameter `void` helper with exact GenericAsm `__syncthreads()`.
- LLVM 14.0.6 maps those four vectors to the twelve scalar NVVM intrinsics `tid`, `ctaid`, `ntid`,
  and `nctaid`, each with x/y/z suffixes. It declares them with exactly `nounwind readnone
  speculatable`, unlike lane-id/warp-size's six-attribute set. `llvm.nvvm.barrier0()` is exactly
  `convergent nounwind`.
- The first runtime fixture's natural UInt index calculation stopped honestly at E52017 because
  unsigned arithmetic remains outside the current subset. The accepted fixture uses the existing
  signed relaxed atomic add to assign unique records, then verifies the unordered complete
  coordinate set. No unsigned arithmetic was added early or hidden with a cast.
- A V4 construction-version-1 builder is successfully discovered; the new helper signature then
  fails as E52018 `extended function construction` before module creation. E52016 remains the
  discovery/legacy-feature incompatibility diagnostic.
- The first full preservation run found that reusing construction version 1's call/return slots for
  vectors and void also changed the synthesized frozen integer facade. Version 2 therefore appends
  extended call/return callbacks; the inherited callbacks retain their scalar-only contract.

## Decision Log

- Decision: treat all x/y/z execution indices and dimensions as one family after the first existing
  wave-register proof.
  Rationale: their provider transport, type, and validation shape is repeated; splitting one
  component per slice would add ceremony without reducing architectural risk.
  Date/author: 2026-08-28, Codex.

- Decision: include synchronization only if the canonical operation and memory/convergence contract
  can be settled without shared-memory representation work.
  Rationale: the primitive belongs to core execution, but a producer-side storage issue must not be
  hidden in this slice.
  Date/author: 2026-08-28, Codex. Revisit after the initial IR trace.

- Decision: encode each whole UInt3 execution semantic as one zero-operand V4 catalog row and use
  ordinary fixed-vector extraction for x/y/z.
  Rationale: the source and final IR produce one vector value; one operation per axis would duplicate
  semantics and make the ABI scale with source syntax rather than the canonical value.
  Date/author: 2026-08-28, Codex.

- Decision: append vector construction/extraction and extended call/return callbacks as construction
  interface version 2 while retaining the exact version-1 table and inherited scalar callbacks.
  Rationale: vector types and element extraction are structural operations with distinct ownership
  rules; extended value transport must not redefine the frozen scalar prefix, and the execution
  meanings stay in the typed value-operation catalog.
  Date/author: 2026-08-28, Codex.

- Decision: include the canonical group-sync barrier in this slice.
  Rationale: its final-IR producer and zero-argument convergent NVVM intrinsic are exact and require
  no shared-storage representation. Shared allocation and general memory-order policy remain Slice
  67 work.
  Date/author: 2026-08-28, Codex.

## Outcomes and Retrospective

The implemented inventory is four zero-operand UInt3 operations, one zero-operand Void barrier,
fixed-vector type construction, and constant element extraction. CUDA 12.9 direct NVVM and NVRTC
both emitted every `%tid`, `%ctaid`, `%ntid`, and `%nctaid` x/y/z component plus `bar.sync`; matching
root `ptxas` accepted both. On the RTX 5090, a `3 x 2 x 2` grid of `4 x 3 x 2` blocks produced all
288 unique thread/block coordinate records with exact block/grid dimensions through both routes.
The complete Release prefix passes 425/425 and its sorted LF-terminated registered-name ledger has
SHA-256 `641fcaf6a0da63e30a6146beb3e46e261d58297299aa33d180a1f86d73e4f0e5`;
removing the six Slice 66 names reproduces Slice 65's 419-name hash exactly. The rebuilt Debug host
passes the 11-test preservation sample and the final post-format V4 ABI check. The six family-level
names increase the five measured test/support files from 28,396 to 29,241 physical lines; this is
the complete execution/vector/barrier family setup rather than per-component slice duplication.

## Context and Current Pipeline

The backend already lowers ordinary scalar structure, global pointer ABI, raw buffer storage,
atomic add, float32, and selected wave intrinsics. It does not yet claim the canonical CUDA thread
and grid execution semantics used by normal indexing code.

## Scope and Non-Goals

In scope are canonical thread/block/grid index and dimension components, their UInt transport,
typed V4 operation rows, synchronized execution only if the audit gate passes, a grid-stride or
multi-dimensional representative workload, exact unsupported/malformed evidence, design, ledger,
and this plan.

Out of scope are shared-memory allocation, cooperative groups, cluster dimensions, dynamic
parallelism, arbitrary barriers/fences/orders, performance claims, and unrelated wave completion.

## Architecture and Invariants

Recognize only canonical target-selected final IR by exact assembly/signature or canonical opcode as
appropriate. One parameterized operation family carries the execution-register semantic and axis;
do not add one ABI callback/feature per component. Provider LLVM intrinsic IDs and objects remain
private. Every call validates arity, type, ownership, insertion point, and supported axis before
mutation.

The representative kernel must use the public Slang operations, enough blocks/threads to exercise
nonzero block indices, and output checks that distinguish thread, block, block-dimension, and
grid-dimension errors rather than merely completing successfully.

## Interfaces and Dependencies

Extend the V4 semantic catalog and provider operation dispatch, final-IR preflight/emission,
fake/family tests, integration/runtime harness, design, ledger, and this plan. Use V3 fallback only
for Slice 63 capabilities; do not extend frozen V3 for new semantics.

## Milestones

1. Trace and freeze canonical source/IR signatures and LLVM/NVVM contracts.
2. Add all index/dimension V4 rows and provider cases with shared validation.
3. Settle or explicitly defer synchronization at the audit gate.
4. Add family invalid/capability/IR/PTX tests and the representative runtime workload.
5. Run focused/full/Debug/`ptxas`/RTX lanes, document, audit, and commit.

## Validation and Acceptance

Run new family tests, complete V4/V3 ABI compatibility, full Release NVVM prefix, Debug
preservation, compatible-assembly verification, CUDA 12.9 `ptxas -v`, and NVVM-vs-NVRTC RTX cases
covering multiple blocks and nontrivial dimensions outside the sandbox.

Accept if every claimed component is independently observable; the provider catalog grows by rows
rather than callbacks/features; unsupported semantics fail before instruction mutation; PTX uses
the expected special-register/barrier families; runtime results agree exactly; established names
continue passing; design/ledger and plan are current; formatting and audit pass.

## Self-Review and Input-Shape Audit

Inventory axis encoding, signature rows, provider intrinsic selection, any synchronization helper,
and test parameterization. Prove the input is canonical and that an axis is a true semantic
parameter, not a helper-name or string-parsing shortcut. Audit convergence/storage ownership before
keeping synchronization logic.

Completed inventory:

- `asNVVMSupportedUInt3Type` and the semantic type matcher survive. They recognize only the exact
  canonical `vector<uint, 3>` produced as the four CUDA helpers' result; they do not introduce a
  second representation or coerce another vector shape.
- Exact GenericAsm catalog rows survive. `hlsl.meta.slang` is the producer and the catalog matches
  the complete helper signature and exact assembly, not helper names or substrings. Each row maps a
  whole vector semantic; axes remain ordinary canonical swizzle constants.
- The swizzle preflight/emission case survives. `kIROp_Swizzle` is the intentional producer shape,
  and the provider's constant element extraction is the correct structural consumer. It accepts
  only UInt result, UInt3 base, one constant index, and range 0 through 2.
- `_getExecutionRegisterIntrinsicIDs` survives at the provider boundary as the one mapping from
  Slang-owned operation IDs to LLVM-owned intrinsic IDs. The reverse declaration audit calls that
  same mapping rather than maintaining a second intrinsic list.
- The barrier case survives. The canonical zero-parameter Void helper maps directly to the
  documented convergent zero-parameter intrinsic. It neither infers memory order nor repairs
  malformed shared storage.
- The legacy-writer attribute cases survive. Normal LLVM 14 assembly proves two genuinely distinct
  declaration sets; exact validation plus counted normalization is owned by the LLVM-14-to-LLVM-7
  wire boundary. Removing it makes CUDA 12.9 reject otherwise valid execution-register assembly.
- The atomic ticket in the runtime fixture is test scaffolding only. It composes two already valid
  source semantics to avoid claiming Slice 68 UInt arithmetic, and host-side set validation proves
  all 288 execution coordinates independently of scheduling order.
- The appended extended call/return callbacks survive the preservation audit. The full suite proved
  that changing the inherited slots altered the frozen integer facade; keeping those slots scalar
  and selecting the appended callbacks only from V4 generic construction restores one contract per
  interface version without a type guess or fallback.

## Failure and Recovery

If final IR is not stable across components, stop and repair/settle the producer representation.
If synchronization needs storage/memory modeling owned by Slice 67, document and defer it rather
than add a special case. Individual catalog rows are independently removable. Never stage
`external/slang-binaries/` or runtime artifacts.

## Artifacts and Hand-Off

Retain source/final-IR traces, intrinsic mapping, normal/compatible assembly, NVVM/NVRTC PTX,
`ptxas -v`, runtime inputs/results, suite hashes/counts, and audit. Commit this completed plan with
Slice 66.
