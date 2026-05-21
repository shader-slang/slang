# Prompt: tests-agentic/ast-reference/modifiers/

See [`_common.md`](_common.md) for universal rules. Those rules apply
to this bundle and override nothing here unless explicitly noted.

## Target

Produce the test bundle at `tests-agentic/ast-reference/modifiers/`,
anchored to
[`docs/llm-generated/ast-reference/modifiers.md`](../../../docs/llm-generated/ast-reference/modifiers.md).

Audience: nightly CI. The bundle exercises the **concrete `Modifier`
and `Attribute` subclasses** in the Slang AST through their
**user-observable consequences** at parse / check / emit time. The
doc enumerates a very large family of modifier and attribute classes
(parameter-direction `in`/`out`/`inout`, storage `static`/`const`/
`groupshared`, visibility `public`/`private`/`internal`, interpolation
`nointerpolation`/`sample`, matrix layout `row_major`/`column_major`,
HLSL semantics `: SV_*` / `: register(...)`, attribute markers
`[unroll]` / `[shader(...)]` / `[numthreads(...)]`, mutability
`[mutating]`, autodiff `[Differentiable]` / `[ForwardDerivative(fn)]`,
inheritance control `[open]` / `[sealed]`, GLSL `layout(...)`, type
modifiers `unorm` / `snorm` / `no_diff`, ...). Each concrete kind has
a documented role; a test in this bundle picks one and writes the
smallest Slang program that exercises that role, and verifies the
documented behavior is observable.

This bundle aims for **15–25 tests**: a representative cross-section
of modifier/attribute families, not exhaustive enumeration. Skip
kinds whose only observable surface is internal AST shape (see
`## Out of scope` below).

## The translation rule: claims to observations

`modifiers.md` describes the **shape** of internal AST modifier
classes (key fields, parent class in C++, the parser entry point
that produces the class). Slang as a compiler does not expose its
AST. So a claim such as "`HLSLGroupSharedModifier` is the modifier
class behind the `groupshared` keyword" is testable only through
its observable consequence: a `groupshared` declaration parses, and
the emitted HLSL/GLSL/SPIR-V/Metal contains the corresponding
target-specific spelling (`groupshared` / `shared` / `Workgroup` /
`threadgroup`). This bundle therefore follows the rule:

- **Testable** ⇔ "if the documented role of this modifier/attribute
  were not implemented, the program-text behavior we wrote would
  change in a way `slangc` reports."
- **Not testable through slangc** ⇔ "the claim is about which C++
  class the parser allocated, the exact spelling of a key field,
  the parent class in the C++ hierarchy, or the FIDDLE tag."

Concretely, here is the split for this doc:

### Observable claims (write tests for these)

For each concrete modifier/attribute family the **role** column
(the prose "Summary" cell) is the testable claim. Examples:

- `InModifier`, `OutModifier`, `InOutModifier` — direction modifiers
  on parameters. `out` actually writes a value back; `inout` reads
  and writes; `in` is read-only.
- `ConstModifier` — `const` on a global / local makes it
  immutable; assigning to a `const` is diagnosed.
- `HLSLStaticModifier` — `static` at module scope produces a
  module-private variable; `static` inside a function produces a
  function-static variable observable across calls.
- `HLSLGroupSharedModifier` — `groupshared` is rejected outside a
  `[shader("compute")]` entry context with a diagnostic, OR emits
  as the appropriate target-specific shared spelling.
- `PublicModifier` / `PrivateModifier` / `InternalModifier` —
  visibility controls cross-module name lookup; a `private` member
  is not visible to extension code declared outside the type.
- `HLSLNoInterpolationModifier`, `HLSLSampleModifier` —
  interpolation qualifiers on varying inputs appear in emitted
  HLSL/GLSL.
- `HLSLRowMajorLayoutModifier` / `HLSLColumnMajorLayoutModifier` —
  `row_major` / `column_major` carry through to emitted backend
  text.
- `HLSLRegisterSemantic` (`: register(...)`) — register binding
  affects emitted HLSL (or vk::binding mapping for SPIRV).
- `EntryPointAttribute` (`[shader("stage")]`) — the attribute
  marks the function as an entry point of a particular pipeline
  stage; the emitted code carries the stage-appropriate signature.
- `NumThreadsAttribute` (`[numthreads(x,y,z)]`) — workgroup
  dimensions appear in the emitted HLSL/GLSL/SPIR-V/Metal.
- `UnrollAttribute` (`[unroll]`) — the loop attribute is accepted
  on a `for`/`while` and appears in emitted HLSL as `[unroll]`.
- `MutatingAttribute` (`[mutating]`) — a `[mutating]` method can
  assign through `this`; a non-mutating method cannot.
- `DifferentiableAttribute` (`[Differentiable]`) — function is
  marked differentiable; can be passed to `fwd_diff` / `bwd_diff`.
- `NoDiffModifier` (`no_diff`) — type modifier suppresses
  differentiation through a parameter.
- `OpenAttribute` / `SealedAttribute` (`[open]` / `[sealed]`) —
  inheritance control on an interface or type.
- `RequireCapabilityAttribute` (`[require_capability(...)]`) —
  ties a decl to a capability set; calling from an incompatible
  context diagnoses.

### Negative claims (one diagnostic test each, where natural)

- `ConstModifier` — assignment to a `const` variable diagnoses.
- `OutModifier` — passing an `out` parameter without an l-value
  diagnoses; an unwritten `out` may be diagnosed too.
- `MutatingAttribute` — assignment through `this` in a
  non-`[mutating]` method diagnoses.
- `PrivateModifier` — accessing a `private` member from outside
  diagnoses.
- `InOutModifier` — passing a `const` or rvalue to an `inout`
  parameter diagnoses.

### Not testable through slangc (do NOT write tests for these)

The doc carries many claims about the **internal AST shape** of
these modifiers — which C++ class is the parent, which fields a
class stores, whether `MemoryQualifierSetModifier` aggregates a
bitmask of flags, whether `UncheckedAttribute` is the
parser-time representation. These are unobservable through
`slangc` text I/O. Record them under `## Out of scope` in
`README.md`. Examples:

- That `InOutModifier` derives from `OutModifier` in C++ (only
  the `inout` write-back behavior is observable).
- That `Attribute` derives from `AttributeBase` which derives from
  `Modifier` (only the attribute's effect is observable).
- That `UncheckedAttribute` is the parser-time shape before the
  checker resolves it (only the resolved behavior is observable).
- That `HLSLLayoutSemantic` is the abstract base of `register` and
  `packoffset` (only the binding behavior is observable).
- That `MemoryQualifierSetModifier` aggregates a bitmask of
  GLSL memory qualifiers (only the resulting GLSL emit is
  observable, and SPIR-V emit through GLSL is GPU-bound).
- That `SynthesizedModifier`, `IgnoreForLookupModifier`,
  `VarReassignedModifier`, `ExistentialOpenedOnVarModifier` are
  internal checker-only flags (no user spelling).
- That `IntrinsicOpModifier`, `TargetIntrinsicModifier`,
  `BuiltinTypeModifier`, `MagicTypeModifier`, `BuiltinModifier`
  are core-module-only and not user-spellable.
- The Mermaid "Family hierarchy" graph as a graph — abstract
  intermediates (`VisibilityModifier`, `InterpolationModeModifier`,
  `MatrixLayoutModifier`, `HLSLSemantic`, `RayPayloadAccessSemantic`,
  `TypeModifier`, `WrappingTypeModifier`,
  `ResourceElementFormatModifier`, `AttributeBase`,
  `InheritanceControlAttribute`) carry no user spelling.

If you find yourself thinking "this would verify that the AST node
allocated is class X" or "this would assert that field F holds
value V", stop — that is a source-targeting probe in disguise.
Re-frame as "the documented role of this modifier/attribute".

## Avoid duplication with sibling bundles

- `syntax-reference/keywords-and-builtins/` — already covers
  **keyword spelling and recognition** (`in`, `out`, `inout`,
  `const`, `static`, `groupshared`, `public`, `private`, ...).
  This bundle's tests must go further than "the keyword parses":
  exercise the **modifier's documented role** (`const` rejects
  assignment, `out` writes back, `[mutating]` enables `this`
  assignment).
- `pipeline/02-parse-ast/` — already covers **parse-stage
  modifier attachment** (modifier attaches to a decl, attribute
  parses as `[name(args)]`). This bundle's tests observe
  behaviors that show up **after** parsing — type checking,
  emit text, diagnostics.
- `ast-reference/declarations/` — covers declarations that
  *carry* modifiers (e.g. `VarDecl` with `const`); the
  declarations bundle picks the decl angle, this bundle picks
  the modifier angle.
- `ast-reference/base/` — covers `Modifier` as an abstract base
  (modifier list on `ModifiableSyntaxNode`, source loc on a
  modifier). This bundle picks the concrete subclass roles.

If a claim is best tested in a sibling bundle, do not duplicate
it here.

## Allowed secondary doc citations

- `docs/llm-generated/ast-reference/base.md`
- `docs/llm-generated/ast-reference/declarations.md`
- `docs/llm-generated/syntax-reference/grammar.md`
- `docs/llm-generated/syntax-reference/keywords-and-builtins.md`

If you would cite anything else, stop and record a doc-gap finding
in `README.md`.

## Source files you may consult for _verification only_

- `source/slang/slang-ast-modifier.h`
- `source/slang/slang-parser.cpp`

You may look at these to verify that a claim in the doc is
realizable (e.g. that `groupshared` is spelled `groupshared`,
that `[numthreads(x,y,z)]` exists). You may **not** mine them for
behavioral claims that the doc does not make.

## Required structure

1. `README.md` with the structure named in `_common.md`. List
   internal-AST-shape claims under `## Out of scope`.
2. 15 to 25 `.slang` test files (size cap 60). The doc is large
   but most claims collapse into a few observable categories
   (direction, storage, visibility, interpolation, layout,
   semantic, attribute-marker, mutability, differentiability).

## Test directives

Modifier/attribute claims split into two observation styles:

- **Diagnostic** — for "the misuse is rejected" claims
  (`const` assignment, `private` access from outside,
  non-`[mutating]` assignment through `this`). Use
  `//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):` and
  copy the exact wording from the runner's "Suggested annotations"
  output.
- **Functional / emit-text** — for "the modifier/attribute
  shows up in emitted code" claims (`[numthreads]`,
  `[unroll]`, `row_major`, `nointerpolation`, `: register(...)`,
  `[shader("compute")]`). Use
  `//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage <S>`
  (or `-target glsl` / `-target spirv-asm`) — pick the target
  where the documented spelling lives.
- **Interpret** — for direction / mutability / storage
  semantics (`out` writes back, `[mutating]` allows `this`
  assignment, `static` survives across calls). Use
  `//TEST:INTERPRET(filecheck=CHECK):` with a `printf` that
  observes the resulting value.

Per `_common.md`'s multi-backend rule, when the modifier's emit
spelling is target-specific, prefer ONE target where the role is
clearly observable rather than enumerating all backends. The bundle
is about modifier behavior, not target-emit text divergence (that is
covered by `pipeline/06-emit/` and target-specific bundles).

## Cast and observation reminders (carried from `_common.md`)

- Use **constructor-style casts** (`int('A')`), not C-style
  (`(int)'A'`). The C-style form returns 0 in `slangi`.
- `slangi` `printf` does not support `%s`.
- `static const int x = N;` at file scope is the cleanest pattern
  for asserting a compile-time-known result.
- The runner's "Suggested annotations" is the source of truth for
  diagnostic caret positions.
- Caret `^` placement in `//CHECK:` requires column `>= 10`. For
  earlier columns, use the `/*CHECK: ... */` block-comment form.
- DCE strips locally-unused code before emit. To observe a value
  in emitted text, the value must escape — write to a buffer
  parameter or return it from an entry point.
- `[[...]]` is a FileCheck regex-variable reference. Metal's
  `[[kernel]]`, HLSL's `[[vk::binding(...)]]` cannot be checked
  with literal `[[...]]`. Use bare substrings.
- `slangi` cannot host `cbuffer` or non-const `static` module
  globals. For those, use `-target hlsl` / `-target glsl`.

## Quality checklist (in addition to `_common.md`'s)

- [ ] Every test's `doc_ref` resolves to an anchor in
      `ast-reference/modifiers.md` (or one of the listed secondary
      docs).
- [ ] The bundle exercises a representative slice of major
      modifier/attribute families: parameter direction, storage,
      visibility, interpolation, matrix layout, HLSL semantic,
      stage-entry attribute, loop-hint attribute, mutability
      attribute, differentiability attribute, inheritance-control
      attribute, type modifier.
- [ ] At least 3 negative / diagnostic tests covering modifier
      misuse (`const` assignment, `private` access, non-mutating
      `this` assignment).
- [ ] No test asserts the C++ class identity of an AST node, the
      name of a private field, or the parent class in the C++
      hierarchy.
- [ ] No test depends on a GPU. `INTERPRET`, text-emit
      `SIMPLE(filecheck=...)`, and `DIAGNOSTIC_TEST` directives
      carry the whole bundle.
- [ ] No test was written by inspecting an uncovered source line.
- [ ] `README.md` `## Doc gaps observed` is honest.
- [ ] Internal-AST-shape claims (parent class in C++, private field
      names, FIDDLE tag, synthesized-only classes) are recorded
      under `## Out of scope`, not as tests.
