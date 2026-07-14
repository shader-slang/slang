---
generated: true
model: claude-opus-4.8
generated_at: 2026-06-29T15:20:45Z
source_commit: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
watched_paths_digest: 8a4f880d8b981d67ad88abf7aa75a535d3a572cbe530098477dde31879574746
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
- It hash-conses *hoistable* values (types, constants, certain pure
  operators) so that two structurally equal values share one `IRInst*`.
  The flag bit `kIROpFlag_Hoistable` declared in
  [slang-ir.h](../../../../source/slang/slang-ir.h) tags opcodes that are
  deduplicated this way. The separate `kIROpFlag_Global` flag marks
  opcodes that are always hoisted to module scope but are **never**
  deduplicated.
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
| `BuiltinOperatorExpr` (checker fast-path arithmetic / comparison / bitwise / unary) | A single pure value inst (`IRAdd`, `IRMul`, `IREq`, `IRBitAnd`, `IRNeg`, ...) emitted directly by `lowerBuiltinOperatorExpr` |
| `InvokeExpr` (general operator / function call) | An `IRCall` (after callable resolution) |
| `MemberExpr` | A `IRFieldAddress` / `IRFieldExtract` (lvalue vs rvalue) |
| `LiteralExpr` | A constant inst (`IRIntLit`, `IRFloatLit`, ...) |
| `WitnessTable` (synthesized in checking) | An `IRWitnessTable` |

Phi-style joining is encoded as block parameters (`IRParam` at the
start of a block) rather than explicit `phi` instructions; branches
to a block carry the parameter values as arguments. The rationale is
explained in [../../../design/ir.md](../../../design/ir.md).

### Builtin operators bypass call lowering

Most built-in arithmetic does not lower as a call. During checking, a
recognized scalar/vector/matrix operator on builtin numeric or `bool`
operands is rewritten to a `BuiltinOperatorExpr` carrying a resolved
`BuiltinOperationKind` (see `convertToBuiltinArithmeticOp` and the
class comment in
[slang-check-expr.cpp](../../../../source/slang/slang-check-expr.cpp) /
[slang-ast-expr.h](../../../../source/slang/slang-ast-expr.h)).
`lowerBuiltinOperatorExpr` switches on that enum and emits the matching
pure IR op directly via `emitIntrinsicInst`, skipping callable
resolution entirely. The element type only matters for `%`, which picks
`IRFRem` for floating-point operands and `IRIRem` otherwise. Only the
fast-path operators reach this visitor: `?:`, `&&`, and `||`
(short-circuiting / ternary) are still lowered through their dedicated
control-flow paths, so those `BuiltinOperationKind` values are an
`SLANG_UNEXPECTED` here rather than handled.

Compile-time integer expressions take a parallel path on the `Val`
side. `visitBuiltinOperationIntVal` in
[slang-lower-to-ir.cpp](../../../../source/slang/slang-lower-to-ir.cpp)
lowers a `BuiltinOperationIntVal` (the checked, folded form of a
constant operator expression) to the hoistable `constexpr*` opcode keyed
on its `BuiltinOperationKind` (`emitConstexprAdd`, `emitConstexprDiv`,
`emitConstexprSelect`, ...). Keying on the enum replaced an older path
that matched the operator's source name string; the `constexpr*` ops are
hoistable so equal compile-time expressions deduplicate to one inst.

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

### Variadic pack-count witnesses

A `countof(Pack) == Count` constraint on a variadic generic is recorded
during checking as a `GenericVariadicPackCountConstraintDecl` whose
satisfaction is a *proof-only* witness — the front end has already
verified the relationship, and the witness carries no runtime data.
Lowering models this with the same hidden-parameter / proof-only
witness-table representation as other data-free generic witnesses:

- `emitGenericConstraintDecl` for a
  `GenericVariadicPackCountConstraintDecl` emits a hidden `IRParam` of
  `WitnessTableType(void)` on the enclosing `IRGeneric`, registered as
  the lowered value of the constraint.
- `visitDeclaredVariadicPackCountWitness` lowers the *use* of that
  constraint as an `emitDeclRef` to the same `void` witness-table type.
- `visitConcreteVariadicPackCountWitness` lowers an already-satisfied
  (concrete) instance to one module-level proof-only `IRWitnessTable`,
  cached on `SharedIRGenContext::concreteVariadicPackCountWitnessTable`
  so every specialized call site reuses a single table rather than
  emitting a fresh one. A global-generic-param form is handled by
  `visitGenericVariadicPackCountConstraintDecl`, which emits an
  `IRGlobalGenericParam` of the same witness type.

The point of using a witness-table-shaped value (instead of a runtime
`countof`) is that the count is a compile-time fact: the witness only
needs a concrete `IRInst` for the generic param / call argument slot.

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

### Debug-info gating

Lowering decides per construct whether to attach source-level debug
information. Notably, `isSynthesizedConstructorDecl` in
[slang-lower-to-ir.cpp](../../../../source/slang/slang-lower-to-ir.cpp)
makes both `maybeEmitDebugLine` and `maybeAddDebugLocationDecoration`
skip a Slang-synthesized initializer (a `ConstructorDecl` with the
`SynthesizedDefault` or `SynthesizedMemberInit` flavor). Such a function
has no user-authored body, so emitting a `DebugLine` / `IRDebugLocationDecoration`
for it would let a debugger step into compiler-generated code and walk
the struct/member declaration lines. The discrimination is by
constructor *flavor*, not by the mangled `$init` name, because a
user-written `__init` mangles the same way but must keep its debug
info.

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
