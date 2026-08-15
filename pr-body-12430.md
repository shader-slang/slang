> **Design spike (draft), requested by @tangent-vector in [#12430](https://github.com/shader-slang/slang/issues/12430#issuecomment-5299401967).** This is not a merge-ready fix. It adds the AST-level `ExistentialType` you proposed and validates whether it rules out the reproducers, with a measured blast-radius report. Findings — including where it does *and* does not help, and the cost of the full rule — are in the Process report below and posted on the issue.

## Motivation

Reaching a `static` interface requirement through an existential (interface) type crashes the compiler with an internal error instead of producing a diagnostic (issue #12430). The clearest case is **Reproducer 2**:

```slang
interface IV { associatedtype Assoc; static Assoc makeIt(); }
struct V : IV { typealias Assoc = float; static float makeIt() { return 1.0; } }
__generic<T : IV> T.Assoc callStatic() { return T.makeIt(); }
[numthreads(1,1,1)] void computeMain(uint3 t : SV_DispatchThreadID) { var r = callStatic<IV>(); }
```

`callStatic<IV>()` passes the interface `IV` itself as the argument for a generic parameter constrained `T : IV`. Today this is accepted and later asserts in AST-to-IR lowering (`slang-lower-to-ir.cpp` `SLANG_RELEASE_ASSERT(irWitnessTable)`). As @tangent-vector observed on the issue, the root problem is that the compiler has no representation to distinguish *the interface `IV`* from *the existential type of `IV` (`dyn IV`)*, and **the existential type of an interface does not conform to that same interface** — so `callStatic<IV>()` should be rejected at type-check.

The compiler already declares a clean diagnostic for exactly the "type argument doesn't conform" case (`E38029`); it just never fired here because `IV` was (incorrectly) found to conform to `IV`.

## Proposed solution

Introduce the AST-level `ExistentialType` from @tangent-vector's proposal and make the type system reject the ill-formed use:

- A new `ExistentialType(interfaceOrConjunction) : Type` stores a `Type*` for the interface/conjunction it is the existential of. It prints as `dyn IFoo`.
- In a data-type context (here: an explicit generic **type argument**), an interface `DeclRefType` implicitly forms an `ExistentialType`.
- In the subtype/conformance check, `ExistentialType(IY)` does **not** conform to `IY`, and a concrete `X` is **not** a subtype of `ExistentialType(IY)` (reflexivity `ExistentialType(IY) : ExistentialType(IY)` is preserved).
- For lowering, `ExistentialType(IFoo)` lowers to the **same** IR as `IFoo` (per the "minimize churn" guidance), since well-formed existentials only survive to lowering after checking.

With these, `callStatic<IV>()`'s explicit argument becomes `dyn IV`; the constraint `T : IV` is checked as `dyn IV : IV`, which now fails, and the existing `E38029` fires:

```
error[E38029]: type argument 'dyn IV' does not conform to the required interface 'IV'
```

**Scope note (this is a spike).** The full design — forming `ExistentialType` in *every* proper-type context and dropping self-conformance everywhere — is a large, systemic change (measured below). To validate the hypothesis with minimal churn, this PR forms the existential only where a generic **type parameter carries an interface conformance constraint** (`T : IFoo`), which is exactly the ill-formed shape. Unconstrained type parameters used as existential *container* elements (`Optional<IFoo>`, `IFoo[]`) are left untouched, because today's dynamic-dispatch model legitimately relies on an interface being usable there.

## Change summary

| File | Change |
| --- | --- |
| `slang-ast-type.h` / `.cpp` | New `ExistentialType : public Type` (one `Type` operand; prints `dyn IFoo`; canonicalizes to a distinct `ExistentialType`, never to the wrapped interface). |
| `slang-ast-builder.h` / `.cpp` | `getExistentialType(Type*)` factory. |
| `slang-check-conformance.cpp` | `checkAndConstructSubtypeWitness`: for non-reflexive relations, an `ExistentialType` on either side produces no subtype witness (encodes `dyn IY ⊄ IY` and `X ⊄ dyn IY`); reflexivity `dyn IY : dyn IY` is preserved. New `maybeFormExistentialType(Type*)` helper. |
| `slang-check-overload.cpp` | `CheckGenericArguments`: after coercing an explicit type argument, form the existential when the parameter carries a *required* interface conformance constraint (`genericTypeParamHasConformanceConstraint`; equality and `where optional` constraints are excluded). |
| `slang-lower-to-ir.cpp` | `visitExistentialType` → lower to the wrapped interface's IR. |
| `slang-check-impl.h` | Declarations. |
| `tests/.../diagnose-existential-static-requirement.slang`, `diagnose-existential-inherited-static-requirement.slang` | New regression tests: Reproducer 2 and the generic (inherited-base) form of Reproducer 1 now yield `E38029` (were ICEs). |
| `tests/.../diagnose-optional-constraint-existential-arg.slang` | New regression: an interface passed to a `where optional T : IFoo` parameter is NOT wrapped, still rejected via the pre-existing `E33180` — guards the optional-constraint carve-out. |
| `tests/.../diagnose-explicit-specialize-with-interface.slang`, `diagnose-explicit-specialize-method-with-interface.slang` | Updated expectations: these existing explicit-specialize rejections now report `E38029` instead of `E33180` (still correct rejections). |

## Concepts and vocabulary

- **Existential type / `dyn IFoo`** — the type of a value known to conform to `IFoo` but whose concrete type is erased. Distinct from the interface `IFoo` itself. This PR adds the AST node for it; the pre-existing `ExtractExistentialType` is the *dual* (the concrete-but-unknown type obtained when an existential is *opened*), not this.
- **Facet / inheritance info** — `getInheritanceInfo(T).facets` is the flattened list of supertypes `T` transitively conforms to; `checkAndConstructSubtypeWitness` scans it to find a conformance witness. For a bare interface used as a subtype, this list currently contains a self-facet, which is why `isSubtype(IV, IV)` wrongly succeeds today.
- **Conformance constraint vs. container element** — `T : IFoo` (parameter must conform to `IFoo`) vs. `Optional<IFoo>` (an interface used as an existential element). This PR forms the existential only in the former.

## Process report

**`ExistentialType` node (`slang-ast-type.h/.cpp`).** A single-operand `Type` wrapper modeled on `ModifiedType`/`TypeType`. The one subtle point is `_createCanonicalTypeOverride`: it canonicalizes the wrapped interface but returns an `ExistentialType`, never the bare interface. This is required because `Val::equals` compares canonicalized (`resolve()`d) pointers; if `dyn IV` canonicalized to `IV`, the facet scan's `facetType->equals(superType)` would match and self-conformance would return — defeating the whole fix. The node is a genuinely new representation (the shape it distinguishes did not exist before), so this is not a second spelling of an existing type.

**Non-conformance rule (`checkAndConstructSubtypeWitness`).** For non-reflexive relations, the guard returns no witness when either side is an `ExistentialType` (equal existential types fall through to the self-facet path, preserving reflexivity). This is the front-end rejection @tangent-vector identified as the right layer — upstream of both IR throw sites, so it does not touch the typeflow-specialize `else` arm or any `SLANG_RELEASE_ASSERT` (the approach that was rejected in #10578). Input-shape check: the `ExistentialType` reaching here is a well-formed, intentionally-produced node (from `maybeFormExistentialType`), not an accidental shape — so handling it here is correct, and the rule it encodes (a box does not conform to its own interface) is the intended semantics, not a workaround.

**Existential formation (`slang-check-overload.cpp`).** Formed at the explicit-generic-type-argument site, gated on `genericTypeParamHasConformanceConstraint`. The gate is the load-bearing scoping decision: forming the existential for *unconstrained* parameters broke 68 test base files across four directories (a lower-bound sample; see below), because `Optional<IFoo>`/`IFoo[]` become `Optional<dyn IFoo>` and `dyn IFoo` is not yet member-transparent in the checker. Restricting to conformance-constrained parameters isolates the ill-formed case (`callStatic<IV>()`) from valid container uses.

**Lowering (`slang-lower-to-ir.cpp`).** `visitExistentialType` delegates to the wrapped interface's lowering, per the "same IR, minimize churn" guidance. Well-formed existentials only reach lowering after checking, so no new IR representation is needed for this pass.

### What this spike establishes (validation results, measured on a from-scratch build at base `b4853080d174f36bc9e7b2d8d8e9fb71d6fc38db`)

- ✅ **Reproducer 2 is ruled out at type-check** — the headline result @tangent-vector asked for. `callStatic<IV>()` now emits `E38029` instead of the `irWitnessTable` internal error.
- ✅ **The generic form of Reproducer 1** (`makeZero<IV>()` / `getZero<IV>()`) is also ruled out — same clean `E38029`.
- ❌ **The bare form of Reproducer 1** (`IV.dzero()`) is **not** fixed by this change. There `IV` is a static-member-access base, not a generic type argument, so it never reaches the formation site; it still hits an internal error (measured at this base SHA: `slang-ir.cpp` `SLANG_RELEASE_ASSERT(witnessTableVal && witnessTableVal->getOp() != kIROp_StructKey)`, an `E99997`). This spike did not investigate or implement a fix for that path — a follow-up would need to determine where and how an interface used as a static-member-access base should be rejected (the exact site and whether existential formation there is sufficient are unvalidated hypotheses). (This matches @tangent-vector's own hedge that this may not be the entirety of the problem.)
- Related **#10892** reproducer is unchanged by this PR (different producer — `collect-global-uniforms` on a global param), confirmed as a reciprocal regression check.

### Cost of the rule (blast radius)

The full "form `dyn IFoo` in all proper-type contexts, drop self-conformance everywhere" rule is **not** low-churn. As a *lower-bound sample*: forming the existential at all explicit generic-argument sites (not the whole design, and only over four test directories) failed **68** regression base files, because much of Slang's existential/dynamic-dispatch machinery relies on an interface being usable as a generic argument. This is directional evidence for @tangent-vector's expectation that the systematic fix is large — not a precise total for the all-contexts rollout. Narrowing to conformance-constrained parameters removes that collateral (generics 251/251, interfaces 75/75, autodiff 900/900, dynamic-dispatch 695/695 — all pass). Within those four measured suites, the only existing-test diagnostic change is on two tests that were *already errors*, whose code shifts `E33180` ("specializing with an existential type is not allowed") → `E38029` ("type argument doesn't conform") — arguably more accurate under this model, but a maintainer decision on which surface diagnostic to keep. The constrained-argument subset is the low-churn slice that validates the approach.

Closes #12430 is intentionally **not** asserted — this spike rules out Reproducer 2 but leaves the bare Reproducer 1 form and the broader rollout open.
