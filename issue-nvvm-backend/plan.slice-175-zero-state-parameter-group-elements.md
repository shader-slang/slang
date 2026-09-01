# Zero-state parameter-group elements

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires each completed experimental-backend slice plan to be committed with its implementation;
the completed plan therefore follows that established exception to the active working-log policy.

## Purpose and Observable Result

After this slice, an exact `ParameterBlock<T>` or `ConstantBuffer<T>` may retain a finite
zero-state aggregate element behind its pointer-sized handle. The parameter-group-only storage
grammar must represent empty structs and structs whose fields recursively have zero state without
making empty ordinary values, helper aggregates, or conventional-global records generally legal.

The bounded primary probe is discovery `bugs/type-legalize-bug-1`. It must execute correctly
through native CUDA and direct NVVM O0/O3 before promotion. Existing nested parameter-block,
constant-buffer, resource-aggregate, empty-value negative, and dynamic-dispatch workloads are
regression gates.

## Progress

- [x] (2026-09-01) Completed and committed Slice 174 as `6224ee81e`; frozen v1 remains
  413/413/413 over 427 and discovery advances to 71/71/71 over 72.
- [x] (2026-09-01) Re-ranked the remaining discovery Pareto and selected its sole healthy backend
  failure.
- [x] (2026-09-01) Captured the final linked IR, retained layout, and optimized consumers for
  `type-legalize-bug-1`.
- [x] (2026-09-01) Defined a parameter-group-only finite zero-state storage grammar while ordinary
  aggregate, copyable, and helper values remain nonempty.
- [x] (2026-09-01) Carried the bounded probe through every principled downstream cascade without opaque-pointer
  fallback or ordinary empty-aggregate widening.
- [x] (2026-09-01) Promoted stable O0/O3 coverage, regenerated both exact corpora, measured,
  documented, validated, self-reviewed, and prepared the exact Slice 175 commit.

## Surprises and Discoveries

- The general aggregate-storage grammar already recurses through nested parameter groups, so the
  failure is not an omitted `ParameterBlock` opcode.
- Final CUDA legalization produces `GlobalParams { RWStructuredBuffer<int> outputBuffer;
  ParameterBlock<B> gB; }` with size 24, alignment eight, and `gB` at offset 16.
- `B` contains one `A` field and `A` has no data fields. The existing recursive grammar rejects
  `A` because ordinary supported structs are deliberately nonempty, which causes the surrounding
  parameter group classifier to reject its pointer-sized handle.
- Dynamic-dispatch optimization removes the `gB` load and replaces the relevant interface value
  with its known tag. The launch ABI still retains the `gB` handle because global-parameter
  collection owns the complete source parameter layout.
- After storage classification and layout proof were widened, module-scope validation stopped at
  the retained empty `A` declaration. `_addNVVMReachableStructTypes` had already removed the
  parameter-group wrapper and therefore reapplied the ordinary nonempty aggregate rule. Carrying
  the explicit parameter-group role through this canonical declaration closure resolved the same
  representation cascade.

## Decision Log

- Decision: add a parameter-group-element storage grammar that permits finite zero-state structs,
  rather than allowing empty structs in the ordinary copyable/helper/storage algebras.
  Rationale: the pointer-sized parameter-group handle is the externally visible storage leaf, and
  its exact typed pointee can be represented by nested empty LLVM structs. No other value family
  needs to change its nonempty invariant.
  Date/author: 2026-09-01, Codex.
- Decision: keep the typed element representation instead of lowering unsupported pointees to an
  opaque byte pointer.
  Rationale: LLVM 14 supports empty literal structs, the existing provider operation constructs
  them, and retaining the canonical recursive type avoids an opaque compatibility fallback.
  Date/author: 2026-09-01, Codex.
- Decision: keep provider ABI revision 32.
  Rationale: existing zero-field struct, typed global pointer, aggregate storage, field address,
  load, and call operations can express the observed canonical graph.
  Date/author: 2026-09-01, Codex.

## Outcomes and Retrospective

Slice 175 unlocks discovery `bugs/type-legalize-bug-1` in both modes and promotes two permanent
direct lanes. Frozen v1 remains exactly 452 workloads/427 healthy references and 413/413/413,
with zero semantic change and zero old-correct regression. Discovery remains exactly 82/72 and
advances from 71/71/71 to 72/72/72, with exactly one gain and no loss.

The selected prefix passes 433/433 and the permanent `nvvm` category passes 80/80. The
representative gate compiles and assembles through CUDA 12.9 for native NVRTC, direct O0 SM70, and
direct O3 SM70/SM80/SM90. Provider ABI revision 32 remains unchanged.

## Context and Current Pipeline

Global-parameter collection stores nested parameter groups as pointer-sized fields in a synthesized
constant-buffer-backed `GlobalParams` struct. `asNVVMSupportedParameterGroupType` currently admits
the handle only when its element is in the ordinary nonempty aggregate-storage algebra.
`_lowerParameterGroupType` then recursively lowers that element in the parameter-group storage
role and constructs an address-space-one typed pointer. Conventional-global layout validation
compares the provider field layout against retained CUDA offsets and total size.

## Scope and Non-Goals

In scope are exact parameter-block and constant-buffer types, finite empty/nested-empty struct
elements, typed zero-field provider structs, retained pointer-sized global layout, the bounded
discovery workload, adjacent negatives, exact corpus regeneration, and representative
measurements.

Out of scope are empty ordinary helper values, opaque byte-pointer fallback, arbitrary incomplete
types, recursive/cyclic aggregates, source/fixture-name checks, syntax reconstruction, malformed
upstream IR, diagnostic weakening, provider callbacks, and external workloads.

## Architecture and Invariants

- Only a canonical parameter-group element role may treat an empty struct as finite storage.
- Every nonempty field in that grammar must recursively be an already-supported aggregate-storage
  leaf, fixed array, nested parameter group, or finite zero-state struct.
- Active-type detection rejects cycles before provider mutation.
- The provider representation retains the exact struct nesting, including zero-field structs; it
  never substitutes an opaque byte pointee.
- Conventional-global layout must still reproduce the retained pointer offset, pointer alignment,
  and complete `GlobalParams` size.
- Any attempted whole-value load or helper transport must independently satisfy the corresponding
  value/signature grammar; parameter-group handle admission does not imply those operations.

## Interfaces and Dependencies

Parameter-group element classification and type lowering live in
`source/slang/slang-emit-nvvm-type-lowering.{h,cpp}`. Conventional-global layout, field handling,
load validation, and emission live in `source/slang/slang-emit-nvvm.cpp`. The revision-32 provider
already accepts zero-field struct construction and typed pointer creation.

## Milestones

1. Preserve the final `B -> A -> empty` type graph, `GlobalParams` retained layout, and absence or
   presence of parameter-group consumers after optimization.
2. Add one finite parameter-group-element storage classifier and use it only for parameter-group
   recognition and parameter-group-storage type lowering.
3. Add focused positive/negative proof if the repository workload alone cannot prove the boundary;
   carry the primary probe through all newly exposed failures.
4. Promote the stable workload and run build, focused O0/O3 differential tests, selected prefix,
   permanent category, both exact corpora, and SM70/SM80/SM90 measurement.
5. Update design, ledger, five-part report, and this plan; format, audit, stage exactly the slice
   files excluding `external/slang-binaries/`, and commit.

## Validation and Acceptance

All builds/tests run outside the sandbox with Windows-native tools and the isolated Release
provider. Acceptance requires exact corpus identities 452/427 and 82/72; O0/O3 differential
results; zero old-correct regression; selected-prefix and permanent-category success; retained
negative diagnostic ownership; PTX assembly for the promoted gate; changed-line formatting;
artifact integrity; and an exact staged-file audit.

## Failure and Recovery

If the final workload dereferences a zero-state parameter-group element through an operation not
owned by this representation, split that operation and retain only independently proven handle
storage. If the provider rejects exact empty structs, stop and record that concrete ABI gap rather
than substituting an opaque pointer. Never widen ordinary empty aggregates to make the test pass.

## Artifacts and Hand-Off

Keep dumps, PTX, and logs under ignored `build/nvvm-census` paths. Retain the completed plan only
with a committed result under the user's workflow exception. Distill durable representation rules
into `docs/design/nvvm-backend.md`, exact status into the capability ledger and separate corpus
artifacts, and every producer/input-shape decision into the Slice 175 five-part report.
