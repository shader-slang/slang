# Recursive helper-pointer transport

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires each completed experimental-backend slice plan to be committed with its implementation;
the completed plan therefore follows that established exception to the active working-log policy.

## Purpose and Observable Result

After this slice, helper calls use one recursive pointer/value algebra for canonical CUDA
`UserPointer` leaves and generic local storage. A copyable aggregate passed through `out` or
`inout` must validate through the already-supported local pointer path, and a helper parameter may
contain more than one finite level of typed user-pointer indirection when every pointee is itself a
supported helper value.

The bounded primary probes are frozen
`language-feature/dynamic-dispatch/return-interface-from-dispatch` and discovery
`language-feature/dynamic-dispatch/ptr-to-interface-double-indirect`. Existing copyable-value,
resource, shared-memory, and helper-call lanes are regression gates. The unrelated discovery
`optimization/arrray-storage-lowering` constant-buffer field-address failure is not widened by this
slice.

## Progress

- [x] (2026-09-01) Completed and committed Slice 172 as `c77a08792`; frozen v1 advances to
  412/412/412 over 427 and discovery remains 69/69/69 over 72.
- [x] (2026-09-01) Re-ranked both healthy-failure sets and traced the three apparent helper/ABI
  candidates through their final canonical linked IR.
- [x] (2026-09-01) Split discovery `arrray-storage-lowering` because its first failure is the
  unrelated `Ptr<cbuffer<Params_natural>, ..., ScalarLayout>` field-address shape.
- [x] (2026-09-01) Identified the two exact helper-pointer omissions shared by the retained frozen
  and discovery probes.
- [x] (2026-09-01) Implemented the recursive helper-pointer representation and complete local-copyable pointer
  validation without changing provider ABI revision 32.
- [x] (2026-09-01) Carried both bounded probes to O0/O3 differential correctness and promoted useful permanent
  regression lanes.
- [x] (2026-09-01) Regenerated both exact corpora and measurements; documented, validated, self-reviewed, and prepared
  Slice 173.

## Surprises and Discoveries

- The frozen existential workload does not require a new existential representation. Dynamic-
  dispatch lowering already produces a copyable `{uint tag, AnyValue4 payload}` tuple and exact
  `OutParam<Tuple>` / `BorrowInOutParam<Tuple>` helper parameters. Call validation recognizes the
  local copyable pointer, but `_validatePointerValue` only repeats its numeric and array subsets,
  so the supported aggregate stops as `producer=param, consumer=call`.
- Discovery double indirection lowers `IFoo*` to the same canonical `Ptr<Tuple, UserPointer,
  DefaultLayout>` used by the global parameter. `__getAddress(localPtr)` produces a generic local
  `Ptr<Ptr<Tuple>>`, while `dispatchViaDoublePtr` receives the complete CUDA
  `Ptr<Ptr<Tuple>, UserPointer, DefaultLayout>`. The helper-value grammar currently accepts the
  inner pointer only because `Tuple` is copyable; it does not recurse through the outer pointer.
- The census runner intentionally uses the Release `slang-test`, while initial compile checks
  rebuilt Debug targets. Rebuilding the exact Release runner made the focused probes exercise the
  implementation; the unchanged Debug-run diagnostics were a build-target mismatch, not evidence
  against the representation.
- Standalone measurement initially lacked the type conformances supplied by `TEST_INPUT`. The
  harness now accepts validated optional compiler arguments, allowing dynamic-dispatch gates to
  preserve their canonical linkage without source or fixture inference.

## Decision Log

- Decision: treat these failures as one recursive helper-pointer transport slice, while preserving
  their two distinct validation sites.
  Rationale: both are canonical products of helper ABI lowering and both require the same rule:
  finite supported pointee values retain exact typed pointers across local and helper boundaries.
  Date/author: 2026-09-01, Codex.
- Decision: add a recursive device-helper-pointer classifier rather than declaring pointers
  copyable or adding existential-specific cases.
  Rationale: a pointer is a valid helper leaf but is not a byte-copyable aggregate under the
  existing storage algebra. Keeping those concepts separate preserves launch/storage layout
  invariants and supports arbitrary finite pointer depth without fixture knowledge.
  Date/author: 2026-09-01, Codex.
- Decision: keep provider ABI revision 32.
  Rationale: existing typed pointer construction, load, store, call, and address-space operations
  express the complete canonical operation graph; the gap is compiler-side classification.
  Date/author: 2026-09-01, Codex.

## Outcomes and Retrospective

Slice 173 unlocks one frozen and one discovery workload in both modes and promotes four permanent
lanes. Frozen v1 remains exactly 452/427 and advances from 412/412/412 to 413/413/413, with one
gain and zero old-correct regression. Discovery remains exactly 82/72 and advances from
69/69/69 to 70/70/70, also with one gain and zero loss.

The selected prefix passes 433/433 and the permanent `nvvm` category passes 76/76. Both
measurement gates compile and assemble through CUDA 12.9 for native NVRTC, direct O0 SM70, and
direct O3 SM70/SM80/SM90. Provider ABI revision 32 remains unchanged.

## Context and Current Pipeline

Dynamic-dispatch lowering represents an interface value as a finite tuple and interface pointers
as typed CUDA user pointers to that tuple. Helper type lowering already has separate copyable,
pointer-bearing helper, local-pointer, reference-pointer, shared-pointer, and device-pointer roles.
Preflight validates exact helper signatures, argument relations, and every pointer producer before
the provider module is mutated. Emission then lowers helper values to generic LLVM pointers while
preserving proven global provenance for ordinary memory access.

## Scope and Non-Goals

In scope are finite recursively typed CUDA user pointers whose pointees are helper values, exact
generic local pointers to those values, copyable aggregate `out`/`inout` parameters, the two
bounded corpus probes, adjacent negatives, existing generic provider operations, exact corpus
regeneration, and bounded measurements.

Out of scope are arbitrary raw pointers, recursive/cyclic type graphs, fixture-name checks,
existential syntax reconstruction, constant-buffer field addressing, malformed upstream IR,
compatibility fallbacks, diagnostic weakening, provider callbacks, and external workloads.

## Architecture and Invariants

- A canonical CUDA user pointer is admitted only with the exact four-operand `Ptr`, read-write
  access, `UserPointer` address space, and `DefaultLayout` spelling already produced by CUDA
  legalization.
- Its pointee must recursively belong to the finite helper-value algebra; active-type detection
  rejects cycles deterministically.
- Copyable values remain distinct from pointer-bearing helper values. Aggregate storage and launch
  ABI admission do not expand merely because helper transport expands.
- A generic local pointer and a user-pointer helper parameter may relate only when their exact
  pointee types match and the producer proves the local/global address-space role.
- Unsupported shapes stop before provider mutation and retain producer/type/operation diagnostics.

## Interfaces and Dependencies

Recursive classifiers and typed lowering live in
`source/slang/slang-emit-nvvm-type-lowering.{h,cpp}`. Pointer validation, helper argument matching,
provenance, and emission live in `source/slang/slang-emit-nvvm.cpp`. Focused fake-provider and real
PTX assembly coverage lives in the split NVVM unit-test files. Existing revision-32 generic
provider operations are sufficient.

## Milestones

1. Preserve final canonical IR and diagnostics for both probes and reject the unrelated candidate.
2. Implement one finite recursive device-helper-pointer classifier and reuse it only in helper
   value/signature/call/pointer paths; add the missing local-copyable aggregate pointer validation.
3. Add focused source/fake-provider/real-provider proof and retain adjacent unsupported pointer
   diagnostics.
4. Promote exact successes; run build, focused tests, selected prefix, permanent category, both
   exact corpora, and SM70/SM80/SM90 measurements.
5. Update design, ledger, five-part report, and plan; format, audit, stage exactly the slice files
   excluding `external/slang-binaries/`, and commit.

## Validation and Acceptance

All builds/tests run outside the sandbox with Windows-native tools and the isolated Release
provider. Acceptance requires exact corpus identities 452/427 and 82/72; O0/O3 differential
results; zero old-correct regression; selected-prefix and permanent-category success; retained
adjacent diagnostic ownership; PTX assembly for promoted gates; formatting; artifact integrity;
and an exact staged-file audit.

## Failure and Recovery

If the two probes require incompatible pointer representations, split them and keep only the
independently proven invariant. If a required canonical pointer operation cannot be expressed by
revision 32, stop and record that exact gap before revising the provider. Never treat a pointer as
copyable storage, reconstruct an interface from source syntax, or patch emitted LLVM text.

## Artifacts and Hand-Off

Keep dumps, PTX, and logs under ignored `build/nvvm-census` paths. Retain the completed plan only
with a committed result under the user's workflow exception. Distill durable helper-pointer rules
into `docs/design/nvvm-backend.md`, exact status into the capability ledger and separate corpus
artifacts, and every producer/input-shape decision into the Slice 173 five-part report.
