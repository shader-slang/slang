---
generated: true
model: claude-opus-4.8
generated_at: 2026-06-12T10:15:50Z
source_commit: eb9403ef595a99c2ff6def1d538dbd7a792d9371
watched_paths_digest: d6bb63223a0f2c41cb6dae09f1e45a4ddd44111fc422e5baf80f5ec043e38517
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# AST-to-IR Lowering

This document covers the stage where a fully checked AST is translated
into the Slang intermediate representation. The intended reader is a
developer modifying lowering for a language feature, or adding an IR
opcode that the lowering step must produce.

## Inputs and outputs

- **Input**: a checked AST inside a `TranslationUnitRequest`, plus an
  `ASTBuilder` (see [03-semantic-check.md](03-semantic-check.md)).
- **Output**: a fresh `IRModule` containing IR definitions for every
  function, type, generic, and global variable defined in that
  translation unit. The lowering step does **not** include the IR for
  `import`ed modules; those are linked in by a later IR pass before
  code generation.

## Lowering driver

The entry point is in
[slang-lower-to-ir.h](../../../../source/slang/slang-lower-to-ir.h):

```cpp
RefPtr<IRModule> generateIRForTranslationUnit(
    ASTBuilder* astBuilder,
    TranslationUnitRequest* translationUnit);
```

There are two related entry points for specializations:

- `generateIRForSpecializedComponentType` produces a small IR module
  recording how a `SpecializedComponentType` binds specialization
  parameters to concrete arguments.
- `generateIRForTypeConformance` produces an IR module that exposes a
  user-supplied type conformance as a public symbol so that linking
  can keep the relevant witness table alive.

The implementation lives in
[slang-lower-to-ir.cpp](../../../../source/slang/slang-lower-to-ir.cpp).
The lowering visitor descends the AST top-down: it walks decls,
allocates corresponding IR instructions, recurses into bodies (whose
unparsed forms have by now been parsed and checked, see
[03-semantic-check.md](03-semantic-check.md)), and lowers
expressions and statements into SSA value instructions and basic
blocks.

## IRBuilder and instruction creation

`IRBuilder` (declared in
[slang-ir.h](../../../../source/slang/slang-ir.h)) is the canonical way
to create IR instructions:

- It owns the current insertion point inside a block / function /
  module.
- It hash-conses *hoistable* and *global* values (types, constants,
  certain pure operators) so that two structurally equal values share
  one `IRInst*`. The flag bits `kIROpFlag_Hoistable` and
  `kIROpFlag_Global` declared in
  [slang-ir.h](../../../../source/slang/slang-ir.h) tag opcodes that take
  part in this deduplication.
- It exposes typed convenience emitters (`emitVar`, `emitCall`,
  `emitAdd`, ...) plus a generic `createIntrinsicInst` for opcodes
  that do not have a dedicated emitter.

Hoistable / global value semantics are the topic of
[../../../design/ir.md](../../../design/ir.md); this document does not
duplicate the rules. The opcode catalogue itself is in
[../cross-cutting/ir-instructions.md](../cross-cutting/ir-instructions.md).

The build-time generated header `slang-ir-insts-enum.h` (under
`build/source/slang/fiddle/`, derived from
[slang-ir-insts.lua](../../../../source/slang/slang-ir-insts.lua)) is
included by [slang-ir.h](../../../../source/slang/slang-ir.h) and
provides the `IROp` enum used throughout lowering.

## Mapping AST constructs to IR

The lowering visitor maps each AST family to a small set of IR
constructs. This table is illustrative, not exhaustive — the code in
[slang-lower-to-ir.cpp](../../../../source/slang/slang-lower-to-ir.cpp)
is authoritative.

| AST | Resulting IR |
| --- | --- |
| `ModuleDecl` | An `IRModule` (top-level container) |
| `FuncDecl` | An `IRFunc` containing one or more `IRBlock`s |
| `VarDecl` (global) | An `IRGlobalVar` |
| `VarDecl` (local) | An `IRVar` allocated inside a block |
| `StructDecl` | An `IRStructType` with `IRStructField` children |
| `InterfaceDecl` | An `IRInterfaceType` whose requirement-key entries are `IRStructKey`s or hoistable `IRBuiltinRequirementKey`s (see [Generics and existentials](#generics-and-existentials)) |
| `GenericDecl` | An `IRGeneric` (a function-shaped instruction whose body computes type-level values) |
| `BlockStmt` | A sequence of basic blocks; locals turn into `IRVar` |
| `IfStmt`, `ForStmt`, `WhileStmt`, `SwitchStmt` | Structured branches whose join point is an explicit operand on the terminator (see [../../../design/ir.md](../../../design/ir.md) for the structured-CFG encoding) |
| `ReturnStmt` | An `IRReturn` terminator |
| `BinaryExpr`, arithmetic / comparison `Expr` | Pure value insts (`IRAdd`, `IRMul`, `IREq`, ...) |
| `InvokeExpr` (function call) | An `IRCall` |
| `MemberExpr` | A `IRFieldAddress` / `IRFieldExtract` (lvalue vs rvalue) |
| `LiteralExpr` | A constant inst (`IRIntLit`, `IRFloatLit`, ...) |
| `WitnessTable` (synthesized in checking) | An `IRWitnessTable` |

Phi-style joining is encoded as block parameters (`IRParam` at the
start of a block) rather than explicit `phi` instructions; branches
to a block carry the parameter values as arguments. The rationale is
explained in [../../../design/ir.md](../../../design/ir.md).

## Generics and existentials

Generics survive lowering as ordinary IR: an `IRGeneric` is a
function-shaped instruction whose body runs at IR-time to compute the
specialized inner instruction. Specialization itself is **not**
performed during lowering; it is handled by IR passes
(`slang-ir-specialize`, `slang-ir-bind-existentials`,
`slang-ir-defunctionalization`, and friends — see
[05-ir-passes.md](05-ir-passes.md)). This separation lets the lowered
IR remain target-agnostic and keeps the lowering step relatively
small.

Witness tables (computed by
[slang-check-conformance.cpp](../../../../source/slang/slang-check-conformance.cpp))
become `IRWitnessTable` insts whose entries map interface
requirements to the concrete implementations.

Each interface requirement is identified by a *requirement key*.
`getInterfaceRequirementKey` in
[slang-lower-to-ir.cpp](../../../../source/slang/slang-lower-to-ir.cpp)
returns an `IRInst*` (cached per requirement `Decl` in
`SharedIRGenContext::interfaceRequirementKeys`) of one of two shapes:

- For an ordinary requirement (most methods, associated types), a
  per-decl `IRStructKey` that is a distinct `global` symbol unified
  across modules by its `key_<mangled>` linkage name.
- For a *recognized built-in* requirement — one tagged with
  `BuiltinRequirementModifier` (e.g. `IDifferentiable.Differential`,
  `.dzero`, `.dadd`), or the conformance constraint of such a built-in
  associated type — a hoistable `IRBuiltinRequirementKey` whose identity
  is its `BuiltinRequirementKind` operand. Because it is hoistable, the
  same logical built-in requirement deduplicates to a single key inst
  across decls and across the precompiled-core-module boundary, so a
  witness lookup and the witness-table entry always agree. The key also
  carries an `IRBuiltinRequirementDecoration` so role-scanning consumers
  (autodiff) can find a requirement by role rather than by position in
  the requirement list. The witness-table entry's `lookupKey` operand
  is therefore typed `IRInst` (not `IRStructKey`).

`visitGenericTypeConstraintDecl` lowers a constraint that is a direct
member of an `InterfaceDecl` (an interface-level `__constraint`, see
[03-semantic-check.md](03-semantic-check.md)) as the requirement key for
that requirement, and a non-equality subtype constraint relocated to
interface level gets a `WitnessTableType` requirement value, matching
how the bound was lowered when nested inside the associated type.

## Diagnostics during lowering

Most diagnostic-worthy issues are caught in semantic checking, but a
handful of constructs become problems only when lowering tries to
encode them — typically because a feature is unsupported on a given
target or a synthesized witness cannot be produced. For example, when
the assignment-lowering switch in
[slang-lower-to-ir.cpp](../../../../source/slang/slang-lower-to-ir.cpp)
reaches an assignment whose left-hand side it cannot encode, it now
emits `Diagnostics::UnsupportedAssignmentTarget` (recovering the
nearest non-zero source location from the builder's source-loc info)
rather than aborting via `SLANG_UNIMPLEMENTED_X`. Lowering errors flow
through the same `DiagnosticSink` used by the rest of the front-end
(see [../cross-cutting/diagnostics.md](../cross-cutting/diagnostics.md)).

## Module-level outputs

Beyond the IR module itself, the lowering step records a few side
artefacts on the surrounding `Module` and component types:

- The list of entry-point IR functions (each lowered `FuncDecl` that
  was registered as an entry point).
- Type-conformance bookkeeping (used by
  `generateIRForTypeConformance`).
- Layout intent on global parameters — actual layout assignment is
  performed later by IR passes
  (`slang-ir-layout`, `slang-ir-collect-global-uniforms`, ...).

After lowering, the IR module is the input to the IR-pass pipeline
described in [05-ir-passes.md](05-ir-passes.md).

## Adjacent pipelines

Two adjacent pipelines run before and alongside the post-link
IR-pass pipeline:

- [04b-pre-link-passes.md](04b-pre-link-passes.md) — the
  per-translation-unit, target-agnostic mandatory pass sequence
  inside `generateIRForTranslationUnit`, executed before the IR
  module is cached on the `Module` and pulled into
  `linkAndOptimizeIR` by `linkIR`. This is the page to consult
  when asking "where do `lowerErrorHandling`, `synthesizeBitFieldAccessors`,
  or `performMandatoryEarlyInlining` run, and what gates them?".
- [04c-layout-ir.md](04c-layout-ir.md) — `TargetProgram::createIRModuleForLayout`
  builds a separate, per-target IR module whose only contents are
  `IRLayoutDecoration`s on stub globals and entry points. It is not
  the executable per-translation-unit module and does not run the
  mandatory passes above, but an existing layout module is considered
  by `linkIR` (`slang-ir-link.cpp` lines 2120-2127) so its
  layout-decorated global symbols participate in linking.
