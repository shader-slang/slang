# Connect typed transcendental operations to libdevice

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires the completed slice plan to be committed with its implementation, overriding the normal
working-log policy for this experimental backend.

## Purpose and Observable Result

After this slice, direct NVVM compiles and executes the existing
`tests/compute/transcendental.slang` and `tests/compute/transcendental-double.slang` fixtures. Exact
Float32 and Float64 sine/cosine helpers become typed provider operations, the provider emits calls
to the corresponding libdevice declarations, and the direct path explicitly requests the
toolkit-matched `libdevice.10.bc` only when an accepted module contains one of those operations.

## Progress

- [x] (2026-08-29) Completed Slice 125 as `1461a7566`; Release provider/host builds, both promoted
  dynamic-dispatch fixtures, and the complete NVVM prefix passed 393/393.
- [x] (2026-08-29) Probed every remaining compute fixture with an active CUDA lane and no direct
  NVVM lane. Captured the first direct diagnostic for 24 fixtures and found
  `bound-check-zero-index.slang` already compiles.
- [x] (2026-08-29) Audited the apparent five-fixture `struct field address` cluster and rejected it
  as a slice boundary because its producers are four distinct families: arrays of resources,
  parameter blocks, erased constant-buffer arrays, and structured layouts outside copyable storage.
- [x] (2026-08-29) Captured the exact final helper shapes for both transcendental fixtures:
  `Func(Float, Float)` and `Func(Double, Double)` terminated by `$P_sin($0)` or `$P_cos($0)`.
- [x] (2026-08-29) Added exact typed semantic rows, provider libdevice calls, and demand
  propagation without widening arbitrary GenericAsm or external calls.
- [x] (2026-08-29) Added focused fake/real-provider and negative coverage, then promoted both
  fixture lanes.
- [x] (2026-08-29) Inspected and assembled PTX, ran Release provider/host builds and the complete
  NVVM prefix, updated durable status, formatted, and self-reviewed the slice.

## Surprises and Discoveries

- The remaining-CUDA census currently partitions into exact first boundaries: five
  `struct field address`, five `helper function parameter`, two transcendental `GenericAsm`, one
  `makeArray`, one `basic-block parameter`, several fixtures requiring test-specific specialization,
  and one already-supported fixture. Diagnostic equality alone is not enough to define a slice.
- The two transcendental fixtures contain only sine and cosine, each in the same one-block
  canonical CUDA helper shape already used by the typed GenericAsm catalog. Float32 and Float64
  differ only in their exact semantic type and libdevice symbol.
- Slice 18 already implemented coherent toolkit selection, an explicit
  `requiresCUDADeviceLibrary` downstream option, lazy/normal library addition, and compiler tests.
  The direct emitter has never supplied that demand bit because no accepted direct operation has
  required libdevice until now.
- A fake direct compile that loads the logical `nvvm` alias intentionally has no selected-library
  path and therefore cannot derive a coherent toolkit root. The end-to-end demand test must use
  `setDownstreamCompilerPath` with its temporary toolkit, matching the production contract instead
  of relying on ambient `CUDA_PATH`.

## Decision Log

- Decision: make Float32/Float64 sine and cosine exact typed value operations and retain GenericAsm
  text only as compiler-side recognition metadata.
  Rationale: the optimized producer has a complete checked helper signature. Passing assembly text
  or libdevice symbol names through the builder would weaken the generic typed interface and move
  Slang spelling knowledge into the provider.
  Date/author: 2026-08-29, Codex.
- Decision: have the provider own the mapping from typed sine/cosine descriptors to
  `__nv_sinf`, `__nv_cosf`, `__nv_sin`, and `__nv_cos` declarations.
  Rationale: these are the physical NVVM/libdevice implementation names. The provider already owns
  LLVM construction and can validate exact types, module ownership, dominance, and declarations.
  Date/author: 2026-08-29, Codex.
- Decision: derive downstream libdevice demand from accepted operation requirements and pass it
  explicitly through the direct-backend continuation.
  Rationale: source spelling and target-wide unconditional linking are both too broad. The exact
  validated semantic set is the narrow source of truth, and libdevice-free modules must preserve
  Slice 18's no-I/O behavior.
  Date/author: 2026-08-29, Codex.

## Outcomes and Retrospective

Exact scalar Float32/Float64 sine and cosine now compile through typed provider operations and the
selected toolkit's libdevice. Both existing fixtures pass their direct runtime and PTX lanes. The
8,717-byte Float32 and 12,663-byte Float64 PTX modules assemble with CUDA 12.9.86 to 6,328-byte and
8,112-byte cubins. Release provider/host builds pass, focused exact and negative tests pass, and the
complete NVVM prefix passes 395/395. The whole Float32 fixture prefix still reproduces its
pre-existing Dawn/WebGPU bind-group failure; the new direct lanes pass independently.

The self-review found no new AST/IR reconstruction, fallback, arbitrary symbol API, text
manipulation, unconditional library demand, or compatibility shim. The production special cases
are exactly the four canonical helpers: validation selects their catalog rows, the provider maps
those typed rows to libdevice symbols, and the downstream option follows the accepted requirement
set.

## Context and Current Pipeline

After linking, specialization, CUDA varying legalization, and optimization, the relevant helpers
are equivalent to:

    func sin(Float x) -> Float  { GenericAsm("$P_sin($0)") }
    func cos(Float x) -> Float  { GenericAsm("$P_cos($0)") }
    func sin(Double x) -> Double { GenericAsm("$P_sin($0)") }
    func cos(Double x) -> Double { GenericAsm("$P_cos($0)") }

The entry point calls two of those helpers, quantizes the results, and writes an existing
`RWStructuredBuffer<Float>`. Direct validation already recognizes exact GenericAsm helper bodies
through `NVVMSemantics::kCatalog`, preflights typed operations before module creation, and emits
them through `emitValueOperation`. The provider already emits LLVM intrinsics and ordinary typed
operations through this interface. The downstream compiler can already add the coherent toolkit's
libdevice module when `DownstreamCompileOptions::requiresCUDADeviceLibrary` is true.

## Scope and Non-Goals

In scope are scalar Float32/Float64 sine and cosine; the exact four GenericAsm helper signatures;
typed builder capability queries and emission; exact external libdevice declarations/calls;
operation-derived downstream library demand; fake and real provider tests; both existing compute
fixtures; direct runtime/PTX lanes; PTX assembly; durable design status; and this plan.

Out of scope are arbitrary external functions, passing symbol or assembly strings through the
builder ABI, vectors, Float16 transcendentals, fast approximate CUDA intrinsics, every other math
operation, broad libdevice catalogs, unconditional libdevice linking, fallback to NVRTC, source
name matching, compatibility aliases, and unrelated census boundaries.

## Architecture and Invariants

- The canonical helper's exact GenericAsm string and complete typed signature select one catalog
  row; the provider receives only the typed operation descriptor and values.
- The provider accepts only scalar Float32/Float64 unary sine/cosine descriptors, validates the
  active insertion point and operand type/ownership, declares the exact libdevice function in the
  same module, and emits one typed call.
- Catalog metadata records whether an exact operation requires libdevice. Validation aggregates
  that bit only after the helper shape is accepted; rejected GenericAsm must not request a library.
- The shared downstream continuation sets `requiresCUDADeviceLibrary` only for the direct NVVM
  invocation and otherwise preserves all existing callers' zero/default behavior.
- Float mode, denormal mode, target SM, libNVVM, and libdevice continue through the established
  downstream policy. This slice does not choose an approximate function based on optimization.

## Interfaces and Dependencies

Expected committed areas are the forward-only builder value-operation enum/catalog, provider
operation construction, direct operation requirements, direct downstream invocation, fake/real
provider and emitter tests, both compute fixtures, `docs/design/nvvm-backend.md`, the capability
ledger if it tracks these tests, and this plan. CUDA 12.9 supplies libNVVM, matching
`nvvm/libdevice/libdevice.10.bc`, runtime execution, and `ptxas`.

## Milestones

1. Add exact sine/cosine operation IDs and four catalog rows, including an internal libdevice
   demand flag. Bump the forward-only ABI if protocol negotiation requires it and extend exact
   fake/provider capability tests.
2. Implement provider declaration/call emission for the four descriptors and verify emitted LLVM
   14 assembly plus LLVM-7-compatible serialization contains exact declarations and calls.
3. Propagate the aggregate libdevice requirement through `emitNVVMForEntryPoints` into the existing
   downstream option. Prove an accepted transcendental requests the library and an ordinary module
   does not.
4. Promote both fixtures, inspect runtime and PTX, assemble with the selected SM, run Release/full
   gates, update docs/plan, format, perform the input-shape audit, and commit.

## Validation and Acceptance

Acceptance requires focused malformed-descriptor and exact-operation tests before provider
mutation; fake direct-emitter coverage for all four typed semantics and library demand; real
provider LLVM/legacy assembly with exact libdevice declarations/calls; a real direct compile using
the selected toolkit library; all existing lanes plus new direct CUDA runtime/PTX lanes for both
fixtures; CUDA 12.9 `ptxas`; Release provider and host builds; the complete
`slang-unit-test-tool/nvvm` prefix; pinned formatting; and `git diff --check`.

The self-review inventories every new enum, catalog row, provider mapping, demand flag, plumbing
parameter, helper, fallback, and special case. Each retained item must name one of the four exact
optimized helper producers and a failing test. Remove any arbitrary symbol API, text parser,
unconditional library flag, duplicated semantic matcher, source-name test, approximate-operation
choice, provider-only fallback, or compatibility shim.

## Failure and Recovery

If LLVM assembly, legacy serialization, libdevice linking, libNVVM verification, PTX assembly, or
runtime results fail, retain IR/LLVM/PTX/cubin/log evidence under ignored `build/slice126-*` and
trace the exact descriptor through catalog, provider, and downstream option. Do not inline an
approximation, silently use NVRTC, weaken the helper signature, link libdevice for unrelated
modules, reset unrelated work, or stage `external/slang-binaries/`.

## Artifacts and Hand-Off

Keep the census outputs and generated LLVM/PTX/cubin/log artifacts under ignored
`build/slice126-*`. Distill the four exact semantic mappings, libdevice demand contract, CUDA
evidence, and next measured corpus boundary into `docs/design/nvvm-backend.md`, then commit this
plan with the implementation as explicitly requested.
