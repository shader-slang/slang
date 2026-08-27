# Slice 31: Add exact scalar float32 addition

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user requires the
completed slice plan to ship with its implementation, so this plan will be committed with Slice 31.

## Purpose and Observable Result

After this slice, the direct NVVM route compiles and runs this exact raw CUDA kernel shape:

```slang
[CUDAKernel]
void computeMain(
    uniform Ptr<float, Access::ReadWrite, AddressSpace::Device> destination,
    uniform float left,
    uniform float right)
{
    *destination = left + right;
}
```

The canonical linked IR uses scalar `Float`, an AS1 read-write pointer to `Float`, one `kIROp_Add`,
and one store. Slang owns the exact float32/ABI/operation policy, the V3 provider owns LLVM `float`
and unflagged `fadd` construction, libNVVM accepts the audited NVVM-2.0 text, matching-toolkit
`ptxas` accepts the PTX, and CUDA runtime results agree with NVRTC for exactly representable finite
normal inputs.

This is the first post-scalability semantic slice. It demonstrates that Slice 29's type owner and
Slice 28's generic provider growth support a second scalar family without adding another V2 field,
per-operation host wrapper, fake state bundle, or repeated end-to-end harness.

## Progress

- [x] (2026-08-27) Re-read the durable roadmap, Slice 28-30 architecture, current type legalizer,
  V3 table negotiation, provider validators, and the unsupported floating-point matrix.
- [x] (2026-08-27) Selected exact float32 addition as the smallest type-family expansion that
  exercises a new provider type and operation family without libdevice, constants, helpers, or
  control flow.
- [x] (2026-08-27) Captured the exact 193-name baseline and SHA-256
  `1f35f717b93e1cb62c3f872e99b819386ab9c5474b203256e58ee1bdb41c97b7`, then probed the final
  linked-IR, LLVM, NVVM, and PTX shapes.
- [x] (2026-08-27) Extended V3 compatibly with generic floating-point type/binary operations and
  exact feature negotiation, including provider invalid/no-mutation tests.
- [x] (2026-08-27) Legalized and emitted the exact float32 parameter/pointer/store/add graph while
  retaining direct loads and every adjacent floating producer as deterministic rejection boundaries.
- [x] (2026-08-27) Added fake topology, real builder, differential PTX, `ptxas`, and runtime
  evidence; updated durable design/ledger records; and passed Release 201/201 plus Debug 10/10.

## Surprises and Discoveries

- Observation: canonical floating arithmetic uses the same `kIROp_Add` opcode as integer addition;
  the result type selects semantics.
  Evidence: `core.meta.slang` maps numeric `add` to `kIROp_Add`, while
  `_validateNVVMFunction` and `_emitNVVMModule` currently classify that opcode as signed-i32 only.
  Consequence: extend the existing typed dispatch at preflight/emission rather than inventing a
  float-only IR spelling or rewriting the linked IR.

- Observation: V3's current 448-byte x64/272-byte x86 core can grow without invalidating existing
  V3 providers.
  Evidence: `NVVMIRBuilder::initialize(V3)` retains only the lesser of provider and local sizes,
  and semantic availability is already carried by independent feature bits.
  Consequence: append a coherent floating prefix, require it only when the float32-add bit is
  advertised, and continue accepting the exact Slice 28 V3 core for all established programs.

- Observation: several adjacent floating tests currently stop at their entry-point parameters.
  Evidence: float multiply, negate, equality, inequality, and ordered-comparison fixtures all expect
  E52017 `entry-point parameter` because `isNVVMSupportedParameterType` admits only established
  integer/resource shapes.
  Consequence: accepting float32 parameters will intentionally move those tests to their actual
  unsupported operation. Update each expectation to that later producer while keeping provider
  discovery at zero; do not broaden those operations in this slice.

- Observation: accepting float pointers makes a direct `*destination` read reach the load producer.
  Evidence: the initially generalized scalar load path could emit a float load, although the
  motivating add kernel consumes only parameter values and stores its result.
  Consequence: keep direct float loads rejected at E52017 `load result type`. A later memory slice
  must establish its own source topology, provider evidence, PTX load contract, and runtime oracle.

## Decision Log

- Decision: add one generic V3 floating-point type constructor and one generic floating binary
  dispatcher, not an `emitFloat32Add` callback.
  Rationale: type construction and same-shaped floating binary operations are semantic families.
  Future operations may extend stable enums/features without growing the table or host wrapper.
  Date/author: 2026-08-27, Codex.
  Revisit when: a floating operation has a different result/operand ABI or constrained-mode contract
  that cannot be represented by the binary family.

- Decision: the advertised semantic and the provider type constructor are both exact scalar
  float32; bit widths other than 32 fail.
  Rationale: accepting an unexercised half/double provider type would create capability without
  Slang ABI, precision policy, PTX, or runtime evidence.
  Date/author: 2026-08-27, Codex.
  Revisit when: a later slice supplies complete half or double evidence.

- Decision: allow float32 only in raw entry-point parameters, AS1 float pointers, stores, and
  addition results in this slice; direct float loads remain unsupported.
  Rationale: float constants, phis, helper ABI, subtraction/multiplication/negation/comparisons,
  casts, arrays, resources, atomics, and libdevice calls each introduce independent contracts.
  Date/author: 2026-08-27, Codex.
  Revisit when: the next bounded float slice selects one of those producers with its own evidence.

- Decision: runtime equality uses exactly representable finite normal operands and expected sums.
  Rationale: this proves launch ABI and arithmetic routing without accidentally claiming NaN,
  signed-zero, denormal, rounding-mode, or fast-math behavior beyond Slice 18's compiler policy.
  Date/author: 2026-08-27, Codex.
  Revisit when: a numerical-policy slice explicitly covers those edge classes.

## Outcomes and Retrospective

The exact raw source lowers to three canonical entry parameters—read-write AS1 `Ptr<Float>`,
`Float`, `Float`—one `kIROp_Add` whose ordered operands are parameters 1 and 2, and one store of that
result through parameter 0 with alignment 4. The fake observes one float construction, one AS1
float-pointer construction, parameter kinds `[FloatPointer, Float, Float]`, one generic
`FloatingBinary/ADD` record, and the exact store consumer. There is no load, constant, helper, phi,
cast, or reconstructed value graph.

The V3 x64 layout grows from the exact 448-byte core to 464 bytes. On x86, the old core's complete
terminal-callback minimum is 268 bytes, the appended float prefix completes at 276 bytes, and
structure padding makes `sizeof` 280. Exact old cores are accepted without the feature; advertised
partial sizes and either null callback are rejected; future-larger tables are clamped. V2 is
unchanged and exposes no float feature. Provider invalid tests reject bit width 16, integer/mixed,
foreign-module, unavailable, non-dominating, null, and terminated-insertion shapes before mutation.

The real builder verifies LLVM and audited NVVM-2.0 text containing a `float addrspace(1)*` kernel
parameter, one unflagged `fadd float`, a four-byte-aligned `store float`, and kernel annotation. No
additional text rewrite is needed. Direct NVVM and NVRTC agree on `[64, 32, 32]`, token-safe
`add.f32`, one global 32-bit store, and no global load; matching-root CUDA 12.9 `ptxas` accepts both.
On the RTX 5090 both routes produce `3.75`, `-7.5`, and `768` for the three exact finite-normal
cases.

Accepting float parameters intentionally moves multiply, negation, atomic, and comparison fixtures
to their exact unsupported operation; direct load now stops at `load result type`, and half/double
still stop at `entry-point parameter`. The registered prefix grows by eight names from 193 to 201.
Its exact sorted LF-terminated SHA-256 is
`73434ac732eccaf42c9fad54ad2956b13aa5e2371e9e2e72d5fbbc2aaaf6e2e2`. Release passes 201/201,
focused Slice 31 coverage passes 9/9, Debug preservation passes 10/10, and both Release/Debug builds
plus the standalone provider build succeed.

## Context and Current Pipeline

`emitEntryPointsDirectlyToNVVM` first calls `validateIRForDirectNVVM`, which walks the selected entry
point and reachable helpers through `_validateNVVMFunction`. Parameters are admitted by
`isNVVMSupportedParameterType`; the completed slice admits exact float parameters and pointers,
keeps float loads outside the subset, validates float stores, and dispatches `kIROp_Add` by its
canonical result type. Provider discovery still occurs only after that whole walk succeeds.

Emission creates one `NVVMTypeLoweringContext` per provider module. It lowers canonical source types
to module-owned handles, declares functions, maps parameters/blocks, then emits each ordinary
instruction. The current add arm always calls `NVVMIRBuilder::emitIntegerBinaryOperation`.
Serialization uses the provider's audited NVVM-2.0 text writer before the registered libNVVM
compiler produces PTX.

The V3 API freezes complete V2 as a compatibility core, carries four feature words, and currently
ends in generic integer unary/binary/compare callbacks. The real provider owns LLVM objects behind
opaque handles and validates module ownership, type, availability, dominance, insertion state, and
output clearing before mutation. Slice 31 extends those same ownership rules to one floating family.

## Scope and Non-Goals

In scope are canonical scalar float32 entry parameters, AS1 read/write float pointers, float
stores, one two-operand `kIROp_Add`, a generic V3 floating type/binary prefix, one feature bit,
fake/real provider validation, direct topology, differential PTX, `ptxas`, runtime comparison, and
the adjacent-negative expectation changes exposed by accepting float parameters.

Out of scope are direct float loads, float constants, block parameters/phis/loops, helper
parameters/results/calls, subtraction, multiplication, division, remainder, negation, comparisons,
casts, half, double, vectors, matrices, arrays, structs, raw resources, local/shared/global
variables, floating atomics, libdevice demand, FMA contraction, fast-math flags, denormal/rounding
expansion, and performance claims.

## Architecture and Invariants

Append this coherent optional prefix to V3 conceptually:

```text
getFloatingPointType(module, bitWidth, outType)
emitFloatingBinary(module, operation, left, right, outValue)
```

The first operation creates provider-owned LLVM `float` only for bit width 32 and clears failed
outputs. The binary operation enum initially contains ADD. The provider accepts ADD only for two
available, same-module, same-function, same-type LLVM `float` operands that dominate an unterminated
insertion point; it emits one unflagged `CreateFAdd`. Unknown operations and half/double, integer,
mismatched, foreign, unavailable, or non-dominating operands fail before mutation.

Feature bit `SCALAR_FLOAT32_ADD` requires both appended callbacks and the complete appended size.
An exact Slice 28 V3 table without that bit remains valid for every established program. A table
advertising the bit with a partial prefix or null callback is malformed. Future-larger tables and
unknown feature bits retain the existing clamping/forward-compatibility rules. V2 synthesizes no
float feature and therefore stops a float program at E52016 before module creation.

The type legalizer recognizes exact canonical `BaseType::Float` and exact AS1 `Ptr<float>` with Read
or ReadWrite access. Source access remains legality metadata; both pointer qualifiers share one LLVM
`float addrspace(1)*` representation key. Stores require exact pointee/value agreement and
four-byte alignment; direct float loads remain rejected. The add producer and both operands must
all be canonical float32 and available; integer ADD keeps its current path and feature.

The fake records a generic floating binary operation with ordered operands, result, and insertion
block. It does not gain `floatAddStorage`, `floatAddCallCount`, or a dedicated callback body. New
tests reuse Slice 30's shared compile/`ptxas`/runtime patterns where shapes match, while keeping a
separate floating descriptor family because its type, ABI, PTX, and oracle differ from integers.

## Interfaces and Dependencies

Expected production changes are in:

- `source/compiler-core/slang-nvvm-ir-builder-api.h` for the append-only V3 prefix, operation enum,
  feature bit, and minimum sizes;
- `source/compiler-core/slang-nvvm-ir-builder.{h,cpp}` for size/feature validation and the two generic
  facade methods;
- `source/slang-llvm-nvvm/slang-llvm-nvvm.cpp` for LLVM type/FAdd construction and provider export;
- `source/slang/slang-emit-nvvm-type-lowering.{h,cpp}` for canonical float32/pointer legalization;
- `source/slang/slang-emit-nvvm.cpp` for preflight feature/type/operation validation and typed add,
  pointer and store emission.

Test/support changes remain in the decomposed NVVM files from Slice 27. Durable results update
`docs/design/nvvm-backend.md`, `docs/design/nvvm-backend-capability-ledger.md`, and this plan. No new
third-party dependency, generator, public API, compile target, or packaging rule is introduced.

## Milestones

1. Record the exact 193-name baseline and focused/preservation status. Add the float32-add source and
   inspect the final linked IR to confirm canonical `Float`, AS1 pointer, `kIROp_Add`, and store with
   no cast/helper/constant producer.
2. Append the V3 floating prefix and feature. Prove exact old-V3 compatibility, partial/null
   rejection when advertised, future-size clamping, unknown-op no mutation, output sanitization,
   and V2 absence of the feature.
3. Add provider LLVM `float` and generic FADD construction. Prove exact assembly/bitcode and reject
   bad widths/types/modules/functions/availability/dominance/insertion/output shapes before mutation.
4. Extend canonical type legalization and direct preflight for exact entry float32 values/pointers,
   stores, and ADD. Audit every adjacent floating fixture whose first failure moves; retain direct
   loads and all other non-scope shapes as deterministic E52017 boundaries before provider discovery.
5. Emit the graph through the generic facade and prove fake ordered topology, type-cache reuse,
   missing-feature E52016 before module creation, provider diagnostics, and old integer behavior.
6. Compare direct NVVM with NVRTC for ABI widths, token-safe `add.f32`, and global 32-bit store;
   assemble both with matching-root `ptxas` and run exactly representable finite cases on CUDA.
7. Format, self-review new helpers/special cases with the input-shape audit, run Release/Debug
   evidence, update design/ledger/outcomes, and commit the completed plan and implementation.

## Validation and Acceptance

Build Release and Debug test targets outside the sandbox. Run the complete
`slang-unit-test-tool/nvvm` prefix, the established Debug preservation 10/10, the new real-builder,
direct, capability, differential-PTX, `ptxas`, and runtime tests, and the full existing scalar
runtime matrix. Enumerate the registered names from the Release binary and record the new count/hash.

Provider tests must cover exact Slice 28 V3 compatibility, the new complete prefix, every partial
size while the feature is advertised, either null callback, unknown enum, stale-output clearing, and
pre-mutation validation. Direct negatives must prove float subtraction/multiply/negate/comparison,
loads, helpers/constants/phis/casts, half/double, arrays/resources/atomics, and bad pointer
qualifiers or address spaces remain outside the subset at the correct first producer.

Acceptance requires verified LLVM and NVVM-2.0 assembly, libNVVM compilation, semantic PTX rather
than text equality, matching-root `ptxas` acceptance, NVRTC/NVVM runtime agreement, unchanged
integer evidence, no V2 growth, no per-operation fake/harness duplication, readable formatted code,
and `git diff --check` success.

## Self-Review and Input-Shape Audit

The production helper inventory contains `_asNVVMSupportedDevicePointerType` and
`_validateFloat32Value`. The pointer helper survives because it is the single exact classifier used
by the signed-i32 and float32 pointer entry points; it compares canonical pointee identity and
address space at the type-legalization boundary rather than defining structural equivalence. The
value helper survives because availability/dominance is an existing emitter invariant and the
canonical float type is the source of truth. It neither rebuilds syntax nor searches operand graphs.

The test helpers `_populateFloat32AddKernel` and `_runFloat32AddKernel` survive because they share
provider-module construction and two-route launch mechanics, respectively. The fake uses the
existing family operation stream rather than adding an fadd-specific fallback, counter, or storage
bundle. There are no new recovery paths or silent defaults.

The audited input is the canonical final linked IR produced from the exact raw CUDA source:
`Float` parameters, one device `Ptr<Float>`, `kIROp_Add`, and `kIROp_Store`. This is intentional
canonical input, not an alternate spelling created upstream. The direct emitter owns the selected
backend's supported-subset and feature check, the type context owns representation construction,
and the provider owns LLVM object validity. Removing any of those checks fails the corresponding
negotiation, invalid-provider, topology, or adjacent-negative test at that owner. No producer-side
representation repair or custom semantic equivalence is needed.

## Failure and Recovery

Keep the new V3 feature unadvertised until both provider callbacks and validation pass. Exact old V3
tables remain the rollback path. If libNVVM rejects generic LLVM 14 text, inspect the verifier log and
audited NVVM-2.0 writer before adding any rewrite; only a measured dialect difference with semantic
validation belongs there. If final IR contains a cast/helper/constant, narrow the source fixture or
split that producer into a later slice rather than silently broadening scope.

Do not delete or stage `external/slang-binaries/`. Remove temporary IR/PTX/name manifests and probe
artifacts before committing. Re-running tests is safe; generated outputs stay under ignored build or
temporary directories.

## Artifacts and Hand-Off

The retained evidence is the exact three-parameter linked graph; x64 448/464-byte and x86
268/276/280-byte compatibility data; verified LLVM/NVVM text; `[64, 32, 32]`, `add.f32`, store/no-load
PTX classification; runtime values `3.75`, `-7.5`, and `768`; adjacent-negative migration inventory;
201-name hash `73434ac732eccaf42c9fad54ad2956b13aa5e2371e9e2e72d5fbbc2aaaf6e2e2`;
Release 201/201; Debug 10/10; and the self-review disposition above. The settled contracts are also
recorded in the durable design and capability ledger. Commit this completed plan with Slice 31
before selecting another semantic capability.
