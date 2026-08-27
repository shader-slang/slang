# Slice 34: Add exact scalar float32 multiplication with scalable evidence

This ExecPlan follows `.agent/PLANS.md`. Keep it current. The user requires the completed plan to
ship with the slice, overriding the repository's default working-log policy for this experiment.

## Purpose and Observable Result

After this slice, the direct NVVM route compiles and runs the exact raw CUDA kernel
`*destination = left * right` for an AS1 `Ptr<float>` and two scalar float parameters. V3 gains a
MULTIPLY operation and semantic feature without another callback or table-size change. ADD,
SUBTRACT, and MULTIPLY share one production opcode mapping and descriptor-driven test runners, so
another same-shape floating binary operation does not require copied layer-by-layer test bodies.

## Progress

- [x] (2026-08-27) Recorded the Slice 33 baseline: 214 names, SHA-256
  `6ba1df40ff963723a866c61cbf8518aba7596e23213d5743015397547c90af9d`, Release 214/214, Debug
  10/10.
- [x] (2026-08-27) Audited the canonical producer and current evidence: `kIROp_Mul` already owns
  both integer and float multiplication, while ADD/SUBTRACT currently duplicate selection and
  several complete test bodies.
- [x] (2026-08-27) Added compatible feature/operation negotiation, unflagged provider `fmul`, and
  direct typed lowering while preserving signed-i32 multiplication.
- [x] (2026-08-27) Consolidated same-shape float32 binary source, topology, text, PTX, assembler,
  and runtime evidence behind descriptors and thin registered wrappers; added MULTIPLY coverage at
  every layer.
- [x] (2026-08-27) Formatting, standalone provider plus Release/Debug builds, focused 23/23, full
  Release 221/221, Debug 10/10, durable docs, and the input-shape self-review are complete.

## Surprises and Discoveries

- Observation: the third same-shape floating binary operation would otherwise repeat the Slice 33
  seven-test pattern and the emitter would gain another opcode-specific conditional.
  Evidence: ADD and SUBTRACT already have separate real-builder, fake-direct, capability,
  differential-PTX, `ptxas`, and runtime bodies even though their topology differs only by feature,
  wire operation, source token, and expected instruction.
  Consequence: make those differences descriptor data and keep thin names for diagnostic ownership.

- Observation: entry float types, pointer construction, loads, and stores still require the Slice
  31 ADD/type-prefix feature in addition to an operation-specific bit.
  Consequence: MULTIPLY follows the established compatibility contract in this slice. Separating a
  base float-type feature would change old-provider semantics and is not required to prove this
  operation.

- Observation: once float multiplication became valid, the historical negative source containing
  `int(x * y)` advanced to the following canonical `castFloatToInt`.
  Evidence: the first focused run passed 22/23 and reported E52017 `castFloatToInt` for that case;
  after updating the expected owner, all 23 focused cases passed.
  Consequence: retain the fixture as cast-boundary evidence rather than deleting it.

- Observation: the standalone provider deliberately does not import Slang assertion macros.
  Evidence: its first build rejected `SLANG_RELEASE_ASSERT` in `_emitFloatingBinaryV3`.
  Consequence: each already-validated enum arm writes its result and returns directly; the default
  still returns invalid argument without mutating output.

## Decision Log

- Decision: append `SCALAR_FLOAT32_MULTIPLY` as feature 22 and
  `FLOATING_BINARY_OP_MULTIPLY` as operation 2, with no V3 table member.
  Rationale: semantic feature negotiation belongs in the feature set; the generic two-operand
  callback already owns the ABI shape.
  Date/author: 2026-08-27, Codex.
  Revisit when: a floating operation changes arity/result, type width, or constrained-FP policy.

- Decision: emit ordinary unflagged LLVM `CreateFMul` and use exactly representable finite runtime
  cases.
  Rationale: this preserves the explicit Slice 18 policy and avoids claiming contraction,
  reassociation, NaN, denormal, signed-zero, or rounding behavior.
  Date/author: 2026-08-27, Codex.

- Decision: centralize canonical Slang IR opcode to feature/wire-operation/diagnostic mapping and
  centralize layer-independent floating-binary test facts, while retaining individual registered
  names as thin wrappers.
  Rationale: one source of truth prevents preflight/emission drift and keeps failures attributable
  to negotiation, builder text, direct topology, PTX, assembler, or runtime.
  Date/author: 2026-08-27, Codex.

## Outcomes and Retrospective

Feature 22 and floating operation 2 reuse the exact 464-byte x64/280-byte x86 V3 table. The generic
facade, provider, and fake dispatch MULTIPLY without another callback. The direct graph contains
`[FloatPointer, Float, Float]`, ordered parameter 1/2 inputs to one floating operation, and the
operation result consumed by the aligned destination store; signed-i32 multiplication remains on
the integer family.

LLVM and audited NVVM-2.0 text contain exactly one unflagged `fmul float`, one store, alignment,
kernel metadata, and no `fadd`/`fsub`. NVVM and NVRTC agree on `[64, 32, 32]`, token-safe `mul.f32`,
one global 32-bit store, and no global load/add/sub. CUDA 12.9 `ptxas` accepts both outputs. RTX 5090
runtime results agree for `1.5 * 2 = 3`, `-8 * 0.5 = -4`, and `1024 * -0.25 = -256`.

Seven names raise the prefix from 214 to 221; sorted LF-terminated SHA-256 is
`c24e6b4e82e289c2533444b0b0c0dab6cc44064a1df02d75a79928de94c2afa8`. The focused matrix passes
23/23 and full Release passes 221/221. Descriptor-backed consolidation reduces the five measured
test/support files from 19,608 to 19,512 physical lines while preserving all ADD/SUBTRACT names.
The standalone Release provider and Release/Debug test targets build successfully; the established
Debug compatibility lane passes 10/10.

## Context and Current Pipeline

The final linked IR for `left * right` contains canonical Float parameters and a `kIROp_Mul`.
`_validateNVVMFunction` currently accepts that opcode only as signed i32 in both validation passes,
and `emitNVVMIRFromLinkedIR` always sends it to the generic integer-binary callback. ADD/SUBTRACT
already show the principled type-directed split: canonical float values use the floating family;
canonical signed-i32 values keep the integer family.

At the provider boundary, `NVVMIRBuilder::emitFloatingBinary` maps a stable operation enum to a
semantic feature before calling the generic V3 function pointer. `_emitFloatingBinaryV3` validates
same-function, available, dominating LLVM `float` values at a live insertion point, then creates the
selected unflagged instruction. The fake records the same family, operation, ordered operands, and
result handle.

## Scope and Non-Goals

In scope are scalar float32 entry parameters/device destination, exact two-operand multiplication,
unflagged `fmul`, feature negotiation, a single production mapping for all accepted float binary
opcodes, descriptor-driven same-shape tests, LLVM/NVVM text, differential PTX, matching-root
`ptxas`, runtime agreement, and migrated unsupported-operation expectations.

Out of scope are constants, helpers, phis, casts, divide/remainder, negation, comparison,
FMA/contraction, half/double, vectors/matrices/aggregates, resources/atomics, new provider callbacks,
base-feature renumbering, text rewrites, and performance claims.

## Architecture and Invariants

An exact Slice 33 table without MULTIPLY remains valid and all existing operations keep working.
Advertising MULTIPLY requires the already-complete float prefix. Unknown enum values fail in the
facade before provider mutation. The provider accepts only the established LLVM `float` value
contract and selects ADD, SUBTRACT, or MULTIPLY by stable enum without fast-math flags.

One emitter-owned mapping describes each accepted canonical float binary opcode's semantic feature,
wire operation, and diagnostic. First-pass capability collection and final emission consume that
mapping. The second pass validates both operands through `_validateFloat32Value`. Signed-i32
`kIROp_Mul` remains on the integer feature and callback.

One test descriptor describes each same-shape float binary operation's feature, wire operation,
source, kernel/text token, PTX evidence, and exact runtime inputs. Layer-specific runners consume
only the fields they own. Registered wrappers retain every established ADD/SUBTRACT test name and
add corresponding MULTIPLY names.

## Interfaces and Dependencies

Change the V3 feature/operation enums, existing facade/provider/fake dispatch, direct emitter, and
decomposed NVVM tests. Update the design, capability ledger, and this plan. Do not add a struct
field, export, dependency, build target, provider wrapper, or packaging rule.

## Milestones

1. Append feature/operation constants and prove old feature sets and x64/x86 table sizes remain
   compatible.
2. Extend facade, provider, and fake generic dispatch with output clearing, no mutation for unknown
   operations, and exact `CreateFMul`.
3. Add one emitter mapping and admit/emit canonical float `kIROp_Mul` while preserving integer MUL
   and every adjacent float boundary.
4. Describe ADD/SUBTRACT/MULTIPLY once in test support; route real-builder, direct-topology,
   capability, differential-PTX, `ptxas`, and runtime wrappers through shared layer runners.
5. Prove missing-MULTIPLY E52016 before module construction, exact ordered fake topology, one
   unflagged `fmul float`, `[64,32,32]`, token-safe `mul.f32`, store/no load/add/sub, assembler
   acceptance, and exact finite signed runtime cases through both routes.
6. Format, build standalone provider and Release/Debug test targets, run focused/full lanes, hash
   names, measure test-file lines, update docs, self-review, and commit `slice 34`.

## Validation and Acceptance

Run focused negotiation, invalid-provider, builder, direct, capability, unsupported-boundary,
differential PTX, `ptxas`, and runtime tests, plus retained ADD/SUBTRACT wrappers. Then run the full
Release `slang-unit-test-tool/nvvm` prefix and established Debug 10/10 outside the sandbox. Build the
standalone Release provider and Release/Debug `slang-unit-test`/`slang-test` targets outside the
sandbox. Acceptance requires unchanged V3 sizes/V2, verified LLVM/NVVM text, matching PTX semantics,
runtime agreement, no lost registered names, formatted code, completed input-shape audit, and clean
diff checks.

## Self-Review and Input-Shape Audit

The new production inventory contains one helper, `_getNVVMFloat32BinaryInfo`, and closed enum
switches in the host facade, provider, and fake. The helper maps exactly three canonical `IROp`
values to feature/wire-operation/diagnostic data; it performs no custom equivalence, fallback,
canonicalization, syntax reconstruction, or operand-graph search. Numeric lowering produces
`kIROp_Add`, `kIROp_Sub`, or `kIROp_Mul`, and the canonical result type intentionally distinguishes
the accepted float and signed-i32 spellings. Removing the MUL mapping restores the observed E52017
at multiplication, while removing only the feature bit restores E52016 before module creation.

The provider default arm survives as validation of an out-of-contract wire enum and never mutates
the cleared output. The former floating-multiply negative fixture is intentionally retained: its
valid multiply now exposes the next real producer, `castFloatToInt`, proving no downstream cast
repair was added.

The test-only inventory is `NVVMFloat32BinaryTestCase`, its checked index lookup, one PTX-evidence
selector, and layer-specific negotiation, builder, direct, capability, PTX, assembler, and runtime
runners. They consolidate only same-shape setup. Thin wrappers preserve each established name, and
the runners still check exact feature/operation, ordered operands/result consumer, all mutually
exclusive LLVM/PTX opcodes, output clearing, assembler acceptance, and per-operation runtime cases.

## Failure and Recovery

If libNVVM rejects `fmul`, inspect provider LLVM and audited NVVM text before changing the writer.
If Slang source introduces a cast, helper, or alternate producer, narrow the fixture or split it.
Removing the new descriptor/feature/enum/mapping entry restores Slice 33; the retained ADD/SUBTRACT
wrappers must remain green throughout. Never delete or stage `external/slang-binaries/`.

## Artifacts and Hand-Off

The retained evidence is: feature 22/operation 2 with unchanged 464-byte x64/280-byte x86 V3;
closed production mapping and exact fake graph; one unflagged `fmul float`; `[64,32,32]`, token-safe
`mul.f32`, store/no-load/add/sub; runtime `3`, `-4`, `-256`; 221 names with SHA-256
`c24e6b4e82e289c2533444b0b0c0dab6cc44064a1df02d75a79928de94c2afa8`; 19,608 to 19,512
test/support lines; focused 23/23, full Release 221/221, and Debug 10/10; migrated cast boundary; and
the completed self-review. Stable facts are in the design and ledger. Commit this plan with Slice
34 and continue unless blocked.
