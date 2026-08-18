---
generated: true
model: claude-opus-5
generated_at: 2026-08-03T13:32:49Z
source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
watched_paths_digest: a244dfa19ecf6d79ea826d9b14c775491f6a2445e1ddbc0c633a710605a2aec3
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
struct S : IFoo { int base() { return 6; } }
int call<T : IFoo>(T v) { return v.twice(); }
```

the parser hands over an `S` whose base list is still an unchecked
`Expr`, a `call` body still in `UnparsedStmt` form, and not one `Expr`
carrying a type. Checking resolves `IFoo` in `S`'s base list and in
`T`'s constraint to the same decl (_names resolved_); gives
`v.twice()` the type `int`, which is what makes the call legal
(_types attached_); records the `S : IFoo` witness table
(_conformances recorded_); fills that table's `twice` slot with a
witness for the interface's default body, which `S` never spelled
(_default conformance witnesses synthesized_); and turns the
`UnparsedStmt` into a checked `Stmt` tree (_function bodies fully
parsed and checked_). Nothing here carries a modifier; the checks
that would apply are described under
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
Every file collaborates through `SemanticsContext` /
`SemanticsVisitor` declared in
[slang-check-impl.h](../../../../source/slang/slang-check-impl.h).

The _Example rejection_ column names one diagnostic the file itself
emits, so that each row is a claim a test can be written against
rather than a label. Three rows have none to name:
`slang-check-conformance.cpp` and `slang-check-resolve-val.cpp` emit
no diagnostics at all — they compute a result and leave the reporting
to whoever asked for it — and `slang-check.cpp`'s only diagnostics are
about loading a downstream compiler, not about the program.

| File                                                                                | Concern                                                             | Example rejection                                                                           |
| ----------------------------------------------------------------------------------- | ------------------------------------------------------------------- | ------------------------------------------------------------------------------------------- |
| [slang-check.cpp](../../../../source/slang/slang-check.cpp)                         | Entry point; orchestrates the checking phases                       | none from checking; it sequences the phases                                                 |
| [slang-check-decl.cpp](../../../../source/slang/slang-check-decl.cpp)               | `Decl` checking — types, signatures, default values, attributes     | `E30200`, a declaration conflicting with an earlier one                                     |
| [slang-check-expr.cpp](../../../../source/slang/slang-check-expr.cpp)               | `Expr` checking — type inference, lvalue-ness, conversions          | `E30011`, assigning to something that is not an l-value                                     |
| [slang-check-stmt.cpp](../../../../source/slang/slang-check-stmt.cpp)               | `Stmt` checking — control flow, scope rules, return-type validation | `E30003`, `break` outside a loop or `switch`                                                |
| [slang-check-type.cpp](../../../../source/slang/slang-check-type.cpp)               | Resolves `Type` references that appear in `Expr` form               | `E30060`, an expression used where a type is required                                       |
| [slang-check-overload.cpp](../../../../source/slang/slang-check-overload.cpp)       | Overload resolution; ranks candidates produced by lookup            | `E40018`, the note naming the argument that rejected a candidate                            |
| [slang-check-conformance.cpp](../../../../source/slang/slang-check-conformance.cpp) | Verifies and synthesizes interface conformances                     | none; a missing requirement is reported as `E38100` by its caller in `slang-check-decl.cpp` |
| [slang-check-conversion.cpp](../../../../source/slang/slang-check-conversion.cpp)   | Implicit-conversion ranking and coercion site checks                | `E30523`, too many initializers in an initializer list                                      |
| [slang-check-inheritance.cpp](../../../../source/slang/slang-check-inheritance.cpp) | Inheritance and extension lookup; facet computation                 | `E30815`, a circular `extension`                                                            |
| [slang-check-modifier.cpp](../../../../source/slang/slang-check-modifier.cpp)       | Validates modifier combinations and attribute arguments             | `E31202`, two modifiers from one exclusive group on a decl                                  |
| [slang-check-constraint.cpp](../../../../source/slang/slang-check-constraint.cpp)   | Generic constraint solving (`where`-clauses, witness inference)     | `E30433`, a pack count that fails a `countof(...)` constraint                               |
| [slang-check-resolve-val.cpp](../../../../source/slang/slang-check-resolve-val.cpp) | Resolves and canonicalizes `Type`, `DeclRef`, and witness values    | none; a bad resolution result is reported at the use site                                   |
| [slang-check-shader.cpp](../../../../source/slang/slang-check-shader.cpp)           | Entry-point checks: stage-specific signatures, parameter rules      | `E38007`, an entry point with no stage                                                      |

## Two-pass interaction with the parser

The parser left function and method bodies as `UnparsedStmt` nodes
(see [02-parse-ast.md](02-parse-ast.md)). When the checker reaches
one, it calls `parseUnparsedStmt`
([slang-parser.h](../../../../source/slang/slang-parser.h)) with a
`SemanticsVisitor*` so that the parser can call back into the
checker to disambiguate `<` tokens at parse time. Once the body is
parsed, the checker continues normally over the resulting `Stmt`
tree.

This interleaving means there is no clean parse / check boundary
inside function bodies: parsing and checking happen together,
on demand. The deeper rationale is in
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
routes an outermost generic's defaulted and witness arguments through
the constraint solver's fixpoint (`trySolveGenericArguments`) — the
same path used for inferred arguments — passing only the explicitly
supplied ordinary-argument prefix
(`OverloadCandidate::explicitGenericArgCount` in
[slang-check-impl.h](../../../../source/slang/slang-check-impl.h)) as
fixed caller input so a user-written self-reference argument is not
overwritten by a parameter's default. On solver failure the code
falls through to a per-constraint linear pass that re-derives the
failing constraint to emit a precise diagnostic.

A constraint written on an associated type — whether as
`associatedtype A : IBar`, `associatedtype A where A : IBar`, or
`__constraint A : IBar` — is recorded uniformly as a
`GenericTypeConstraintDecl` requirement of the _enclosing interface_
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
When computing the inheritance of an associated-type access such as
`T.D`, the engine surfaces interface-level `__constraint`s of the
interfaces each anchor type conforms to, re-expresses each through the
anchor's conformance witness, and adds the opposite endpoint as a base
of the access. An equality constraint such as `__constraint A == B`
makes `T.A` and `T.B` mutual bases — a _benevolent_ cycle. The engine
tolerates this by skipping a base whose inheritance info is still being
computed (`_isInheritanceInfoBeingComputed`), accumulating the skipped
in-progress ancestor `DeclRef`s through a `HashSet<DeclRef<Decl>>*
ioSkippedIncompleteFacet` out-parameter; a frame whose skipped set is
non-empty after subtracting itself is contextual (partial), is not
cached, and is recomputed by a later root-level query. A bare-`This`
subject on an interface `__constraint` (which would express inheritance
rather than a checked predicate) is rejected during checking in
`visitGenericTypeConstraintDecl`
([slang-check-decl.cpp](../../../../source/slang/slang-check-decl.cpp)).

When a generic cannot be specialized for a call, the failure reason is
captured eagerly but reported lazily. The constraint solver records a
`GenericArgumentInferenceFailure`
([slang-check-impl.h](../../../../source/slang/slang-check-impl.h)) — a
tagged union whose `Kind` selects both the stored payload and the
diagnostic that will eventually be emitted:

| `Kind`                             | Diagnostic | Triggered by                                                                                                   |
| ---------------------------------- | ---------- | -------------------------------------------------------------------------------------------------------------- |
| `VariadicPackCountMismatch`        | `E30433`   | `takesTwo(1, 2, 3)` against `void takesTwo<each T>(expand each T args) where countof(T) == 2`                  |
| `GenericArityMismatch`             | `E30438`   | a call whose argument list cannot be matched to the generic's parameter list at all                            |
| `OrdinaryGenericParamNotInferred`  | `E30439`   | `f(1)` against `void f<T>(int a)` — no argument mentions `T`                                                   |
| `InterfaceConformanceNotSatisfied` | `E38029`   | `pick(s)` against `T pick<T : IFoo>(T a)` where `S` does not implement `IFoo`                                  |
| `GenericConstraintNotSatisfied`    | `E30440`   | `f(1.5f)` against `void f<T>(T a) where T == int`; the fallback for every constraint that is not a conformance |
| `GenericParamUnificationConflict`  | `E30442`   | `two(x, y)` against `void two<T>(T a, T b)` with unrelated `A` and `B`                                         |

Every arm follows its error with a "see declaration of" note carrying
the candidate's rendered signature (`GenericSignatureTried`), and the
`GenericConstraintNotSatisfied` arm adds a second note pointing at the
`where`-clause. Each `Kind` stores only the offending fields (counts,
the parameter `Decl*`, or the substituted sub/super types); the
expensive message formatting is deferred so that speculative candidates
never pay for it. The failure is attached to the `OverloadCandidate`
and turned into a focused diagnostic only if overload resolution
selects that failed candidate — see the `switch` over
`candidate.genericInferenceFailure.kind` in `CompleteOverloadCandidate`
([slang-check-overload.cpp](../../../../source/slang/slang-check-overload.cpp)).
Before this mechanism, every specialization failure collapsed into the
catch-all `Diagnostics::GenericArgumentInferenceFailed`.

### Differentiability as interface conformance

Differentiability is recorded as an interface conformance of the
_function viewed as a type_, rather than as a modifier fact that later
stages re-derive. Consider:

```slang
[Differentiable]
float f(float x) { return x * x; }
```

`[Differentiable]` parses to a `BackwardDifferentiableAttribute` (see
the `attribute_syntax` declarations in
[core.meta.slang](../../../../source/slang/core.meta.slang), line 470 at
`source_commit`). When `SemanticsDeclHeaderVisitor::checkDifferentiableCallableCommon`
([slang-check-decl.cpp](../../../../source/slang/slang-check-decl.cpp),
line 14950) sees that attribute or a `ForwardDifferentiableAttribute`,
it calls `extendContainerDecl` to synthesize
`extension __func_as_type(f) : IForwardDifferentiable<__func_as_type(f)>`
and then `addSynthesizedFunc` to give that extension the `fwd_diff`
member the interface requires, with `kIROp_ForwardDifferentiate` as its
implementation. The synthesized `fwd_diff` is in turn given the same
pair of conformances, which is what makes higher-order differentiation
resolve through ordinary lookup.

The interface types themselves are built by
`getForwardDiffFuncInterfaceType` and
`getBackwardDiffFuncInterfaceType` (lines 10014 and 10020), which pair
the base function type with the `__hasDiffTypeInfo` witness that
`IForwardDifferentiable<FType>` / `IBackwardDifferentiable<FType>`
demand — those interfaces are declared in
[core.meta.slang](../../../../source/slang/core.meta.slang) at lines 720
and 739, and their requirements (`fwd_diff`, the `BwdCallable` /
`MinimalContext` associated types, `apply_bwd`) are what the checker
must supply.

Because the fact now lives in a witness table, asking "is this callee
differentiable?" is a subtype query instead of a modifier lookup:
`isFuncForwardDifferentiable` and `isFuncBackwardDifferentiable` (lines
5467 and 5476) return the `SubtypeWitness*` that
`tryGetSubtypeWitness` produced, replacing the earlier boolean
`doesCalleeHaveFwdDiff` / `doesCalleeHaveBwdDiff` predicates. Handing
back the witness rather than a `bool` matters because the caller needs
that witness to build and specialize the derivative call.

A `[Differentiable]` annotation on an _interface requirement_ is
handled the same way as the associated-type constraints described
above — as a requirement of the enclosing interface rather than
something nested under the member. `_moveInterfaceDifferentiabilityRequirementToInterface`
(line 14863) starts the `GenericTypeConstraintDecl` under the callable
that owns the generic environment its type mentions, then uses
`liftDeclFromGenericContainers` to hoist it into a standalone generic
requirement directly under the interface. The explicit
`__func_extension fwd_diff(foo)(...)` spelling arrives at the same
representation through `_funcExtensionForwardDiff` /
`_funcExtensionBackwardDiff` (lines 15981 and 16016), which rewrite it
into `extension foo : IForwardDifferentiable<foo>` with the user's body
as the `fwd_diff` member.

The full conceptual model (interfaces, witness tables, existential
types) is in
[../../../design/interfaces.md](../../../design/interfaces.md) and
[../../../design/existential-types.md](../../../design/existential-types.md);
this document only points at the implementation.

## Synthesizing implicit code

Some declarations gain members at check time rather than at parse
time: default conformance witnesses, generated comparison /
construction methods, and several built-in conformances. The
_decisions_ about what to synthesize live primarily in
[slang-check-decl.cpp](../../../../source/slang/slang-check-decl.cpp)
— for example `_synthesizeCtorSignature` for default constructors and
the `trySynthesize*RequirementWitness` routines for interface
requirements — while
[slang-ast-synthesis.cpp](../../../../source/slang/slang-ast-synthesis.cpp)
supplies the `ASTSynthesizer` helpers (`emitBinaryExpr`, `emitVarExpr`,
`emitInvokeExpr`, `emitVarDeclStmt`, ...) that build the AST fragments
those routines emit. The checker calls into this machinery whenever it
needs a member that the user did not write but the language guarantees.

## Modifier validation

Modifier-specific checks live in
[slang-check-modifier.cpp](../../../../source/slang/slang-check-modifier.cpp):
which modifiers are allowed on which decls, mutually exclusive
combinations, attribute argument types, and the differences between
Slang and GLSL input. Modifier nodes themselves are defined in
[slang-ast-modifier.h](../../../../source/slang/slang-ast-modifier.h).

_Mutually exclusive_ is decided by `getModifierConflictGroupKind`
(line 1564), which maps a modifier's `ASTNodeType` to the group it
competes in; a second modifier landing in a group already claimed on
the same decl produces `E31202`. Most modifiers are their own group,
so a plain repeat (`static static int g;`) is the common case, but the
groups with more than one member are the ones worth knowing:
`out`, `inout`, `ref` and `borrow` share a group; `static` and
`uniform` share one; and `nointerpolation`, `noperspective`, `linear`,
`sample` and `centroid` share one more.

The dialect axis here is GLSL rather than HLSL. `checkModifier` (line 1936) computes `isGLSLInput` from the `-allow-glsl` option
(`CompilerOptionName::AllowGLSL`) or from a `GLSLModuleModifier` on
the module, and passes it to `isModifierAllowedOnDecl` (line 1675); a
modifier used in a position that predicate rejects is reported as
`E31201`. Only a few entries actually branch on the flag —
`globallycoherent` and `volatile`, for instance, are additionally
allowed on the fields of a global struct when `isGLSLInput` holds.

### Visibility scopes

`public`, `internal` and `private` are checked like any other
modifier, but the scope each one names is not the source file.
`isDeclVisibleFromScope`
([slang-check-expr.cpp](../../../../source/slang/slang-check-expr.cpp),
line 1144) decides the question:

| Modifier   | Visible from                                                       |
| ---------- | ------------------------------------------------------------------ |
| `public`   | any scope, including other modules                                 |
| `internal` | any scope in the declaring module                                  |
| `private`  | the declaring type or namespace, plus extensions of that same type |

`private` is therefore type-scoped, not file-scoped: a free function
in the _same_ file cannot read a `private` member of a `struct`, and
the read is rejected with `E30600`. Writing `private` where there is
no enclosing type for it to scope to — at global scope, or on an
interface requirement — is rejected up front with `E30603`.

### `dyn interface` restrictions

`validateDynInterfaceUsage` and
`validateDynInterfaceUseWithInheritanceDecl`
([slang-check-decl.cpp](../../../../source/slang/slang-check-decl.cpp),
lines 372 and 442) constrain what a `dyn interface` may declare and
what may conform to one. Both are gated on
`allowExperimentalDynamicDispatch` (line 364) and are a no-op unless
the module's language version is 2026 or later (`-std 2026`) _and_
`-enable-experimental-dynamic-dispatch` was **not** passed — so the
same source compiles differently under the default `-std`.

When the gate is open, the interface may not be generic (`E33072`)
and may not declare an associated type (`E33073`), a generic method
(`E33074`), a `[mutating]` method (`E33075`), a `[Differentiable]`
method (`E33076`), or a non-`dyn` base interface (`E33077`). A
conforming type may not acquire the conformance through an `extension`
(`E33078`) and may not be generic (`E33082`), and its fields may be
neither unsized (`E33079`), opaque (`E33080`), nor non-copyable
(`E33081`) — the dynamic representation has to be a copyable,
fixed-size box.

## Shader-specific checks

[slang-check-shader.cpp](../../../../source/slang/slang-check-shader.cpp)
validates entry points: the function's stage attribute, parameter
modifiers (`in`, `out`, `inout` and stage-specific intrinsics), return
type compatibility with the stage, and resource binding rules.
Failures here surface as diagnostics that reference the
`shader("...")` attribute or the entry-point signature.

Two checks worth calling out are scoped specifically to entry-point
validation rather than the general inference walk:

- **Generic-struct capability requirements.**
  `collectGenericStructTypeUses`
  ([slang-check-shader.cpp](../../../../source/slang/slang-check-shader.cpp))
  recurses through an entry point's signature types to find every
  user-defined generic struct (e.g. a `Foo<int>`, including nested
  inside `Optional<...>`, arrays, or `ConstantBuffer<...>`) and
  validates its `[require(...)]` against the target. The general
  capability-inference walk (`SemanticsDeclReferenceVisitor`) records a
  type's requirements only for a `DirectDeclRef`; a generic
  specialization is a `GenericAppDeclRef` and is skipped, so the
  requirement would otherwise be dropped. The check deliberately lives
  here — not in the inference walk — to avoid forcing every library
  function that names such a type to redeclare those capabilities.
  Builtin generic types carrying `MagicTypeModifier`/
  `IntrinsicTypeModifier` already have more specific diagnostics and are
  filtered out (but still recursed _through_). A requirement the target
  cannot provide — `[require(cpp)] struct Foo<T>` named as `Foo<int>`
  in a SPIR-V entry point's signature — is reported against the entry
  point as `E36107`, followed by a `see using of 'Foo'` note.
- **Unspecialized generic entry points.** A generic entry point such as
  `void main<T>(...)` left genuinely unspecialized lowers to an
  `IRGeneric` rather than an `IRFunc` and used to crash at link time.
  `createSpecializedGlobalAndEntryPointsComponentType`
  ([slang-check-shader.cpp](../../../../source/slang/slang-check-shader.cpp))
  now uses `Linkage::isSpecialized` together with the presence of
  specialization-argument strings to decide, and calls
  `diagnoseGenericEntryPoint` (line 3964) — which emits `E38014`
  against the entry-point name — only for the truly-unspecialized
  case.
- **Conflicting depth outputs.** A fragment entry point may write at
  most one depth system value. Because the per-parameter semantic check
  looks at each semantic in isolation, the conflict is detected
  separately: `collectDepthOutputSemantics`
  ([slang-check-shader.cpp](../../../../source/slang/slang-check-shader.cpp),
  line 536 at `source_commit`) walks every `out` / `inout` parameter and
  the return type — unwrapping `Conditional<T>` and array wrappers and
  recursing into struct fields, so a semantic on a field of an
  `out DepthOut a[1]` is still reached — and a collected count above one
  produces `Diagnostics::MultipleDepthOutputSemantics` (`E30705`),
  naming the second contributor as conflicting with the first.
- **System-value semantic type compatibility.** `isSemanticTypeCompatible`
  (line 112) decides whether a declared type may carry a given
  system-value semantic. Two types match when they have the same shape
  (both scalar, or both vectors of equal element count) and their scalar
  element types fall in the same category — integer, floating-point, or
  bool. That admits sign coercions such as `int3` for a `uint3` semantic
  while still rejecting cross-category ones such as
  `float gi : SV_GroupIndex`, and a shape mismatch such as
  `float pos : SV_Position`; both are reported as `E30701`, whose
  message lists the types the semantic will accept.
- **Ignored binding modifiers on entry-point parameters.** Slang
  silently ignores `[[vk::binding(...)]]`, `[[vk::push_constant]]`,
  `register()`, and `packoffset()` in some positions, which misleads
  users either way. Entry-point parameter checking therefore reports
  `Diagnostics::UnhandledModOnEntryPointParameter` (`E38010`, whose
  message names the modifier and the parameter and says the modifier
  will be ignored) for each such
  modifier. Only the `[[vk::binding(...)]]` case is gated — by
  `_allTargetsSupportVkBindingOnEntryPointParameters` (line 1580) over
  all of the linkage's targets and by
  `isVkBindingCompatibleEntryPointParameterType` (line 920) for the
  parameter's own type — so that it fires only where the attribute
  really would be dropped. `[[vk::push_constant]]`, `register()`, and
  `packoffset()` are diagnosed unconditionally when found on an
  entry-point parameter.

## Failure modes

All semantic-checking errors flow through the `DiagnosticSink`
threaded into `SemanticsContext`. Check-level recovery is generally
"continue with a placeholder type" so that one error does not cascade:
unresolved decls become `ErrorType`-typed, and overload resolution
returns a synthetic `errorExpr` rather than aborting. Diagnostics aim
to name the offending source construct: when `ExpectATypeRepr`
([slang-check-type.cpp](../../../../source/slang/slang-check-type.cpp))
finds an expression that does not denote a type, it builds the
`Diagnostics::ExpectedAType` message from the expression's actual type
and, when available, the referenced name.

Several diagnostics in this stage go beyond naming the construct and
point the user at the likely fix:

- **Per-candidate argument mismatch.** When a call matches no overload,
  the diagnostic now lists each candidate signature _and_ the specific
  argument that rejected it. `slang-check-overload.cpp` records the
  offending argument index and the expected/actual types on the
  candidate, then emits a
  `Diagnostics::OverloadCandidateArgumentTypeMismatch` note per
  candidate. Calling `void g(A, int)` / `void g(B, float)` with two
  `float`s reports `E39999` on the call, then an `E40011`
  `candidate: <signature>` note per candidate, each followed by an
  `E40018` note reading `argument 0 does not match: expected 'A', got
'float'`. Candidates are deduplicated by their rendered signature
  string (not by `Decl*`, which would wrongly collapse distinct
  specializations such as `foo<float>` and `foo<int>`); at most ten
  unique candidates are printed and the remainder are summarized by an
  `E40015` "N more overload candidates" note.
- **"Did you mean ...?" on undefined identifiers.** When a name fails to
  resolve, `slang-check-expr.cpp` walks the in-scope candidates and, via
  `StringUtil::calcLevenshteinDistanceCaseInsensitive`, attaches a
  conservative similar-name suggestion to the existing
  `Diagnostics::UndefinedIdentifier` (`E30015`) rather than emitting a
  detached note. `findClosestInScopeName` (line 5216) fixes the budget
  concretely: nothing is suggested for a name under 3 or over 256
  characters, the allowed distance is
  `min(3, max(1, length / 3))` — roughly one edit per three characters,
  floored at one and capped at three — core-module declarations and
  anything the scope could not access are skipped, and a tie between
  two distinct names suppresses the suggestion so the output does not
  depend on scope-walk order. So `myLongVariableNam` suggests
  `myLongVariableName`, while `ac` does not suggest the in-scope `ab`
  and `sqr` does not suggest the core module's `sqrt`.
- **Discarded `[NoDiscard]` results.** `maybeDiagnoseDiscardedNoDiscardResult`
  ([slang-check-stmt.cpp](../../../../source/slang/slang-check-stmt.cpp))
  fires with `E30059` when the result of a call to a
  `[NoDiscard]`-marked function is thrown away — `f();` written as a
  bare expression statement — recursing through
  comma, ternary-select, and short-circuit forms to find the discarded
  sub-expression. A bare discarded constructor call is deliberately
  excluded.

The diagnostic infrastructure is described in
[../cross-cutting/diagnostics.md](../cross-cutting/diagnostics.md).

`checkModule` drives every `Decl` in the translation unit through the
`DeclCheckState` sequence up to `CapabilityChecked`; there is no
separate errored state, so recovery is expressed as diagnostics plus
error types / expressions substituted in place. The AST is then ready
for IR lowering (see [04-ast-to-ir.md](04-ast-to-ir.md)).
