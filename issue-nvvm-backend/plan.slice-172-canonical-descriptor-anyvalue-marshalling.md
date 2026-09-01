# Canonical descriptor-handle AnyValue marshalling

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires each completed experimental-backend slice plan to be committed with its implementation;
the completed plan therefore follows that established exception to the active working-log policy.

## Purpose and Observable Result

After this slice, an interface payload containing a structured-buffer `DescriptorHandle` has one
canonical CUDA representation while it is packed into and reconstructed from `AnyValue`. Native
CUDA and direct NVVM must agree on both the payload layout and the recovered resource value.

The bounded primary probes are frozen
`language-feature/dynamic-dispatch/layout-descriptor-handle-array`,
`language-feature/dynamic-dispatch/layout-descriptor-handle-dispatch`, and
`language-feature/dynamic-dispatch/layout-descriptor-handle-multi`. All three currently compile in
direct O0 and O3 but produce a runtime mismatch. Existing descriptor-handle, dynamic-dispatch, and
AnyValue lanes are regression gates. Discovery rows are measured exactly but are not speculative
feature targets for this slice.

## Progress

- [x] (2026-09-01) Completed and committed Slice 171 as `244893c1e`; frozen v1 advances to
  409/409/409 over 427 and discovery remains 69/69/69 over 72.
- [x] (2026-09-01) Re-ranked both healthy-failure sets. The three descriptor-handle dynamic-
  dispatch mismatches are the largest coherent in-MVP frozen cluster.
- [x] (2026-09-01) Preserved final IR, layout, provider IR/PTX, and runtime evidence for all three
  probes in O0 and O3.
- [x] (2026-09-01) Identified AnyValue descriptor marshalling as the exact canonical producer and
  `lowerBitCast` scalar extraction of one opaque 16-byte leaf as the first divergence.
- [x] (2026-09-01) Implemented one reusable compiler-side marshalling invariant without widening
  direct-NVVM admission or revising provider ABI revision 32 unless a concrete operation gap is
  proven.
- [x] (2026-09-01) Carried all three bounded probes to correctness and promoted six stable exact
  O0/O3 differential lanes.
- [x] (2026-09-01) Regenerated both exact corpora and measurements; documented, validated,
  self-reviewed, and prepared Slice 172 for commit.

## Surprises and Discoveries

- Slice 154 established that CUDA `DescriptorHandle<T>` uses the same typed provider
  representation as `T`; a structured-buffer handle is therefore a 16-byte `{data, count}` value,
  not an integer token.
- The three probes have advanced from their earlier AnyValue extraction stop to compiled runtime
  mismatches. Admission is no longer the question: the producer-to-consumer byte representation
  must be traced and compared.
- The preserved logs still contained the compiler abort `Unsupported value size`; the census
  labeled it runtime-mismatch because the compare-compute output contract also failed. Deferring
  the exact bit cast converted that abort into the deterministic preflight shape
  `DescriptorHandle -> vector<uint,4>` and exposed the correct target-owned legalization boundary.
- Existing eight-byte texture/sampler descriptor payloads also use AnyValue. The common pass
  exception must therefore remain exact to unsigned four-lane, 16-byte transport; deferring every
  descriptor bit cast would regress already-supported handles.

## Decision Log

- Decision: prioritize the three shared descriptor-handle payload mismatches ahead of isolated
  arithmetic, FP8/BFloat16, or out-of-MVP gaps.
  Rationale: buffers and aggregate/interface transport are explicit usable-compute MVP
  requirements, and one invariant can unlock three real combined workloads.
  Date/author: 2026-09-01, Codex.
- Decision: keep provider ABI revision 32 unless the canonical payload operation cannot be
  expressed through existing typed aggregate, bit, load/store, and cast operations.
  Rationale: current evidence identifies an upstream AnyValue representation issue rather than a
  missing LLVM provider primitive.
  Date/author: 2026-09-01, Codex.
- Decision: preserve only the exact direct 16-byte descriptor/`uint4` bit cast through common
  aggregate-bitcast lowering, then classify a supported raw-buffer resource at preflight.
  Rationale: the common producer owns the semantic payload spelling, while direct type lowering is
  the first layer that knows the opaque handle is `{global T*, uint64 count}`. Existing eight-byte
  handle paths and unsupported resources must not be widened.
  Date/author: 2026-09-01, Codex.
- Decision: express both transport directions as compiler-side recipes over revision-32 generic
  aggregate, vector, pointer-bit, conversion, shift, and bitwise operations.
  Rationale: no canonical operation is missing from the provider; adding a callback would duplicate
  a representation the compiler already classifies exactly.
  Date/author: 2026-09-01, Codex.

## Outcomes and Retrospective

Slice 172 unlocks all three frozen descriptor-layout workloads in O0 and O3 and promotes six
permanent lanes. Frozen v1 remains exactly 452/427 and advances from 409/409/409 to 412/412/412,
with exactly three gains and zero old-correct regression. Discovery remains exactly 82/72 and
69/69/69 with no changed row.

The selected prefix passes 433/433 and the permanent `nvvm` category passes 72/72. All three
measurement gates compile and assemble with CUDA 12.9 for native NVRTC, direct O0 SM70, and direct
O3 SM70/SM80/SM90. Provider ABI revision 32 remains unchanged.

## Context and Current Pipeline

Dynamic-dispatch lowering stores concrete interface values in `AnyValue`. AnyValue marshalling
uses target layout and extraction helpers to flatten values into fixed-width integer storage and
later reconstruct them. CUDA descriptor-handle lowering represents a handle as its underlying
resource type; direct NVVM independently lowers that resource to its typed provider aggregate.
The slice must establish where the producer loses or changes that canonical representation before
the provider sees it.

## Scope and Non-Goals

In scope are structured-buffer descriptor handles inside finite AnyValue payloads, arrays and
struct fields that use the same canonical path, the three bounded frozen probes, existing generic
provider operations, exact corpus regeneration, and bounded measurement gates.

Out of scope are fixture-name checks, downstream direct-NVVM patches for malformed IR, arbitrary
resource serialization, syntax reconstruction, compatibility fallbacks, diagnostic weakening,
FP8/BFloat16, unrelated AnyValue runtime mismatches, provider callbacks without a demonstrated
operation gap, and external workloads.

## Architecture and Invariants

- CUDA target layout and the descriptor-handle representation producer jointly define the payload
  byte layout; marshalling and reconstruction must consume that same representation.
- A descriptor handle must not acquire a second target-specific spelling merely because it crosses
  AnyValue storage.
- Packing and unpacking are inverse operations for every retained finite leaf shape.
- Aggregate members are identified by semantic structure and canonical offsets, never fixture or
  declaration-name checks.
- Unsupported shapes stop deterministically at their owning producer boundary before provider
  mutation.

## Interfaces and Dependencies

AnyValue and leaf extraction logic lives in `source/slang/slang-ir-any-value-marshalling.cpp` and
`source/slang/slang-ir-extract-value-from-type.cpp`. Descriptor-handle representation and direct
NVVM type/value lowering live in the existing CUDA legalization and `slang-emit-nvvm*` paths.
Existing revision-32 provider aggregates, bit operations, casts, loads, and stores are preferred.

## Milestones

1. Capture O0/O3 final IR, provider IR/PTX, layouts, and differential outputs for all three probes.
2. Trace the exact producer, marshalling operations, direct type lowering, and reconstruction.
3. Implement the smallest shared producer-side representation and retain strict adjacent gates.
4. Promote exact successes; run build, focused tests, selected prefix, permanent category, both
   exact corpora, and SM70/SM80/SM90 measurements.
5. Update design, ledger, five-part report, and plan; format, audit, stage exactly the slice files
   excluding `external/slang-binaries/`, and commit.

## Validation and Acceptance

All builds/tests run outside the sandbox with Windows-native tools and the isolated Release
provider. Acceptance requires exact corpus identities 452/427 and 82/72; O0/O3 differential
results; zero old-correct regression; selected-prefix and permanent-category success; retained
diagnostic ownership for unsuccessful probes; PTX assembly for promoted gates; formatting;
artifact integrity; and an exact staged-file audit.

## Failure and Recovery

If the three runtime mismatches do not share one canonical producer/type/operation shape, split
the cluster and retain each honest result. If the required representation cannot be expressed by
revision 32, document the exact operation gap before changing the provider. Never patch emitted
LLVM text, reinterpret a launch/payload block with incompatible offsets, or add fixture checks.

## Artifacts and Hand-Off

Keep dumps, PTX, and logs under ignored `build/nvvm-census` paths. Retain the completed plan only
with a committed result under the user's workflow exception. Distill durable AnyValue/resource
representation rules into `docs/design/nvvm-backend.md`, exact status into the capability ledger
and separate corpus artifacts, and every producer/input-shape decision into the Slice 172
five-part report.
