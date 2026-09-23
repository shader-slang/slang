---
gap_intake_report: true
intake_model: claude-opus-5[1m]
intake_at: 2026-08-12T06:38:25Z
target_doc: name-resolution/overload-resolution.md
target_doc_source_commit_before: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_source_commit_after: 67149d1e03ebf1d4645ddd224ff4647a8ea5db53
gap_count: 8
actions:
  fixed: 8
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated_to_finding: 0
---

# Gap-intake report for name-resolution/overload-resolution.md

## Summary

This is a re-run of the intake for this page, revisiting the single gap
the first pass left `deferred` (`187da073e7f8`, the missing
`AmbiguousConversion` example). The first pass was told no `slangc` was
available and could not construct a verified tie; a native
`build-arm64/Debug/bin/slangc` at the current `HEAD` was used this
time, and `tests/bugs/gh-7856.slang` was minimized against it down to
two lines. That gap moves `deferred` -> `fixed`; the seven other
verdicts are carried forward unchanged from the previous report. Final
breakdown: eight `fixed`, nothing rejected, deferred, or escalated.

## Actions

| Gap ID | Action | Evidence | Fix summary |
| --- | --- | --- | --- |
| 187da073e7f8 | fixed | Confirmed by running the compiler and in the watched paths. `SLANG_ASSERT=release-assert-only ./build-arm64/Debug/bin/slangc -target hlsl x.slang` on the two-line file `struct T { __init(int v) {} __init(uint v) {} }` / `void f(float x) { T t = T(x); }` prints `error[E30080]: ambiguous conversion ... more than one conversion exists from 'float' to 'T'` followed by a `see declaration of 'T.init'` note for each `__init`. This is `tests/bugs/gh-7856.slang` reduced: the same run on that file names its `__init(int64_t)` / `__init(uint64_t)` pair. Why it needs an explicit site: `source/slang/slang-check-conversion.cpp:2579` sets `disallowNestedConversions = (site != CoercionSite::ExplicitCoercion)`, and `source/slang/slang-check-overload.cpp:898-906` turns that flag into an exact-type requirement, so no two distinct parameter types can both apply implicitly; a single-argument call on a type name reaches `_coerce` with `ExplicitCoercion` at `source/slang/slang-check-overload.cpp:3443`. Checked against the compiler: `void f(int x) { T t = T(x); }` compiles clean (exact match costs 0), `void f(double x)` is ambiguous, and `T t = x;` reports `error[E30019]: type mismatch in expression` rather than an ambiguity. The shared cost level is `kConversionCost_GeneralConversion` (`source/slang/slang-ast-support-types.h:162`): `float` and the integer types differ in `conversionKind`, so both pairs fall through every special case in `getBaseTypeConversionCost` to its final `else` (`source/slang-core-module/slang-embedded-core-module-source.cpp:258`, outside watched paths, which is why the doc names only the constant). | added the minimal two-`__init` tie under Conversion costs, with the explicit-vs-implicit site rule that makes it the smallest possible one |
| d326f38b2c3a | fixed | `source/slang/core.meta.slang:2806-2826` — both `(vector<T,2>, T)` and `(T, vector<T,2>)` `__init`s are still declared at `source_commit`, carrying `[deprecated]` and `[RemovedSince(2026, ...)]`; `source/slang/core.meta.slang:4907-4930` documents `[RemovedSince]` as an error at that language version or higher, with the `[deprecated]` message ignored once it fires. Default language version is `SLANG_LANGUAGE_VERSION_LEGACY` (`include/slang.h:5772`, outside watched paths, so the doc says "earlier than 2026" rather than citing it). | stated the current status of the two initializer forms: still declared, deprecation warning under the default language version, error under 2026+ |
| 1444b2f734a0 | fixed | `source/slang/slang-check-impl.h:270-283` — the `GenericConstraintNotSatisfied` payload comment states it is "the general fallback for every constraint kind handled by the witness solver other than conformance (today: equality `where T == X`, type coercion `where U(T)`, non-empty pack `where nonempty(P)`, ...)". Confirmed at the recording sites in `source/slang/slang-check-constraint.cpp:1709-1717` and `:2999-3007`. The gap's premise (unreachable) is wrong; it is unreachable only for conformance constraints. | annotated the `GenericConstraintNotSatisfied` row with which constraint kinds reach it and which are captured as `InterfaceConformanceNotSatisfied` instead |
| b81d8b1c1c82 | fixed | Source agrees with the observation, so the doc was wrong. `source/slang/slang-check-overload.cpp:1430-1438` runs arity before types; `:158-166` gives a `Flavor::Generic` candidate `allowed` = declared parameter count but forces `required = 0`; `:199` emits `Diagnostics::TooManyArguments`. `TryCheckGenericOverloadCandidateTypes` is reached only through `TryCheckOverloadCandidateTypes` (`:835-837`), so its `providedCount > expectedCount` branch (`:406-425`) is dead for over-long lists; the under-filled branch survives step 1 and fires when `allowPartialGenericApp` is false (`:316-320`). | corrected step 3: named `TooManyArguments` for the too-long case and stated the condition under which `GenericArgumentListArityMismatch` actually fires |
| 5da5b747b6e5 | fixed | `source/slang/slang-check-overload.cpp:3140-3156` — the `ParamDecl` arm calls `AddFuncExprOverloadCandidate` only when the parameter's declared type is a `FuncType` (`:3150`). The surface spelling is `functype(...) -> ...`, used in a watched path at `source/slang/hlsl.meta.slang:28642` (`Reduce...(functype(T, T) -> T combineOp)`) and `:26298-26332`. | added the `functype(T, T) -> T` parameter spelling and the `FuncType` precondition to the `Flavor::Expr` bullet |
| 1a7efc520b52 | fixed | Source agrees with the observation. `source/slang/slang-check-expr.cpp:5124` checks an operator's `functionExpr` with `CheckTerm` (ordinary scope lookup); the only member lookups on an operand/callee type are `operator()` at `:5126-5145` and `__subscript` at `:3646-3654`. No argument-dependent lookup exists in `source/`. A non-static member form also cannot match: `TryCheckOverloadCandidateArity` (`source/slang/slang-check-overload.cpp:145-183`) counts only declared parameters, and `this` is not a call argument. The free-function form is confirmed working by the bundle test `operator-overload-on-user-struct-uses-general-path.slang`. Core-module member operators are exactly `IFunc`/`IMutatingFunc::operator()` (`source/slang/core.meta.slang:2027-2041`) and `__subscript`. | replaced the "found as an ordinary member" / implicit-`this` claim with the scope-lookup rule plus the two operators that really are member-resolved |
| 0e8404b8657f | fixed | `source/slang/slang-check-overload.cpp:1929-1942` — the free-form test is `isDeclRefTypeOf<GenericTypeParamDeclBase>(ext->targetType)`, and the source comment right above it writes the spelling as `extension<T:IFoo> T`. The doc had dropped the constraint, giving the unconstrained `extension<T> T` that the checker rejects. | corrected the free-form-extension spelling in comparator step 3 to `extension<T : IFoo> T` and defined it as an extension whose target type is a generic type parameter |
| c8cbcc3a5ffd | fixed | `__prefer` does not occur anywhere in `source/`. The real modifier is `OverloadRankAttribute`, read at `source/slang/slang-check-overload.cpp:2222-2228` and compared at `:2433-2436` (higher rank wins; absent means 0); its surface form is registered as `attribute_syntax [OverloadRank]` at `source/slang/core.meta.slang:4783`, documented `/// @internal`, and its only users in the tree are `core.meta.slang` and `hlsl.meta.slang`. | replaced `__prefer` with `[OverloadRank(N)]` in comparator step 8 and noted its `@internal` core-module role and default rank |
