# Prompt: tests-agentic/ir-reference/differentiation/

See [`_common.md`](_common.md) for universal rules. Those rules apply
to this bundle and override nothing here unless explicitly noted.

## Target

Produce the test bundle at
`tests-agentic/ir-reference/differentiation/`, anchored to
[`docs/llm-generated/ir-reference/differentiation.md`](../../../docs/llm-generated/ir-reference/differentiation.md).

Audience: nightly CI. This bundle is the **per-opcode reference**
for the IR opcodes used by the automatic-differentiation passes:

- differential-pair **construction** opcodes (`MakeDiffPair`);
- differential-pair **projection** opcodes (`GetPrimal`,
  `GetDifferential`);
- the **forward-mode** translation opcode (`ForwardDifferentiate`,
  the lowering of the surface `__fwd_diff(f)` / `fwd_diff(f)`
  expression);
- the **reverse-mode** legacy-bridge translation opcode
  (`LegacyBackwardDifferentiate`, the actual LOWER-TO-IR spelling
  of `__bwd_diff(f)` / `bwd_diff(f)`);
- the **detach** opcode (`detachDerivative`, lowering of the
  surface `detach(...)` call);
- the **`no_diff`** parameter marker (visible as
  `Attributed(T, %no_diff)` on a parameter type at LOWER-TO-IR
  stage);
- the user-facing **differential-pair type** (`DiffPair(T, witness)`)
  that all of the construction/projection opcodes produce or
  consume.

This bundle is adjacent to:

- `ir-reference/types` — the type-level half (`DifferentialPairType`,
  `DifferentialPtrPairType`, the `*FuncType` family for
  differentiated signatures, propagate-context types). The pair
  *type* appears here only as the result/operand type of a
  construction/projection opcode; type-level claims belong to the
  types bundle.
- `ir-reference/structure` — `func`, `key`, `interface`,
  `witness_table`. The autodiff witnesses (`IDifferentiable`,
  `IForwardDifferentiable`, `IBackwardDifferentiable`) surface as
  ordinary structural interfaces in the core-module preamble;
  this bundle observes the differentiation opcodes that *use* those
  witnesses, not the witnesses themselves.
- `ir-reference/misc` — `DifferentiableTypeAnnotation` and
  `DifferentiableTypeDictionaryItem` (the annotation-side opcodes
  that the autodiff passes attach to types). Those are out of scope
  here; record any natural-surface observation of them as a doc
  gap.
- `cross-cutting/ir-instructions` — the category-level sampler.
  This bundle drills the differentiation family in detail.

Anchor each test at the IR opcode that the doc names; do not write
tests for opcodes the doc lists as `(synthesized)` and that have no
natural Slang surface — those are internal to the autodiff IR
passes and never observable from a user program. Record them under
`## Untested claims` in `README.md`.

## The translation rule: claims to observations

`differentiation.md` describes each opcode in tables grouped by role
(`Differential-pair construction`, `Differential-pair projection`,
`Differentiation operators` (`Forward-mode`, `Reverse-mode`,
`Legacy bridge`, `Synthesized derivative witnesses`),
`Autodiff temporaries`, `Differential type info`,
`Checkpointing and rematerialization`). Each row names an
`Opcode`, `C++ wrapper`, `Operands`, `Flags`, `AST origin`, and
`Summary`. The testable consequences are:

- **"Surface form X lowers to opcode Y with operand shape Z"** —
  compile to a text target with `-dump-ir -o /dev/null` and
  FileCheck for the opcode name and its operand shape in the
  LOWER-TO-IR section of the dump. This is the primary mode.
- **"The opcode produces a value of type T"** — observable as the
  `let %N : T = OpcodeName(...)` line in the LOWER-TO-IR dump.
  Pair-construction opcodes produce `DiffPair(T, witness)`;
  projection opcodes produce the underlying scalar.
- **"This opcode is hoistable"** — observable indirectly: two
  syntactic occurrences of `__fwd_diff(f)` in one entry point
  dedupe to one `ForwardDifferentiate(%f)` IR value at LOWER-TO-IR.

### Observable claims (write tests for these)

The catalog row → claim mapping that is reliably observable from
natural surface code via `-dump-ir`:

- **`MakeDiffPair`** — constructing a `DifferentialPair<T>(primal,
  diff)` at a call site of a `__fwd_diff(f)` invocation lowers to
  `let %N : DiffPair(T, %witness) = MakeDiffPair(%primal, %diff)`
  in the caller body. Use `uniform` globals as operands to defeat
  constant folding.
- **`GetPrimal`** — reading `.p` off a returned differential pair
  lowers to `let %N : T = GetPrimal(%pair)`.
- **`GetDifferential`** — reading `.d` off a returned differential
  pair lowers to `let %N : T = GetDifferential(%pair)`.
- **`ForwardDifferentiate(%baseFn)`** — `__fwd_diff(f)` (or the
  alias `fwd_diff(f)`) on a `[ForwardDifferentiable]` function
  lowers to a module-scope (or per-entry-point-scope) `let
  %fwd_diff : Func(DiffPair(...), DiffPair(...)) =
  ForwardDifferentiate(%f)` declaration whose single operand is
  the base function. The opcode is hoistable so two source
  occurrences referring to the same `%f` dedupe to one
  declaration.
- **`LegacyBackwardDifferentiate`** — `__bwd_diff(f)` (or the alias
  `bwd_diff(f)`) on a `[BackwardDifferentiable]` function lowers
  at LOWER-TO-IR to `let %bwd_diff : Func(Void,
  BorrowInOutParam(DiffPair(T, %w)), T) =
  LegacyBackwardDifferentiate(%apply_bwd, %remat, %ctx_t)`. The
  three operands are the bwd-callable function, the remat
  function, and the intermediate-context type. The
  `BackwardDifferentiate` opcode itself (without `Legacy`) is not
  emitted at LOWER-TO-IR — that's a doc gap to record.
- **`detachDerivative`** — `detach(x)` inside a
  `[ForwardDifferentiable]` or `[BackwardDifferentiable]` body
  lowers to `let %N : T = detachDerivative(%x)`.
- **`no_diff` parameter marker** — a parameter declared as
  `no_diff T x` on a differentiable function surfaces as `param
  %x : Attributed(T, %no_diff)` on the function's entry block at
  LOWER-TO-IR, where `%no_diff` is the unique `no_diff` value
  bound at module scope (`let %k : Void = no_diff`).
- **`DiffPair(T, witness)` result type** — every
  `MakeDiffPair`/`ForwardDifferentiate` value carries a result
  type spelled `DiffPair(T, %witness)`; the witness operand is the
  `IDifferentiable` witness for `T`. This is the cross-link
  between the construction opcode and the differential type
  family.

### Untested claims (record under the bundle's out-of-scope heading)

Per the doc, the following opcodes are `(synthesized)` and produced
only by the internal autodiff IR passes (`slang-ir-autodiff-fwd.cpp`,
`slang-ir-autodiff-rev.cpp`, `slang-ir-autodiff-unzip.cpp`,
`slang-ir-autodiff-transcribe.cpp`). They have no natural surface
that produces them at LOWER-TO-IR — observing them would require
inspecting later-pass IR dumps, which is out of scope for the
per-opcode reference bundle:

- **`MakeDiffRefPair`** — synthesized; pointer-typed primal/
  differential pair, used inside autodiff-pass plumbing.
- **`GetDifferentialPtr`**, **`GetPrimalRef`** — synthesized
  pointer-projection counterparts of `GetDifferential`/`GetPrimal`.
- **`ForwardDifferentiatePropagate`**, **`TrivialForwardDifferentiate`**
  — synthesized by the unzip / transcribe pipeline; not produced
  by `__fwd_diff` at LOWER-TO-IR.
- **`BackwardDifferentiate`** itself — the doc lists `__bwd_diff`
  as its AST origin, but LOWER-TO-IR actually emits
  `LegacyBackwardDifferentiate`. Treat the modern opcode as
  out-of-scope at LOWER-TO-IR and record a doc gap.
- **`BackwardDifferentiatePrimal`**, **`BackwardDifferentiatePropagate`**,
  **`BackwardRemat`**, **`TrivialBackwardDifferentiate*`** —
  produced by the unzip pass, not by user surface.
- **`BackwardFromLegacyBwdDiffFunc`** / `BackwardPrimalFromLegacyBwdDiffFunc`
  / `BackwardRematFromLegacyBwdDiffFunc` /
  `BackwardPropagateFromLegacyBwdDiffFunc` — synthesized
  legacy-bridge extraction opcodes.
- **`FunctionCopy`**, **`SynthesizedForwardDerivativeWitnessTable`**,
  **`SynthesizedBackwardDerivativeWitnessTable`**,
  **`MakeIDifferentiableWitness`**,
  **`SynthesizedBackwardDerivativeWitnessTableFromLegacyBwdDiffFunc`**
  — derivative-witness synthesis, internal.
- **`LoadReverseGradient`**, **`ReverseGradientDiffPairRef`**,
  **`PrimalParamRef`**, **`DiffParamRef`** — autodiff temporaries
  that do not survive past the splitting/back-prop pass.
- **`DiffTypeInfo`** — synthesized type-info container.
- **`checkpointObj`**, **`loopExitValue`**, **`ReportCheckpointStore`**
  — checkpointing markers inserted by the reverse-mode pass; no
  surface form at LOWER-TO-IR.

If you would write a test for any of the above, stop — that's an
internal probe, not a documented surface mapping.

## Required structure

1. `README.md` with the structure named in `_common.md`. Use
   `## Untested claims` as the home for the
   unobservable-via-slangc items listed above.
2. 10 to 18 `.slang` test files. Aim for one observable opcode (or
   one observable surface-to-opcode lowering) per test. Group
   tightly-related observations into one file when the same
   surface construct gives the cleanest single-file observation.

## Doc sources

Primary (every `doc_ref` resolves into this):

- `docs/llm-generated/ir-reference/differentiation.md`

Secondary (allowed citations; use sparingly and only when the
primary doc explicitly hands off):

- `docs/llm-generated/ir-reference/types.md`
- `docs/llm-generated/cross-cutting/ir-instructions.md`
- `docs/llm-generated/pipeline/04-ast-to-ir.md`
- `docs/llm-generated/pipeline/05-ir-passes.md`

If you would cite anything else, stop and record a doc-gap finding
in `README.md`.

## Test directives

The standard form used here is:

```
//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute
```

Per the universal `_common.md` rule: combine `-dump-ir` with
**`-target <text-target>`** AND **`-o /dev/null`** so the IR dump
goes to stdout uncontaminated by target text. Use
`pipeline_stage=lower` in `//META` — these are LOWER-TO-IR
observations.

Anchor patterns at user-named symbols (`func %main`, `func %f`,
`let %fwd_diff`, `let %bwd_diff`) to cut through the large IR-dump
preamble. Use the `CHECK:` pattern prefix.

Outputs that escape DCE need to write to an
`RWStructuredBuffer<T>` — purely-internal computations are removed
before the dump.

Use `uniform` globals as the primal and differential operands of
`MakeDiffPair` so constant folding does not collapse them.

Do not use any GPU-only directive.

## Quality checklist (in addition to `_common.md`'s)

- [ ] Every test's `doc_ref` resolves to an anchor in
      `ir-reference/differentiation.md` (or one of the listed
      secondary docs).
- [ ] Every test uses `-target spirv-asm -dump-ir -o /dev/null
      -entry main -stage compute` per CLAUDE.md.
- [ ] Outputs escape DCE: write to an `RWStructuredBuffer<T>` so
      the differentiation observation stays linked.
- [ ] Operands of `MakeDiffPair`/`detach` are non-constant
      (`uniform` globals) so constant folding does not collapse
      the operation.
- [ ] CHECK patterns anchored at a user-named symbol — the IR-dump
      preamble must not match.
- [ ] No test depends on a GPU.
- [ ] No test asserts a C++ wrapper-struct identity
      (`IRMakeDifferentialPair`, `IRForwardDifferentiate`, etc.) —
      the dump shows the camel-case opcode spelling
      (`MakeDiffPair`, `ForwardDifferentiate`, `GetPrimal`,
      `GetDifferential`).
- [ ] README.md `## Doc gaps observed` is honest — explicitly
      record that `__bwd_diff` lowers to `LegacyBackwardDifferentiate`
      (not `BackwardDifferentiate`) at LOWER-TO-IR.

## Lessons captured (apply to this bundle as well)

These bite hard in differentiation-opcode observation tests:

- `-dump-ir` requires `-target <X>` and `-o /dev/null`.
- The IR dump prefixes user IR with a very large autodiff
  preamble (every `IDifferentiable` / `IForwardDifferentiable` /
  `IBackwardDifferentiable` interface, every key, the per-type
  builtin witnesses). Anchor patterns at user-named symbols
  (`func %main`, `func %f`, `let %fwd_diff`, `let %bwd_diff`)
  to skip the preamble.
- `__fwd_diff(f)` lowers to `ForwardDifferentiate(%f)` — a
  single-operand opcode whose result type is the differentiated
  function type `Func(DiffPair(T, %w), DiffPair(T, %w))` for a
  one-arg `float -> float`.
- `__bwd_diff(f)` lowers to **`LegacyBackwardDifferentiate(%apply_bwd,
  %remat, %ctx_t)`** at LOWER-TO-IR, *not* to `BackwardDifferentiate`.
  The unzip pass converts the legacy form to the modern triple
  later in the pipeline. Anchor your reverse-mode test at the
  `LegacyBackwardDifferentiate` spelling and record the
  discrepancy as a doc gap.
- `detach(x)` inside a differentiable function lowers to
  `let %N : T = detachDerivative(%x)`. The opcode spelling is
  lowercase `d` (`detachDerivative`), not `DetachDerivative`.
- `no_diff T x` parameter surfaces as `param %x : Attributed(T,
  %no_diff)` on the entry block where `%no_diff` is the
  module-scope `let %k : Void = no_diff` value. Pin both the
  `Attributed(...)` wrapper and the `no_diff` opcode spelling.
- `DifferentialPair<T>(p, d)` constructor lowers to `MakeDiffPair(%p,
  %d)`. The `p`/`d` projections (`.p`, `.d`) lower to `GetPrimal`/
  `GetDifferential` respectively.
- The `DifferentialPair<T>` constructor at the call site of a
  `__fwd_diff(f)` invocation produces a `MakeDiffPair` value with
  the right witness — but only when the operand types match a
  documented `IDifferentiable` conformance (`float`, `vector<float,N>`).
  Use `float` for the cleanest pin.
- Mark every test function `[ForwardDifferentiable]` (for fwd
  observations) or `[BackwardDifferentiable]` (for bwd). Without
  the marker, the call to `__fwd_diff(f)` is a checker error
  and no IR is emitted.
- The doc lists `MakeDiffPair`'s AST origin as `(synthesized)`,
  but the opcode is naturally produced by the user-written
  `DifferentialPair<T>(...)` constructor at any call site of a
  forward-mode differentiated function. Treat this as an
  observable claim regardless of the AST-origin column, and
  record the doc-gap.
