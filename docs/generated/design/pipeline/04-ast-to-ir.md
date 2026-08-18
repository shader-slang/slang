---
generated: true
model: claude-opus-5
generated_at: 2026-08-03T13:49:43Z
source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
watched_paths_digest: bfaba4260e5950b0732424070a24791f1265e6debdda1d5c6493fbe0abe1e140
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
  translation unit. The lowering step does **not** link in `import`ed
  modules wholesale; those are linked in by a later IR pass before code
  generation. The one exception is targeted prelinking: lowering records
  imported `[unsafeForceInlineEarly]` functions in
  `externalSymbolsToPrelink` (line 13811) and `prelinkIR` (line 15544)
  clones their bodies into this module before it is returned, so the
  mandatory optimization passes can see them (see
  [04b-pre-link-passes.md](04b-pre-link-passes.md)). That clone and the
  whole mandatory pass block run _before_ the `LOWER-TO-IR` snapshot
  `-dump-ir` prints (line 15797), so the first dump is not the raw
  output of the lowering walk: a prelinked body is already there,
  normally inlined into its caller.

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
blocks. There is one visitor family per AST family —
`DeclLoweringVisitor`, `StmtLoweringVisitor`,
`ExprLoweringVisitorBase`, and `ValLoweringVisitor` — all sharing
state through `IRGenContext` / `SharedIRGenContext`.

### `LoweredValInfo` and the lowering environment

A lowered expression is not always a plain `IRInst*`, so lowering
returns a `LoweredValInfo` (line 120). Its `Flavor` enum distinguishes
`None`, a `Simple` r-value, a `Ptr` l-value, and the compound forms
lowering must keep symbolic until a use site decides how to read or
write them:

| Compound flavor        | Slang surface that produces it                                                                                       | Use site emits                                                                                                                                               |
| ---------------------- | -------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| `BoundMember`          | `obj.method` as a value before the call (line 6394), or a field whose base is itself deferred                        | `get_field` / `get_field_addr` for a field (line 1265); the resolved function value for a method                                                             |
| `BoundStorage`         | a `property` / `__subscript` access whose accessor set is more than a lone `get` (line 1124) — `c.doubled`, `buf[i]` | a `call` to the `get` accessor on a read and to `set` on a write; a `ref` accessor instead collapses the value to a `Ptr`                                    |
| `SwizzledLValue`       | a vector swizzle in l-value position — `v.xy = ...` (line 7880)                                                      | `swizzle` on a read (line 1288); on a write, `swizzledStore` when the base is a `Ptr` (line 10370) and otherwise `swizzleSet` plus a store back (line 10327) |
| `SwizzledMatrixLValue` | a matrix swizzle in l-value position — `m._m00_m11 = ...` (line 7795)                                                | on a read, two nested `getElement`s per component plus a `makeVector` when more than one is selected; `matrixSwizzleStore` on a write (line 10404)           |
| `ExtractedExistential` | an interface-typed value opened in l-value position (line 7739)                                                      | the extracted value as it stands; a write re-wraps the source with `makeExistential` first (line 10583)                                                      |
| `ImplicitCastedLValue` | an implicit conversion on an `out` / `inout` argument (line 7768)                                                    | whichever conversion inst `emitCast` picks, on a read; on a write, that cast applied to the source followed by a store into the base (line 10597)            |

`Subscript` is declared but has no construction site. The right-hand
column is what `materialize` (line 1184) and `getSimpleVal` (line 1342)
produce when a flavor is forced down to a single `IRInst*`, and what
the `assign` switch (line 10249) produces on the write side.

Per-environment caches sit on `IRGenEnv`: `mapDeclToValue` for decls
and `mapValToValue` (line 472) for `Val`s. `lowerVal` and `lowerType`
both route through the `lowerValWithCache` helper (line 3053), which
resolves the `Val`, consults `mapValToValue`, and stores only completed
results so recursive `Val` graphs are not disturbed by an in-progress
entry. The cache is per environment rather than global because a nested
generic environment can bind the same `Val` (for example a type
parameter `T` reachable both as a generic argument and through its
conformance witness) to a different IR parameter. `lowerType` (line 3081) still runs `lowerAssociatedVals` and `lowerRelatedTypes` after a
cache hit, because those are contextual side effects on the current
module rather than part of the `Val`-to-IR mapping.

## IRBuilder and instruction creation

`IRBuilder` (declared in
[slang-ir-insts.h](../../../../source/slang/slang-ir-insts.h) line 3158
and forward-declared in
[slang-ir.h](../../../../source/slang/slang-ir.h)) is the canonical way
to create IR instructions:

- It owns the current insertion point inside a block / function /
  module.
- It hash-conses _hoistable_ values (types, constants, certain pure
  operators) so that two structurally equal values share one `IRInst*`.
  The flag bit `kIROpFlag_Hoistable` declared in
  [slang-ir.h](../../../../source/slang/slang-ir.h) tags opcodes that are
  deduplicated this way. The separate `kIROpFlag_Global` flag marks
  opcodes that are always hoisted to module scope but are **never**
  deduplicated.
- It exposes typed convenience emitters (`emitVar`, `emitCallInst`,
  `emitAdd`, ...) plus the generic `emitIntrinsicInst` /
  `createIntrinsicInst` pair for opcodes that do not have a dedicated
  emitter.

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

| AST                                                                                 | Resulting IR                                                                                                                                                                                                    |
| ----------------------------------------------------------------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `ModuleDecl`                                                                        | An `IRModule` (top-level container)                                                                                                                                                                             |
| `FuncDecl`                                                                          | An `IRFunc` containing one or more `IRBlock`s                                                                                                                                                                   |
| `VarDecl` (global)                                                                  | An `IRGlobalVar`                                                                                                                                                                                                |
| `VarDecl` (local)                                                                   | An `IRVar` allocated inside a block                                                                                                                                                                             |
| `StructDecl`                                                                        | An `IRStructType` with `IRStructField` children                                                                                                                                                                 |
| `InterfaceDecl`                                                                     | An `IRInterfaceType` whose requirement-key entries are `IRStructKey`s or hoistable `IRBuiltinRequirementKey`s (see [Generics and existentials](#generics-and-existentials))                                     |
| `GenericDecl`                                                                       | An `IRGeneric` (a function-shaped instruction whose body computes type-level values)                                                                                                                            |
| `BlockStmt`                                                                         | A sequence of basic blocks; locals turn into `IRVar`                                                                                                                                                            |
| `IfStmt`, `ForStmt`, `WhileStmt`, `SwitchStmt`                                      | Structured branches whose join point is an explicit operand on the terminator (see [../../../design/ir.md](../../../design/ir.md) for the structured-CFG encoding)                                              |
| `ReturnStmt`                                                                        | An `IRReturn` terminator                                                                                                                                                                                        |
| `BuiltinOperatorExpr` (checker fast-path arithmetic / comparison / bitwise / unary) | A single pure value inst (`kIROp_Add`, `kIROp_Mul`, `kIROp_Eql`, `kIROp_BitAnd`, `kIROp_Neg`, ...) emitted directly by `lowerBuiltinOperatorExpr`                                                               |
| `InvokeExpr` (general operator / function call)                                     | An `IRCall` (after callable resolution)                                                                                                                                                                         |
| `MemberExpr`                                                                        | A `IRFieldAddress` / `IRFieldExtract` (lvalue vs rvalue)                                                                                                                                                        |
| `IndexExpr`                                                                         | `subscriptValue` (line 7481) emits `getElement` for a value base, `getElementPtr` for a pointer base; a `__subscript` arrives as an `InvokeExpr`, routed through `lowerStorageReference`                        |
| `AssignExpr`                                                                        | `assignExpr` writes through the left side's `LoweredValInfo` — `store` for a `Ptr`, `swizzledStore` for a swizzle, a setter `call` for `BoundStorage`                                                           |
| `BuiltinCastExpr` (checked form of `T(x)` and of implicit numeric conversion)       | The one conversion inst picked by `IRBuilder::emitCast`'s style table: `intCast`, `floatCast`, `castIntToFloat`, `castFloatToInt`, ...                                                                          |
| `BreakStmt`, `ContinueStmt`                                                         | An `unconditionalBranch` to the enclosing statement's break / continue label block                                                                                                                              |
| `Optional<T>`, `Tuple<...>`, `ParameterBlock<T>`                                    | The matching hoistable type inst — `Optional(...)`, `tuple_type(...)`, `ParameterBlock(...)`; scalar / vector / matrix / array spellings are catalogued in [../ir-reference/types.md](../ir-reference/types.md) |
| `LiteralExpr`                                                                       | A constant inst (`IRIntLit`, `IRFloatLit`, ...)                                                                                                                                                                 |
| `CastOptionalExpr`                                                                  | An `if`/`else` diamond around a temporary: `visitCastOptionalExpr` tests `emitOptionalHasValue`, coerces the unwrapped value on the true side, and propagates `emitMakeOptionalNone` on the false side          |
| `WitnessTable` (synthesized in checking)                                            | An `IRWitnessTable`, or — for a `SynthesizedModifier`-tagged conformance — a single intrinsic inst (see [Generics and existentials](#generics-and-existentials))                                                |

A few intrinsic-op call sites are special-cased in
`emitCallToDeclRef` (line 949) rather than emitted verbatim. Notably a `(void)expr`
cast reaches lowering as the builtin `__init(T)` on `void` with opcode
`kIROp_CastToVoid`; lowering evaluates the operand for its side effects
and yields the canonical `IRVoidLit` from `getVoidValue()` instead of
creating a `CastToVoid` inst, so no backend has to know the opcode.

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
`lowerBuiltinOperatorExpr` (line 5402) switches on that enum and emits the
matching pure IR op directly via `emitIntrinsicInst`, skipping callable
resolution entirely. The element type only matters for `%`, which picks
`kIROp_FRem` for floating-point operands and `kIROp_IRem` otherwise
(the element type is unwrapped out of a vector or matrix operand first).
Only the
fast-path operators reach this visitor: `?:`, `&&`, and `||`
(short-circuiting / ternary) are still lowered through their dedicated
control-flow paths, so those `BuiltinOperationKind` values are an
`SLANG_UNEXPECTED` here rather than handled.

Compile-time integer expressions take a parallel path on the `Val`
side. `visitBuiltinOperationIntVal` (line 1901) in
[slang-lower-to-ir.cpp](../../../../source/slang/slang-lower-to-ir.cpp)
lowers a `BuiltinOperationIntVal` (the checked, folded form of a
constant operator expression) to the hoistable `constexpr*` opcode keyed
on its `BuiltinOperationKind` (`emitConstexprAdd`, `emitConstexprDiv`,
`emitConstexprSelect`, ...). Keying on the enum replaced an older path
that matched the operator's source name string; the `constexpr*` ops are
hoistable so equal compile-time expressions deduplicate to one inst.

The surface that reaches these opcodes is arithmetic on a _generic value
parameter_, whose result is needed as a type-level value. Inside
`int f<let N : int>()`, the two array extents

```slang
int a[N / 2];
int b[N + 1];
```

lower to `constexprDiv(%N, 2 : Int)` and `constexprAdd(1 : Int, %N)`.
Only the first is a `BuiltinOperationIntVal`: `+`, `-`, `*` and unary
`-` never form one at all. They arrive as a `PolynomialIntVal`, which
`visitPolynomialIntVal` (line 2009) lowers to the same opcode family,
emitting the constant term first — which is why the operand order of
`constexprAdd` above does not follow the source. A literal-only
expression instead folds to a `ConstantIntVal` during checking and
arrives as a plain `IRIntLit`.

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
`visitInheritanceDecl` (line 11199) creates the table and
`lowerWitnessTable` (line 11087) fills it in:

```cpp
void lowerWitnessTable(
    IRGenContext* subContext,
    WitnessTable* astWitnessTable,
    IRWitnessTable* irWitnessTable,
    DeclRefBase* witnessTableBaseDeclRef)
```

The last parameter is the decl-ref of the _base_ (interface) side of the
conformance, obtained from `getWitnessTableBaseDeclRef` (lines 10904 and
10915). When it is non-null, every requirement witness taken out of the
AST requirement dictionary is `specialize`d through it before being
lowered, so an entry copied from a generic interface carries the
conforming type's substitutions rather than the interface's own
parameters. It is null only when the conformance's base type is not a
`DeclRefType`, which an ordinary `struct S : IFoo<...>` never produces.

In a dump the substitution is an extra `specialize` layer on the entry's
value. Given

```slang
interface IFoo<T> { T zero(); T twice() { return zero(); } }
struct S : IFoo<int> { int zero() { return 0; } }
```

`S`'s table is `witness_table_t(specialize(%IFoo, Int))(%S)`, and its two
entries do not have the same shape:

```
witness_table_entry(%IFoo_zero,  %S_zero)
witness_table_entry(%IFoo_twice, specialize(specialize(%twiceImpl, Int), %S, %table))
```

`zero` is satisfied by a member of `S`, mentions no interface parameter,
and stays flat. `twice` is inherited from the interface's own default
implementation, and the inner `specialize(..., Int)` is the base
specialization: it binds the interface's `T` before the outer
`specialize` supplies the conforming type and its witness table.

Per-entry lowering is factored into `lowerWitnessEntryValue`
(line 10933), which switches on the `RequirementWitness::Flavor`: a
`declRef` witness lowers through `emitDeclRef`, a `val` witness through
`lowerSimpleVal`, and a `witnessTable` witness recursively materializes
a nested `IRWitnessTable` (with its own conformance mangled name and,
for an exported type, `HLSLExport` / `KeepAlive` decorations). The
already-materialized nested tables are memoized in
`IRGenContext::mapASTWitnessTableToIRWitnessTable` (line 676), a
_non-owning_ pointer: the dictionary is owned by the lowering scope, so
copied contexts can share the cache for the current insertion / generic
environment without a single global cache keyed only on the front-end
`WitnessTable*` (which would be too coarse, since an `IRWitnessTable` is
inserted into a specific IR scope and may refer to that scope's generic
parameters).

### Generic interface requirements

A requirement can itself be generic — most commonly a differentiability
constraint attached to a generic interface method, which the checker
records as a sibling `GenericDecl` whose `inner` is a
`GenericTypeConstraintDecl`. Such an entry is not a flat value: it must
supply the method-local generic parameters. `lowerWitnessTable`
recognizes that shape and routes it to
`lowerWitnessEntryValueInGenericWitnessTable` (line 11018), which

- opens a nested `IRGenEnv` and its own nested-witness-table cache,
- emits the requirement's own generic parameters with `emitGenericDecl`
  (line 12905 for the `DeclRef<GenericDecl>` form, line 13002 for the
  bare-`GenericDecl` convenience overload),
- lowers the satisfying witness inside that environment, and
- closes only the generics it opened, using the `stopBeforeGeneric`
  parameter added to `finishOuterGenerics` so the enclosing witness
  table's own generic is left intact.

The result is a requirement-local `IRGeneric` stored as the witness-table
entry value, so a use site becomes
`specialize(lookupWitness(table, key), methodArgs...)` rather than a flat
witness lookup that has dropped the method's generic arguments.
`canDeclLowerToAGeneric` (line 14860) has the matching rule on the decl
side: a `GenericTypeConstraintDecl` lowers to a generic exactly when it
is the `inner` of a `GenericDecl`.

Which constraint decls become _generic parameters_ at all is decided by
`isGenericConstraintParameterDecl` (declared in
[slang-ast-decl.h](../../../../source/slang/slang-ast-decl.h) line 1186);
`emitGenericDecl` consults it before emitting a
hidden parameter for each constraint member, and the substitution-argument
walk in `ExprLoweringContext` uses the same predicate so parameters and
arguments stay in agreement. Constraints that are instead interface
requirements are skipped there and lowered as requirement keys.

### Associated-type bounds

A bound on an associated type — `associatedtype A : IBar`,
`associatedtype A where A : IBar`, or `__constraint A : IBar` — is a
requirement of the _enclosing interface_, a sibling of `A`, not something
nested under it (see
[03-semantic-check.md](03-semantic-check.md)). Lowering follows that
representation:

- `visitAssocTypeDecl` lowers the associated type with an empty list of
  constraint interfaces, because the bounds are no longer its members.
- The `IRInterfaceType` requirement loop emits exactly one entry per
  direct interface member; it no longer walks an associated type's or a
  callable's nested `TypeConstraintDecl`s to synthesize extra entries.
  A relocated non-equality subtype constraint gets a
  `WitnessTableType(bound)` requirement value, wrapped in an `IRGeneric`
  when the constraint is the `inner` of a `GenericDecl` (so a `sup` type
  mentioning a cloned method parameter is lowered inside the matching IR
  generic environment).
- `visitGenericTypeConstraintDecl` (line 10784) therefore recognizes just
  two interface-requirement shapes — a constraint that is a direct member
  of an `InterfaceDecl`, and a constraint that is the `inner` of a
  `GenericDecl` directly under an `InterfaceDecl` — plus the
  global-generic-parameter case. The former per-`AssocTypeDecl` and
  per-`FuncDecl` parent cases are gone.

### Requirement keys

Each interface requirement is identified by a _requirement key_.
`getInterfaceRequirementKey` (line 1713) in
[slang-lower-to-ir.cpp](../../../../source/slang/slang-lower-to-ir.cpp)
returns an `IRInst*` (cached per requirement `Decl` in
`SharedIRGenContext::interfaceRequirementKeys`) of one of two shapes:

- For an ordinary requirement (most methods, associated types), a
  per-decl `IRStructKey` that is a distinct `global` symbol unified
  across modules by its `key_<mangled>` linkage name.
- For a _recognized built-in_ requirement — one tagged with
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

### Differentiability arrives as a conformance

Differentiability is not a modifier that lowering re-derives. The checker
represents it as an interface conformance of the _function viewed as a
type_: a `[Differentiable]` function `f` gets a synthesized
`extension __func_as_type(f) : IForwardDifferentiable<__func_as_type(f)>`
(and the backward analogue), where
`IForwardDifferentiable<FType>` / `IBackwardDifferentiable<FType>` are
declared in
[core.meta.slang](../../../../source/slang/core.meta.slang) at lines 720
and 739. See
[03-semantic-check.md](03-semantic-check.md#differentiability-as-interface-conformance)
for how that representation is built. Lowering consumes it in three
places:

- **The conformance itself.** `visitInheritanceDecl` treats an
  `InheritanceDecl` carrying a `SynthesizedModifier` specially: instead of
  a real `IRWitnessTable` it emits one intrinsic inst whose opcode is the
  modifier's `op` (for the forward case
  `kIROp_SynthesizedForwardDerivativeWitnessTable`),
  typed `WitnessTableType(IForwardDifferentiable<...>)`, with the
  conformance's `Val` operands lowered as the inst's operands. Back-end
  passes reconstruct a table from those operands if one is needed. Because
  the subtype here is a callable rather than an aggregate type, lowering
  also has to reorder its own recursion guard: the placeholder
  `LoweredValInfo` normally installed _before_ `lowerType(subType)` is,
  for a callable subtype, installed _after_ it (lines 11334-11350), so
  that lowering the callable decl-ref sees the real callable and can
  attach its autodiff-associated values instead of recording the
  placeholder as the callable's differentiability witness.
- **The interface members.** A `SynthesizedFuncDecl` (line 13847) — the
  `fwd_diff` member the interface requires — is lowered by creating an
  `IRFunc`, replacing it with `emitIntrinsicInst` of the decl's stored
  `irOp` (`kIROp_ForwardDifferentiate` here), and rewriting the
  decl→value mapping to that inst.
- **Associated values.** `lowerAssociatedVals` (line 4944) reads the
  `DifferentiableAttribute` of the function currently being lowered and
  attaches each associated value with `IRBuilder::addAnnotation` (an
  `Annotation` inst keyed by an `AnnotationKind`). It skips a decl for
  which `isInterfaceRequirement` holds (a requirement's annotations belong
  to the conforming implementation), and, when the key is a `DeclRefBase`,
  substitutes the stored associated value through that decl-ref first, so
  an interface default implementation lowered through a concrete
  conformance produces witness entries with the same substitutions as the
  callee they annotate.

A function-typed conformance also changes what `this` means for the
members of the synthesized extension: `_findReplacementThisParamType`
(line 3967) and `getThisParamTypeForCallable` (line 4033) redirect an
extension whose target type is a callable decl-ref to that callable's own
this-type.

### Variadic pack-count witnesses

A `countof(Pack) == Count` constraint on a variadic generic — spelled
`void f<let N : int, each T>(T x) where countof(T) == N` — is recorded
during checking as a `GenericVariadicPackCountConstraintDecl` whose
satisfaction is a _proof-only_ witness — the front end has already
verified the relationship, and the witness carries no runtime data.
Lowering models this with the same hidden-parameter / proof-only
witness-table representation as other data-free generic witnesses:

- `emitGenericConstraintDecl` for a
  `GenericVariadicPackCountConstraintDecl` emits a hidden `IRParam` of
  `WitnessTableType(void)` on the enclosing `IRGeneric`, registered as
  the lowered value of the constraint.
- `visitDeclaredVariadicPackCountWitness` lowers the _use_ of that
  constraint as an `emitDeclRef` to the same `void` witness-table type.
- `visitConcreteVariadicPackCountWitness` lowers an already-satisfied
  (concrete) instance through the `emitConcreteVariadicPackCountWitness`
  helper, which creates one module-level proof-only `IRWitnessTable` and
  caches it on
  `SharedIRGenContext::concreteVariadicPackCountWitnessTable` (line 528)
  so every specialized call site reuses a single table rather than
  emitting a fresh one. A global-generic-param form is handled by
  `visitGenericVariadicPackCountConstraintDecl` (line 10855), which emits
  an `IRGlobalGenericParam` of the same witness type; that visitor now
  asserts up front that a pack-count constraint is never an interface
  requirement.

All three pieces show up together in the `LOWER-TO-IR` dump of
`int sum<let N : int, each T>(T values) where countof(T) == N`, called as
`sum<2>(1, 2)`: the generic's parameter list ends with
`param %w : witness_table_t(Void)`, the module holds one
`witness_table %t : witness_table_t(Void)(Void);`, and the call site
reads `call specialize(%sum, 2 : Int, TypePack(Int, Int), %t)(...)`.

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
reaches an assignment whose left-hand side it cannot encode, it emits
`Diagnostics::UnsupportedAssignmentTarget` (line 10602), recovering the
nearest non-zero source location from the builder's source-loc info,
rather than aborting via `SLANG_UNIMPLEMENTED_X`. The left-hand sides
that get there are the ones the flavor table above has already collapsed
to an r-value. Indexing an array-valued `property` is the reachable
case, because `s.v` materializes to a `call` to the getter and `[0]`
then extracts an element of that temporary:

```slang
struct S
{
    float _v[2];
    property float v[2] { get { return _v; } set { _v = newValue; } }
}
// ... s.v[0] = 1.0;
// error[E40017]: assignment target is not supported
```

Lowering errors flow through the same `DiagnosticSink` used by the rest
of the front-end (see
[../cross-cutting/diagnostics.md](../cross-cutting/diagnostics.md)).

Two other lowering-time reports are worth knowing about because they
depend on facts only the lowering walk has:

- **Statements before the first `case` label.** When `StmtLoweringVisitor`
  reaches a statement inside a `switch` body while no case label is
  active, that statement is unreachable: control can only enter a switch
  body through the dispatch to a `case` / `default` label, and Slang has no
  `goto` into a body. Lowering warns once for the leading run with
  `Diagnostics::UnreachableCode`, tracked by the
  `warnedUnreachableBeforeFirstCase` flag on the switch-lowering info.
  Previously these statements were silently dropped.
- **Runaway constructor-call lowering.** `visitInvokeExprImpl` counts its
  own recursion depth in `IRGenContext::invokeLoweringRecursionDepth` and,
  past `kMaxIRInvokeLoweringRecursionDepth` (128, line 5513), diagnoses
  `Diagnostics::MaximumTypeNestingLevelExceeded` and yields
  `getPoison(type)` rather than overflowing the native stack on an
  infinitely nesting type that keeps synthesizing constructor calls.
  Nothing in the counter is specific to constructors, though: it advances
  once per nested `InvokeExpr`, because a call's arguments are lowered
  from inside `addDirectCallArgs` (line 5153). A chain of 128 ordinary
  calls — `f(f(...f(x)...))` — is already enough to reach the limit and
  report `fatal error[E39997]: maximum type nesting level exceeded`.

## Module-level outputs

The `IRModule` that `generateIRForTranslationUnit` returns is its only
separate output object; the one other lasting effect is a mutation of
the checked AST — when a registered entry point carries no explicit
`EntryPointAttribute`, lowering creates one (filling its capability set
from the entry-point profile) and calls `addModifier` on the function
decl at line 15223, so that ordinary function lowering recognizes it as
an entry point:

- The entry-point IR functions and their decorations are children of
  that module — the loop at line 15467 lowers each registered entry
  point into it.
- Layout intent on global parameters is _not_ materialized here: no
  `IRLayoutDecoration` is attached during translation-unit lowering.
  Layout assignment happens later, in IR passes (`slang-ir-layout`,
  `slang-ir-collect-global-uniforms`, ...) and in the separate module of
  [04c-layout-ir.md](04c-layout-ir.md).

The caller in
[slang-compile-request.cpp](../../../../source/slang/slang-compile-request.cpp)
installs the returned module on the AST-level `Module` with
`setIRModule` (line 570); that is the whole hand-off.

Two adjacent generation paths build their own modules and are _not_
outputs of translation-unit lowering: `generateIRForTypeConformance`
(line 15982) and `TargetProgram::createIRModuleForLayout` (line 16353).
The latter uses the type-layout lowering helper `_lowerTypeLayoutCommon`
(line 16023), which records the front-end's byte alignment as a
`TypeAlignment` attribute whenever the layout occupies the
`LayoutResourceKind::Uniform` unit at all; the `IRTypeLayout::Builder`
decides whether an attribute is actually emitted.

### Entry-point-scoped decorations

`lowerFrontEndEntryPointToIR` (line 15198) attaches the decorations that
are meaningful only on an entry point. Besides the name / module
decorations, it lifts the `spvShader64BitIndexingEXT` capability onto the
entry point as an `IRShader64BitIndexingDecoration`: it scans the
`inferredCapabilityRequirements` atom sets of the entry-point function
(iterating `getAtomSets()` rather than calling `implies()`, which is
AND-across-alternatives and therefore too strict as a presence test). The
requirement is lifted here rather than left on the attributed callee
because the corresponding SPIR-V execution mode is entry-point scoped.
Work-graph node attributes take the same route in the general function
path (line 14485), each source spelling lowering to its matching
decoration:

| Source attribute                 | Decoration            |
| -------------------------------- | --------------------- |
| `[NodeLaunch(mode)]`             | `nodeLaunch`          |
| `[NodeID(name, arrayIndex)]`     | `nodeID`              |
| `[NodeMaxDispatchGrid(x, y, z)]` | `nodeMaxDispatchGrid` |
| `[NodeDispatchGrid(x, y, z)]`    | `nodeDispatchGrid`    |
| `[MaxRecords(count)]`            | `maxRecords`          |
| `[NodeIsProgramEntry]`           | `nodeIsProgramEntry`  |

The launch mode is kept as an `IRStringLit` rather than an integer, so
the dump reads `[nodeLaunch("broadcasting")]` and HLSL emit can re-emit
the source name. Those attributes are not core-language syntax: their
`attribute_syntax` declarations live in the `experimental.workgraph`
standard module, so the shader must `import experimental.workgraph` and
the compile must pass `-experimental-feature` (alongside `-stage node`
and a `lib_6_8` profile). Without the import, `[NodeLaunch("...")]` is
only an unknown-attribute warning.

### Debug-info gating

Lowering decides per construct whether to attach source-level debug
information. Notably, `isSynthesizedConstructorDecl` (line 9898) in
[slang-lower-to-ir.cpp](../../../../source/slang/slang-lower-to-ir.cpp)
makes `maybeEmitDebugLine` and `maybeAddDebugLocationDecoration`
skip a Slang-synthesized initializer (a `ConstructorDecl` with the
`SynthesizedDefault` or `SynthesizedMemberInit` flavor). Such a function
has no user-authored body, so emitting a `DebugLine` / `IRDebugLocationDecoration`
for it would let a debugger step into compiler-generated code and walk
the struct/member declaration lines. The discrimination is by
constructor _flavor_, not by the mangled `$init` name, because a
user-written `__init` mangles the same way but must keep its debug
info.

The same predicate has a third caller. Constructor lowering (line 14271)
names the object under construction `this` — either the caller-provided
return-destination out-parameter or the by-value local the initializer
returns — so a debugger stopped inside `__init` can inspect the members
being initialized. That call site checks the predicate itself because
`addNameHint` has no internal synthesized-constructor gate, and it adds an
`IRDebugLocationDecoration` to the by-value local so
`slang-ir-insert-debug-value-store` surfaces it.

Debug _source_ records also need care, because they are hoistable and only
collapse across modules when their operands match byte-for-byte.
`getOrEmitDebugSource` (line 9658) therefore spells the emitted filename
with `PathInfo::getMostUniqueIdentity()` — the same spelling the
per-source-file loop in `generateIRForTranslationUnit` uses — while still
looking the `SourceFile` up by the `getName()`-based path. It embeds source
text when the debug level is `Standard` or above _or_ when
`shouldIncludeSourceInDebugInfo()` is set, falling back to reading the
found path off disk (decoded through `SourceFile::decodeContentBlob`, so
the embedded text is BOM-free and stays aligned with the line/column data)
for a `SourceFile` deserialized from a precompiled module, which carries
no content blob of its own.

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
  builds a separate, per-target IR module holding imported global and
  entry-point stubs, the `IRLayoutDecoration`s attached to those stubs
  and to the module root, the type- and variable-layout instructions
  those decorations reference, and (for SPIR-V and Metal)
  `IRRequireCapabilityAtomDecoration`s on the entry-point stubs. It is not
  the executable per-translation-unit module and does not run the
  mandatory passes above, but an existing layout module is considered
  by `linkIR` (which pulls it in via
  `TargetProgram::getExistingIRModuleForLayout`) so its
  layout-decorated global symbols participate in linking.
