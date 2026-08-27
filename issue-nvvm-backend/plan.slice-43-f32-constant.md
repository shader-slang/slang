# Slice 43: Add exact scalar float32 constants

This ExecPlan follows `.agent/PLANS.md`. Keep it current. The user requires the completed plan to
ship with the slice, overriding the repository's default working-log policy for this experiment.

## Purpose and Observable Result

After this slice, the direct NVVM route compiles and runs `*destination = 1.5f` for an AS1
`Ptr<float>` destination. A generic exact-bit floating-constant callback is appended to V3 so this
type/value boundary does not reuse the integer-only constant API or pass decimal text across the
provider boundary.

## Progress

- [x] (2026-08-27) Recorded Slice 42 baseline: 277 names, SHA-256
  `a34a5cdb1532603a18290777a75fe23ea9407f5d294e1d9a1a739ea6b9187ae6`, Release 277/277,
  Debug 10/10, 480-byte x64/288-byte x86 V3 table, and 20,891 measured lines.
- [x] (2026-08-27) Audited canonical `kIROp_FloatLit`, double-backed Slang literal storage, the
  integer-only constant facade/provider/fake path, and append-only V3 suffix ownership.
- [x] (2026-08-27) Appended exact-bit floating-constant negotiation, facade, provider, and fake
  dispatch with old-prefix and bounded-input evidence.
- [x] (2026-08-27) Admitted canonical float32 literals as executable operands without changing
  signed-i32 literals.
- [x] (2026-08-27) Added seven independently named provider/direct/PTX/assembler/runtime evidence
  layers around exact float32 `1.5`.
- [x] (2026-08-27) Formatted; built standalone/Release/Debug targets; passed focused, full Release,
  and Debug lanes; hashed/measured; updated durable docs; and completed the audit.

## Surprises and Discoveries

- Observation: canonical `IRFloatLit` stores an `IRFloatingPointValue`, currently `double`, even
  when its semantic type is Float.
  Consequence: round once to semantic float32 in Slang and pass the resulting IEEE-754 bits; do not
  let host/provider decimal parsing become another rounding boundary.

- Observation: V3 currently ends at the generic floating-compare callback.
  Consequence: append one generic constant callback. The x64 table grows by one pointer; on x86 the
  pointer occupies the existing tail padding and total size remains unchanged.

- Observation: the new constant value family and its first seven-layer evidence matrix add 554
  measured test/support lines, unlike the roughly 50-line marginal comparison rows.
  Consequence: treat this as the base cost of a new exact-bit value family; future independent
  floating widths and constant cases should reuse this callback, fake node, and launch helpers.

## Decision Log

- Decision: append feature 31 `SCALAR_FLOAT32_CONSTANT` and
  `getFloatingPointConstant(module, type, bitWidth, bitPattern, outValue)`.
  Rationale: exact bits preserve semantic values and let future widths reuse the callback under
  independent feature bits without adding one emit call per literal spelling.
  Date/author: 2026-08-27, Codex.

- Decision: support only bit width 32 in this slice and reject nonzero bits above bit 31.
  Rationale: the semantic claim is exact scalar float32; accepting other widths would silently
  create unnegotiated half/double capability.
  Date/author: 2026-08-27, Codex.

## Outcomes and Retrospective

Feature 31 appends a 488-byte x64/288-byte x86 V3 suffix. The exact 480-byte x64/288-byte x86 Slice
42 prefix remains valid without the feature; advertised partial sizes and a null callback fail
negotiation. Facade and provider reject non-32 widths and nonzero high bits, clear stale or
provider-written outputs on failure, validate exact module-owned Float type context, and construct
LLVM `ConstantFP` from the supplied `APInt`/`APFloat` payload.

For `*destination = 1.5f`, direct preflight accepts the canonical Float `kIROp_FloatLit`, requests
only feature 31 for the value, and lowers its double-backed storage once to payload `0x3fc00000`.
The fake records topology `[FloatPointer]`, one `FloatingPointConstant(32, 0x3fc00000)`, and one
aligned store consuming that node. The existing signed-i32 SSA fixture still uses V2
`getIntegerConstant` and its established feature.

Generic LLVM and negotiated NVVM-2.0 text each contain exactly one
`store float 1.500000e+00` without synthetic arithmetic. NVVM and NVRTC agree on `[64]`, one global
32-bit store, and no global load, Float arithmetic, or predicate. CUDA 12.9 `ptxas` accepts both;
the RTX 5090 runtime lane observes the exact float32 value `1.5` through both routes.

Seven names raise Release from 277 to 284 with sorted LF-terminated SHA-256
`3e78b6b3069dd0a12cbde4d78e4d804e5eeace161cdbf86d620262b5e9d9a72d`; removing them reproduces
Slice 42's count and hash exactly. Focused tests pass 14/14, full Release passes 284/284, Debug
preservation passes 10/10, and all standalone/Release/Debug targets build. The five measured
test/support files grow 554 lines from 20,891 to 21,445, establishing the reusable base cost of the
new exact-bit constant family.

## Context and Current Pipeline

`_validateFloat32Value` currently accepts only values already available in the function. A
canonical Float `kIROp_FloatLit` is module-owned and therefore stops as `floatLit`. The lowering
helper handles only signed-i32 literals and release-asserts when an unmapped Float literal reaches
it. V2's `getIntegerConstant` is correctly integer/SSA-specific and must remain frozen.

V3 already owns the canonical Float type and all arithmetic/comparison families. Its size-bounded
copy and feature/suffix validation allow one append-only generic callback while keeping exact Slice
42 providers valid without feature 31.

## Scope and Non-Goals

In scope are canonical scalar Float `kIROp_FloatLit`, exact float32 bit transport, one appended V3
callback/feature, a direct AS1 float store, finite value `1.5f`, and provider/fake/PTX/assembler/
runtime evidence.

Out of scope are half/double, NaN-payload and signed-zero source-spelling claims, integer constant
changes, casts, Float phis/helpers/returns, composite constants, vectors/matrices, constant folding,
fast/constrained math, resources, atomics, libdevice, and performance claims.

## Architecture and Invariants

Feature 31 requires the exact complete new suffix plus `getFloatingPointType`. Slice 42-sized
providers remain valid without the bit. Facade and provider clear outputs on every failure, reject
non-32 widths/high bits before construction, validate module/type ownership, and return an exact
LLVM `ConstantFP` without mutating the module.

Direct preflight recognizes only canonical Float `kIROp_FloatLit`, rounds its double-backed storage
to semantic float once, requests feature 31, and treats the constant as executable on demand.
Lowering transports `FloatAsInt(float(value))` through the callback and caches the returned handle.
Signed-i32 constants retain `SCALAR_SSA` and the frozen V2 callback.

## Interfaces and Dependencies

Append one feature, callback typedef/table field/suffix-size macro, and facade method. Extend host
validation, provider, fake value recording, direct Float validation/lowering, tests, design,
ledger, and plan. Add no export, ABI version, V2 field, operation enum, dependency, target, decimal
text parser, or general constant framework.

## Milestones

1. Append feature 31/callback with 488-byte x64/288-byte x86 complete layout and exact Slice 42
   compatibility when the feature is absent.
2. Construct exact float32 `ConstantFP` from the transported 32-bit payload after bounded validation.
3. Admit canonical Float literals on demand and preserve signed-i32 constant feature/callback paths.
4. Add seven named negotiation, provider, direct, capability, differential, `ptxas`, and runtime
   tests around `1.5f` and exact payload `0x3fc00000`.
5. Format/build, run focused/full/Debug lanes, hash, measure, document, audit, commit `slice 43`.

## Validation and Acceptance

Run seven new tests plus V3 layout, exact old prefix, unknown/invalid Float operations, signed-i32
constant/SSA preservation, adjacent Float arithmetic, and unsupported matrix. Run full Release NVVM
and Debug 10/10 outside sandbox; build standalone Release provider and Release/Debug test targets.

Accept old-prefix compatibility, rejected partial/null suffixes, exact bit transport and output
sanitization, one Float constant store in both text dialects, direct fake payload `0x3fc00000`,
matching `[64]` PTX with one global 32-bit store and no load/arithmetic/predicate, `ptxas`, RTX/NVRTC
result `1.5f`, exact name continuity, formatted code, completed audit, and clean diff checks.

## Self-Review and Input-Shape Audit

Inventory the callback/facade/helper changes and fake value-kind extension. Record the canonical
producer, semantic type/value source of truth, exact rounding boundary, revert failure, and
emitter/provider ownership. Confirm no decimal transport, syntax reconstruction, graph walk,
alternate constant shape, structural equivalence helper, text rewrite, or integer-path change.

The production inventory contains one appended feature/callback, one facade method, provider
construction from exact bits, `_asExecutableFloat32Constant`, and the two bounded literal branches
in value validation/lowering. The fake adds one corresponding storage/value kind and exact payload
recording. No fallback, custom semantic equivalence, arbitrary graph walk, text rewrite, syntax
reconstruction, or provider-side rounding survives.

Normal Slang lowering of `*destination = 1.5f` produces one module-owned canonical
`kIROp_FloatLit` whose semantic data type is canonical Float and whose value source of truth is the
existing `IRFloatingPointValue`. Module ownership is intentional for literals, so requiring the
literal to appear in the function's available-value set would reject a canonical producer shape.
The direct validation boundary therefore owns literal admission, just as its signed-i32 sibling
already does. It rounds the double-backed value exactly once with `float(value)` and transports the
resulting bits. Removing this branch restores the `floatLit` unsupported diagnostic; removing
materialization reaches the pre-existing assertion in `_getLoweredNVVMValue`.

The provider owns LLVM object construction because opaque LLVM handles cannot cross the C ABI. It
uses the transported bits rather than rebuilding source syntax or parsing decimal text. Feature 31
and width/high-bit checks make float32 the only negotiated semantic claim. The integer branch is
unchanged apart from becoming the first explicit arm before the Float arm, and the focused scalar
SSA tests prove its callback and feature ownership remain intact. Test-only helpers model the new
wire/value family and share compilation, PTX, assembly, and CUDA-launch machinery; they do not
repair or reinterpret production IR.

## Failure and Recovery

If LLVM or libNVVM rejects the exact constant, inspect generic and negotiated text before changing
representation. If PTX embeds the payload as integer bits, use ABI/store/runtime evidence rather
than require a particular move spelling. Removing the appended feature/callback and Float-literal
path restores Slice 42. Never stage `external/slang-binaries/`.

## Artifacts and Hand-Off

Retain old/new table sizes, exact payload and LLVM/NVVM text, direct `[FloatPointer]` topology,
`[64]` PTX, `ptxas`, RTX/NVRTC output, counts/hashes, and marginal lines. Distill to design/ledger
and ship this completed plan with Slice 43.
