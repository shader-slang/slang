---
gap_intake_report: true
intake_model: claude-opus-5[1m]
intake_at: 2026-08-11T16:28:00Z
target_doc: pipeline/03-semantic-check.md
target_doc_source_commit_before: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_source_commit_after: ec47ea72b6aa5fefc3b36f8a780dbd3ecf5b1f6e
gap_count: 10
actions:
  fixed: 8
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated_to_finding: 2
---

# Gap-intake report for pipeline/03-semantic-check.md

## Summary

Two gaps were escalated: both are compiler defects already tracked in
`docs/generated/tests/_meta/findings/`, and the document was deliberately not
edited for either. The remaining eight were all confirmed in the watched paths
and fixed. Six of them were "this section is a label, not a claim" gaps — the
`GenericArgumentInferenceFailure` kinds became a table with the six diagnostic
codes, the files-and-responsibilities table gained an *Example rejection*
column, the four undocumented shader-check bullets gained their codes, the
three "point the user at the fix" bullets gained theirs plus the concrete
edit-distance rule, the inputs/outputs contract gained a before/after example,
and the two vague modifier-validation phrases were replaced by the actual
mechanisms. The remaining two added the `public`/`internal`/`private` scopes
and the `dyn interface` restriction set with its `-std 2026` gate. The page
grew from 21,404 to 29,107 bytes against a 32,768-byte cap.

Three fixes say more than the gap asked. The files table records that
`slang-check-conformance.cpp` and `slang-check-resolve-val.cpp` emit **no**
diagnostics at all and that `slang-check.cpp`'s only diagnostics concern
loading a downstream compiler — that is the real reason those rows read as
labels. The visibility table adds namespaces and same-type extensions to the
`private` scope, which `isDeclVisibleFromScope` also honours. And the
modifier-validation "dialect" claim was corrected rather than merely
illustrated: the source has no HLSL-vs-Slang switch there; the axis is GLSL,
selected by `-allow-glsl` or a `GLSLModuleModifier`.

## Escalated gaps

- **e71d82030914** (`drift-from-source`, `#failure-modes`) — the document says
  check-level recovery is *generally* "continue with a placeholder type", and
  the source agrees: `checkModule` runs every decl through the `DeclCheckState`
  sequence with no errored state, substituting error types/expressions.
  E99997 is by construction an `InternalError` abort, not a designed recovery
  path, so the compiler — not the page — is wrong. Covered by
  `docs/generated/tests/_meta/findings/check-decl-enum-tag-not-first-internal-error.yaml`.
  No edit made; writing "recovery sometimes aborts" into the page would bless
  the crash.
- **aaff39b7a28b** (`undocumented-behavior`, `#failure-modes`) — atom-granular
  capability reporting is real
  (`SemanticsDeclCapabilityVisitor::diagnoseUndeclaredCapability` loops the
  simplified failed-atom set and calls `diagnoseCapabilityProvenance`,
  `source/slang/slang-check-decl.cpp:21780` and `:21860`), but the observed
  behaviour it produces — eight byte-identical "see definition of 'glslCaller'"
  notes for one error — is the defect, not the documentable rule. Covered by
  `docs/generated/tests/_meta/findings/check-decl-capability-provenance-duplicate-notes.yaml`.
  No edit made.

## Actions

| Gap ID       | Action               | Evidence                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          | Fix summary                                                                                                                                     |
| ------------ | -------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------- |
| eb418aa11258 | fixed                | Emission sites in watched paths: `source/slang/slang-check-overload.cpp:3510` (`NoApplicableOverloadForNameWithArgs`), `:3571` (`OverloadCandidate`), `:3578` (`OverloadCandidateArgumentTypeMismatch`), `:3525` (`maxCandidatesToPrint = 10`), `:3589` (`MoreOverloadCandidates`); `slang-check-expr.cpp:5375` (`UndefinedIdentifier`); `slang-check-stmt.cpp:781` (`DiscardedNoDiscardResult`) with the constructor exclusion at `:775`. Edit budget read directly from `slang-check-expr.cpp:5216,5236,5238` and the tie rule at `:5299-5308`. Codes from `source/slang/slang-diagnostics.lua:1231,3973,4009,4023,4030,1400`. Printed forms verified by the bundle tests `overload-candidate-argument-mismatch-note.slang`, `undefined-identifier-did-you-mean.slang`, `undefined-identifier-no-suggestion-short-name.slang`, `undefined-identifier-no-builtin-suggestion.slang`, `nodiscard-bare-statement-rejected.slang`. | added `E39999`/`E40011`/`E40018`/`E40015`, `E30015`, `E30059`, the `min(3, max(1, length/3))` edit budget with its 3/256-character bounds and tie rule, and a one-line example per bullet |
| e71d82030914 | escalated-to-finding | Source agrees with the document: `checkModule` has no errored `DeclCheckState`, so recovery is error-type substitution, and `E99997` is an `InternalError` abort rather than any documented path. Existing finding: `docs/generated/tests/_meta/findings/check-decl-enum-tag-not-first-internal-error.yaml`. Document not edited.                                                                                                                                                                                                                                                                                        | —                                                                                                                                               |
| aaff39b7a28b | escalated-to-finding | Per-atom loop confirmed in watched `source/slang/slang-check-decl.cpp:21780,21860` calling `diagnoseCapabilityProvenance` (`:21665`), which ends in `SeeDefinitionOf` per call. The repetition is the defect, not the rule; existing finding: `docs/generated/tests/_meta/findings/check-decl-capability-provenance-duplicate-notes.yaml`. Document not edited.                                                                                                                                                                                          | —                                                                                                                                               |
| acc5a39fd9e8 | fixed                | One diagnostic confirmed per watched file: `slang-check-decl.cpp:13849` (`Redeclaration`, 30200), `slang-check-expr.cpp:3819` (`AssignNonLvalue`, 30011), `slang-check-stmt.cpp:206` (`BreakOutsideLoop`, 30003), `slang-check-type.cpp:155` (`ExpectedAType`, 30060), `slang-check-overload.cpp:3578` (40018), `slang-check-conversion.cpp:1475` (`TooManyInitializers`, 30523), `slang-check-inheritance.cpp:262` (`CircularityInExtension`, 30815), `slang-check-modifier.cpp:2527` (`DuplicateModifier`, 31202), `slang-check-shader.cpp:1772` (`EntryPointHasNoStage`, 38007). `slang-check-conformance.cpp` has zero `diagnose` calls; `slang-check.cpp`'s only three are `DxilNotFound` / `FailedToLoadDownstreamCompiler` / `NoteFailedToLoadDynamicLibrary`. Codes from `slang-diagnostics.lua`. | added an *Example rejection* column plus a lead-in paragraph explaining the three rows that have no diagnostic of their own                      |
| 112a152074eb | fixed                | The six arms and their diagnostics read from watched `source/slang/slang-check-overload.cpp:1510-1612` (`VariadicPackCountDoesNotMatch`, `GenericSpecializationArityMismatch`, `GenericParameterCouldNotBeInferred`, `GenericArgumentDoesNotSatisfyConstraint`, `GenericParameterUnificationConflict`, `TypeArgumentDoesNotConformToInterface`, each followed by `GenericSignatureTried`); `Kind` list at `slang-check-impl.h:219-228`. Codes 30433/30438/30439/30440/30442/38029 from `slang-diagnostics.lua:1994,2029,2036,2043,2064,4417`. Triggering shapes are the verified bodies of the six `generic-inference-*.slang` bundle tests. | replaced the prose `Kind` list with a `Kind` / diagnostic / trigger table and named the two accompanying notes                                    |
| b6be39692f6d | fixed                | Snippet shape lifted from the bundle test `synthesized-interface-default-witness.slang` (interface with a default `twice()` body, conformer supplying only `base()`, generic `call<T : IFoo>` dispatching through the witness). The synthesis path is `trySynthesize*RequirementWitness` / `findWitnessForInterfaceRequirement` in watched `source/slang/slang-check-decl.cpp`; the body handoff is `parseUnparsedStmt` as already documented in this page.                                                                                                | added a five-line snippet and a clause per output-contract item saying what checking changed about it                                            |
| 6c2f3e92f11c | fixed                | Exclusive groups read from watched `source/slang/slang-check-modifier.cpp:1564-1660` (`out`/`inout`/`ref`/`borrow` → `OutModifier`; `static`/`uniform` → `HLSLStaticModifier`; the five interpolation modifiers → one group) and the report site at `:2516-2532` (`DuplicateModifier`, 31202). The dialect gate is `:1936-1965`: `isGLSLInput` from `CompilerOptionName::AllowGLSL` or `GLSLModuleModifier`, fed to `isModifierAllowedOnDecl` (`:1675`), whose `GloballyCoherentModifier` / `HLSLVolatileModifier` arm (`:1730-1740`) is one of the few that branches on it; rejection is `ModifierNotAllowed` (31201). CLI spelling `-allow-glsl` at `source/slang/slang-options.cpp:1209`. Verified by the bundle test `modifier-duplicate-rejected.slang` (`E31202`). One consolidated edit with gaps 24e48fc6695e and edc4f026c950. | named the concrete conflict groups with `E31202`, and corrected "HLSL-vs-Slang dialect" to the actual `-allow-glsl` gate with `E31201`           |
| 24e48fc6695e | fixed                | `isDeclVisibleFromScope` in watched `source/slang/slang-check-expr.cpp:1144-1176` (public → always; internal → same `getModuleDecl`; private → nearest enclosing `AggTypeDeclBase`/`NamespaceDeclBase` on the scope chain, plus the same-target-type extension walk at `:1178-1210`); reported as `DeclIsNotVisible` at `:1296` and `slang-check-overload.cpp:279`. `InvalidUseOfPrivateVisibility` for the no-enclosing-type case at `slang-check-modifier.cpp:2188-2216`. Codes 30600/30603 from `slang-diagnostics.lua:2355,2370`; printed form verified by `private-member-not-accessible.slang`. Part of the one consolidated Modifier-validation edit (with 6c2f3e92f11c, edc4f026c950). | added a "Visibility scopes" subsection with the three-row scope table, the type-scoped-not-file-scoped statement, `E30600` and `E30603`          |
| edc4f026c950 | fixed                | `validateDynInterfaceUsage` (watched `source/slang/slang-check-decl.cpp:372`) and `validateDynInterfaceUseWithInheritanceDecl` (`:442`), both gated on `allowExperimentalDynamicDispatch` (`:364`) = `EnableExperimentalDynamicDispatch` **or not** `isSlang2026OrLater` (`:352`). Arms at `:385-437` and `:473-499` give 33072-33077 and 33078-33082; codes from `slang-diagnostics.lua:1657,1664,1720` and neighbours. Printed forms verified by `dyn-interface-member-restrictions.slang`, `dyn-interface-inheritance-restrictions.slang`, `dyn-interface-conformance-restrictions.slang`. CLI spelling `-enable-experimental-dynamic-dispatch` at `source/slang/slang-options.cpp:1215`. Part of the one consolidated Modifier-validation edit (with 6c2f3e92f11c, 24e48fc6695e). | added a "`dyn interface` restrictions" subsection with the eleven codes and the `-std 2026` / `-enable-experimental-dynamic-dispatch` gate       |
| 5c70edc29364 | fixed                | All four in watched `source/slang/slang-check-shader.cpp`: `EntryPointUsesUnavailableCapability` at `:2437` with the `SeeUsingOf` note at `:2463`; `diagnoseGenericEntryPoint` at `:3964` emitting `EntryPointCannotBeGeneric`; `SystemValueSemanticInvalidType` at `:325` behind `isSemanticTypeCompatible` (`:112`); `UnhandledModOnEntryPointParameter` at `:2341-2362`. Codes 36107/38014/30701/38010/30705 from `slang-diagnostics.lua:2451,4275,3683,4254,3711`. Printed forms verified by `entry-point-generic-struct-require-rejected.slang`, `entry-point-unspecialized-generic-rejected.slang`, `semantic-type-cross-category-rejected.slang`, `semantic-type-shape-mismatch-rejected.slang`, `entry-point-param-{vk-binding,register,push-constant}-modifier-warns.slang`. | added `E36107`, `E38014`, `E30701`, `E38010` and `E30705` to the five bullets, each with a one-line recognisable trigger                         |
