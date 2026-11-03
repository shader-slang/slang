---
generated: true
model: claude-sonnet-5-5
generated_at: 2026-11-02T21:23:55Z
source_commit: 2518a942d59b1228fc8cc70a37739baab82b6002
watched_paths_digest: d36bd32950d8e216a645e2f63012cb6f30c73a414e519ae8b99f0ce0aa00a0c6
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
  generation. The one exception is targeted prelinking: lowering queues
  two kinds of imported symbol in `externalSymbolsToPrelink` (declared at
  line 540) and `prelinkIR` (called at line 15605) clones them into this
  module before it is returned (see
  [04b-pre-link-passes.md](04b-pre-link-passes.md)). The first kind is an
  imported `[unsafeForceInlineEarly]` function (queued at line 13889), so
  the mandatory optimization passes can see its body. The second kind is
  an interface owned by another module: `tryBorrowInterfaceFromOwningModule`
  (line 12030) lowers such an interface as a bare declaration plus a
  queued reference to the owning module's definition (queued at line 12127),
  instead of re-deriving every requirement from the AST; the clone replaces
  the declaration before any mandatory pass runs. It falls back to
  deriving the interface from the AST when the decl belongs to the module
  being lowered, its linkage name would be obfuscated, the owning module
  has no lowered IR yet, or no interface is registered under the mangled
  name. `reproduceInterfaceDecorationsReadBeforePrelink` gives the bare
  declaration the `IRComInterfaceDecoration` that `visitInheritanceDecl`
  reads before prelink runs. That clone and the
  whole mandatory pass block run *before* the `LOWER-TO-IR` snapshot
  `-dump-ir` prints (line 15853), so the first dump is not the raw
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

Two related entry points: `generateIRForSpecializedComponentType` records
how a `SpecializedComponentType` binds specialization parameters, and
`generateIRForTypeConformance` exposes a user-supplied type conformance
as a public symbol so linking keeps the witness table alive.

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

| Compound flavor | Slang surface that produces it | Use site emits |
| --- | --- | --- |
| `BoundMember` | `obj.method` as a value before the call (line 5845), or a field whose base is itself deferred | `get_field` / `get_field_addr` for a field (`extractField`, line 1157); the resolved function value for a method |
| `BoundStorage` | a `property` / `__subscript` access whose accessor set is more than a lone `get` (line 1146) — `c.doubled`, `buf[i]` | a `call` to the `get` accessor on a read and to `set` on a write; a `ref` accessor instead collapses the value to a `Ptr` |
| `SwizzledLValue` | a vector swizzle in l-value position — `v.xy = ...` (line 7456) | `swizzle` on a read (line 1306); on a write, `swizzledStore` when the base is a `Ptr` (line 10127) and otherwise `swizzleSet` plus a store back (line 10084) |
| `SwizzledMatrixLValue` | a matrix swizzle in l-value position — `m._m00_m11 = ...` (line 7366) | on a read, two nested `getElement`s per component plus a `makeVector` when more than one is selected; `matrixSwizzleStore` on a write (line 10161) |
| `ExtractedExistential` | an interface-typed value opened in l-value position (line 7310) | the extracted value as it stands; a write re-wraps the source with `makeExistential` first (line 10351) |
| `ImplicitCastedLValue` | an implicit conversion on an `out` / `inout` argument (line 7339) | In the argument position this flavor is written for, `tryGetAddress` takes the base's address and emits a single cast inst yielding the `Ptr(T)` the call consumes — `inOutImplicitCast` for `ParamPassingMode::BorrowInOut`, `outImplicitCast` otherwise (line 9938). The read-cast / write-cast-then-store form applies only where no address is available. Only round-trippable types qualify: an `int`/`float` cast here is rejected with `E30047` plus note `E30063`. |

`Subscript` is declared but has no construction site. The right-hand
column is what `materialize` (line 1206) and `getSimpleVal` (line 1364)
produce when a flavor is forced down to a single `IRInst*`, and what
the `assign` switch (line 10006) produces on the write side.

Per-environment caches sit on `IRGenEnv`: `mapDeclToValue` for decls
and `mapValToValue` (line 472) for `Val`s. `lowerVal` and `lowerType`
both route through `lowerValWithCache` (line 3099), which only looks up
and stores the key it is given, caching completed results so recursive
`Val` graphs are not disturbed by an in-progress entry. `lowerVal`
resolves its argument (`val->resolve()`) before the lookup, whereas
`lowerType` keys the cache with the type it was passed. The cache is per
environment because a nested generic environment can bind the same `Val`
to a different IR parameter. `lowerType` (line
3127) still runs `lowerAssociatedVals` and `lowerRelatedTypes` after a
cache hit, because those are contextual side effects on the current
module rather than part of the `Val`-to-IR mapping.

## IRBuilder and instruction creation

`IRBuilder` (declared in
[slang-ir-insts.h](../../../../source/slang/slang-ir-insts.h) line 3201
and forward-declared in
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
- It exposes typed convenience emitters (`emitVar`, `emitCallInst`,
  `emitAdd`, ...) plus the generic `emitIntrinsicInst` /
  `createIntrinsicInst` pair for opcodes that do not have a dedicated
  emitter.

Hoistable / global value semantics are the topic of
[../../../design/ir.md](../../../design/ir.md); the opcode catalogue is in
[../cross-cutting/ir-instructions.md](../cross-cutting/ir-instructions.md).

The build-time generated `slang-ir-insts-enum.h`, derived from
[slang-ir-insts.lua](../../../../source/slang/slang-ir-insts.lua) and
included by [slang-ir.h](../../../../source/slang/slang-ir.h), provides the
`IROp` enum used throughout lowering.

## Mapping AST constructs to IR

The lowering visitor maps each AST family to a small set of IR
constructs. This table is illustrative; the code in
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
| `BuiltinOperatorExpr` (checker fast-path arithmetic / comparison / bitwise / unary) | A single pure value inst (`kIROp_Add`, `kIROp_Mul`, `kIROp_Eql`, `kIROp_BitAnd`, `kIROp_Neg`, ...) emitted directly by `lowerBuiltinOperatorExpr` |
| `InvokeExpr` (general operator / function call) | An `IRCall` (after callable resolution) |
| `MemberExpr` | A `IRFieldAddress` / `IRFieldExtract` (lvalue vs rvalue) |
| `IndexExpr` | `subscriptValue` (line 7046) emits `getElement` for a value base, `getElementPtr` for a pointer base; a `__subscript` arrives as an `InvokeExpr`, routed through `lowerStorageReference` |
| `AssignExpr` | `assignExpr` writes through the left side's `LoweredValInfo` — `store` for a `Ptr`, `swizzledStore` for a swizzle, a setter `call` for `BoundStorage` |
| `BuiltinCastExpr` (checked form of `T(x)` and of implicit numeric conversion) | The one conversion inst picked by `IRBuilder::emitCast`'s style table: `intCast`, `floatCast`, `castIntToFloat`, `castFloatToInt`, ... |
| `BreakStmt`, `ContinueStmt` | An `unconditionalBranch` to the enclosing statement's break / continue label block |
| `Optional<T>`, `Tuple<...>`, `ParameterBlock<T>` | The matching hoistable type inst — `Optional(...)`, `tuple_type(...)`, `ParameterBlock(...)`; scalar / vector / matrix / array spellings are catalogued in [../ir-reference/types.md](../ir-reference/types.md) |
| `LiteralExpr` | A constant inst (`IRIntLit`, `IRFloatLit`, ...) |
| `CastOptionalExpr` | An `if`/`else` diamond around a temporary: `visitCastOptionalExpr` tests `emitOptionalHasValue`, coerces the unwrapped value on the true side, and propagates `emitMakeOptionalNone` on the false side |
| `WitnessTable` (synthesized in checking) | An `IRWitnessTable`, or — for a `SynthesizedModifier`-tagged conformance — a single intrinsic inst (see [Generics and existentials](#generics-and-existentials)) |

A few intrinsic-op call sites are special-cased in
`emitCallToDeclRef` (line 977) rather than emitted verbatim. Notably a `(void)expr`
cast reaches lowering as the builtin `__init(T)` on `void` with opcode
`kIROp_CastToVoid`; lowering evaluates the operand for its side effects
and yields the canonical `IRVoidLit` from `getVoidValue()` instead of
creating a `CastToVoid` inst, so no backend has to know the opcode.

Phi-style joining is encoded as block parameters (`IRParam` at the start
of a block) rather than `phi` instructions; see
[../../../design/ir.md](../../../design/ir.md).

### Builtin operators bypass call lowering

Most built-in arithmetic does not lower as a call. During checking, a
recognized scalar/vector/matrix operator on builtin numeric or `bool`
operands is rewritten to a `BuiltinOperatorExpr` carrying a resolved
`BuiltinOperationKind` (see `convertToBuiltinArithmeticOp` in
[slang-check-expr.cpp](../../../../source/slang/slang-check-expr.cpp)).
`lowerBuiltinOperatorExpr` (line 4859) switches on that enum and emits the
matching pure IR op directly via `emitIntrinsicInst`, skipping callable
resolution entirely. The element type only matters for `%`, which picks `kIROp_FRem` or
`kIROp_IRem` from the node's `elementTypeIsFloatingPoint` flag; the checker
sets it because the operand element type can still be an unspecialized
generic parameter at lowering time. `?:`, `&&`, and `||` are still lowered
through their dedicated control-flow paths and are `SLANG_UNEXPECTED` here.

Compile-time integer expressions take a parallel path on the `Val`
side. `visitBuiltinOperationIntVal` (line 1936) in
[slang-lower-to-ir.cpp](../../../../source/slang/slang-lower-to-ir.cpp)
lowers a `BuiltinOperationIntVal` (the checked, folded form of a
constant operator expression) to the hoistable `constexpr*` opcode keyed
on its `BuiltinOperationKind` (`emitConstexprAdd`, `emitConstexprDiv`,
`emitConstexprSelect`, ...). The `constexpr*` ops are hoistable, so equal compile-time expressions
deduplicate to one inst.

Arithmetic on a generic value parameter (for example `int a[N / 2];`)
is the surface that reaches these opcodes. `+`, `-`, `*` and unary `-`
instead arrive as a `PolynomialIntVal`, which `visitPolynomialIntVal`
(line 2038) lowers to the same opcode family; a literal-only expression
folds to a `ConstantIntVal` and becomes a plain `IRIntLit`.

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
`visitInheritanceDecl` (line 10983) creates the table and
`lowerWitnessTable` (line 10867) fills it in:

```cpp
void lowerWitnessTable(
    IRGenContext* subContext,
    WitnessTable* astWitnessTable,
    IRWitnessTable* irWitnessTable,
    DeclRefBase* witnessTableBaseDeclRef)
```

The last parameter is the decl-ref of the *base* (interface) side of the
conformance, obtained from `getWitnessTableBaseDeclRef` (lines 10684 and
10695). When it is non-null, every requirement witness taken out of the
AST requirement dictionary is `specialize`d through it before being
lowered, so an entry copied from a generic interface carries the
conforming type's substitutions. It is null only when the conformance's
base type is not a `DeclRefType`.

A requirement satisfied by a member of the conforming type and mentioning
no interface parameter stays flat, while one inherited from the
interface's default implementation carries an inner `specialize` binding
the interface's own parameters, then an outer one supplying the
conforming type and its witness table. Names in a `-dump-ir` listing go
through `scrubName` ([slang-ir.cpp](../../../../source/slang/slang-ir.cpp)
line 7752), which escapes every non-alphanumeric byte as `x` plus its hex
value, so `IFoo_zero` prints as `IFoox5Fzero`.

Per-entry lowering is factored into `lowerWitnessEntryValue`
(line 10713), which switches on the `RequirementWitness::Flavor`: a
`declRef` witness lowers through `emitDeclRef`, a `val` witness through
`lowerSimpleVal`, and a `witnessTable` witness recursively materializes
a nested `IRWitnessTable`, memoized in the non-owning
`IRGenContext::mapASTWitnessTableToIRWitnessTable` (line 688) so copied
contexts share the cache for the current insertion / generic environment.

### Generic interface requirements

A requirement can itself be generic — most commonly a differentiability
constraint attached to a generic interface method, which the checker
records as a sibling `GenericDecl` whose `inner` is a
`GenericTypeConstraintDecl`. Such an entry is not a flat value: it must
supply the method-local generic parameters. `lowerWitnessTable`
recognizes that shape and routes it to
`lowerWitnessEntryValueInGenericWitnessTable` (line 10798), which

- opens a nested `IRGenEnv` and its own nested-witness-table cache,
- emits the requirement's own generic parameters with `emitGenericDecl`
  (lines 13016 and 13113),
- lowers the satisfying witness inside that environment, and
- closes only the generics it opened, via the `stopBeforeGeneric`
  parameter of `finishOuterGenerics`.

The result is a requirement-local `IRGeneric` stored as the witness-table
entry value, so a use site becomes
`specialize(lookupWitness(table, key), methodArgs...)`.
`canDeclLowerToAGeneric` (line 14917) has the matching rule on the decl
side: a `GenericTypeConstraintDecl` lowers to a generic exactly when it
is the `inner` of a `GenericDecl`.

Which constraint decls become *generic parameters* at all is decided by
`isGenericConstraintParameterDecl` (declared in
[slang-ast-decl.h](../../../../source/slang/slang-ast-decl.h) line 1244);
`emitGenericDecl` and the substitution-argument walk in
`ExprLoweringContext` both consult it. Constraints that are instead
interface requirements are lowered as requirement keys.

### Associated-type bounds

A bound on an associated type — `associatedtype A : IBar`,
`associatedtype A where A : IBar`, or `__constraint A : IBar` — is a
requirement of the *enclosing interface*, a sibling of `A`, not something
nested under it (see
[03-semantic-check.md](03-semantic-check.md)). Lowering follows that
representation:

- `visitAssocTypeDecl` lowers the associated type with an empty list of
  constraint interfaces, because the bounds are no longer its members.
- The `IRInterfaceType` requirement loop still expands property and
  subscript members into one entry per accessor and skips
  `InterfaceDefaultImplDecl` containers, but it no longer walks an
  associated type's or a callable's nested `TypeConstraintDecl`s to
  synthesize extra entries; each relocated constraint is a direct member
  that gets its own entry. A relocated non-equality subtype constraint gets a
  `WitnessTableType(bound)` requirement value, wrapped in an `IRGeneric`
  when it is the `inner` of a `GenericDecl`.
- `visitGenericTypeConstraintDecl` (line 10564) therefore recognizes just
  two interface-requirement shapes — a constraint that is a direct member
  of an `InterfaceDecl`, and a constraint that is the `inner` of a
  `GenericDecl` directly under an `InterfaceDecl` — plus the
  global-generic-parameter case. The former per-`AssocTypeDecl` and
  per-`FuncDecl` parent cases are gone.

### Requirement keys

Each interface requirement is identified by a *requirement key*.
`getInterfaceRequirementKey` (line 1748) in
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
  across decls and across the precompiled-core-module boundary. The key
  also carries an `IRBuiltinRequirementDecoration` so autodiff can find a
  requirement by role rather than by position. The witness-table entry's
  `lookupKey` operand is therefore typed `IRInst` (not `IRStructKey`).

### Differentiability arrives as a conformance

Differentiability is not a modifier that lowering re-derives. The checker
represents it as an interface conformance of the *function viewed as a
type*: a `[Differentiable]` function `f` gets a synthesized
`extension __func_as_type(f) : IForwardDifferentiable<__func_as_type(f)>`
(and the backward analogue), where
`IForwardDifferentiable<FType>` / `IBackwardDifferentiable<FType>` are
declared in
[core.meta.slang](../../../../source/slang/core.meta.slang) at lines 767
and 786. See
[03-semantic-check.md](03-semantic-check.md#differentiability-as-interface-conformance)
for how that representation is built. Lowering consumes it in three places:

- **The conformance itself.** `visitInheritanceDecl` treats an
  `InheritanceDecl` carrying a `SynthesizedModifier` specially: instead of
  a real `IRWitnessTable` it emits one intrinsic inst whose opcode is the
  modifier's `op` (for the forward case
  `kIROp_SynthesizedForwardDerivativeWitnessTable`),
  typed `WitnessTableType(IForwardDifferentiable<...>)`, with the
  conformance's `Val` operands lowered as the inst's operands. Back-end
  passes reconstruct a table from those operands if one is needed. Because
  the placeholder `LoweredValInfo` normally installed *before*
  `lowerType(subType)` is installed *after* it for a callable subtype
  (lines 11118-11134), so lowering the callable decl-ref sees the real
  callable.
- **The interface members.** A `SynthesizedFuncDecl` (line 13915) — the
  `fwd_diff` member the interface requires — is lowered by creating an
  `IRFunc`, replacing it with `emitIntrinsicInst` of the decl's stored
  `irOp` (`kIROp_ForwardDifferentiate` here), and rewriting the
  decl→value mapping to that inst.
- **Associated values.** `lowerAssociatedVals` (line 4402) reads the
  `DifferentiableAttribute` of the function being lowered and attaches
  each associated value with `IRBuilder::addAnnotation`. It skips a decl
  for which `isInterfaceRequirement` holds, and substitutes a stored
  value through its decl-ref when the key is a `DeclRefBase`.

A function-typed conformance also changes what `this` means for the
synthesized extension's members, but the redirect lives in checking:
lowering asks `findEffectiveThisParamInfo` (declared in
[slang-syntax.h](../../../../source/slang/slang-syntax.h)) for the
recorded type and passing mode and adds it in `addThisParameter`
(line 3961).

### Variadic pack-count witnesses

A `countof(Pack) == Count` constraint on a variadic generic — spelled
`void f<let N : int, each T>(T x) where countof(T) == N` — is recorded
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
  (concrete) instance through the `emitConcreteVariadicPackCountWitness`
  helper, which creates one module-level proof-only `IRWitnessTable` and
  caches it on
  `SharedIRGenContext::concreteVariadicPackCountWitnessTable` (line 534)
  so every specialized call site reuses a single table rather than
  emitting a fresh one. A global-generic-param form is handled by
  `visitGenericVariadicPackCountConstraintDecl` (line 10635), which emits
  an `IRGlobalGenericParam` of the same witness type; that visitor now
  asserts up front that a pack-count constraint is never an interface
  requirement.

## Diagnostics during lowering

Most diagnostic-worthy issues are caught in semantic checking, but a
handful of constructs become problems only when lowering tries to
encode them — typically because a feature is unsupported on a given
target or a synthesized witness cannot be produced. For example, when
the assignment-lowering switch in
[slang-lower-to-ir.cpp](../../../../source/slang/slang-lower-to-ir.cpp)
reaches an assignment whose left-hand side it cannot encode, it emits
`Diagnostics::UnsupportedAssignmentTarget` (line 10382), recovering the
nearest non-zero source location from the builder's source-loc info,
rather than aborting via `SLANG_UNIMPLEMENTED_X`. The left-hand sides
that get there are the ones the flavor table above has already collapsed
to an r-value. Indexing an array-valued `property` (`s.v[0] = 1.0;`) is
the reachable case, because `s.v` materializes to a getter `call` and `[0]`
extracts from that temporary; it reports
`error[E40017]: assignment target is not supported`.

Lowering errors flow through the same `DiagnosticSink` used by the rest
of the front-end (see
[../cross-cutting/diagnostics.md](../cross-cutting/diagnostics.md)).

Three other lowering-time reports are worth knowing about because they
depend on facts only the lowering walk has:

- **Statements before the first `case` label.** With no case label
  active, a statement in a `switch` body is unreachable, since control
  enters only through a `case` / `default` dispatch. Lowering warns once
  for the leading run with `Diagnostics::UnreachableCode`, tracked by
  `warnedUnreachableBeforeFirstCase`.
- **A `switch` body with no `case` or `default` label.** `visitSwitchStmt`
  takes its no-cases early return before `lowerSwitchCases`, so that return
  reports `Diagnostics::UnreachableCode` itself, naming the first
  statement via `findFirstNonEmptyStmt`. A body that discards nothing
  (`{ }`, `{ ; }`) stays silent.
- **Runaway constructor-call lowering.** `visitInvokeExprImpl` counts its
  own recursion depth in `IRGenContext::invokeLoweringRecursionDepth` and,
  past `kMaxIRInvokeLoweringRecursionDepth` (128, line 4957), diagnoses
  `Diagnostics::MaximumTypeNestingLevelExceeded` and yields
  `getPoison(type)` rather than overflowing the native stack on an
  infinitely nesting type that keeps synthesizing constructor calls.
  The counter advances once per nested `InvokeExpr` (arguments are
  lowered from `addDirectCallArgs`, line 4605), and the guard at line 4958
  runs before the increment, so 128 nested calls still compile and 129 is
  the first to report `fatal error[E39997]: maximum type nesting level
  exceeded`.

## Module-level outputs

The `IRModule` that `generateIRForTranslationUnit` returns is its only
separate output object; the one other lasting effect is a mutation of
the checked AST — when a registered entry point carries no explicit
`EntryPointAttribute`, lowering creates one (filling its capability set
from the entry-point profile) and calls `addModifier` on the function
decl at line 15305, so that ordinary function lowering recognizes it as
an entry point:

- The entry-point IR functions and their decorations are children of
  that module — the loop at line 15528 lowers each registered entry
  point into it.
- Layout intent on global parameters is *not* materialized here; layout
  assignment happens later, in IR passes and in the separate module of
  [04c-layout-ir.md](04c-layout-ir.md).

The caller in
[slang-compile-request.cpp](../../../../source/slang/slang-compile-request.cpp)
installs the returned module on the AST-level `Module` with
`setIRModule` (line 774).

Two adjacent generation paths build their own modules and are *not*
outputs of translation-unit lowering: `generateIRForTypeConformance`
(line 16063) and `TargetProgram::createIRModuleForLayout` (line 16434).
The latter records the front-end's byte alignment as a `TypeAlignment`
attribute through `_lowerTypeLayoutCommon` (line 16104) whenever the layout
occupies the `LayoutResourceKind::Uniform` unit.

### Entry-point-scoped decorations

`lowerFrontEndEntryPointToIR` (line 15280) attaches the decorations that
are meaningful only on an entry point. Besides the name / module
decorations, it lifts the `spvShader64BitIndexingEXT` capability onto the
entry point as an `IRShader64BitIndexingDecoration` through the helper
`lowerEntryPointShader64BitIndexingRequirement` (line 15260), which scans
the atom sets of the capability set it is given (`getAtomSets()` rather
than `implies()`, which is too strict as a presence test).
`TargetProgram::createIRModuleForLayout` calls the same helper
(line 16565). The requirement is lifted onto the entry point because the
SPIR-V execution mode is entry-point scoped.
Work-graph node attributes (`[NodeLaunch]`, `[NodeID]`,
`[NodeMaxDispatchGrid]`, `[NodeDispatchGrid]`, `[MaxRecords]`,
`[NodeIsProgramEntry]`) take the same route in the general function path
(line 14545), each lowering to its matching decoration; the launch mode is
kept as an `IRStringLit`. Their `attribute_syntax` declarations live in
the `experimental.workgraph` standard module, so the shader must
`import experimental.workgraph` and the compile must pass
`-experimental-feature`.

### Debug-info gating

Lowering decides per construct whether to attach source-level debug
information. Notably, `isSynthesizedConstructorDecl` (line 9551) in
[slang-lower-to-ir.cpp](../../../../source/slang/slang-lower-to-ir.cpp)
makes `maybeEmitDebugLine` and `maybeAddDebugLocationDecoration`
skip a Slang-synthesized initializer (a `ConstructorDecl` with the
`SynthesizedDefault` or `SynthesizedMemberInit` flavor). Such a function
has no user-authored body, so a debugger should not step into it. The
discrimination is by constructor *flavor*, not the mangled `$init` name,
because a user-written `__init` mangles the same way but must keep its
debug info.

The same predicate has a third caller: constructor lowering (line 14342)
names the object under construction `this` and adds an
`IRDebugLocationDecoration` to the by-value local, since `addNameHint`
has no internal synthesized-constructor gate.

Debug *source* records are hoistable and collapse across modules only when
operands match byte-for-byte, so `getOrEmitDebugSource` (line 9309) spells
the filename with `PathInfo::getMostUniqueIdentity()`, matching the
per-source-file loop in `generateIRForTranslationUnit`. It embeds source
text when the debug level is `Standard` or above or
`shouldIncludeSourceInDebugInfo()` is set, reading the path off disk for a
`SourceFile` deserialized from a precompiled module.

After lowering, the IR module is the input to the IR-pass pipeline
described in [05-ir-passes.md](05-ir-passes.md).

## Adjacent pipelines

Two adjacent pipelines:

- [04b-pre-link-passes.md](04b-pre-link-passes.md) — the
  per-translation-unit, target-agnostic mandatory pass sequence
  inside `generateIRForTranslationUnit`, executed before the IR module is
  cached on the `Module`.
- [04c-layout-ir.md](04c-layout-ir.md) — `TargetProgram::createIRModuleForLayout`
  builds a separate, per-target IR module holding imported global and
  entry-point stubs and the layout decorations and instructions attached
  to them. It is not the executable per-translation-unit module and does
  not run the mandatory passes above, but `linkIR` pulls an existing layout
  module in via `TargetProgram::getExistingIRModuleForLayout` so its
  layout-decorated global symbols participate in linking.
