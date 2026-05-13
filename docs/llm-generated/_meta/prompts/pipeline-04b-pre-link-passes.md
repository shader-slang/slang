# Prompt: pipeline/04b-pre-link-passes.md

See [_common.md](_common.md) for the universal rules and the
**Pre-link mandatory-pass page contract**.

## Target

Produce `docs/llm-generated/pipeline/04b-pre-link-passes.md` —
the ordered, control-flow-aware reference for the IR pass
sequence that runs inside `generateIRForTranslationUnit`
([slang-lower-to-ir.cpp](../../../source/slang/slang-lower-to-ir.cpp)
line ~14386) **before** the per-module IR is cached on the
`Module` and pulled into `linkAndOptimizeIR` by `linkIR`.

Audience: a compiler developer who needs to find where in the
pre-link pipeline a particular pass runs, why it runs there, and
how it interacts with the mandatory-early-inlining loop.

## Scope

- **Per-module, pre-link only.** Cover passes that run on the
  per-translation-unit IR module before it is cached on the
  `Module` and made available for link.
- **Target-agnostic.** The same passes run for every shader
  target. There is no per-target switch in
  `generateIRForTranslationUnit`. State this explicitly in the
  intro; do not invent target-specific arms.
- **Includes the AST walk that produces the IR module.** Phase A
  documents `IRModule::create` and the per-entry-point and
  per-decl walks that produce the initial IR, because that is
  what feeds the mandatory passes.
- **Excludes post-link passes.** The catalog of post-link passes
  is `pipeline/05-ir-passes.md`; the ordered per-target post-link
  sequence is `target-pipelines/*.md`.
- **No `SLANG_PASS` macro.** The calls in this region are plain
  function calls of the form `passName(module)`,
  `passName(module, sink)`, or member calls like
  `eliminateDeadCode(func, dceOptions)`. Do not annotate them
  with `SLANG_PASS`.

## Phase decomposition

Four phases — see the **Pre-link mandatory-pass page contract**
for boundaries. Brief recap:

- **Phase A — AST walk and IR emission** (lines 14408-14514):
  `IRModule::create`, `module->setName`, optional
  `ExperimentalModuleDecoration`, debug-source emission, the
  entry-point loop calling `lowerFrontEndEntryPointToIR`, the
  full-decl walk via `ensureAllDeclsRec`, the
  `kIROp_GlobalHashedStringLiterals` aggregate, the optional
  NVAPI slot decoration, and the closing
  `validateIRModuleIfEnabled`.
- **Phase B — Mandatory pre-optimization transformations**
  (lines 14536-14563): `prelinkIR`, `lowerErrorHandling`,
  `lowerDefer`, `synthesizeBitFieldAccessors`,
  `lowerExpandType`, `insertDebugValueStore`.
- **Phase C — Mandatory optimization passes** (lines 14569-14650):
  `constructSSA`, `applySparseConditionalConstantPropagation`,
  the `!minimumOptimizations` gate around `simplifyCFG` +
  `peepholeOptimize(getPrelinking)`, the per-function-DCE loop
  with `keepExportsAlive`/`keepLayoutsAlive`/`useFastAnalysis`,
  optional `invertLoops`, and the
  `performMandatoryEarlyInlining` fixed-point loop with its
  inner per-modified-function simplification cluster.
- **Phase D — Non-essential validation, stripping, and
  finalization** (lines 14652-14771): the
  `shouldRunNonEssentialValidation` block
  (`checkForRecursiveTypes`, `propagateConstExpr`,
  `checkForUsingUninitializedValues`, `checkForMissingReturns`
  with `target=None`, `checkAutoDiffUsages`,
  `checkForOperatorShiftOverflow`,
  `addDecorationsForGenericsSpecializedWithExistentials` (2025+
  only), `checkForMeshOutputReads`), the stripping block
  (`stripFrontEndOnlyInstructions`, `stripImportedWitnessTable`,
  trailing `eliminateDeadCode`, optional `obfuscateModuleLocs`),
  `validateIRModuleIfEnabled`, and
  `module->buildMangledNameToGlobalInstMap`.

## Diagram conventions

- Use `flowchart TD` per phase. Default to one node per call.
- Gates between calls become diamond rhombi
  (`{reqSet.foo}` / `{getBoolOption(...)}` / etc.).
- The mandatory-early-inlining loop in Phase C is the only loop
  in the pipeline; render it as a back-edge from the loop-body
  exit to the `performMandatoryEarlyInlining` node, with a
  `(changed?)` diamond.
- Per-function DCE iterations (line 14585-14589) and per-modified-
  function simplification (line 14633-14645) are not whole-module
  loops — render them as one node with a note clarifying the
  per-function semantics.

## Conditional gates

Group the gates table by:

1. Option-set toggles:
   - `shouldPerformMinimumOptimizations()`
   - `shouldRunNonEssentialValidation()`
   - `shouldObfuscateCode()`
   - `shouldHaveSourceMap()`
   - `getBoolOption(CompilerOptionName::LoopInversion)`
   - `getBoolOption(CompilerOptionName::TraceCoverage)` (gates
     `context->traceCoverage` but does not directly gate a pass
     here; flag this).
   - `getDebugInfoLevel()` (Standard or higher gates debug-source
     emission and `insertDebugValueStore`).
2. Context predicates:
   - `sink->getErrorCount() != 0` (early-exit from the
     non-essential validation block at line 14657).
   - `translationUnit->getModuleDecl()->findModifier<ExperimentalModuleAttribute>()`.
   - `translationUnit->getModuleDecl()->findModifier<NVAPISlotModifier>()`.
3. Per-translation-unit predicates:
   - `translationUnit->getModuleDecl()->languageVersion >= SlangLanguageVersion::SLANG_LANGUAGE_VERSION_2025`.

## Loops

Single loop. Document:

- The `for(;;)` at line 14624.
- Termination: outer loop terminates when
  `performMandatoryEarlyInlining` reports `changed == false` and
  no inner-cluster pass mutates the module.
- Per-iteration body: `performMandatoryEarlyInlining` →
  `peepholeOptimizeGlobalScope` → (if `!minimumOptimizations`)
  for each modified function: `constructSSA`, SCCP, peephole,
  fast `simplifyCFG`, DCE.
- Note: per-function DCE outside the loop (line 14585-14589) and
  per-changed-function simplification inside the loop iterate
  over functions, not over passes; they are not loops in the
  pipeline sense.

## Notable passes

Cover at least:

- `prelinkIR` — pulls in `[unsafeForceInlineEarly]` and
  `externalSymbolsToPrelink`; explain how this differs from the
  post-link `linkIR`.
- `lowerErrorHandling` — throw + `tryCall` → `Result<T,E>`
  returns and `call` + `ifElse`.
- `lowerDefer` — `defer` statement lowering.
- `synthesizeBitFieldAccessors` — bit-field accessor synthesis,
  intentionally placed before the inlining loop so the bodies
  can be inlined and simplified.
- `lowerExpandType` — variadic-generics `IRExpandType` →
  `IRExpand` lowering.
- `performMandatoryEarlyInlining` and the surrounding loop —
  why the loop exists; what it means for shader authors
  (`[unsafeForceInlineEarly]` is a hard requirement, not a hint).
- `stripFrontEndOnlyInstructions` — what counts as
  "front-end-only"; the
  `shouldStripNameHints = shouldObfuscateCode()` link.
- `obfuscateModuleLocs` — only fires when both `shouldObfuscateCode`
  and `shouldHaveSourceMap` are true.
- `module->buildMangledNameToGlobalInstMap` — sets up the
  mangled-name lookup used later by `linkIR`.

## Adjacent constructs

Three short paragraphs:

- `prelinkIR` (also a Phase B notable pass; this paragraph
  emphasizes the link relationship rather than the import set).
- `SpecializedComponentTypeIRGenContext::process` (line ~14783):
  short IR module for a specialized component type; no
  mandatory passes.
- `TargetProgram::createIRModuleForLayout` (line ~15327): one
  sentence summary, then point to
  [04c-layout-ir.md](../../pipeline/04c-layout-ir.md) for the
  deep dive.

## Quality checklist (in addition to the universal one and the contract)

- [ ] Every function call inside `generateIRForTranslationUnit`
      that mutates the IR is listed in exactly one phase table.
- [ ] The mandatory-early-inlining loop is rendered with a
      visible back-edge and a (changed?) gate.
- [ ] No mention of `SLANG_PASS` for pre-link passes.
- [ ] No per-target arms (the pipeline is target-agnostic).
- [ ] `## Adjacent constructs` covers `prelinkIR`,
      `SpecializedComponentTypeIRGenContext::process`, and
      `createIRModuleForLayout` — and only those three.
