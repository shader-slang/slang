# Lower selected scalar truthiness helpers

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires the completed slice plan to be committed with its implementation, overriding the normal
working-log policy for this experimental backend.

## Purpose and Observable Result

After this slice, canonical CUDA-prelude scalar `all` and `any` helpers convert selected Bool,
integer, Float16, and Float32 values to Bool through the generic typed value-operation path. The
direct compiler recognizes only the exact `GenericAsm("bool($0)")` helper body, synthesizes a typed
zero, and requests the already established `NOT_EQUAL` operation. The existing
`tests/compute/logic-no-short-circuit-evaluation.slang` fixture should pass direct runtime and PTX
lanes with its unchanged `30, 31, 32, 33` result.

## Progress

- [x] (2026-08-29) Reproduced the post-Slice-115 stop and traced it to scalar `all(int)`; the
  adjacent scalar `all(bool)` helper has the same body and an older identity-only special case.
- [x] (2026-08-29) Confirmed the shared value-operation family already supports selected integer,
  floating-point, and Boolean inequality, and the builder already constructs typed scalar zeros.
- [x] (2026-08-29) Replaced the Bool-only identity matcher with one exact selected-scalar
  truthiness resolver and reused typed `NOT_EQUAL` without changing the builder ABI.
- [x] (2026-08-29) Added focused `all`/`any` family and malformed-signature coverage, and made the
  fake provider preserve complete typed catalog descriptors while retaining its routing evidence.
- [x] (2026-08-29) Promoted the existing fixture, validated its direct runtime/PTX lanes and PTX
  assembly, ran the full NVVM prefix, documented the result, formatted, and self-reviewed.

## Surprises and Discoveries

- `all(vector<T, N>)` is already ordinary IR: a bounded loop dynamically extracts each lane and
  calls scalar `all(T)`. The vector reduction itself needs no provider feature. Only the scalar
  CUDA fallback remains as `GenericAsm("bool($0)")`.
- Slice 91 special-cased Bool because its conversion is an identity. That leaves the same canonical
  helper unsupported for integer and floating-point specializations even though typed inequality
  and zero construction are already available.
- The fake provider initially preserved complete descriptors only for family-resolved operations.
  Signed i32 and Float32 inequality use exact catalog entries, so their records looked untyped even
  though the emitter and real provider received the right descriptors. Recording exact catalog
  operations through one typed helper fixed the test-double inconsistency while preserving its
  existing family-call and emitted-operation evidence.
- Running every lane of the promoted fixture produced 12 passes, two ignores, and one unrelated
  WebGPU failure caused by an invalid bind-group layout on this machine. The newly added direct
  CUDA runtime lane and direct PTX lane each pass in isolation and are the acceptance evidence for
  this slice.

## Decision Log

- Decision: represent scalar truthiness as `value != typed_zero` through the existing generic value
  operation instead of adding a source-builtin callback or a new semantic operation ID.
  Rationale: this exactly matches the LLVM/SPIR-V implementations in the prelude, covers every
  already selected scalar family, and keeps the provider interface unchanged.
  Date/author: 2026-08-29, Codex.
- Decision: route Bool through the same Boolean inequality family rather than retain the bespoke
  identity return.
  Rationale: `x != false` is exactly `x`, LLVM can fold it, and one structural resolver is simpler
  than parallel Bool and numeric interpretations of the same canonical helper.
  Date/author: 2026-08-29, Codex.
- Decision: preserve complete typed descriptors for exact fake-provider catalog operations instead
  of weakening the focused assertions for exact signed-i32 and Float32 cases.
  Rationale: the generic ABI always carries those descriptors. Making both fake routing paths
  record the same contract tests the real interface and removes an accidental observability gap.
  Date/author: 2026-08-29, Codex.

## Outcomes and Retrospective

The direct emitter now recognizes only the canonical one-block, one-parameter selected-scalar
`GenericAsm("bool($0)")` helper and lowers Bool, signed/unsigned integer, Float16, and Float32
specializations to `value != typed_zero`. Preflight and emission share the exact descriptor; a
malformed two-parameter spelling remains rejected as `GenericAsm` before provider mutation. No
builder ABI or LLVM-provider source changed.

The original `logic-no-short-circuit-evaluation.slang` now passes its direct CUDA runtime lane with
the unchanged output `30, 31, 32, 33`, and its direct PTX lane contains the selected kernel and
global store. The 872-byte PTX module assembles with CUDA 12.9.86 `ptxas -arch=sm_70` to a
2,920-byte cubin. Release `slang-unit-test` builds, both focused tests pass, and the complete NVVM
prefix passes 384/384.

Self-review inventory:

- `_resolveNVVMScalarTruthiness` survives. The exact input is the CUDA prelude's selected scalar
  helper: one parameter, one block, Bool result, and the sole `bool($0)` terminator. That spelling
  is canonical and intentionally target-specific; the emitter translates its already checked
  semantics rather than reconstructing a source expression or accepting arbitrary asm.
- `NVVMScalarTruthiness::getOperationDesc` survives as the single preflight/emission source of
  truth. It delegates legality and diagnostics to the established semantic catalog/family
  resolver; removing it restores the motivating fixture's `GenericAsm` failure.
- The typed-zero dispatch survives. Integer and Bool inputs use the existing integer-constant
  callback, floating inputs use the existing floating-constant callback, and both preserve the
  parameter's provider type. There is no fallback: unsupported types fail the structural resolver.
- `_recordFakeNVVMBuilderCatalogScalarOperation` survives only in the test double. Exact catalog
  and family-resolved ABI calls carry identical descriptors; the helper preserves that source of
  truth plus the legacy routing counters. Reverting it makes signed-i32 and Float32 descriptors
  unobservable without changing production behavior.
- The former Bool identity special case is removed. Keeping it would create two interpretations of
  the same canonical helper and bypass provider capability preflight for one supported type.

## Context and Current Pipeline

After Slice 115, the original short-circuit fixture's globals are correctly initialized in a local
kernel context and passed into `assignFunc`. Final linked IR contains three vector `all` helpers.
Their bounded loops and dynamic selected-value extraction are already supported, but the integer
loop calls a scalar `Func(Bool, Int)` whose sole body instruction is
`GenericAsm("bool($0)")`. Direct preflight rejects that terminator before provider mutation.

The builder's typed value-operation descriptor can already express Bool results and two exact
integer, floating-point, or Boolean operands for `NOT_EQUAL`. Its constant API creates zero for
the same selected scalar provider types. No textual asm needs to cross the LLVM shield.

## Scope and Non-Goals

In scope are exact one-block, one-parameter `GenericAsm("bool($0)")` helpers returning Bool from
selected scalar Bool, signed/unsigned integer, Float16, or Float32 values; typed zero construction;
and the vector `all`/`any` reductions that call those helpers.

Out of scope are arbitrary GenericAsm parsing, vector-valued direct truthiness helpers, Double,
resource/pointer/aggregate truthiness, other casts, source-level short-circuit changes, a new
builder ABI operation, and unrelated CUDA-prelude intrinsics.

## Architecture and Invariants

- The matcher checks the complete canonical helper body: one block, one parameter, Bool result,
  only the GenericAsm terminator, exactly its string operand, and one selected scalar input.
- The source parameter descriptor is duplicated as both operands of a typed `NOT_EQUAL`; emission
  supplies the original parameter and a zero of its exact provider type.
- Preflight records the same operation descriptor emission uses and queries provider support before
  any module mutation.
- Unsupported strings, extra parameters, extra instructions/blocks, non-Bool results, and
  unsupported parameter families retain the precise `GenericAsm` diagnostic.

## Interfaces and Dependencies

No builder ABI or provider implementation change is planned. The compiler reuses
`getIntegerConstant`, `getFloatingPointConstant`, and `emitValueOperation`. The fake provider
already records constant kinds and complete operation descriptors. CUDA 12.9 runtime and `ptxas`
provide external semantic and assembly evidence.

## Milestones

1. Replace the Bool identity matcher with an exact structural scalar-truthiness resolver that
   builds a validated `NOT_EQUAL` descriptor.
2. Preflight that descriptor, create its typed zero during emission, emit the comparison, and
   return the result through generic callbacks.
3. Add focused selected-family and malformed-helper coverage; promote the original short-circuit
   fixture to direct runtime/PTX testing.
4. Format, build, run focused/full/runtime/PTX/`ptxas` validation, update durable status and this
   plan, self-review, and commit.

## Validation and Acceptance

Acceptance requires Release host builds; focused fake coverage proving integer, floating-point,
and Boolean truthiness descriptors and typed zeros; an adjacent malformed-helper negative before
provider mutation; direct runtime/PTX lanes for the original fixture; CUDA 12.9
`ptxas -arch=sm_70`; the full `slang-unit-test-tool/nvvm` prefix; pinned formatting; and
`git diff --check`.

The self-review inventories the resolver, zero-construction dispatch, removal of the identity
branch, and all test adjustments. Confirm the exact helper producer and input shape, verify that
the established semantic catalog owns the inequality contract, and remove any text parser,
provider-specific cast, or silent fallback.

## Failure and Recovery

If a selected family fails provider verification or libNVVM compilation, retain its exact
diagnostic and narrow the claimed scalar family rather than special-case emitted LLVM. Keep probes
under ignored `build/slice116-*`; do not reset unrelated work or stage
`external/slang-binaries/`.

## Artifacts and Hand-Off

Keep generated IR, PTX, cubin, and logs under ignored `build/slice116-*`. Distill the final
truthiness contract, validation evidence, and next measured corpus boundary into
`docs/design/nvvm-backend.md`, then commit this plan with the implementation as explicitly
requested.
