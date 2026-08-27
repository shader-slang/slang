# Slice 45: Generalize scalar functions and add float32 helper calls

This ExecPlan follows `.agent/PLANS.md`. Keep it current. The user requires the completed plan to
ship with the slice, overriding the repository's default working-log policy for this experiment.

## Purpose and Observable Result

After this slice, the direct NVVM route compiles and runs a kernel that passes two canonical
float32 values to a reachable helper, receives its float32 result, and stores it. One append-only
generic call/valued-return pair serves scalar Integer and Float functions without adding callbacks
per scalar type, while frozen V2 signed-i32 helpers remain compatible.

## Progress

- [x] (2026-08-28) Recorded Slice 44 baseline: 291 names, SHA-256
  `c18462cd303630788566c59409f369ef57a46614652571a97663acf0ffb01690`, Release 291/291,
  Debug 10/10, 504-byte x64/296-byte x86 V3 table, and 22,154 measured lines.
- [x] (2026-08-28) Audited helper closure/signature validation, role-based type lowering, frozen V2
  integer call/return callbacks, provider ownership/dominance checks, and fake call topology.
- [x] (2026-08-28) Appended generic scalar call/valued-return negotiation and shared
  provider/facade/fake dispatch; V3 is 520 bytes on x64 and 304 bytes on x86.
- [x] (2026-08-28) Admitted canonical float32 helper signatures, arguments, results, and returns
  while preserving frozen V2 signed-i32 helpers.
- [x] (2026-08-28) Added seven independently named provider/direct/PTX/assembler/runtime evidence
  layers around a retained two-argument Float helper.
- [x] (2026-08-28) Formatted, built, tested, hashed, measured, documented, completed the input-shape
  audit, and prepared the complete slice for commit.

## Surprises and Discoveries

- Observation: function-type construction, function declaration, and parameter retrieval already
  transport opaque provider types; only helper policy plus V2 call/return callbacks hardcode i32.
  Consequence: generalize the operation boundary and role policy without duplicating declaration or
  parameter APIs.

- Observation: a helper can validly mix Integer and Float scalar parameters independently of its
  result type.
  Consequence: choose the generic path from the complete canonical helper signature, not merely the
  call result, and make the generic provider validate every exact parameter type.

- Observation: the existing floating-sine unsupported case used to stop at its Float helper result.
  Once canonical Float helpers became valid, it advanced to the next unsupported instruction,
  `castFloatToInt`; every other unsupported-matrix boundary remained stable.
  Consequence: update that one expected first stop rather than retaining an obsolete signature
  rejection that would mask the newly supported shape.

- Observation: this slice adds 683 measured test/support lines, from 22,154 to 22,837, because it
  establishes generic typed call/result/return fake mechanics and seven evidence names.
  Consequence: later scalar types and mixed signatures can reuse one ABI and fake family rather
  than adding per-type callbacks or call representations.

## Decision Log

- Decision: append feature 33 `GENERIC_SCALAR_FUNCTIONS` plus generic
  `emitCall(module, callee, arguments, count, outValue)` and
  `emitValueReturn(module, value)` callbacks.
  Rationale: the callee function type and value type already encode all scalar types; a callback per
  type would duplicate both ABI and wrapper surface.
  Date/author: 2026-08-28, Codex.

- Decision: retain frozen V2 integer call/return facade and provider paths, dispatching them only
  for all-i32 helper signatures. Use the generic V3 pair when any helper result or parameter is
  Float.
  Rationale: exact old providers and signed-i32 function semantics must remain valid while mixed
  scalar signatures need one coherent generic contract.
  Date/author: 2026-08-28, Codex.

## Outcomes and Retrospective

Feature 33 appends one complete generic call/valued-return pair. The x64 V3 table grows from 504 to
520 bytes and x86 grows from 296 to 304 bytes; exact Slice 44 tables remain accepted when they do
not advertise the feature. Floating-point type discovery and both callbacks are mandatory, and
null/short advertised suffixes are rejected without wrapper mutation.

The direct fake graph has a Void kernel signature `[FloatPointer, Float, Float]` and a Float helper
signature `[Float, Float]`. Original kernel parameters 1 and 2 feed one typed Float call; helper
parameters feed one Float addition and generic valued return; the call result reaches the sole
store. Generic LLVM and negotiated NVVM-2.0 text each contain one Float helper definition, one
`call float`, one `ret float`, and one `fadd float`.

NVVM/NVRTC PTX agree on `[64, 32, 32]`, one Float add, one global 32-bit store, and no global load
or Float predicate; matching CUDA 12.9 `ptxas` accepts both. RTX 5090 runs agree for finite values
and preserve exact `-0.0 + -0.0` and `+0.0 + -0.0` results. The seven new names increase the five
measured files by 683 physical lines, from 22,154 to 22,837. The focused matrix passes 14/14,
Release passes 298/298 with sorted LF-terminated name-set SHA-256
`71658634899192b09f2d12461c25a5efb9d85c3c4f2db7c285ba35ef35d44066`, and removing the seven new
names reproduces Slice 44's 291-name hash exactly. Debug preservation passes 10/10.

## Context and Current Pipeline

Preflight walks the finite direct-call closure, rejects recursion/indirect use, validates distinct
symbols, declares every function before emitting bodies, and obtains every parameter by ABI
position. Current policy admits only signed i32 helper results/parameters and validates every call
argument/return as i32. Emission therefore always invokes the frozen V2 integer call/return pair.

The LLVM provider already owns exact function type, same-module callee, non-variadic signature,
argument count/type/availability, insertion point, return type, and value availability checks. Only
its scalar classification is integer-specific. The fake records calls generically but assumes every
call result and valued return is Integer.

## Scope and Non-Goals

In scope are canonical scalar Float helper parameters/results, matching Float call arguments and
returns, one append-only generic callback pair/feature, shared Integer/Float fake call typing, a
two-argument float32 addition helper, and provider/direct/PTX/assembler/runtime evidence.

Out of scope are void helper returns, pointer/vector/aggregate helper values, indirect calls,
recursion, varargs, declarations without bodies, calling conventions/attributes, libdevice calls,
half/double, tail calls, inlining policy, and performance claims.

## Architecture and Invariants

Feature 33 requires the exact complete two-callback suffix and floating-point type discovery.
Slice 44-sized providers remain valid without it. Generic provider calls accept only same-module,
non-variadic functions whose non-void result and every parameter are scalar Integer or Float;
arguments must exactly match and be usable at the current unterminated insertion point. Generic
returns accept the same scalar families and must exactly match the current function result.

Direct preflight retains the canonical reachable-function closure and exact semantic signature as
the source of truth. All-i32 signatures request feature 3/V2 exactly as before. A signature with
any Float requests feature 33; argument and return validation dispatch by their canonical type.
Type lowering admits the same scalar set for helper roles. Emission chooses legacy or generic
callbacks from the complete callee/current-function signature; no alternate function ABI exists.

## Interfaces and Dependencies

Append one feature, two callback typedefs/table fields/suffix macro, and two facade methods. Refactor
provider call/return validation into shared scalar helpers and teach fake function results/calls/
returns their scalar kind. Extend type policy, direct validation/emission, tests, design, ledger, and
plan. Add no ABI version, V2 field, export, dependency, target, text rewrite, syntax reconstruction,
or per-type callback.

## Milestones

1. Append feature 33 and the generic call/return pair with exact Slice 44 compatibility.
2. Share provider and fake scalar-function mechanics while retaining frozen V2 adapters.
3. Admit canonical Float helper signatures/arguments/returns and dispatch by semantic signature.
4. Add seven named negotiation, provider, direct, capability, differential, `ptxas`, and runtime
   tests around a two-argument Float helper.
5. Format/build, run focused/full/Debug lanes, hash, measure, document, audit, commit `slice 45`.

## Validation and Acceptance

Run the seven new tests plus V3 layout, old-prefix, provider invalid/no-mutation, signed-i32 helper
preservation, adjacent Float arithmetic/phi/constants, and unsupported matrix. Run full Release
NVVM and Debug 10/10 outside sandbox; build standalone Release provider and Release/Debug targets.

Accept old-prefix compatibility, rejected partial/null suffixes, exact Float helper definition,
`call float`, and `ret float` in both text dialects, direct typed argument/call/return topology,
matching `[64, 32, 32]` PTX with Float add and global store but no global load/predicate, `ptxas`,
matching finite and signed-zero RTX/NVRTC results, exact name continuity, formatted code, completed
audit, and clean diff checks.

## Self-Review and Input-Shape Audit

The new-helper/special-case inventory is the generic callback/facade pair, provider shared
call/return helpers, fake scalar result/call/return typing, helper-role type policy, canonical
signature classifier, scalar-value validator, and direct signature dispatch. All survive this
audit:

- Slang linking produces the exact valid input shape: one reachable defined helper with canonical
  Float result/parameter types, a direct `IRCall`, positional exact-typed arguments, and an
  exact-typed `IRReturn`. The existing finite closure walk and semantic function signature are the
  source of truth. Removing Float helper admission restores the former helper-result rejection, so
  preflight/type lowering/emission are the layers that must admit this already-canonical shape.
- The signature classifier runs only after helper-target validation and uses release assertions for
  that contract. It selects generic V3 when any canonical result or parameter is Float and frozen
  V2 only for all-i32 signatures. It does not infer type from one call result, rediscover context by
  walking operands, or silently route an unsupported signature.
- Direct validation maps call arguments by their canonical ABI position, checks exact semantic type
  equality and dominance, and validates helper returns against the function result. Emission reuses
  the same callee/current-function signature; no alternate ABI, syntax value, or local function
  representation is constructed.
- The provider owns opaque LLVM function/value handles and therefore owns same-module,
  non-variadic, exact scalar signature, insertion-point availability, return-type, and dominance
  checks. Shared helpers express those type-independent invariants. V2 adapters require Integer;
  generic V3 admits scalar Integer/Float without custom type equivalence.
- The fake's existing `Call` representation now carries a scalar result kind, and generic returns
  use one scalar record rather than Float-specific call/return nodes. Separate integer-return
  records preserve frozen-V2 evidence. Exact-bit runtime comparison strengthens the reusable Float
  arithmetic launcher without changing compilation semantics.

No syntax conversion, arbitrary graph walk, alternate function ABI, custom equivalence, fallback,
silent default, per-type callback, text rewrite, or downstream repair of malformed IR was
introduced.

## Failure and Recovery

If linking inlines the helper, inspect linked IR and select a fixture retained by the normal linked
pipeline rather than disabling optimization globally. If libNVVM inlines the helper in PTX, retain
LLVM/NVVM text plus runtime as semantic evidence. Removing the appended suffix and Float helper
policy restores Slice 44. Never stage `external/slang-binaries/`.

## Artifacts and Hand-Off

Retain old/new table sizes, exact generic/NVVM helper/call/return text, direct typed call graph,
matching PTX ABI, `ptxas`, RTX/NVRTC results, counts/hashes, line growth, and audit. Distill durable
evidence to design/ledger and ship this completed plan with Slice 45.
