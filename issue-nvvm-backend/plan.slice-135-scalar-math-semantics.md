# Represent the common scalar math family

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires the completed slice plan to be committed with its implementation, overriding the normal
working-log policy for this experimental backend.

## Purpose and Observable Result

After this slice, the common scalar math helpers inside the residual ordinary-intrinsic cluster are
represented by reusable typed semantics rather than admitted one fixture at a time. The measured
family contains 34 of the 47 post-Slice-134 first blockers: scalar absolute value, transcendental
and exp/log operations, rounding/root operations, `fmod`, `frac`, `isnan`, and `sign` over their
canonical selected scalar signatures.

The slice will use the existing generic value-operation descriptor/query/emit interface. It may add
one forward-only ABI revision containing only missing concrete operation IDs; it will not add a
math-specific callback, fixture checks, source-name dispatch, or semantic text rewriting. Existing
operation IDs such as `SQRT` and `TRUNC` must be reused. Every workload that becomes correct at both
O0 and O3 will receive direct regression lanes; later blockers remain measured failures.

## Progress

- [x] (2026-08-30) Committed Slice 134 as `c07ab0288`; the 452-row census reports 47 remaining MVP
  ordinary-intrinsic failures and no old-correct regression.
- [x] (2026-08-30) Grouped all 47 rows by exact assembly and specialized signature. Thirty-four
  first blockers form one common scalar math/classification family; output-parameter transport,
  bit reinterpretation, and Half transport account for the remainder.
- [x] (2026-08-30) Traced each canonical CUDA prelude producer and defined exact integer, Half, Float32, Float64,
  Boolean-result, and Int32-result contracts.
- [x] (2026-08-30) Extended the typed semantic vocabulary and shared resolver without adding a provider callback;
  prove exact libdevice demand before provider mutation.
- [x] (2026-08-30) Implemented and tested integer/floating absolute value, scalar libdevice calls, typed composite
  operations, classification, and deterministic adjacent rejection.
- [x] (2026-08-30) Reran the 34-row family and the fixed 452-row census at O0/O3, promoted all newly correct
  workloads, refresh Pareto and representative metrics, format, validate, self-review, and commit
  Slice 135.

## Surprises and Discoveries

- The residual family is much larger than its leading spellings suggest. `abs` blocks seven rows
  and `tan` five, while the same scalar-math representation covers 22 more first blockers across
  exp/log, round/root, classification, and binary operations.
- Four `abs(int)` helpers appear in matrix/vector fixtures because vector mapping specializes to
  scalar linked helpers. The canonical direct-NVVM shape is still scalar; admitting arbitrary
  vector descriptors would widen a representation that the producer does not emit.
- Existing `SQRT` and `TRUNC` IDs already describe two members. New operation vocabulary must not
  duplicate them merely because Float64 overloads are newly measured.
- `$P_frac($0)` and `$P_sign($0)` are composite semantics rather than standalone generic LLVM
  instructions. Their implementation must be typed composition, not a guessed libdevice symbol.
- The family removes all 34 selected first blockers, but only 27 workloads become correct. Six
  expose later deterministic blockers and `matrix-float.slang` exposes a runtime mismatch at both
  optimization levels. Promotion remains based on full differential correctness, not compilation.
- The first aggregate promotion run also ran unrelated WebGPU directives and reproduced four local
  Dawn failures. `-api-only -api cuda` isolates the 27 native plus 54 direct lanes and passes 81/81.

## Decision Log

- Decision: select the 34-row common scalar math family as one vertical slice.
  Rationale: it is the largest coherent subset of the largest remaining ordinary-intrinsic cluster
  and represents operations expected in real numerical kernels. Output-parameter and bit-transport
  helpers have different ABI invariants and remain separate measured work.
  Date/author: 2026-08-30, Codex.
- Decision: classify exact final helper text and concrete signature through the generic one-block
  value-helper recognizer established in Slice 134.
  Rationale: CUDA target specialization produces those linked `IRGenericAsm` functions. Assembly
  plus specialized types are the canonical producer output; fixture paths and source names are not.
  Date/author: 2026-08-30, Codex.
- Decision: use exact libdevice calls for CUDA scalar math where ordinary LLVM instructions do not
  fully define the CUDA contract, and typed LLVM composition for `frac`, `isnan`, `sign`, and
  integer/Half absolute value.
  Rationale: libdevice is the existing CUDA semantic source for selected scalar math. Composite
  operations have explicit type-level definitions and need no provider API beyond generic value
  emission.
  Date/author: 2026-08-30, Codex.

## Outcomes and Retrospective

Builder ABI revision 27 adds 20 missing scalar-math operation IDs to the existing generic callback.
The compiler's one-block helper classifier is table-driven; the shared resolver owns exact signed
integer/Half/Float32/Float64, Bool-result, and Int32-result legality plus pre-mutation libdevice
demand. The provider uses exact libdevice functions, typed composite construction, and generalized
Float32/Float64 square-root validation without semantic text rewriting.

The focused 34-row run is correct for 27 workloads, advances six to later canonical blockers, and
exposes one matrix runtime mismatch at both O0 and O3. The 27 correct workloads receive 54 direct
lanes; the full promoted CUDA set passes 81/81 and the selected regression prefix passes 404/404.
The fixed 452-row census reaches 264 correct at O0 and 260 at O3, an exact +27/+27 delta with no
old-correct regression. Among 427 healthy MVP rows, O0/O3/both correctness is 263/258/255. The
ordinary-intrinsic cluster falls from 47 to 18.

The three representative gates remain correct. Direct O3 PTX remains accepted by CUDA 12.9 for
SM70, SM80, and SM90; physical runtime remains on SM120. CUDA 13 and physical SM70/SM80/SM90
workers remain infrastructure gaps. The remaining leading MVP clusters are wave/reconvergence
(31), helper ABI contracts (28), aggregate/pointer/layout transport (23), ordinary intrinsics (18),
and ordinary numeric/bit operations (17).

Self-review inventory: the 20 new IDs survive because each is carried by the established generic
descriptor and exercised by canonical CUDA-prelude helpers; the table-driven spelling mapping
replaces the previous six-branch chain; Float classification and sign families survive because
their result types differ structurally from same-type floating unary math; exact libdevice mapping
survives real Float32/Float64 corpus comparison; and the five typed composites survive focused
provider checks and differential workloads. No fixture dispatch, compatibility fallback, syntax
reconstruction, or malformed-upstream repair was added. Removing the family resolver admissions
returns the 34 rows to E52017, while removing a provider construction makes its exact descriptor
fail focused emission, so the producer/consumer boundary is demonstrated at the intended layer.

## Context and Current Pipeline

CUDA prelude intrinsic expansion leaves one-block linked `IRFunc` helpers containing final
`IRGenericAsm` spellings such as `$P_abs($0)`, `$P_tan($0)`, or `$P_pow($0, $1)`. Slice 133 made
those diagnostics exact. Slice 134 generalized compiler recognition so one typed helper path now
handles exact min/max and integer-bit operations before falling through to unsupported E52017.

The host and isolated LLVM 14 provider share the semantic resolver. A descriptor carries operation,
result type, operand types, and arity through exact support query and emission. The module's
libdevice requirement is collected before provider creation and module mutation. The provider emits
typed LLVM construction, serializes strictly validated NVVM IR 2.0 text, links the selected CUDA
toolkit's libdevice when required, and asks libNVVM to verify and compile PTX.

## Scope and Non-Goals

In scope are canonical scalar absolute value; selected Float32/Float64 `acos`, `asin`, `atan`,
`atan2`, `ceil`, `exp`, `exp2`, `floor`, `fmod`, `frac`, `log`, `log2`, `log10`, `pow`, `round`,
`rsqrt`, `sqrt`, `tan`, and `trunc`; Float32/Float64 `isnan`; and Float32/Float64 `sign` returning
Int32. Half absolute value is in scope only if typed construction and real libNVVM evidence preserve
its exact scalar contract. Selected scalar signed-integer absolute value is in scope.

Out of scope are `frexp` and `sincos` output-parameter ABI, Double/UInt32 multi-result bit transport,
opaque Half packing/unpacking, vectors as provider descriptors, wave/reconvergence work, unrelated
helper ABI failures, and new toolkit workers. If a candidate math symbol or overload lacks exact
libdevice or typed evidence, keep that descriptor unsupported and record it rather than guessing.

## Architecture and Invariants

The compiler accepts only one-block, asm-only helpers whose exact spelling maps to a concrete
operation and whose specialized signature resolves through the shared family. Ordinary same-type
Float32/Float64 unary and binary math is distinct from Boolean `isnan` and Int32 `sign` results.
Integer absolute value accepts signed selected scalars and preserves the same type. Half absolute
value, if retained, is exact same-type scalar construction without libdevice.

Capability discovery and emission must use the same resolver. Device-library demand is part of the
resolution and must be known before provider mutation. Provider symbol lookup is one operation,
width, and arity mapping shared by support tests and emission. Unsupported widths, vectors, result
types, arities, and assembly spellings retain deterministic E52017 or provider rejection.

## Interfaces and Dependencies

Reuse `SlangNVVMBuilderValueOperationsAPI`. Add only missing `SlangNVVMValueOperation` IDs in one
forward-only ABI revision. Reuse `SQRT` and `TRUNC`. Extend the shared semantic catalog, compiler
helper classifier, fake-provider recorder, provider libdevice map/typed construction, and focused
tests together. The selected CUDA 12.9 `libdevice.10.bc` is the local symbol/behavior authority;
CUDA 13 coverage remains an explicit infrastructure requirement.

## Milestones

First, freeze the 34-row exact inventory and map each spelling/signature to typed semantics. Second,
add resolver and adjacent-negative coverage and extend the generic helper classifier. Third,
implement real-provider typed/libdevice lowering and prove LLVM 14, NVVM IR 2.0, libNVVM, PTXAS,
and runtime behavior. Fourth, rerun the family at O0/O3 and promote only full differential
successes. Finally, run the fixed census, selected prefix, representative metrics/SM70/80/90
assembly, formatter, diff/self-review, and commit all durable evidence.

## Validation and Acceptance

Acceptance requires Release host and isolated-provider builds, focused fake- and real-provider
tests, the selected NVVM prefix, all promoted direct lanes, the 34-row family, and the complete
452-row three-mode census. Compare exact workload sets against Slice 134 and require zero
old-correct regression. Record compilation success, runtime mismatches, preflight/provider failures,
healthy-MVP O0/O3/both correctness, post-slice Pareto counts, and which rows only reach later
blockers. All representative gates must remain correct and their direct O3 PTX must assemble for
SM70, SM80, and SM90.

## Failure and Recovery

Build and generated census artifacts stay under existing build directories and are safe to rerun.
If a libdevice symbol is absent or an overload fails verification/runtime comparison, retain the
exact evidence and remove that admission. Do not substitute a merely similar LLVM operation, add a
fixture exception, or patch malformed upstream IR. Generated mirrors, logs, PTX, cubins, and raw
inventories remain ignored.

## Self-Review

Before commit, inventory every operation ID, mapping, helper, composite, and special case. For each,
record the canonical producer, exact contract, owning tests, and revert result. Remove duplicated
operation vocabulary, source/fixture dispatch, syntax reconstruction, compatibility fallbacks,
unqueried provider operations, and accidental vector/width admission. Confirm that every libdevice
demand is resolved pre-mutation and every retained composite preserves NaN, signed-zero, sentinel,
and integer-overflow behavior required by its canonical helper.

## Artifacts and Hand-Off

Commit the completed plan, implementation, promoted fixtures, post-slice census TSV/Pareto JSON,
and Slice-135 report. Keep generated raw artifacts ignored. The report must distinguish newly
correct rows from later blockers and keep the fixed denominator, representative gates, and
infrastructure gaps visible.
