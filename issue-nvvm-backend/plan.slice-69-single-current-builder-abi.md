# Slice 69: Collapse onto one exact NVVM builder ABI

This ExecPlan follows `.agent/PLANS.md`. Keep it current. The user requires the completed plan to
ship with the slice, overriding the repository's default working-log policy for this experiment.

## Purpose and Observable Result

After this slice, Slang and `slang-llvm-nvvm` share one forward-only builder ABI. Loading performs
one exact revision handshake and then uses one foundation, construction, and value-operation
interface. No historical API export, structure prefix, interface version, or fallback remains.

## Progress

- [x] (2026-08-28) Audited V1-V4 exports, loader fallbacks, facade adapters, provider tables, fake
  libraries, and compatibility tests.
- [x] (2026-08-28) Replaced the versioned public ABI with one exact current contract and revision
  handshake.
- [x] (2026-08-28) Routed the facade directly through the current subinterfaces and removed
  compatibility state.
- [x] (2026-08-28) Consolidated fake/provider negotiation tests around exact success and fail-fast
  mismatch.
- [x] (2026-08-28) Rebuilt both provider and host, formatted changed C/C++ files, and passed the
  complete 332-test Release NVVM prefix including real-provider, `ptxas`, and GPU runtime evidence.

## Surprises and Discoveries

- V4 already avoided a monolithic V5 by querying three subinterfaces, but construction retained
  three prefix-compatible versions and the facade rebuilt V1/V2-shaped dispatch tables from them.
- The historical API definitions occupy most of the ABI header and drive thousands of lines of
  partial-prefix and fallback tests even though the experimental provider ships with the host.
- The host test bin contained a stale pre-slice provider DLL after building the isolated provider
  project. Copying the freshly built DLL into `build/Release/bin` made the exact export visible;
  the tests then exercised the intended provider rather than failing every real-provider load.
- The repository formatting script could not run end to end because this machine lacks `gersemi`,
  `prettier`, and `shfmt` in WSL. The repository-pinned Windows clang-format 17.0.6 binary was
  available and successfully formatted every changed C/C++ file.

## Decision Log

- Decision: support exactly one ABI revision and one exact layout at a time.
  Rationale: this prototype has no independently supported older provider; preserving it slows API
  improvement without serving a distribution contract.
  Date/author: 2026-08-28, Codex.
- Decision: retain a scalar revision argument on the stable loader entry point.
  Rationale: rejecting an accidentally loaded stale DLL before reading its tables prevents memory
  corruption and is mismatch detection, not backward compatibility.
  Date/author: 2026-08-28, Codex.

## Outcomes and Retrospective

The builder boundary now has one root type, one export, one revision handshake, and three exact
subinterfaces. The root, subinterfaces, semantic type descriptors, and operation descriptors no
longer carry prefix sizes, interface versions, or reserved compatibility fields. Host and provider
contain no historical API type, export probe, negotiated table prefix, fallback, or synthesized
legacy dispatch table.

The compatibility-heavy builder tests lost 5,402 lines while retaining current behavior and
real-provider coverage; obsolete emitter prefix/feature-negotiation cases removed another 2,203
lines. The seven main ABI/facade/provider/fake-test files have a net reduction of 8,110 lines. The
new exact tests cover successful loading, missing export, wrong revision, incompatible metadata,
missing interface, and incomplete foundation/construction/value-operation tables. The full Release
NVVM prefix passes 332/332 and includes compatible assembly, libNVVM compilation, CUDA `ptxas`, and
GPU/NVRTC runtime parity on this machine.

## Context and Current Pipeline

The host dynamically loads `slang-llvm-nvvm`, probes V4 through V1, queries versioned V4 tables,
then copies current callbacks into synthetic legacy tables. Every facade call branches on the
negotiated generation. The provider exports four entry points and builds every historical table.
The compiler itself already targets the V4 descriptor path, so these branches do not represent
different current semantics.

## Scope and Non-Goals

In scope are the ABI header, loader/facade, provider exports/tables, fake builder, exact negotiation
tests, build export lists, durable design/status documentation, and this plan. Semantic feature-bit
removal is Slice 70; the current feature behavior may remain temporarily after this slice.

Out of scope are new IR operations, numeric types, CUDA semantics, LLVM upgrades, and compatibility
with any earlier experimental builder binary.

## Architecture and Invariants

The entry point takes the host's exact ABI revision and returns failure on any mismatch. A
successful call returns one root with immutable provider-owned current subinterfaces. All callbacks
are required, all LLVM objects remain opaque, and the provider library lifetime owns the tables.
No caller reads a partial structure or selects a historical interface version.

## Interfaces and Dependencies

Replace `slang_getNVVMBuilderAPI_V1` through `_V4` with `slang_getNVVMBuilderAPI`. Remove version and
size fields from the root, subinterfaces, and operation descriptors. Keep the query split by
foundation, construction, and value operations, but query only by current interface identity.

## Milestones

1. Define the exact unsuffixed ABI and update provider exports.
2. Replace loader negotiation and legacy dispatch with exact current initialization.
3. Update the fake provider and remove historical negotiation tests.
4. Rebuild, run focused and broad regressions, format, audit, and commit.

All four milestones are complete.

## Validation and Acceptance

Run the Release build and NVVM unit-test prefix outside the sandbox. Exercise exact success,
missing export, wrong revision, missing interface/callback, and normal current dispatch. Run real
provider serialization/compilation evidence retained by the established suite. Accept only if no
V1-V4 API/table/export, size-prefix, or interface-version fallback remains and emitted behavior is
unchanged.

## Failure and Recovery

Compile failures after removing aliases identify a real remaining dependency on a historical type.
Fix the consumer to use the current table rather than adding an alias. If a callback proves
genuinely optional, stop and establish a semantic capability query instead of restoring prefix
compatibility. Never stage `external/slang-binaries/`.

## Artifacts and Hand-Off

Retain the final removed-code measurements, exact handshake tests, validation counts, and input-
shape/special-case audit here. Commit this completed plan with Slice 69.

The only new validation helpers check the exact ABI metadata and required callback tables at the
dynamic-library boundary. They do not repair or reinterpret compiler IR. The fake builder's typed
phi instrumentation dispatches integer and floating types to their existing recorders so behavior
assertions remain readable; the public callback remains the one generic current operation. No new
AST/IR shape fallback, semantic reconstruction, or downstream special case was introduced.
