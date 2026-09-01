# Lower the canonical ordinary Texture2D gather family

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires each completed experimental-backend slice plan to be committed with its implementation,
overriding the normal working-log policy for this branch.

## Purpose and Observable Result

After this slice, the direct NVVM path lowers the complete canonical non-shadow CUDA `Texture2D`
gather family through one typed provider operation. Red, green, blue, and alpha selection share a
component field; Float32, Int32, and UInt32 gathers return four lanes; and the CUDA offset overload
is accepted only after its otherwise-ignored signed `int2` parameter is proven.

The primary frozen workload is `tests/hlsl-intrinsic/texture-2d-gather.slang`, with deterministic
native CUDA output for two red gathers. The broader compile-only workload
`tests/hlsl-intrinsic/texture-2d-gather-element-type.slang` proves scalar, two-lane, and three-lane
texture element declarations, float/signed/unsigned result kinds, two component selectors, and the
offset overload. Corpus v1 and discovery remain separate contracts.

## Progress

- [x] (2026-09-01) Selected ordinary Texture2D gather from the frozen Pareto as a common resource
  operation that exercises one reusable family rather than one fixture spelling.
- [x] (2026-09-01) Traced all accepted spellings and helper parameters to the generated Gather
  extension in `source/slang/hlsl.meta.slang`.
- [x] (2026-09-01) Revised the typed provider ABI, implemented exact compiler classification and
  emission, and added focused fake and real-provider coverage.
- [x] (2026-09-01) Differentially validated the runtime fixture, promoted stable direct lanes,
  replayed both corpora, measured representative targets, formatted, self-reviewed, and documented.

## Surprises and Discoveries

- The CUDA producer deliberately uses `$TR`, the four-component function result, as the
  `tex2Dgather` template argument even when the texture's logical element type has one, two, or
  three lanes. The provider therefore needs the result type, not the declared texture lane count.
- The generated offset overload has the same CUDA assembly as its base overload. Its signed
  `int2` offset remains part of the checked helper ABI but must not become a provider operand.
- Ordinary component names reduce to selectors 0, 1, 2, and 3; both `Gather` and `GatherRed`
  intentionally select zero.
- The shielded host wrapper also owns texture operation ID and operand-count validation. Adding the
  provider operation without updating that contract produced `SLANG_E_INVALID_ARG` before the
  provider callback ran.
- Read-only texture description admitted only one, two, or four lanes. The canonical
  `Texture2D<float3>` gather proved that a resource's declared element shape is broader than the
  lane sets supported by sample/fetch operations.
- PTX `tld4` requires an explicit coordinate-type suffix. LLVM accepts the inline assembly without
  `.f32`, but `ptxas` correctly rejects that incomplete instruction spelling.

## Decision Log

- Decision: add one `GATHER` texture operation plus an explicit component selector in provider ABI
  revision 34.
  Rationale: component is typed operation metadata, while texture and coordinate are runtime
  operands. Four operation IDs would duplicate one semantic and generic value operations cannot
  encode texture access.
  Date/author: 2026-09-01, Codex.
- Decision: accept the base and single-offset non-shadow, non-array Texture2D helper topologies in
  one slice.
  Rationale: one prelude generator owns both; the offset omission is an explicit CUDA producer
  contract, not downstream syntax reconstruction or a compatibility fallback.
  Date/author: 2026-09-01, Codex.

## Outcomes and Retrospective

Provider ABI revision 34 exposes one gather operation and component field. The compiler recognizes
only the exact generated non-shadow Texture2D helpers, validates all semantic parameters, and emits
two runtime operands. The provider serializes all 12 component/result-kind combinations and CUDA
12.9 assembles actual O0/O3 libNVVM output. Focused fake and real-provider tests pass.

The runtime fixture agrees with native CUDA in O0 and O3 and owns two permanent direct lanes; the
compile-only fixture owns two more direct lanes for the wider family. Frozen corpus v1 stays
exactly 452 rows/427 healthy references and advances from 416/416/416 to 417/417/417 O0/O3/both.
Only `texture-2d-gather.slang#cuda-1` changes, and there are no old-correct regressions. All-row
direct totals are 431 correct, three runtime mismatches, and 18 preflight failures in each mode.

Discovery stays exactly 82 rows/72 healthy references at 72/72/72 with no changed row. The selected
prefix passes 437/437 and the permanent `nvvm` category passes 90/90. CUDA 12.9 assembles the native
reference, direct O0 SM70, and direct O3 SM70/SM80/SM90 gates. The one-repetition reference emitted
SM75 PTX in 710.0 ms/8,945 bytes; direct SM70 measured 428.0 ms/6,519 bytes at O0 and
409.9 ms/1,091 bytes at O3.

## Context and Canonical Ownership

Consider `Texture2D<float3> t; float4 v = t.GatherGreen(s, uv);`. The generated extension in
`hlsl.meta.slang` returns `vector<T.Element,4>` and emits exactly
`tex2Dgather<$TR>($0, ($2).x, ($2).y, 1)`. The canonical helper parameters are the texture,
sampler, and Float32 coordinate; the offset overload adds a signed `int2`. Direct-NVVM preflight
owns recognizing this finalized CUDA-prelude helper and proving its complete types. The provider
owns mapping the typed Texture2D gather descriptor to `tld4.{r|g|b|a}.2d.v4.{f32|s32|u32}`.

The compiler must not parse arbitrary source syntax or accept nearby shadow, array, cube, status,
or multi-offset helpers. It records only the canonical runtime operands—the opaque texture handle
and Float32 `float2` coordinate—after validating sampler and optional offset parameters.

## Scope and Non-Goals

In scope are ordinary non-array Texture2D gather selectors 0 through 3, Float32/Int32/UInt32
four-lane results, base and one-offset helper topology, provider ABI revision 34, fake topology,
real LLVM serialization/libNVVM/PTX assembly, fixture promotion, both corpus replays, and
SM70/80/90 measurement.

Out of scope are shadow/comparison gathers, texture arrays, cube textures, status overloads,
multiple offsets, Half or 64-bit elements, source reconstruction, compatibility callbacks,
fixture-name checks, and malformed-IR repair.

## Validation and Acceptance

Acceptance requires Release host/provider builds and tests outside the sandbox; focused fake
classification proving all components and exact adjacent-shape rejection; real-provider IR and
PTX assembly for float/signed/unsigned gathers; deterministic native/direct O0/direct O3 agreement
for the runtime fixture; frozen v1 exactly 452/427 with no old-correct regression; discovery
exactly 82/72; selected-prefix and permanent-category passes; representative PTX assembly for
native, O0 SM70, and O3 SM70/80/90; changed-line clang-format 17; and `git diff --check`.

## Self-Review Inventory

Audit the new resolver, descriptor field, provider branch, emission branch, and test-only helper
sources. For each retained special case, record its exact producer, why the helper is canonical,
which test fails without it, and why this layer owns the transformation. Confirm the offset is
discarded only after exact signed-`int2` validation and no compatibility path remains.

## Artifacts and Recovery

Keep transient IR, logs, PTX, cubins, and timings below ignored `build/nvvm-census/slice179-*`.
Commit the completed plan with implementation, tests, promoted directives, durable docs, census
TSV/JSON, measurement manifest, and five-part report. The changes are isolated to the direct NVVM
route and can be reverted without changing NVRTC.
