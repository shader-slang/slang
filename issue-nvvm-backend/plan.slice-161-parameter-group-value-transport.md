# Transport parameter-group pointers as canonical resource values

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires the completed slice plan to be committed with this experimental backend.

## Purpose and Observable Result

After this slice, a selected `ParameterBlock<T>` or `ConstantBuffer<T>` is one pointer-valued leaf
in the direct-NVVM resource value algebra. The same exact pointer representation can appear as a
raw CUDA entry parameter and as a field of a finite helper aggregate. The bounded targets are
discovery `generic-shader-object-cbuffer2` and `nested-parameter-block-3`; both must execute
correctly at direct O0 and O3 before promotion.

## Progress

- [x] (2026-08-31) Ranked Slice 160 cross-corpus failures and isolated the two parameter-group
  value workloads from the adjacent immutable-reference and exotic helper ABI shapes.
- [x] (2026-08-31) Audited the exact entry and nested-helper parameter-group producers and
  recursive layout contract.
- [x] (2026-08-31) Extended the existing resource value classifier and role lowering without
  changing provider ABI revision 30.
- [x] (2026-08-31) Built, probed both targets at O0/O3, recorded cascades, and promoted the two
  stable correct rows.
- [x] (2026-08-31) Ran the selected prefix, both exact corpora, representative measurements,
  formatting attempt, integrity checks, and self-review.
- [x] (2026-08-31) Completed durable documentation and the five-part report for the Slice 161
  commit.

## Surprises and Discoveries

- Slice 149 already selected a loaded `ParameterBlock<T>` or `ConstantBuffer<T>` as an immutable
  global pointer to `T` when storage and ordinary value representations agree. Slice 160 still
  excludes that pointer-valued wrapper as a raw entry parameter and as a leaf inside a finite
  helper struct.
- `nested-parameter-block-3` has a helper parameter `Scene` containing a nested
  `ParameterBlock<MaterialSystem>`. It is not the same shape as discovery
  `array-storage-lowering`, whose helper parameter is an immutable
  `BorrowInParam<Params_natural>` pointer. The latter remains outside this slice.
- After parameter-group type admission, `nested-parameter-block-3` first exposed the exact
  `fieldExtract` producer for a parameter-group pointer inside a first-class helper aggregate.
  `generic-shader-object-cbuffer2` exposed the corresponding raw entry `IRParam` producer.
- The generic entry workload then reached the old `CUDA kernel decoration` preflight gate. The
  canonical CUDA source path treats an ordinary compute `IREntryPointDecoration` as kernel
  identity, and direct NVVM already marks the selected entry as a kernel. Explicit
  `IRCudaKernelDecoration` identifies only source `[CUDAKernel]`; requiring it rejected an
  ordinary `[numthreads]` entry after the real launch ABI had become representable.

## Decision Log

- Decision: treat a selected parameter group as a pointer-valued resource leaf rather than
  flattening its element or copying parameter-group storage into a helper value.
  Rationale: `_lowerParameterGroupType` already defines the canonical provider representation as
  an address-space-1 pointer to recursively validated storage.
  Date/author: 2026-08-31, Codex.
- Decision: keep immutable aggregate references and provider address-space casts out of this slice.
  Rationale: their canonical pointer type and caller provenance require a separate ABI decision;
  accepting them as ordinary parameter-group values would merge distinct contracts.
  Date/author: 2026-08-31, Codex.
- Decision: recognize exactly `IRParam`, a typed `IRFieldExtract`, and the already-supported
  conventional-global `IRLoad(IRFieldAddress)` as parameter-group pointer producers.
  Rationale: these are the three canonical producers observed at entry, helper-value, and
  collected-global boundaries. Arbitrary values of parameter-group type remain rejected.
  Date/author: 2026-08-31, Codex.
- Decision: remove the explicit CUDA-kernel-decoration gate and make ordinary compute entry
  coverage positive.
  Rationale: the selected entry-point decoration is the canonical producer-side kernel contract;
  the CUDA source emitter and direct NVVM kernel marking already consume it. Keeping a second
  source-attribute requirement was an obsolete bring-up restriction, not an ABI invariant.
  Date/author: 2026-08-31, Codex.

## Outcomes and Retrospective

Both selected discovery workloads are correct at direct O0 and O3 and gain four permanent lanes.
Discovery remains exactly 82 workloads/72 healthy references and improves from 61/61/61 to
63/63/63 O0/O3/both-mode correctness, with zero old-correct loss. Each direct mode now has 63
correct, nine preflight, two provider, seven infrastructure, and one runtime mismatch.

Frozen corpus v1 remains exactly 452 workloads/427 healthy references and unchanged at
390/394/390 O0/O3/both-mode correctness, with zero old-correct loss. Across all frozen rows,
native CUDA is 449 correct/three infrastructure; direct O0 is 403 correct, 36 preflight, eight
runtime mismatch, and five provider; direct O3 is 408 correct, 36 preflight, and eight runtime
mismatch. The selected prefix passes 427/427.

All nineteen representative direct-O3 gates assemble with CUDA 12.9 for SM70, SM80, and SM90.
The new nested parameter-group method gate measures 242.5 ms and 1093-byte PTX at direct O3 SM70
versus 361.6 ms and 9096 bytes through NVRTC O3; direct O0 measures 237.1 ms and emits 4513-byte
PTX. The generic parameter-group entry gate measures 237.9 ms and 847-byte PTX versus 357.8 ms and
8897 bytes; direct O0 measures 235.3 ms and emits 4592-byte PTX. These measurements remain
exploratory.

The repository formatter was attempted with `--modified`, but this machine does not provide
gersemi, clang-format, prettier, or shfmt. Manual review, `git diff --check`, JSON parsing, exact
TSV identity/count checks, and measurement completeness checks pass.

The implementation needed no provider callback or ABI revision. A parameter group remains one
global pointer value; admission proves recursively that its pointee has identical storage and
ordinary value representations. Exact producer recognition preserves that value at the raw entry,
nested aggregate, and conventional-global boundaries. The separate explicit CUDA-kernel
decoration gate was removed after its producer audit showed ordinary entry-point decoration is the
canonical kernel identity.

## Context and Current Pipeline

`asNVVMSupportedParameterGroupType` accepts only `ParameterBlock<T>` and `ConstantBuffer<T>` whose
element is established aggregate storage. `_lowerParameterGroupType` recursively lowers `T` in
parameter-group storage role and constructs a global pointer. Conventional global fields and
loaded parameter-group SSA values already use that representation.

`asNVVMSupportedResourceStructType` recursively classifies numeric, pointer, raw-buffer, surface,
texture, sampler, descriptor, atomic, and nested-struct leaves, but not a parameter-group leaf.
`isNVVMSupportedParameterType` likewise excludes a top-level parameter group. These two omissions
produce the selected failures before provider mutation.

## Scope and Non-Goals

In scope are selected parameter-group values at raw entry parameters; parameter groups nested in a
finite resource struct; recursive size/alignment and storage/value-identity validation; existing
generic pointer/function/aggregate operations; the two discovery targets; permanent lanes after
differential correctness; both corpus snapshots; measurements; and durable documentation.

Out of scope are `BorrowInParam<T>`, `RefParam<T>`, pointer-to-pointer helpers, parameter groups
whose storage and value representations differ, recursive parameter-group graphs that the literal
struct provider cannot represent, resource arrays, address-space casts, new provider callbacks,
provider ABI changes, fixture-name checks, source reconstruction, fallbacks, and corpus-v2
activation.

## Architecture and Invariants

- A selected parameter-group value is exactly a global pointer to its canonical element storage.
- The element's parameter-group storage representation must be recursively finite and compatible
  with its ordinary loaded value before the pointer can participate in helper aggregate transport.
- A raw entry parameter group uses that pointer value directly; it is not an aggregate `byval`
  parameter and does not use Slice 160's physical/semantic split.
- A parameter-group leaf inside a helper struct remains an LLVM pointer field in the first-class
  struct. The helper struct itself follows the existing generic aggregate ABI.
- Provider ABI revision 30 remains unchanged.
- Frozen corpus v1 remains exactly 452/427 and discovery exactly 82/72, reported separately with
  zero old-correct regression required.

## Interfaces and Dependencies

Production work is expected in `source/slang/slang-emit-nvvm-type-lowering.cpp` and may require
exact validation/emission changes in `source/slang/slang-emit-nvvm.cpp`. Focused tests may extend
the existing NVVM unit support only if its fake type model can express the canonical pointer leaf
without introducing a parallel representation. Correct workloads gain direct O0/O3 directives.
Census, Pareto, measurement, design, ledger, plan, and report artifacts follow the existing Slice
160 locations and conventions.

## Milestones

1. Establish one cycle-safe parameter-group resource-leaf predicate that proves element storage is
   finite and has the selected loaded-value identity.
2. Reuse that predicate in resource-struct recursion, entry-parameter legality, and type lowering.
   Keep the direct pointer role separate from aggregate `byval` handling.
3. Build and probe both targets. Record the first independent cascade rather than widening into
   immutable references, arrays, or provider casts.
4. Promote correct targets, run the prefix and complete corpora, refresh Pareto/measurement
   evidence, and complete the input-shape/self-review audit.

## Validation and Acceptance

All CMake builds and tests run outside the sandbox with Windows-native tools. Acceptance requires
O0/O3 differential correctness for every promoted target; zero old-correct regression; the
selected 427-test prefix; unchanged exact corpus identities; representative PTX assembly for
SM70, SM80, and SM90; provider ABI revision 30; formatting attempt; `git diff --check`; JSON/TSV
integrity; and an exact staged-file audit excluding `external/slang-binaries/`.

## Failure and Recovery

If a target reaches an immutable reference, address-space conversion, or unrelated operation,
retain only a representation proved by another target and record the cascade. If storage and value
representations differ, stop at typed preflight and plan producer-side legalization; do not insert
textual IR patches or flatten the semantic wrapper. Generated probes remain under ignored
`build/` paths.

## Artifacts and Hand-Off

Commit this completed plan with the implementation because the user explicitly requires it.
Retain Slice 161 frozen/discovery TSV and Pareto JSON, any refreshed measurement manifest, the
five-part report, promoted lanes, and design/ledger updates. Raw logs remain under `build/`.
