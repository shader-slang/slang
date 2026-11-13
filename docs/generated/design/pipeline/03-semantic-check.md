---
generated: true
model: claude-sonnet-5-5
generated_at: 2026-11-02T21:22:45Z
source_commit: 2518a942d59b1228fc8cc70a37739baab82b6002
watched_paths_digest: cd6860f053b52ab46cc0816a230dc20093e248da0db8a4bb4c551e2dee5c5a14
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Semantic Checking

This document covers the semantic-checking stage: turning a raw AST
into a fully resolved, type-checked AST that is ready to be lowered
into IR. The intended reader is a developer working on type checking,
name resolution, overload resolution, or interface conformance.

## Inputs and outputs

- **Input**: an AST produced by
  [02-parse-ast.md](02-parse-ast.md), with function bodies still in
  `UnparsedStmt` form.
- **Output**: the same AST, but with names resolved (every
  `DeclRef`-bearing node points at the canonical decl), types attached
  (every `Expr` carries a `Type*`), conformances recorded, modifiers
  validated, default conformance witnesses synthesized, and function
  bodies fully parsed and checked.

Concretely, for

```slang
interface IFoo { int base(); int twice() { return base() * 2; } }
struct S : IFoo { int base() { return 6; } static const int tag = 1; }
int call<T : IFoo>(T v) { return v.twice(); }
```

the parser hands over an `S` whose base list is still an unchecked
`Expr`, a `call` body still in `UnparsedStmt` form, and not one `Expr`
carrying a type. Checking resolves `IFoo` in `S`'s base list and in
`T`'s constraint to the same decl (*names resolved*); gives
`v.twice()` the type `int` (*types attached*); records the `S : IFoo`
witness table (*conformances recorded*); fills its `twice` slot with a
witness for the interface's default body (*default conformance
witnesses synthesized*); and turns the `UnparsedStmt` into a checked
`Stmt` tree (*function bodies fully parsed and checked*). The `static`
on `S::tag` is the *modifiers validated* case: `checkModifier` asks
`isModifierAllowedOnDecl` whether `static` may sit on a struct field
and `getModifierConflictGroupKind` whether another modifier has claimed
its group. Both are described under
[Modifier validation](#modifier-validation).

The result is the input to AST → IR lowering
([04-ast-to-ir.md](04-ast-to-ir.md)).

## SemanticsVisitor

The checker is implemented as a family of visitor subclasses that
share state through `SemanticsContext`. The base visitor lives in
[slang-check-impl.h](../../../../source/slang/slang-check-impl.h) and is
declared as:

```cpp
struct SemanticsVisitor : public SemanticsContext
```

The top-level entry point is `checkTranslationUnit` in
[slang-check.cpp](../../../../source/slang/slang-check.cpp), which the
front-end calls once per `TranslationUnitRequest` after parsing has
collected the decls.

### Files and responsibilities

The `slang-check-*.cpp` family in
[source/slang/](../../../../source/slang) splits the work by concern.

The *Example rejection* column names one diagnostic the file itself
emits, so each row is a claim a test can be written against. Three rows
have none: `slang-check-conformance.cpp` and
`slang-check-resolve-val.cpp` emit no diagnostics, leaving reporting to
the caller, and `slang-check.cpp`'s only diagnostics are about loading a
downstream compiler.

| File | Concern | Example rejection |
| --- | --- | --- |
| [slang-check.cpp](../../../../source/slang/slang-check.cpp) | Entry point; orchestrates the checking phases | none from checking; it sequences the phases |
| [slang-check-decl.cpp](../../../../source/slang/slang-check-decl.cpp) | `Decl` checking — types, signatures, default values, attributes | `E30200`, a declaration conflicting with an earlier one |
| [slang-check-expr.cpp](../../../../source/slang/slang-check-expr.cpp) | `Expr` checking — type inference, lvalue-ness, conversions | `E30011`, assigning to something that is not an l-value |
| [slang-check-stmt.cpp](../../../../source/slang/slang-check-stmt.cpp) | `Stmt` checking — control flow, scope rules, return-type validation | `E30003`, `break` outside a loop or `switch` |
| [slang-check-type.cpp](../../../../source/slang/slang-check-type.cpp) | Resolves `Type` references that appear in `Expr` form | `E30060`, an expression used where a type is required |
| [slang-check-overload.cpp](../../../../source/slang/slang-check-overload.cpp) | Overload resolution; ranks candidates produced by lookup | `E40018`, the note naming the argument that rejected a candidate |
| [slang-check-conformance.cpp](../../../../source/slang/slang-check-conformance.cpp) | Verifies and synthesizes interface conformances | none; a missing requirement is reported as `E38100` by its caller in `slang-check-decl.cpp` |
| [slang-check-conversion.cpp](../../../../source/slang/slang-check-conversion.cpp) | Implicit-conversion ranking and coercion site checks | `E30523`, too many initializers in an initializer list |
| [slang-check-inheritance.cpp](../../../../source/slang/slang-check-inheritance.cpp) | Inheritance and extension lookup; facet computation | `E30815`, a circular `extension` (see below) |
| [slang-check-modifier.cpp](../../../../source/slang/slang-check-modifier.cpp) | Validates modifier combinations and attribute arguments | `E31202`, two modifiers from one exclusive group on a decl |
| [slang-check-constraint.cpp](../../../../source/slang/slang-check-constraint.cpp) | Generic constraint solving (`where`-clauses, witness inference) | `E30433`, a pack count that fails a `countof(...)` constraint |
| [slang-check-resolve-val.cpp](../../../../source/slang/slang-check-resolve-val.cpp) | Resolves and canonicalizes `Type`, `DeclRef`, and witness values | none; a bad resolution result is reported at the use site |
| [slang-check-shader.cpp](../../../../source/slang/slang-check-shader.cpp) | Entry-point checks: stage-specific signatures, parameter rules | `E38007`, an entry point with no stage |

`E30815` is narrower than "circular extension" suggests. It is not a
general cycle detector over extension target types; it fires only when
*the same `ExtensionDecl`* is re-entered while its own inheritance
information is still being computed. `getInheritanceInfo(DeclRef<ExtensionDecl>)`
pushes an `InheritanceCircularityInfo` node naming the extension onto a
linked stack and recurses;
`_checkForCircularityInExtensionTargetType`
([slang-check-inheritance.cpp](../../../../source/slang/slang-check-inheritance.cpp)
line 254) walks that stack on entry and, on finding the same `Decl*`,
reports `CircularityInExtension` and returns an empty `InheritanceInfo`
so the recursion terminates.

Cycles that never re-enter one extension are *other* diagnostics:
mutually-referential extension targets, a self-referential `typealias`
in an extension, or an extension reached through `This` report `E30027`,
`E30813`, or the fatal `E40002` instead. `_isInheritanceInfoBeingComputed`
exists so that an equality constraint such as `__constraint A == B`,
which makes each of `T.A` and `T.B` a base of the other, is skipped
during linearization rather than reported as a cycle.

## Two-pass interaction with the parser

The parser left function and method bodies as `UnparsedStmt` nodes
(see [02-parse-ast.md](02-parse-ast.md)). When the checker reaches
one, `SemanticsVisitor::maybeParseStmt`
([slang-check-decl.cpp](../../../../source/slang/slang-check-decl.cpp),
line 15606) calls `parseUnparsedStmt`
([slang-parser.h](../../../../source/slang/slang-parser.h)) with a
`SemanticsVisitor*` so that the parser can call back into the
checker to disambiguate `<` tokens at parse time. Once the body is
parsed, the checker continues over the resulting `Stmt` tree, so there
is no clean parse / check boundary inside function bodies. The deeper
rationale is in
[../../../design/parsing.md](../../../design/parsing.md).

## Name lookup and `DeclRef`

Name resolution produces `DeclRef`s — a decl plus a substitution that
records how its generic and outer-context parameters have been bound.
The concrete `DeclRefBase` operations (`DirectDeclRef`, `LookupDeclRef`,
and substitution application) are implemented in
[slang-ast-decl-ref.cpp](../../../../source/slang/slang-ast-decl-ref.cpp).
The algorithmic rules — scope construction, the lookup algorithm,
shadowing, visibility filtering, and overload resolution — live in
the dedicated [../name-resolution/](../name-resolution) subtree.
Start at [../name-resolution/index.md](../name-resolution/index.md).
For the deeper rationale on decl-refs themselves see
[../../../design/decl-refs.md](../../../design/decl-refs.md).

## Generic specialization and constraints

The checker implements generic-parameter resolution through:

- [slang-check-constraint.cpp](../../../../source/slang/slang-check-constraint.cpp)
  — accumulates and solves type / value / witness constraints.
- [slang-check-conformance.cpp](../../../../source/slang/slang-check-conformance.cpp)
  — finds (or synthesizes) the witness that a type satisfies an
  interface required by a constraint.
- [slang-check-resolve-val.cpp](../../../../source/slang/slang-check-resolve-val.cpp)
  — validates `Val` substitutions after generic resolution.

When resolving a generic application,
`TryCheckOverloadCandidateConstraints`
([slang-check-overload.cpp](../../../../source/slang/slang-check-overload.cpp))
routes an outermost generic's defaulted and witness arguments (a nested
generic keeps the per-constraint linear pass) through
the constraint solver's fixpoint (`trySolveGenericArguments`) — the
same path used for inferred arguments — passing only the explicitly
supplied ordinary-argument prefix
(`OverloadCandidate::explicitGenericArgCount` in
[slang-check-impl.h](../../../../source/slang/slang-check-impl.h)) as
fixed caller input so a user-written self-reference argument is not
overwritten by a parameter's default. On solver failure the code
falls through to the per-constraint linear pass that re-derives the
failing constraint to emit a precise diagnostic.

A constraint written on an associated type — whether as
`associatedtype A : IBar`, `associatedtype A where A : IBar`, or
`__constraint A : IBar` — is recorded uniformly as a
`GenericTypeConstraintDecl` requirement of the *enclosing interface*
(a sibling of `A`), not nested under `A`. In that unified
representation `findWitnessForInterfaceRequirement`
([slang-check-decl.cpp](../../../../source/slang/slang-check-decl.cpp))
satisfies an interface-level constraint requirement by re-checking
the subtype (or, for an `==` constraint, type-equality) relationship
after `This` has been replaced by the conforming type, rather than by
finding a member of that type. A witness already installed by
conformance synthesis — for example an `enum`'s synthesized
`__Tag : __BuiltinIntegerType` (including the `bool`-tagged case,
where no real subtype witness exists and a `NoneWitness` marks the
compiler-trusted constraint satisfied) — is honored by the
witness-table early-out at the top of that function.

Linearized inheritance lists are computed by `getInheritanceInfo` /
`_calcInheritanceInfo` in
[slang-check-inheritance.cpp](../../../../source/slang/slang-check-inheritance.cpp).
For an associated-type access such as `T.D`, the engine surfaces the
interface-level `__constraint`s of the interfaces each anchor type
conforms to, re-expresses each through the anchor's conformance witness,
and adds the opposite endpoint as a base of the access. For the
*benevolent* cycle made by `__constraint A == B` it skips a base whose
inheritance info is still being computed, accumulating the skipped
in-progress ancestor `DeclRef`s through a `HashSet<DeclRef<Decl>>*
ioSkippedIncompleteFacet` out-parameter; a frame whose skipped set is
non-empty after subtracting itself is partial, is not cached, and is
recomputed by a later root-level query. A bare-`This` subject on an
interface `__constraint` is rejected in `visitGenericTypeConstraintDecl`
([slang-check-decl.cpp](../../../../source/slang/slang-check-decl.cpp),
line 4755) as `ConstraintSubjectCannotBeThisType`.

When a generic cannot be specialized for a call, the failure reason is
captured eagerly but reported lazily. The constraint solver records a
`GenericArgumentInferenceFailure`
([slang-check-impl.h](../../../../source/slang/slang-check-impl.h)) — a
tagged union whose `Kind` selects both the stored payload and the
diagnostic that will eventually be emitted:

| `Kind` | Diagnostic | Triggered by |
| --- | --- | --- |
| `VariadicPackCountMismatch` | `E30433` | `takesTwo(1, 2, 3)` against `void takesTwo<each T>(expand each T args) where countof(T) == 2` |
| `GenericArityMismatch` | `E30438` | a call whose argument list cannot be matched to the generic's parameter list at all |
| `OrdinaryGenericParamNotInferred` | `E30439` | `f(1)` against `void f<T>(int a)` — no argument mentions `T` |
| `InterfaceConformanceNotSatisfied` | `E38029` | `pick(s)` against `T pick<T : IFoo>(T a)` where `S` does not implement `IFoo` |
| `GenericConstraintNotSatisfied` | `E30440` | `f(1.5f)` against `void f<T>(T a) where T == int`; the fallback for every constraint that is not a conformance |
| `GenericParamUnificationConflict` | `E30442` | `two(x, y)` against `void two<T>(T a, T b)` with unrelated `A` and `B` |

Every arm follows its error with a "see declaration of" note carrying
the candidate's rendered signature (`GenericSignatureTried`), and the
`GenericConstraintNotSatisfied` arm adds a note pointing at the
`where`-clause. Each `Kind` stores only the offending fields; message
formatting is deferred so speculative candidates never pay for it. The failure is attached to the `OverloadCandidate`
and turned into a focused diagnostic only if overload resolution
selects that failed candidate — see the `switch` over
`candidate.genericInferenceFailure.kind` in `CompleteOverloadCandidate`
([slang-check-overload.cpp](../../../../source/slang/slang-check-overload.cpp)).
A candidate whose failure `Kind` is `None` still falls back to the
catch-all `Diagnostics::GenericArgumentInferenceFailed`.

### Differentiability as interface conformance

Differentiability is recorded as an interface conformance of the
*function viewed as a type*, rather than as a modifier fact that later
stages re-derive. Consider:

```slang
[Differentiable]
float f(float x) { return x * x; }
```

`[Differentiable]` parses to a `BackwardDifferentiableAttribute` (see
the `attribute_syntax` declarations in
[core.meta.slang](../../../../source/slang/core.meta.slang), lines 489,
498 and 517; `ForwardDifferentiableAttribute` and
`BackwardDifferentiableAttribute` both derive from `DifferentiableAttribute`
in [slang-ast-modifier.h](../../../../source/slang/slang-ast-modifier.h)).
`SemanticsDeclHeaderVisitor::synthesizeDifferentiabilityRequirements`
([slang-check-decl.cpp](../../../../source/slang/slang-check-decl.cpp),
line 16160) runs for each non-synthesized `FunctionDeclBase` in the
`DeclCheckState::ReadyForReference` arm of the decl-check dispatch, after
`SignatureChecked` has recorded the effective `this` parameter. On seeing
`ForwardDifferentiableAttribute` or `BackwardDifferentiableAttribute` it
calls `extendContainerDecl` to synthesize
`extension __func_as_type(f) : IForwardDifferentiable<__func_as_type(f)>`
and then `addSynthesizedFunc` to give that extension the `fwd_diff`
member the interface requires, with `kIROp_ForwardDifferentiate` as its
implementation. The synthesized `fwd_diff` is in turn given the same
pair of conformances (`IForwardDifferentiable` and
`IBackwardDifferentiable`), which is what makes higher-order
differentiation resolve through ordinary lookup. A backward-mode extension
synthesizes the `BwdCallable` and `MinimalContext` structs and the
`apply_bwd` and `remat` members. A decl carrying
`[TreatAsDifferentiable]` or `HasTrivialForwardDerivativeAttribute` gets
the forward extension with the trivial ops (`kIROp_TrivialForwardDifferentiate`).

Each mode must end up with one conformance, so the synthesized forward
(backward) extension is skipped when the decl itself carries
`[ForwardDerivative]` (`[BackwardDerivative]`); the later translation of
that attribute supplies it. The guard sees only attributes on the decl,
so `[ForwardDerivativeOf]` can still produce both.

The interface types are built by `getForwardDiffFuncInterfaceType` and
`getBackwardDiffFuncInterfaceType` (lines 10414 and 10420), which pair
the base function type with the `__hasDiffTypeInfo` witness that
`IForwardDifferentiable<FType>` / `IBackwardDifferentiable<FType>`
demand. Those interfaces are declared in
[core.meta.slang](../../../../source/slang/core.meta.slang) at lines 767
and 786; their requirements (`fwd_diff`; `BwdCallable`, `MinimalContext`,
`apply_bwd`, `remat`, `bwd_diff`) are what the checker must supply.

Because the fact now lives in a witness table, asking "is this callee
differentiable?" is a subtype query instead of a modifier lookup:
`isFuncForwardDifferentiable` and `isFuncBackwardDifferentiable` (lines
5806 and 5815) return the `SubtypeWitness*` that `tryGetSubtypeWitness`
produced. The caller needs the witness to build the derivative call.

A differentiability annotation (`[ForwardDifferentiable]`,
`[BackwardDifferentiable]`, `[Differentiable]`, `[MaybeDifferentiable]`)
on an *interface requirement* is handled like the associated-type
constraints above: a sibling requirement of the enclosing interface
rather than something nested under the member. The same function creates a
`FuncConstraintDecl` (a `GenericTypeConstraintDecl`; marked
`OptionalConstraintModifier` for `[MaybeDifferentiable]`) whose subject is
the requirement viewed as a type, and
`_moveInterfaceDifferentiabilityRequirementToInterface` (line 16036)
starts it under the callable that owns the generic environment its type
mentions, then uses `liftDeclFromGenericContainers` to hoist it into a
standalone generic requirement directly under the interface. The explicit
`__func_extension fwd_diff(foo)(...)` spelling arrives at the same
representation through `_funcExtensionForwardDiff` (line 17207), which rewrites it
into `extension foo : IForwardDifferentiable<foo>` with the user's body
as the `fwd_diff` member. `_funcExtensionBackwardDiff` (line 17240)
instead builds an `IBackwardDifferentiable` conformance whose `bwd_diff`
is a synthesized `kIROp_FunctionCopy` of the user's function, plus
synthesized backward-context members.

The full conceptual model (interfaces, witness tables, existential
types) is in
[../../../design/interfaces.md](../../../design/interfaces.md) and
[../../../design/existential-types.md](../../../design/existential-types.md);
this document only points at the implementation.

## Synthesizing implicit code

Some declarations gain members at check time: default conformance witnesses, generated comparison /
construction methods, and several built-in conformances. The
*decisions* about what to synthesize live primarily in
[slang-check-decl.cpp](../../../../source/slang/slang-check-decl.cpp)
— for example `_synthesizeCtorSignature` for default constructors and
the `trySynthesize*RequirementWitness` routines for interface
requirements — while
[slang-ast-synthesis.cpp](../../../../source/slang/slang-ast-synthesis.cpp)
supplies the `ASTSynthesizer` helpers (`emitBinaryExpr`, `emitVarExpr`,
`emitInvokeExpr`, `emitVarDeclStmt`, ...) that build the AST fragments
those routines emit. The checker calls into this machinery for members the user did not
write but the language guarantees.

## Modifier validation

Modifier-specific checks live in
[slang-check-modifier.cpp](../../../../source/slang/slang-check-modifier.cpp):
which modifiers are allowed on which decls, mutually exclusive
combinations, attribute argument types, and the differences between
Slang and GLSL input. Modifier nodes themselves are defined in
[slang-ast-modifier.h](../../../../source/slang/slang-ast-modifier.h).

*Mutually exclusive* is decided by `getModifierConflictGroupKind`
(line 1564), which maps a modifier's `ASTNodeType` to the group it
competes in; a second modifier landing in a group already claimed on
the same decl produces `E31202`. Most modifiers are their own group,
so a plain repeat (`static static int g;`) is the common case, but the
groups with more than one member are the ones worth knowing:
`out`, `inout`, `ref`, `__ref_readonly`, `__ref_writeonly` and `borrow`
share a group; `static` and `uniform` share one; `nointerpolation`,
`noperspective`, `linear` and `pervertex` share one; and `sample` and
`centroid` share a separate one, so an interpolation mode combines with
a sampling location (`noperspective centroid`).

The dialect axis here is GLSL rather than HLSL. `checkModifier` (line
1946) computes `isGLSLInput` from
`SharedSemanticsContext::isGLSLSourceLanguage`
([slang-check-impl.h](../../../../source/slang/slang-check-impl.h), line
1056), which is true when the translation unit's source language is GLSL
(false for a context with no translation unit), and passes it to
`isModifierAllowedOnDecl` (line 1681); a
modifier used in a position that predicate rejects is reported as
`E31201` (*modifier '<name>' is not allowed here*).

A concrete case that reaches it is `globallycoherent` on a **function
parameter**:

```slang
void f(globallycoherent int x) { }
```

Without GLSL input the arm for `GloballyCoherentModifier` and
`HLSLVolatileModifier` (line 1741) requires `as<VarDecl>(decl)`, and a
parameter is a `ParamDecl`, which derives from `VarDeclBase` as a
*sibling* of `VarDecl`
([slang-ast-decl.h](../../../../source/slang/slang-ast-decl.h) lines
321, 339, 648), so the modifier is rejected. Many other combinations are
settled earlier, by the parser or by plain acceptance, and never reach
`E31201`.

The GLSL flag widens that arm only a little. Both branches already
accept a *struct field* (`struct G { globallycoherent int a; }` is
accepted either way); `isGLSLInput` adds the parameter case above
(`as<ParamDecl>(decl)`) and global declarations that are a
`VarDeclBase` without being a `VarDecl`. Only a handful of entries
branch on the flag at all.

### Visibility scopes

`public`, `internal` and `private` are checked like any other
modifier, but the scope each one names is not the source file.
`isDeclVisibleFromScope`
([slang-check-expr.cpp](../../../../source/slang/slang-check-expr.cpp),
line 1181) decides the question:

| Modifier | Visible from |
| --- | --- |
| `public` | any scope, including other modules |
| `internal` | any scope in the declaring module |
| `private` | the declaring type or namespace, plus extensions of that same type |

`private` is type-scoped, not file-scoped: a free function in the
*same* file cannot read a `private` member of a `struct` (`E30600`). Writing `private` where there is
no enclosing type for it to scope to — at global scope, or on an
interface requirement — is rejected up front with `E30603`.

### `dyn interface` restrictions

`validateDynInterfaceUsage` and
`validateDynInterfaceUseWithInheritanceDecl`
([slang-check-decl.cpp](../../../../source/slang/slang-check-decl.cpp),
lines 383 and 453) constrain what a `dyn interface` may declare and
what may conform to one. `validateDynInterfaceUsage` is gated on
`allowExperimentalDynamicDispatch` (line 375) and is a no-op unless
the module's language version is 2026 or later (`-std 2026`) *and*
`-enable-experimental-dynamic-dispatch` was **not** passed — so the
same source compiles differently under the default `-std`. In
`validateDynInterfaceUseWithInheritanceDecl` the same gate covers only
the extension and generic-conformer checks; the field checks are not
gated.

When the gate is open, the interface may not be generic (`E33072`)
and may not declare an associated type (`E33073`), a generic method
(`E33074`), a `[mutating]` method (`E33075`), a `[Differentiable]`
method (`E33076`), or a non-`dyn` base interface (`E33077`). A
conforming type may not acquire the conformance through an `extension`
(`E33078`) and may not be generic (`E33082`); a conforming aggregate's
fields may be neither unsized (`E33079`), opaque (`E33080`), nor
non-copyable (`E33081`), gate open or not — the dynamic representation has to be a copyable,
fixed-size box.

## Shader-specific checks

[slang-check-shader.cpp](../../../../source/slang/slang-check-shader.cpp)
validates entry points: the function's stage attribute, parameter
modifiers (`in`, `out`, `inout` and stage-specific intrinsics), return
type compatibility with the stage, and resource binding rules.

Checks scoped to entry-point validation rather than the general
inference walk:

- **Generic-struct capability requirements.**
  `collectGenericStructTypeUses`
  ([slang-check-shader.cpp](../../../../source/slang/slang-check-shader.cpp))
  recurses through an entry point's signature types (parameters and
  return type, through each struct's fields with substitutions applied
  and through `Val` operands) to find every user-defined generic struct
  with a non-empty requirement (e.g. a `Foo<int>`, including nested
  inside `Optional<...>`, arrays, or `ConstantBuffer<...>`) and
  validates its `[require(...)]` against the target. The general
  capability-inference walk (`SemanticsDeclReferenceVisitor`) records a
  type's requirements only for a `DirectDeclRef`; a generic
  specialization is a `GenericAppDeclRef` and is skipped, so the
  requirement would otherwise be dropped. The check lives
  here, not in the inference walk, so library functions that name such
  a type need not redeclare its capabilities.
  Builtin generic types carrying `MagicTypeModifier`/
  `IntrinsicTypeModifier` are filtered out (but still recursed *through*). A requirement the target
  cannot provide — `[require(cpp)] struct Foo<T>` named as `Foo<int>`
  in a SPIR-V entry point's signature — is reported against the entry
  point as `E36107`, followed by `see using of 'Foo'`, `see definition
  of` and `see declaration of` modifier notes
  (`validateEntryPointCapabilities`, line 1813).
- **Unspecialized generic entry points.** A generic entry point such as
  `void main<T>(...)` left genuinely unspecialized lowers to an
  `IRGeneric` rather than an `IRFunc` and used to crash at link time.
  `createSpecializedGlobalAndEntryPointsComponentType`
  ([slang-check-shader.cpp](../../../../source/slang/slang-check-shader.cpp))
  uses `Linkage::isSpecialized` together with the presence of
  specialization-argument strings to decide, and calls
  `diagnoseGenericEntryPoint` (line 4269) — which emits `E38014`
  against the entry-point name — only for the truly-unspecialized
  case. The check runs only when the `SkipCodeGen` option is unset,
  since checking alone needs no concrete entry points.
- **Conflicting depth outputs.** A fragment entry point may write at
  most one depth system value. Because the per-parameter semantic check
  looks at each semantic in isolation, the conflict is detected
  separately: `collectDepthOutputSemantics`
  ([slang-check-shader.cpp](../../../../source/slang/slang-check-shader.cpp),
  line 649 at `source_commit`) walks every `out` / `inout` parameter and
  the return type — unwrapping `Conditional<T>` and array wrappers and
  recursing into struct fields, so a semantic on a field of an
  `out DepthOut a[1]` is still reached — and a collected count above one
  produces `Diagnostics::MultipleDepthOutputSemantics` (`E30705`),
  naming the second contributor as conflicting with the first.
- **System-value semantic type compatibility.** `isSemanticTypeCompatible`
  (line 175) decides whether a declared type may carry a given
  system-value semantic. Two types match when they have the same
  element count (a scalar counts as one, so `float` matches `float1`) and their scalar
  element types fall in the same category — integer, floating-point, or
  bool. That admits sign coercions such as `int3` for a `uint3` semantic
  while still rejecting cross-category ones such as
  `float gi : SV_GroupIndex`, and a shape mismatch such as
  `float pos : SV_Position`; both are reported as `E30701`, whose
  message lists the types the semantic will accept.
- **Ignored binding modifiers on entry-point parameters.** Slang
  ignores `[[vk::binding(...)]]`, `[[vk::push_constant]]`,
  `register()`, and `packoffset()` in some positions. Entry-point
  parameter checking therefore reports
  `Diagnostics::UnhandledModOnEntryPointParameter` (`E38010`, whose
  message names the modifier and the parameter and says the modifier
  will be ignored) for each such
  modifier. Only the `[[vk::binding(...)]]` case is gated — by
  `_allTargetsSupportVkBindingOnEntryPointParameters` (line 1693) over
  all of the linkage's targets and by
  `isVkBindingCompatibleEntryPointParameterType` (line 1033) for the
  parameter's own type — so that it fires only where the attribute
  really would be dropped. `[[vk::push_constant]]`, `register()`, and
  `packoffset()` are diagnosed unconditionally when found on an
  entry-point parameter.

## Failure modes

All semantic-checking errors flow through the `DiagnosticSink`
threaded into `SemanticsContext`. Check-level recovery is generally
"continue with a placeholder type" so that one error does not cascade:
unresolved decls become `ErrorType`-typed, and overload resolution
returns a synthetic `errorExpr`. When `ExpectATypeRepr`
([slang-check-type.cpp](../../../../source/slang/slang-check-type.cpp))
finds an expression that does not denote a type, it builds the
`Diagnostics::ExpectedAType` message from the expression's actual type
and, when available, the referenced name.

Several diagnostics point the user at the likely fix:

- **Per-candidate argument mismatch.** When a call matches no overload,
  the diagnostic lists each candidate signature *and* the specific
  argument that rejected it. `slang-check-overload.cpp` records the
  offending argument index and the expected/actual types on the
  candidate, then emits a
  `Diagnostics::OverloadCandidateArgumentTypeMismatch` note per
  candidate. Calling `void g(A, int)` / `void g(B, float)` with two
  `float`s reports `E39999` on the call, then an `E40011`
  `candidate: <signature>` note per candidate, each followed by an
  `E40018` note reading `argument 0 does not match: expected 'A', got
  'float'`. Generic candidates rejected for an unsatisfied conformance or
  `where`-clause are listed first, each with an `E40022` or `E40023`
  note (plus the constraint-declaration note for the latter) built by
  `diagnoseGenericConstraintFailureNote`. Candidates are deduplicated by their rendered signature
  string (not by `Decl*`, which would collapse specializations such as
  `foo<float>` and `foo<int>`); at most ten
  unique candidates are printed and the remainder are summarized by an
  `E40015` "N more overload candidates" note.
- **"Did you mean ...?" on undefined identifiers.** When a name fails to
  resolve, `slang-check-expr.cpp` walks the in-scope candidates and, via
  `StringUtil::calcLevenshteinDistanceCaseInsensitive`, attaches a
  conservative similar-name suggestion to the existing
  `Diagnostics::UndefinedIdentifier` (`E30015`) rather than emitting a
  detached note. `findClosestInScopeName` (line 5455) fixes the budget
  concretely: nothing is suggested for a name under 3 or over 256
  characters, the allowed distance is
  `min(3, max(1, length / 3))` — roughly one edit per three characters,
  floored at one and capped at three — core-module declarations and
  anything the scope could not access are skipped, and a tie between
  two distinct names suppresses the suggestion so the output does not
  depend on scope-walk order. So `myLongVariableNam` suggests
  `myLongVariableName`, while `sqr` does not suggest the core module's
  `sqrt`.
- **Discarded `[NoDiscard]` results.** `maybeDiagnoseDiscardedNoDiscardResult`
  ([slang-check-stmt.cpp](../../../../source/slang/slang-check-stmt.cpp),
  line 742) fires with `E30059` when the result of a call to a
  `[NoDiscard]`-marked function is thrown away — `f();` written as a
  bare expression statement, or in a `for` side-effect expression —
  peeling parentheses and recursing through comma, ternary-select, and
  short-circuit forms to find the discarded sub-expression. A `void`
  call and a bare discarded constructor call are deliberately
  excluded.

The diagnostic infrastructure is described in
[../cross-cutting/diagnostics.md](../cross-cutting/diagnostics.md).

`checkModule` drives every `Decl` in the translation unit through the
`DeclCheckState` sequence up to `CapabilityChecked`; there is no
separate errored state, so recovery is expressed as diagnostics plus
error types / expressions substituted in place. The AST is then ready
for IR lowering (see [04-ast-to-ir.md](04-ast-to-ir.md)).
