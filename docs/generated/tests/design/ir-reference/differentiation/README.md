---
generated: true
model: claude-opus-5[1m]
generated_at: 2026-08-13T00:00:00+00:00
source_commit: c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
watched_paths_digest: 222f123db9618a770d0c176108af6e0d45268a23ffc2b9fb944eef03bbe467fa
source_doc: docs/generated/design/ir-reference/differentiation.md
source_doc_digest: 4897cc418e438c54c148715c0567f3267618967d3bceb61cce00c1f4ef0c04ea
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Tests for design/ir-reference/differentiation

## Intent

This bundle is the per-opcode regression reference for the IR
differentiation family catalogued in
[`docs/generated/design/ir-reference/differentiation.md`](../../../../design/ir-reference/differentiation.md).
Each test takes one catalog row (or one paragraph of the "Notable
opcodes" prose), finds the ordinary Slang surface the doc names as that
row's origin, and asserts the opcode, its operand order and its result
type in the `LOWER-TO-IR` section of a `-dump-ir` dump. The three
producer families the doc distinguishes are all exercised: core-module
intrinsics (`DifferentialPair<T>` construction and projection,
`diffPair`, `detach`), semantic-checking synthesis (the members created
for `[Differentiable]`, `[TreatAsDifferentiable]`,
`[HasTrivialForwardDerivative]`, `[BackwardDerivative(...)]`,
`[PrimalSubstitute(...)]` and `__func_extension __apply`), and the
built-in requirement keys lowering attaches for the recognized
`IDifferentiable` family.

The observation form is
`-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute`,
with every CHECK anchored at a user-named symbol (`func %main`,
`func %f`, a captured `let` value, or the `### LOWER-TO-IR:` /
`###` section boundaries) so the very large autodiff preamble cannot
satisfy a pattern by accident. Three classes of claim are observed
outside that form. Claims about what does _not_ reach generated code —
the doc's "No opcode in this family produces target code",
`processPairTypes`, and `removeDetachInsts` — fan out one emission
directive per text-emit target (`hlsl`, `glsl`, `spirv-asm`, `metal`,
`wgsl`, `cuda`, `cpp`) paired with a `COMPARE_COMPUTE -cpu` run on the
values themselves. Claims about the _generated names_ the reverse- and
forward-mode passes build (`s_bwdProp_`, `s_bwdCallableCtx_`, `s_fwd_`)
are emission-only and fan out the same way, since the name exists only
in target text. Claims about a rejected surface or a compiler report
(`__func_extension` without `-experimental-feature`, the checkpoint
report behind `-report-checkpoint-intermediates`) are observed as
diagnostics. Opcodes the doc marks as having no AST origin, or no
producer at all at `source_commit`, are recorded under
`## Untested claims` rather than probed through internal pass dumps.

## Claims

Enumerated per [`_claims.md` §1](../../../_meta/prompts/_claims.md), grouped by
the doc's own headings. This page is a per-opcode reference written for a
compiler engineer, so a large fraction of it describes internal compiler
structure — Lua group membership, C++ wrapper generation, which pass constructs
which inst — that no Slang program can distinguish; those claims are enumerated
here too but are classified `internal-source-fact` or `implementation-detail`
in `## Untested claims` rather than given tests.

### Source

1. The core module is the origin of every differential-pair opcode and of `detachDerivative`.
2. `DifferentialPair<T>.__init` declares `__intrinsic_op($(kIROp_MakeDifferentialPair))` and its `p` / `v` / `d` properties declare `DifferentialPairGetPrimal` / `DifferentialPairGetDifferential`.
3. `DifferentialPtrPair<T>` declares the corresponding pointer-flavored four.
4. The free function `diffPair(primal, diff)` is a second spelling of `MakeDiffPair`.
5. `detach<T>(T x)` is the origin of `detachDerivative`.
6. Semantic checking is the origin of most `TranslateBase` opcodes, via the `irOp` field on `SynthesizedFuncDecl`, and all of them go through one lowering site.
7. The autodiff passes themselves produce the remainder, resolved and memoized by `TranslationContext::maybeTranslateInst`.
8. No opcode in this family produces target code.
9. `builtinRequirementKey` is hoistable so it may survive specialization as an unreferenced global, and `ensureGlobalInst` skips it explicitly as metadata that produces no code.
10. A `‡` in the C++ wrapper column means the `IRFoo` struct is hand-written rather than generated from the Lua entry, and a `†` in the Operands column means the entry declares only `min_operands` so the wrapper carries no named accessors.

### Family hierarchy

11. `MakeDifferentialPairBase` has the leaves `MakeDiffPair` and `MakeDiffRefPair`, `DifferentialPairGetDifferentialBase` has `GetDifferential` and `GetDifferentialPtr`, and `DifferentialPairGetPrimalBase` has `GetPrimal` and `GetPrimalRef`.
12. `TranslateBase` is the only abstract group with a large membership, and the three pair groups exist so that a consumer can match both the value and pointer flavors at once.

### Differential-pair construction

13. `MakeDiffPair` takes `primal, differential` and bundles a value-typed primal with its differential.
14. `MakeDiffRefPair` takes `primal, differential` and bundles a pointer-typed primal with its differential pointer.

### Differential-pair projection

15. `GetDifferential` takes `pair` and reads the differential component of a `DifferentialPair`.
16. `GetDifferentialPtr` reads the differential pointer of a `DifferentialPtrPair`.
17. `GetPrimal` takes `pair` and reads the primal component of a `DifferentialPair`.
18. `GetPrimalRef` takes `ptrPair` and reads the primal pointer of a `DifferentialPtrPair`.
19. The four operand spellings are inconsistent for one logical role, but all four are the pair being projected and all four are reached through `getBase()` in practice.

### Differentiation operators

20. Everything in this section is a child of the hoistable `TranslateBase` group, so identical translation requests dedupe to a single IR value before the translation pass ever sees them.
21. The dedupe is keyed on the request itself — opcode plus operands — so it is the base function that decides identity, not the call site.
22. The worked example's two `__fwd_diff(f)` expressions lower to one `let %fwd = ForwardDifferentiate(%f)` that both `call` insts share, however many times `__fwd_diff(f)` is written and whatever arguments each call passes.
23. Every entry declares only `min_operands`, except `ForwardDifferentiate` which also names `baseFn`.

### Forward-mode

24. `ForwardDifferentiate` (operand `baseFn`, hoistable) asks for the forward-mode JVP derivative of a function value, from `ForwardDifferentiateExpr` (`__fwd_diff(...)`) or `ForwardDifferentiateVal`.
25. `TrivialForwardDifferentiate` asks for a derivative that runs the primal and returns zero output differentials, ignoring incoming tangents.
26. `TrivialForwardDifferentiate`'s AST origin is the `SynthesizedFuncDecl` `fwd_diff` created for `[TreatAsDifferentiable]` / `[HasTrivialForwardDerivative]`.
27. `ForwardDifferentiatePropagate` has no AST origin and is emitted by the unzip pass as the forward-mode propagate function used while unzipping a reverse-mode body.

### Reverse-mode

28. `BackwardDifferentiate` asks for the whole reverse-mode bundle for a function value.
29. `BackwardDifferentiatePrimal` is the primal-only phase that computes and returns the values the propagate phase will need, from the `apply_bwd` `SynthesizedFuncDecl`.
30. `BackwardDifferentiatePropagate` is the propagate phase that consumes the recorded context and an output adjoint and produces input adjoints.
31. `BackwardRemat` is the rematerialization phase that recomputes primal values from the minimal context instead of reading a full checkpoint.
32. `TrivialBackwardDifferentiate` is the trivial-adjoint counterpart of `BackwardDifferentiate` and has no AST origin.
33. `TrivialBackwardDifferentiatePrimal` is the trivial primal phase, from a `SynthesizedFuncDecl` created for `[TreatAsDifferentiable]`.
34. `TrivialBackwardDifferentiatePropagate` is the trivial propagate phase.
35. `TrivialBackwardRemat` is the trivial remat phase, from the `remat` `SynthesizedFuncDecl` created for `[TreatAsDifferentiable]`.
36. None of the reverse-mode opcodes reaches target code, but the functions and types the reverse-mode passes build from them do, under generated names a reader will meet in emitted HLSL or CUDA.
37. The propagate function gets the prefix `s_bwdProp_` and the full intermediate-context struct the prefix `s_bwdCallableCtx_`, so `f` yields `s_bwdProp_f` alongside a `s_bwdCallableCtx_f` struct carrying the hoisted primal state.
38. Forward mode is the same shape one prefix over: the forward derivative of `<orig>` is built as `s_fwd_<orig>`.

### Legacy bridge

39. The translation pass resolves the root `BackwardFromLegacyBwdDiffFunc` and the three projection opcodes from a shared `(targetFunc, legacyBwdDiffFunc)` operand pair, each projection taking one element out of the resolved five-tuple.
40. `LegacyBackwardDifferentiate` does not follow that shape: it carries three functions — `apply_bwd`, `remat`, and propagate — read as operands 0, 1 and 2.
41. `LegacyBackwardDifferentiate` is the old-style combined reverse-mode request, from a `SynthesizedFuncDecl`.
42. `BackwardFromLegacyBwdDiffFunc` has no AST origin and is built by the translation pass to root the legacy five-tuple.
43. `BackwardPrimalFromLegacyBwdDiffFunc` projects the primal phase out of a legacy combined function.
44. `BackwardRematFromLegacyBwdDiffFunc` projects the remat phase.
45. `BackwardPropagateFromLegacyBwdDiffFunc` projects the propagate phase.

### Synthesized derivative witnesses

46. `FunctionCopy` names an existing function as the body of a synthesized derivative member and is translated to its own operand.
47. `SynthesizedForwardDerivativeWitnessTable` stands for an `IForwardDifferentiable` witness table that has to be built for a higher-order derivative.
48. `SynthesizedBackwardDerivativeWitnessTable` is the same for `IBackwardDifferentiable`.
49. `MakeIDifferentiableWitness` has no AST origin and requests an `IDifferentiable` witness for a `DifferentialPair` / `DifferentialPtrPair` type.
50. `SynthesizedBackwardDerivativeWitnessTableFromLegacyBwdDiffFunc` would bridge a legacy combined reverse function into the modern witness form, but has no producer at this commit.
51. `IdentityRemat` marks the remat phase as the identity, for a user-provided `__apply` whose `MinimalContext` is its `BwdCallable`.
52. The `__func_extension` surface behind the `_funcExtensionApply` and `_funcExtensionBackwardDiff` origins is experimental and is rejected in user code unless `-experimental-feature` is passed.
53. The core-module exemption is what lets meta code use the same syntax, so `IdentityRemat` has no non-experimental user surface at all.

### Autodiff placeholders

54. Four opcodes were introduced for the reverse-mode splitting pass to mark values that stand in for a gradient or for one half of an `inout` parameter, and none of them has a producer at this commit.
55. `LoadReverseGradient` (operand `value`) is the placeholder for the accumulated derivative to pass as a nested call's `dOut` argument.
56. `ReverseGradientDiffPairRef` (operands `primal, diff`) is the placeholder pair carrying the primal and accumulated derivative of an `inout` argument.
57. `PrimalParamRef` (operand `referencedParam`) references an `inout` parameter for use in the primal half of a split function.
58. `DiffParamRef` (operand `referencedParam`) references an `inout` parameter for use in the back-prop half.

### Differential type info

59. `DiffTypeInfo` would hold the witness tables describing a function's differentiable types, and `lowerDiffTypeInfoInsts` rewrites it to a `makeTuple`.

### Built-in requirement keys

60. The `IDifferentiable` / `IBackwardDifferentiable` / `IBwdCallable` interfaces are recognized by the compiler, and the autodiff passes need to look their requirements up by role rather than by entry position.
61. `builtinRequirementKey` (operand `kindOperand: IRIntLit`, hoistable) is the requirement key for a recognized built-in interface requirement, deduplicated by construction from its kind.
62. `BuiltinRequirementDecoration` (operand `kindOperand: IRIntLit`) records which `BuiltinRequirementKind` a requirement key represents and is attached alongside the key.
63. The Lua keys are lowercase-initial for the key and capitalized for the decoration, while the generated enumerators come from `struct_name`.

### Checkpointing and rematerialization

64. `checkpointObj` has no AST origin and marks a value as a distinct copy for checkpointing, so hoisting can keep in-loop and out-of-loop uses apart.
65. `loopExitValue` has no AST origin and records the value of an SSA variable at a loop exit so reverse mode can read it.
66. `ReportCheckpointStore` (operands `storedType, originalFunc, storeRef`) has no AST origin and marks that a checkpoint store was inserted, consumed by the checkpoint-report emitter.
67. `detachDerivative` (operand `value`) returns its operand unchanged but blocks derivative propagation through it, from `detach<T>(T x)`, `visitDetachExpr` and `visitTreatAsDifferentiableExpr`.

### `MakeDiffPair`

68. `MakeDiffPair`'s result type is an `IRDifferentialPairType`.
69. `MakeDiffPair` is not autodiff-internal: user code writing `DifferentialPair<float>(x, dx)` or `diffPair(x, dx)` gets one directly, which is why the module is marked as needing the autodiff finalization passes even when it never mentions `fwd_diff` or `bwd_diff`.
70. The forward-mode transcriber also inserts `MakeDiffPair` whenever a primal flows alongside its derivative.
71. `processPairTypes` rewrites `MakeDiffPair` into a `makeStruct` over the auto-generated pair struct.

### `GetDifferential` / `GetPrimal`

72. `GetDifferential(pair)` and `GetPrimal(pair)` are the projections that reverse `MakeDiffPair`, spelled `.d` and `.p` / `.v` in source.
73. `lowerPairAccess` rewrites all four projections into a field access on the lowered pair struct, and `lowerMakePair` rewrites both constructors into a `makeStruct`.
74. No pass folds `GetPrimal(MakeDiffPair(a, b))` down to `a` before that rewrite.
75. The `GetPrimalRef` / `GetDifferentialPtr` variants project the pointer-flavored pairs built by `MakeDiffRefPair`.

### The translation dictionary and the five-tuple

76. `TranslateBase` opcodes are requests, not results, and each is resolved through the module's translation dictionary and memoized, so a request is translated at most once per module.
77. `BackwardDifferentiate` translates to a `makeTuple` of five elements: the primal function, the remat function, the propagate function, the full intermediate-context type, and the minimal-context type.
78. `BackwardDifferentiatePrimal`, `BackwardRemat`, `BackwardDifferentiatePropagate`, `BackwardDiffIntermediateContextType` and `BackwardDiffMinimalContextType` are each resolved by synthesizing a one-operand `BackwardDifferentiate` for the same base function and returning tuple element 0 through 4.
79. The `Trivial*` family and the `*FromLegacyBwdDiffFunc` family follow the same shape with `TrivialBackwardDifferentiate` and `BackwardFromLegacyBwdDiffFunc` as their roots.
80. The individual phase opcodes therefore need no operands beyond the base function, because the tuple index carries the phase.
81. `TrivialBackwardDifferentiate` and `BackwardFromLegacyBwdDiffFunc` are never produced from an AST decl and exist only as scratch roots the translation pass builds for itself.

### `ForwardDifferentiate`

82. `ForwardDifferentiate(baseFn)` names the JVP of `baseFn` and is the one differentiation operator a user can produce directly from an expression.
83. It is also emitted when lowering a `ForwardDifferentiateVal` stored in a witness table.
84. Because the opcode is hoistable, two `__fwd_diff` references to the same function are the same IR value, and the translation pass therefore builds the derivative once.

### `BackwardDifferentiate`

85. The Lua entry declares `min_operands = 3` and the hand-written wrapper declares three accessors, but every producer at this commit builds the inst with exactly one operand, so the opcode should be treated as single-operand.
86. `min_operands` becomes `IROpInfo::fixedArgCount`, which nothing reads.
87. The user-facing `__bwd_diff` form does not reach IR, because semantic checking resolves `BackwardDifferentiateExpr` earlier and `visitBackwardDifferentiateExpr` calls `SLANG_UNEXPECTED`.
88. The same is true of `PrimalSubstituteExpr`: `[PrimalSubstitute]` and `[PrimalSubstituteOf]` are AST-level attributes and there is no `PrimalSubstitute` opcode.

### `BackwardDifferentiatePrimal`, `BackwardRemat`, and `BackwardDifferentiatePropagate`

89. On the extension created for a `[Differentiable]` callable, `apply_bwd` gets `irOp = kIROp_BackwardDifferentiatePrimal`, `remat` gets `kIROp_BackwardRemat`, and the propagate member comes from requirement-witness synthesis.
90. Differentiability therefore reaches lowering as a conformance rather than as a flag on the function.
91. When such a conformance is looked up later, the entry must be found by requirement key or by built-in role, never by position.

### Reverse-mode context

92. There is no opcode spelled `BackwardDiffPrimalContext` or `BackwardDiffPropagateContext`; the reverse-mode context is carried entirely by type opcodes.
93. Those context types are produced from `SynthesizedStructDecl`s and resolved as elements 3 and 4 of the translation five-tuple.

### `builtinRequirementKey`

94. Unlike an ordinary `key` / `StructKey`, which is a distinct global symbol per requirement decl unified across modules by its `key_<mangled>` linkage name, the built-in key is hoistable and deduplicated by construction from its `kind` operand.
95. The same logical requirement therefore resolves to one key inst whether it is referenced from the canonical interface constraint, from a synthesized constraint, or across the precompiled-core-module boundary, and no linkage decoration is needed.
96. `getInterfaceRequirementKey` computes the role from the requirement's `BuiltinRequirementModifier`, promoting `DifferentialType` to `DifferentialWitness` when the requirement being keyed is the associated conformance rather than the associated type.
97. `getInterfaceEntryByBuiltinRequirement` scans that decoration to find an entry by role.
98. Because a built-in requirement can be reached through dynamic dispatch, the `lookupKey` operand of `GetDispatcher` is typed as a plain `IRInst` rather than `IRStructKey`.

### `checkpointObj` and `ReportCheckpointStore`

99. `checkpointObj(value)` does not itself store anything; it makes a distinct copy of a value so that uses inside a loop body and uses outside it can be hoisted independently.
100. `ReportCheckpointStore`'s `storeRef` operand is a weak reference, so if the store is later eliminated the operand becomes `Poison` and the reporting walk skips that entry.
101. Dead-code elimination gives that operand its weak status, the conservative `mightHaveSideEffects` test is what keeps the marker itself alive, and the reporting walk removes it.

### `detachDerivative`

102. `detachDerivative(value)` returns its operand unchanged but blocks derivative propagation through it, so the value looks constant to the autodiff system.
103. It has two spellings in source: the core-module `detach<T>(T x)` intrinsic, and `no_diff` / `[TreatAsDifferentiable]` applied to an expression, where the materialized `IRLoad` is wrapped so that `no_diff` behaves the same on local-array indexing as on resource indexing.
104. `removeDetachInsts`, called from `finalizeAutoDiffPass`, deletes them once differentiation is done.
105. `no_diff` on a parameter declaration is a different construct and produces no `detachDerivative` inst at all; there it is a modifier moved onto the `ParamDecl`.

### C++ wrappers: hand-written vs generated

106. Every opcode on this page has an `IR<struct_name>` wrapper, none is wrapper-less, and hand-written and generated wrappers are mutually exclusive by construction.
107. Seventeen wrappers in this family are hand-written.
108. The FIDDLE macro inside a hand-written struct still injects the isa-test, the `kOp` enumerator, and the accessors derived from the Lua `operands` list, while an entry that declares only `min_operands` yields no accessors at all.
109. A hand-written leaf's bare `IRUse base;` member is not extra storage: it aliases operand 0.
110. The two abstract pair bases are hand-written and their accessor names differ from the Lua operand names of their leaves.

### Opcodes with no producer at HEAD

111. Six opcodes in this family are fully declared but nothing constructs them at `source_commit`.
112. The four placeholders each have an `IRBuilder` helper but no caller anywhere in `source/`.
113. `LoadReverseGradient` and `ReverseGradientDiffPairRef` are still named in two consumer switches, while `PrimalParamRef` and `DiffParamRef` have neither producer nor consumer.
114. `DiffTypeInfo` has a consumer but no producer: `__hasDiffTypeInfo(T)` parses to a `HasDiffTypeInfoConstraintDecl` solved to a `HasDiffTypeInfoWitness`, but that witness lowers to a void value and the constraint emits a void parameter, so no `DiffTypeInfo` inst is ever built.
115. `SynthesizedBackwardDerivativeWitnessTableFromLegacyBwdDiffFunc` has neither a producer nor a consumer.

## Functional coverage

| Claim                                                                                                                                                                                                                                  | Intent     | Anchor                                                                                                                                                                                                           | Tests                                                                                                    |
| ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ---------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------- |
| C29, C30, C31, C89, C90: A [Differentiable] callable reaches lowering as a conformance whose apply_bwd, propagate and remat members are BackwardDifferentiatePrimal, BackwardDifferentiatePropagate and BackwardRemat requests over the base function. | functional | [#backwarddifferentiateprimal-backwardremat-and-backwarddifferentiatepropagate](../../../../design/ir-reference/differentiation.md#backwarddifferentiateprimal-backwardremat-and-backwarddifferentiatepropagate) | [`backward-phase-operators.slang`](backward-phase-operators.slang)                                       |
| C87: The user-facing \_\_bwd_diff form does not reach IR as a BackwardDifferentiate inst, because semantic checking resolves the expression earlier.                                                                                    | negative   | [#backwarddifferentiate](../../../../design/ir-reference/differentiation.md#backwarddifferentiate)                                                                                                               | [`bwd-diff-does-not-emit-backward-differentiate.slang`](bwd-diff-does-not-emit-backward-differentiate.slang) |
| C88: [PrimalSubstitute] and [PrimalSubstituteOf] are AST-level attributes with no PrimalSubstitute opcode, so the substitute is resolved before lowering.                                                                               | functional | [#backwarddifferentiate](../../../../design/ir-reference/differentiation.md#backwarddifferentiate)                                                                                                               | [`primal-substitute-has-no-opcode.slang`](primal-substitute-has-no-opcode.slang)                         |
| C60, C62: Every builtinRequirementKey is emitted with a BuiltinRequirementDecoration carrying the same requirement-kind operand, so a lookup can find a witness-table entry by role instead of by position.                             | functional | [#built-in-requirement-keys](../../../../design/ir-reference/differentiation.md#built-in-requirement-keys)                                                                                                       | [`builtin-requirement-decoration.slang`](builtin-requirement-decoration.slang)                           |
| C61, C94, C95: builtinRequirementKey is hoistable and deduplicated by construction from its kind operand, so one logical built-in requirement is a single key inst with no linkage decoration.                                          | functional | [#builtinrequirementkey](../../../../design/ir-reference/differentiation.md#builtinrequirementkey)                                                                                                               | [`builtin-requirement-key-deduped.slang`](builtin-requirement-key-deduped.slang)                         |
| C96: The requirement role is promoted from DifferentialType to DifferentialWitness when the requirement being keyed is the associated conformance rather than the associated type, so the two get distinct keys.                        | functional | [#builtinrequirementkey](../../../../design/ir-reference/differentiation.md#builtinrequirementkey)                                                                                                               | [`builtin-requirement-key-differential-witness.slang`](builtin-requirement-key-differential-witness.slang) |
| C66: ReportCheckpointStore is the diagnostic channel for the checkpointing machinery, and its stored type and originating function are what the checkpoint-report walk prints.                                                          | functional | [#checkpointobj-and-reportcheckpointstore](../../../../design/ir-reference/differentiation.md#checkpointobj-and-reportcheckpointstore)                                                                           | [`report-checkpoint-store-diagnostic.slang`](report-checkpoint-store-diagnostic.slang)                   |
| C5, C67: The core-module detach<T>(T x) intrinsic lowers to a detachDerivative inst whose single operand is the value being detached and whose result type is that value's type.                                                        | functional | [#detachderivative](../../../../design/ir-reference/differentiation.md#detachderivative)                                                                                                                         | [`detach-derivative.slang`](detach-derivative.slang)                                                     |
| C102, C104: detachDerivative blocks derivative propagation through its operand and is deleted once differentiation is done, so no detach spelling survives into any target text.                                                        | functional | [#detachderivative](../../../../design/ir-reference/differentiation.md#detachderivative)                                                                                                                         | [`blocked-derivative-propagation.slang`](blocked-derivative-propagation.slang)                           |
| C103: no_diff applied to an expression wraps the materialized load in detachDerivative, so a local-array element read is detached the same way a resource read is.                                                                      | functional | [#detachderivative](../../../../design/ir-reference/differentiation.md#detachderivative)                                                                                                                         | [`no-diff-expression-detach.slang`](no-diff-expression-detach.slang)                                     |
| C105: no_diff on a parameter declaration is a modifier rather than an expression and produces no detachDerivative inst at all.                                                                                                          | boundary   | [#detachderivative](../../../../design/ir-reference/differentiation.md#detachderivative)                                                                                                                         | [`no-diff-param-produces-no-detach.slang`](no-diff-param-produces-no-detach.slang)                       |
| C13, C68: MakeDiffPair carries a differential-pair result type over a vector element type, spelled DiffPair(Vec(Float, 3), witness).                                                                                                    | boundary   | [#differential-pair-construction](../../../../design/ir-reference/differentiation.md#differential-pair-construction)                                                                                             | [`make-diff-pair-vector.slang`](make-diff-pair-vector.slang)                                             |
| C1, C2, C13: The DifferentialPair<T> constructor lowers to MakeDiffPair whose two operands are the primal and the differential, in that order.                                                                                          | functional | [#differential-pair-construction](../../../../design/ir-reference/differentiation.md#differential-pair-construction)                                                                                             | [`make-diff-pair.slang`](make-diff-pair.slang)                                                           |
| C3, C14: The DifferentialPtrPair<T> constructor lowers to MakeDiffRefPair, the pointer-typed sibling of MakeDiffPair.                                                                                                                   | functional | [#differential-pair-construction](../../../../design/ir-reference/differentiation.md#differential-pair-construction)                                                                                             | [`make-diff-ref-pair.slang`](make-diff-ref-pair.slang)                                                   |
| C2, C17, C72: Both the .p and the .v getter of DifferentialPair<T> lower to GetPrimal on the pair, producing the primal element type.                                                                                                   | functional | [#differential-pair-projection](../../../../design/ir-reference/differentiation.md#differential-pair-projection)                                                                                                 | [`get-primal.slang`](get-primal.slang)                                                                   |
| C2, C15, C72: The .d getter of DifferentialPair<T> lowers to GetDifferential on the pair, producing the differential element type.                                                                                                      | functional | [#differential-pair-projection](../../../../design/ir-reference/differentiation.md#differential-pair-projection)                                                                                                 | [`get-differential.slang`](get-differential.slang)                                                       |
| C3, C16, C18, C75: The .p and .d getters of DifferentialPtrPair<T> lower to GetPrimalRef and GetDifferentialPtr, the pointer-flavoured projections.                                                                                     | functional | [#differential-pair-projection](../../../../design/ir-reference/differentiation.md#differential-pair-projection)                                                                                                 | [`get-primal-ref-get-differential-ptr.slang`](get-primal-ref-get-differential-ptr.slang)                 |
| C26: A [TreatAsDifferentiable] function gets a synthesized fwd_diff member built from TrivialForwardDifferentiate instead of ForwardDifferentiate.                                                                                      | functional | [#forward-mode](../../../../design/ir-reference/differentiation.md#forward-mode)                                                                                                                                 | [`trivial-forward-differentiate.slang`](trivial-forward-differentiate.slang)                             |
| C26: [HasTrivialForwardDerivative] is the second attribute whose synthesized fwd_diff member is built from TrivialForwardDifferentiate rather than ForwardDifferentiate.                                                                | expansion  | [#forward-mode](../../../../design/ir-reference/differentiation.md#forward-mode)                                                                                                                                 | [`has-trivial-forward-derivative-attribute.slang`](has-trivial-forward-derivative-attribute.slang)       |
| C25: A trivial forward derivative runs the primal and returns zero output differentials, ignoring the incoming tangents.                                                                                                                | functional | [#forward-mode](../../../../design/ir-reference/differentiation.md#forward-mode)                                                                                                                                 | [`trivial-forward-derivative-zero-differential.slang`](trivial-forward-derivative-zero-differential.slang) |
| C20, C21, C22, C84: ForwardDifferentiate is hoistable, so two \_\_fwd_diff references to the same function are one IR value that both call sites share.                                                                                 | boundary   | [#forwarddifferentiate](../../../../design/ir-reference/differentiation.md#forwarddifferentiate)                                                                                                                 | [`forward-differentiate-hoisted-dedupe.slang`](forward-differentiate-hoisted-dedupe.slang)               |
| C24, C82: \_\_fwd_diff(f) lowers to a single-operand ForwardDifferentiate whose operand is the base function and whose result type is the JVP function type over differential pairs.                                                    | functional | [#forwarddifferentiate](../../../../design/ir-reference/differentiation.md#forwarddifferentiate)                                                                                                                 | [`forward-differentiate.slang`](forward-differentiate.slang)                                             |
| C74: No pass folds GetPrimal or GetDifferential applied directly to a MakeDiffPair back to the constructor operand before the pair-lowering rewrite.                                                                                    | functional | [#getdifferential--getprimal](../../../../design/ir-reference/differentiation.md#getdifferential--getprimal)                                                                                                     | [`get-primal-of-make-diff-pair-not-folded.slang`](get-primal-of-make-diff-pair-not-folded.slang)         |
| C40, C41: LegacyBackwardDifferentiate carries three function operands read as apply_bwd, remat and propagate at positions 0, 1 and 2.                                                                                                   | functional | [#legacy-bridge](../../../../design/ir-reference/differentiation.md#legacy-bridge)                                                                                                                               | [`legacy-backward-differentiate.slang`](legacy-backward-differentiate.slang)                             |
| C39, C43, C44, C45: The three legacy-bridge projection opcodes each take the same (targetFunc, legacyBwdDiffFunc) operand pair and differ only in which phase they project.                                                             | functional | [#legacy-bridge](../../../../design/ir-reference/differentiation.md#legacy-bridge)                                                                                                                               | [`legacy-bridge-projections.slang`](legacy-bridge-projections.slang)                                     |
| C8, C69: A module that constructs and projects a differential pair without mentioning fwd_diff or bwd_diff still runs the autodiff finalization passes, so no differentiation opcode reaches any target text.                           | functional | [#makediffpair](../../../../design/ir-reference/differentiation.md#makediffpair)                                                                                                                                 | [`pair-opcodes-produce-no-target-code.slang`](pair-opcodes-produce-no-target-code.slang)                 |
| C4: The free function diffPair(primal, diff) is a second user spelling of MakeDiffPair and yields the same opcode as the DifferentialPair<T> constructor.                                                                               | functional | [#makediffpair](../../../../design/ir-reference/differentiation.md#makediffpair)                                                                                                                                 | [`make-diff-pair-diff-pair-free-function.slang`](make-diff-pair-diff-pair-free-function.slang)           |
| C71, C73: processPairTypes rewrites MakeDiffPair into a makeStruct over an auto-generated pair struct, so the pair reaches target text as a two-field struct.                                                                           | functional | [#makediffpair](../../../../design/ir-reference/differentiation.md#makediffpair)                                                                                                                                 | [`pair-lowered-to-struct-in-target-text.slang`](pair-lowered-to-struct-in-target-text.slang)             |
| C114: The \_\_hasDiffTypeInfo constraint lowers to a void generic parameter satisfied by a void value, so no DiffTypeInfo inst is ever built.                                                                                           | functional | [#opcodes-with-no-producer-at-head](../../../../design/ir-reference/differentiation.md#opcodes-with-no-producer-at-head)                                                                                         | [`has-diff-type-info-constraint-void-param.slang`](has-diff-type-info-constraint-void-param.slang)       |
| C36, C38: The forward-mode transcriber names the derivative it builds for a function f with the s_fwd\_ prefix, so f yields s_fwd_f in every text target.                                                                               | functional | [#reverse-mode](../../../../design/ir-reference/differentiation.md#reverse-mode)                                                                                                                                 | [`forward-derivative-emitted-name-prefix.slang`](forward-derivative-emitted-name-prefix.slang)           |
| C36, C37: Reverse mode gives the propagate function the s_bwdProp\_ prefix and the full intermediate-context struct the s_bwdCallableCtx\_ prefix, so f yields s_bwdProp_f alongside a s_bwdCallableCtx_f struct.                       | functional | [#reverse-mode](../../../../design/ir-reference/differentiation.md#reverse-mode)                                                                                                                                 | [`reverse-mode-emitted-name-prefixes.slang`](reverse-mode-emitted-name-prefixes.slang)                   |
| C33, C34, C35: A [TreatAsDifferentiable] callable gets the trivial reverse-mode phases TrivialBackwardDifferentiatePrimal, TrivialBackwardDifferentiatePropagate and TrivialBackwardRemat instead of the ordinary ones.                 | functional | [#reverse-mode](../../../../design/ir-reference/differentiation.md#reverse-mode)                                                                                                                                 | [`trivial-backward-phase-operators.slang`](trivial-backward-phase-operators.slang)                       |
| C51: A user-provided \_\_apply whose minimal context is its BwdCallable gets a remat member built from IdentityRemat over the apply_bwd member.                                                                                         | functional | [#synthesized-derivative-witnesses](../../../../design/ir-reference/differentiation.md#synthesized-derivative-witnesses)                                                                                         | [`identity-remat.slang`](identity-remat.slang)                                                           |
| C52, C53: The \_\_func_extension surface behind the IdentityRemat row is experimental and is rejected in user code unless -experimental-feature is passed.                                                                              | negative   | [#synthesized-derivative-witnesses](../../../../design/ir-reference/differentiation.md#synthesized-derivative-witnesses)                                                                                         | [`func-extension-requires-experimental-feature.slang`](func-extension-requires-experimental-feature.slang) |
| C46: FunctionCopy names an existing user function as the body of a synthesized derivative member, taking that function as its single operand.                                                                                           | functional | [#synthesized-derivative-witnesses](../../../../design/ir-reference/differentiation.md#synthesized-derivative-witnesses)                                                                                         | [`function-copy.slang`](function-copy.slang)                                                             |
| C47, C48: The derivative of a differentiable function is itself given IForwardDifferentiable and IBackwardDifferentiable witness tables through SynthesizedForwardDerivativeWitnessTable and SynthesizedBackwardDerivativeWitnessTable. | functional | [#synthesized-derivative-witnesses](../../../../design/ir-reference/differentiation.md#synthesized-derivative-witnesses)                                                                                         | [`synthesized-derivative-witness-tables.slang`](synthesized-derivative-witness-tables.slang)             |

## Untested claims

| Claim                                                                                                                                                                                                                                            | Reason                | Anchor                                                                                                                                             | Why untested                                                                                                                                                                                                       |
| -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | --------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| C54–C58: The four placeholder opcodes `LoadReverseGradient`, `ReverseGradientDiffPairRef`, `PrimalParamRef` and `DiffParamRef` mark values standing in for a gradient or for one half of an `inout` parameter during reverse-mode splitting.      | implementation-detail | [#autodiff-placeholders](../../../../design/ir-reference/differentiation.md#autodiff-placeholders)                                                 | The doc states each has an `IRBuilder` helper but no caller anywhere in `source/` at this commit, so no Slang program can produce one; only the absence is observable, which is not a useful regression assertion. |
| C28, C85, C86: `BackwardDifferentiate` is the reverse-mode root request and should be treated as single-operand even though the Lua entry declares `min_operands = 3`, which becomes a `fixedArgCount` nothing reads.                             | link-stage-only       | [#backwarddifferentiate](../../../../design/ir-reference/differentiation.md#backwarddifferentiate)                                                 | The opcode is built only by the translation pass, which runs after this bundle's `lower` observation point; the operand-count discrepancy is a C++ header fact with no dump rendering.                             |
| C89–C91 (lookup half): a conformance entry must be found by requirement key or by built-in role, never by position.                                                                                                                              | internal-source-fact  | [#backwarddifferentiateprimal-backwardremat-and-backwarddifferentiatepropagate](../../../../design/ir-reference/differentiation.md#backwarddifferentiateprimal-backwardremat-and-backwarddifferentiatepropagate) | A rule about how pass code must read a witness table. The dump shows entries paired with keys either way, so no input distinguishes a positional consumer from a by-key one.                                       |
| C60, C63, C97, C98: the Lua key/decoration capitalization, `getInterfaceEntryByBuiltinRequirement` scanning the decoration, and `GetDispatcher`'s `lookupKey` being typed as a plain `IRInst`.                                                    | internal-source-fact  | [#built-in-requirement-keys](../../../../design/ir-reference/differentiation.md#built-in-requirement-keys)                                         | Statements about Lua entry spelling, a helper's scan loop, and a C++ operand type. The observable consequence — key and decoration carrying the same kind — is covered by `builtin-requirement-decoration.slang`.  |
| C64, C65, C99–C101: `checkpointObj` and `loopExitValue` mark values for the primal-hoisting pass, and `ReportCheckpointStore`'s weak `storeRef` operand becomes `Poison` when its store is deleted so the reporting walk skips it.                | link-stage-only       | [#checkpointing-and-rematerialization](../../../../design/ir-reference/differentiation.md#checkpointing-and-rematerialization)                     | All three are listed with "no AST origin" and are inserted by the unzip and primal-hoist passes, so nothing in the `LOWER-TO-IR` dump this bundle observes can contain them; only the emitted report is reachable. |
| C10, C106–C110: seventeen wrappers in this family are hand-written rather than FIDDLE-generated, a hand-written leaf's bare `IRUse base;` member aliases operand 0, and the abstract bases' accessor names differ from their leaves' operand names. | internal-source-fact  | [#c-wrappers-hand-written-vs-generated](../../../../design/ir-reference/differentiation.md#c-wrappers-hand-written-vs-generated)                   | Statements about C++ header layout and the FIDDLE generator; no slangc input or output distinguishes a hand-written wrapper from a generated one.                                                                  |
| C59: `DiffTypeInfo` would hold the witness tables describing a function's differentiable types and `lowerDiffTypeInfoInsts` would rewrite it to a `makeTuple`.                                                                                    | implementation-detail | [#differential-type-info](../../../../design/ir-reference/differentiation.md#differential-type-info)                                               | The rewrite has no producer to feed it, so only the "never built" half is observable; that half is covered by `has-diff-type-info-constraint-void-param.slang`.                                                    |
| C19: the four projection opcodes spell their single operand `pair`, `ptrPair` or nothing, yet all four are reached through `getBase()`.                                                                                                           | internal-source-fact  | [#differential-pair-projection](../../../../design/ir-reference/differentiation.md#differential-pair-projection)                                   | A statement about Lua operand names and C++ accessor names. The dump renders the operand positionally, so no input reveals which spelling a leaf declared.                                                         |
| C23: every entry in the differentiation-operator tables declares only `min_operands`, except `ForwardDifferentiate` which also names `baseFn`.                                                                                                    | internal-source-fact  | [#differentiation-operators](../../../../design/ir-reference/differentiation.md#differentiation-operators)                                         | Whether a wrapper got a named accessor is a C++ compile-time property; the IR dump prints operands the same way in both cases.                                                                                     |
| C11, C12: the three pair groups and `TranslateBase` exist so a consumer can match value and pointer flavors, or every translation request, with one `as<>` test.                                                                                  | internal-source-fact  | [#family-hierarchy](../../../../design/ir-reference/differentiation.md#family-hierarchy)                                                           | Abstract Lua group membership is not rendered in the IR dump — only leaf opcode names are — so no test can distinguish a leaf's group from its name.                                                               |
| C27: `ForwardDifferentiatePropagate` is the forward-mode propagate function used while unzipping a reverse-mode body.                                                                                                                             | link-stage-only       | [#forward-mode](../../../../design/ir-reference/differentiation.md#forward-mode)                                                                   | The row states there is no AST origin and that the unzip pass emits it, so it appears only in post-lowering pass dumps that this per-opcode bundle deliberately does not anchor to.                                |
| C42: `BackwardFromLegacyBwdDiffFunc` roots the legacy five-tuple and is built by the translation pass for its own use.                                                                                                                            | link-stage-only       | [#legacy-bridge](../../../../design/ir-reference/differentiation.md#legacy-bridge)                                                                 | Listed as having no AST origin; only the three projections it roots have a user surface, and those are covered by `legacy-bridge-projections.slang`.                                                               |
| C70: the forward-mode transcriber also inserts `MakeDiffPair` whenever a primal flows alongside its derivative.                                                                                                                                   | link-stage-only       | [#makediffpair](../../../../design/ir-reference/differentiation.md#makediffpair)                                                                   | This insertion happens inside the transcription pass, after the `LOWER-TO-IR` snapshot; the user-written constructor spelling is what this bundle observes instead.                                                |
| C111–C113, C115: six opcodes are fully declared with nothing constructing them, the four placeholders have builder helpers but no callers, and `SynthesizedBackwardDerivativeWitnessTableFromLegacyBwdDiffFunc` has neither producer nor consumer. | implementation-detail | [#opcodes-with-no-producer-at-head](../../../../design/ir-reference/differentiation.md#opcodes-with-no-producer-at-head)                           | These are census statements about the compiler source at `source_commit`; a test could only assert an absence that holds for any unrelated reason.                                                                 |
| C32: `TrivialBackwardDifferentiate` is the trivial-adjoint counterpart of `BackwardDifferentiate` and is built by the translation pass as the root of the trivial five-tuple.                                                                     | link-stage-only       | [#reverse-mode](../../../../design/ir-reference/differentiation.md#reverse-mode)                                                                   | The row states it has no AST origin; the `[TreatAsDifferentiable]` surface produces the three trivial phases directly, which `trivial-backward-phase-operators.slang` covers.                                      |
| C92, C93: the reverse-mode context is carried entirely by the `BackwardDiffIntermediateContextType` / `BackwardDiffMinimalContextType` type opcodes and their trivial and legacy variants, resolved as tuple elements 3 and 4.                    | out-of-bundle         | [#reverse-mode-context](../../../../design/ir-reference/differentiation.md#reverse-mode-context)                                                   | The doc hands these off to `types.md`, so the sibling bundle `design/ir-reference/types` owns them; this bundle only pins them incidentally as the phase signatures of the reverse-mode operators.                 |
| C6, C7, C9: three distinct producers (core module, semantic checking via `SynthesizedFuncDecl.irOp`, and the autodiff passes) put these opcodes into a module, and `ensureGlobalInst` skips a surviving `builtinRequirementKey` as metadata.       | internal-source-fact  | [#source](../../../../design/ir-reference/differentiation.md#source)                                                                               | A description of which compiler component constructs each inst; the emitter skip in particular is defined by producing no output, which no CHECK can distinguish from the inst not existing.                       |
| C49, C50: `MakeIDifferentiableWitness` requests an `IDifferentiable` witness for a `DifferentialPair` / `DifferentialPtrPair` type, and `SynthesizedBackwardDerivativeWitnessTableFromLegacyBwdDiffFunc` has neither producer nor consumer.        | implementation-detail | [#synthesized-derivative-witnesses](../../../../design/ir-reference/differentiation.md#synthesized-derivative-witnesses)                           | The first is emitted inside the forward-mode transcriber; the second is documented as appearing nowhere in `source/` outside the Lua definition. Neither has a Slang surface that could produce one.               |
| C76–C81: each `TranslateBase` request is resolved once per module through the translation dictionary, with `BackwardDifferentiate` translating to a five-element `makeTuple` whose index carries the phase.                                        | link-stage-only       | [#the-translation-dictionary-and-the-five-tuple](../../../../design/ir-reference/differentiation.md#the-translation-dictionary-and-the-five-tuple) | Memoization and tuple construction happen inside the translation pass, well after the `LOWER-TO-IR` snapshot; the `lower`-stage dump shows the requests, never their resolutions.                                  |
| C83: `ForwardDifferentiate` is also emitted when lowering a `ForwardDifferentiateVal` stored in a witness table.                                                                                                                                  | internal-source-fact  | [#forwarddifferentiate](../../../../design/ir-reference/differentiation.md#forwarddifferentiate)                                                   | Both paths produce the identical `ForwardDifferentiate(%f)` line, so the dump cannot attribute an inst to the expression path versus the witness-table path.                                                       |
| The forward-mode transcriber names the derivative it builds for a function f with the s_fwd_ prefix, so f yields s_fwd_f in every text target.                                                                                                   | unsupported-on-target | [#reverse-mode](../../../../design/ir-reference/differentiation.md#reverse-mode)                                                                   | Absent target: spirv-asm . C38: SPIR-V assembly carries no `OpName` for the synthesized `s_fwd_` function, so the generated name is unobservable there; the other six text-emit targets carry it and are covered.              |
| Reverse mode gives the propagate function the s_bwdProp_ prefix and the full intermediate-context struct the s_bwdCallableCtx_ prefix, so f yields s_bwdProp_f alongside a s_bwdCallableCtx_f struct.                                             | unsupported-on-target | [#reverse-mode](../../../../design/ir-reference/differentiation.md#reverse-mode)                                                                   | Absent target: spirv-asm . C37: SPIR-V assembly carries no `OpName` for the synthesized `s_bwdProp_` function or the `s_bwdCallableCtx_` struct type, so neither prefix is observable there.                                   |
| processPairTypes rewrites MakeDiffPair into a makeStruct over an auto-generated pair struct, so the pair reaches target text as a two-field struct.                                                                                               | unsupported-on-target | [#makediffpair](../../../../design/ir-reference/differentiation.md#makediffpair)                                                                   | Absent target: spirv-asm . C71: the rewritten pair becomes an unnamed `OpTypeStruct` with no `OpName` and no member names, so the `DiffPair_float` spelling and its `primal` / `differential` fields are unobservable there.   |

## Doc gaps observed

| Anchor                                                                                                                   | Kind            | Gap                                                                                                                                                                                                                                                                                                           | Suggested addition                                                                                                                                                                                                                                                         |
| ------------------------------------------------------------------------------------------------------------------------ | --------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| [#backwarddifferentiate](../../../../design/ir-reference/differentiation.md#backwarddifferentiate)                       | missing-surface | The Reverse-mode table gives `BackwardDifferentiate` the AST origin "`BackwardDifferentiateVal` via `visitBackwardDifferentiateVal`", but the section body then says `__bwd_diff` does not reach IR at all. A reader is left without an answer to "then what does `__bwd_diff(f)` become?".                   | State in the section that `__bwd_diff(f)` resolves to the callable's synthesized `bwd_diff` member — a `LegacyBackwardDifferentiate` value — so the call site lowers to an ordinary `call` on it, and note that no user surface produces `BackwardDifferentiate` directly. |
| [#checkpointobj-and-reportcheckpointstore](../../../../design/ir-reference/differentiation.md#checkpointobj-and-reportcheckpointstore) | missing-surface | The section says `ReportCheckpointStore` is "the diagnostic channel for the same machinery" and describes a "reporting walk", but never names the compiler option that turns the report on, so a reader cannot reach it. The report is only produced under `-report-checkpoint-intermediates`.                | Name `-report-checkpoint-intermediates` on the `ReportCheckpointStore` row (or in the section body) and show one line of its output, e.g. `note: checkpointing context of N bytes associated with: 'f'`, so the marker's `originalFunc` and `storedType` operands are visible. |
| [#detachderivative](../../../../design/ir-reference/differentiation.md#detachderivative)                                 | missing-surface | The section lists two source spellings, "the core-module `detach<T>(T x)` intrinsic, and `no_diff` / `[TreatAsDifferentiable]` applied to an expression", but does not warn that `no_diff` on a _parameter declaration_ is a different construct that produces no `detachDerivative` inst at all.             | Add a sentence distinguishing `no_diff` as an expression modifier (which yields `detachDerivative`) from `no_diff` as a parameter modifier (which yields an `Attributed(T, no_diff)` parameter type instead), and name the page that owns the attributed-type spelling.    |
| [#forward-mode](../../../../design/ir-reference/differentiation.md#forward-mode)                                         | missing-surface | The `AST origin` column names internal checking entry points (`checkDifferentiableCallableCommon`, `trySynthesizeDiffFuncRequirementWitness`, `_funcExtensionBackwardDiff`, `_funcExtensionApply`) but never the user-level attribute that triggers each one, so the rows cannot be reached from source code. | Add a "user surface" column, or one sentence per subsection, mapping each synthesis entry point to its attribute: `[Differentiable]` / `[BackwardDifferentiable]`, `[TreatAsDifferentiable]`, `[BackwardDerivative(f)]`, and `__func_extension __apply`.                   |
| [#differentiation-operators](../../../../design/ir-reference/differentiation.md#differentiation-operators)               | missing-example | The section says identical translation requests "dedupe to a single IR value before the translation pass ever sees them", but shows no example, so a reader cannot tell whether the dedupe is per-function, per-call-site, or per-entry-point.                                                                | Add a three-line example with two `__fwd_diff(f)` expressions in one entry point and the single `let %f_fwd_diff = ForwardDifferentiate(%f)` line they share, to show the dedupe is keyed on the base function.                                                            |
| [#makediffpair](../../../../design/ir-reference/differentiation.md#makediffpair)                                         | missing-example | The section says `processPairTypes` "rewrites it into a `makeStruct` over the auto-generated pair struct", but a reader inspecting generated HLSL/Metal/WGSL may see either a `DiffPair_float` struct or no trace of the pair at all, and the doc does not say which case is which.                           | Add a short note that the rewritten struct survives into target text only when a differentiated call passes the pair across a function boundary, and that a purely local pair is scalarized away before emit.                                                              |
| [#reverse-mode](../../../../design/ir-reference/differentiation.md#reverse-mode)                                         | ambiguous-claim | The section says the generated names are ones "a reader will meet in emitted HLSL or CUDA", but the derivative function name is present in HLSL, GLSL, Metal, WGSL, CUDA and C++ and absent from SPIR-V assembly, which carries no `OpName` for it. The list of two targets reads as exhaustive.              | Replace "in emitted HLSL or CUDA" with a note that the prefixes appear in every source-level text target and that SPIR-V output does not preserve them, so a SPIR-V reader has to match on structure rather than on the name.                                              |
| [#synthesized-derivative-witnesses](../../../../design/ir-reference/differentiation.md#synthesized-derivative-witnesses) | missing-surface | `IdentityRemat`'s AST origin is given as "`SynthesizedFuncDecl` `remat` from `_funcExtensionApply`" with no hint that the `__func_extension` surface behind it is experimental. The section body says the decl is "diagnosed" but does not give the diagnostic, which is a warning (E30131), not an error.    | Note on the `IdentityRemat` row that its only surface is gated behind `-experimental-feature`, and state in the body that the gate emits warning E30131 and drops the declaration, so downstream uses then fail with a separate error.                                     |
