# Common contract for every generated document

Every prompt under this directory inherits the rules below. Individual
prompts only describe the structure and content specific to their target
document.

## Inputs you will receive

When invoking an agent against one of these prompts, pass at minimum:

1. The list of **watched paths** for the target document, expanded to the
   actual files at the current commit. Use
   `regenerate.py show <doc>` to obtain this list.
2. The contents of any documents listed under `depends_on` for the target
   document, as additional context.
3. The target document's manifest key (e.g. `pipeline/05-ir-passes.md`),
   which is also the workspace-relative path under `docs/generated/design/`.
4. The current `HEAD` commit SHA (for the front-matter `source_commit`
   field) and the current `watched_paths_digest`
   (`regenerate.py digest <doc>`).

## Mandatory front-matter

Every generated document must begin with:

```yaml
---
generated: true
model: <model identifier>
generated_at: <ISO 8601 timestamp, UTC, seconds precision>
source_commit: <full git SHA>
watched_paths_digest: <hex sha256 from regenerate.py digest>
warning: "Auto-generated. May drift from source. Do not edit by hand."
---
```

Everything below the closing `---` is the body of the document.

## Universal style rules

- Use Markdown headings (`#`, `##`, ...). The first heading must be `#`
  and is the document title.
- When you cite a source file, use a markdown link with the
  workspace-relative path, e.g. `[slang-parser.cpp](../../../../source/slang/slang-parser.cpp)`.
  Every such link must resolve at `source_commit`.
- When you reference a sibling LLM-generated doc, link relatively, e.g.
  `[../pipeline/05-ir-passes.md](../pipeline/05-ir-passes.md)`.
- Do not copy verbatim prose from `docs/design/`. You may cite those
  documents as further reading.
- Do not invent file paths, function names, or symbols. If unsure, omit
  the claim.
- Prefer concrete, code-anchored statements ("the `Lexer::lexToken`
  method in [slang-lexer.cpp](../../../../source/compiler-core/slang-lexer.cpp)
  classifies tokens by ...") over abstract claims ("the lexer is
  efficient and clean").
- Use mermaid diagrams when they materially clarify structure. Follow
  the project's mermaid-syntax conventions: no spaces in node IDs,
  no explicit colors, escape special characters in labels with quotes.

## Universal content rules

- Stay within the size cap shown in the manifest. If the natural content
  is larger, summarize and split off detail into separate docs (and note
  that the cap should be raised in the manifest as a follow-up).
- The first paragraph of the body must say, in plain language, what the
  document covers and who its intended reader is.
- Do not duplicate large excerpts of source code. Cite a few essential
  signatures and link the rest.
- If you cannot find the information needed for a section, write a short
  paragraph explaining that the information was not located in the
  watched paths, and propose which additional paths should be added to
  the manifest.

## Forbidden content

- Per-function reference documentation that belongs in Doxygen.
- Speculative roadmap items not present in the source.
- Editorial opinions about the codebase ("this is well-designed",
  "this is hacky"). Stick to descriptive claims.
- Verbatim text from `docs/design/`, `docs/user-guide/`, the language
  reference, or upstream third-party documentation.

## AST reference family contract

The pages under `docs/generated/design/ast-reference/` share a common shape.
A prompt that targets one of those pages must produce a document with
the following sections, in order:

1. `# <Family> Reference` title.
2. `## Source` — one paragraph linking the header that owns the family
   (e.g. [slang-ast-decl.h](../../../../source/slang/slang-ast-decl.h)) plus,
   for families that correspond to parsed syntax, the relevant parser
   entry points in
   [slang-parser.cpp](../../../../source/slang/slang-parser.cpp).
3. `## Family hierarchy` — a brief mermaid or bullet diagram showing the
   abstract intermediate classes inside the family. Cite the base class
   relationships declared in
   [slang-ast-base.h](../../../../source/slang/slang-ast-base.h).
4. `## Nodes` — a single table with one row per concrete (FIDDLE-declared)
   class in the family, columns:

   | Column | Content |
   | --- | --- |
   | Class | Backtick-wrapped exact class name as declared in the header. |
   | Parent | Closest ancestor a reader needs to know — not always the immediate C++ base. Use the most semantically informative one (e.g. `BinaryExpr` rather than `OperatorExpr`). |
   | Key fields | 1-4 fields that define the node's data shape, in `name: Type` form. Long field lists are summarized as "...plus N more; see header". |
   | Grammar | Markdown link to the most specific anchor in [syntax-reference/grammar.md](../syntax-reference/grammar.md), or the literal text `(none)` for nodes that are synthesized rather than parsed (e.g. synthetic conformance decls). |
   | Summary | One short clause; no markup beyond inline code. |

5. `## Notable nodes` — short prose callouts (2-5 sentences each) for the
   half-dozen or so nodes per family where the table form leaves out
   essential context. A node is "notable" if any of the following holds:
   it has non-obvious semantics, it participates in a system that spans
   multiple AST families (generics, overload resolution, existentials),
   it is created by synthesis rather than parsing, or it is the entry
   point for a parser section a reader is likely to want to navigate to.
   Each callout names the class in a sub-heading and cites at most one
   header line range if it adds clarity. Do not include code blocks.
6. `## See also` — bullet list linking peer reference pages
   (`[../ast-reference/...](../ast-reference/...)`) and the relevant
   `docs/generated/design/pipeline/` doc. Always include the parsing
   page when the family is parsed.

Coverage rule: every concrete class in the family that the header
declares via `FIDDLE()` belongs in the `## Nodes` table. Abstract
intermediate bases (those declared via `FIDDLE(abstract)`) appear in the
`## Family hierarchy` diagram but not in the table. If you find a class
you cannot classify, list it in `## Nodes` with `(unclear)` in the
`Summary` column and an explicit note rather than omitting it.

Forbidden in AST reference pages: per-method or per-virtual-function
documentation, IR-level information (`IR*` classes belong to
[../cross-cutting/ir-instructions.md](../cross-cutting/ir-instructions.md)),
and tutorial-style explanations of compiler theory (use the
[../glossary.md](../glossary.md)).

## Name-resolution family contract

The pages under `docs/generated/design/name-resolution/` document the
*algorithmic* rules of identifier scopes, lookup, shadowing, visibility,
and overload resolution. They sit between the parsing overview in
[../pipeline/02-parse-ast.md](../pipeline/02-parse-ast.md) and the
semantic-checking overview in
[../pipeline/03-semantic-check.md](../pipeline/03-semantic-check.md):
the pipeline pages explain *where* in the compile flow name resolution
happens; the name-resolution pages explain *what rules* are applied.

A prompt that targets one of those pages (other than `index.md`) must
produce a document with the following sections, in order:

1. `# <Topic>` title — for example `# Scopes`, `# Lookup`,
   `# Visibility`, `# Overload Resolution`.
2. `## Source` — one paragraph identifying the primary header(s) and
   `.cpp` file(s) that own the topic, each linked with a
   workspace-relative path. Include both the declaration site and the
   implementation site when they are split (e.g. `slang-lookup.h` +
   `slang-lookup.cpp`).
3. `## Concepts` — short definitions of the 3-6 data structures, enums,
   or flag types the page builds on. Each item names the C++ type in
   backticks and cites the header line range if it materially clarifies.
   Prefer a compact bullet list over a table for this section.
4. `## Algorithm` or `## Rules` — the heart of the page. Use whichever
   heading better fits the topic (`scopes.md` and `visibility.md` use
   `## Rules`; `lookup.md` and `overload-resolution.md` use
   `## Algorithm`). The body must:
   - Walk through the algorithm or ruleset step by step.
   - Cite at least one function or method per step
     (`functionName` in `[file.cpp](path)`).
   - Include a mermaid flowchart when there is a multi-step pipeline,
     following the project's mermaid syntax conventions.
5. `## Edge cases and failure modes` — enumerate the corner cases the
   reader is most likely to hit. Each case is a short sub-bullet with
   the symptom, the rule that handles it, and the diagnostic (or
   silent behavior) the compiler produces. Where relevant, cite the
   `Diagnostics::*` identifier from
   [../../../../source/slang/slang-diagnostics.h](../../../../source/slang/slang-diagnostics.h).
6. `## See also` — bullet list linking peer name-resolution pages,
   the relevant `ast-reference/` page(s), the relevant
   `pipeline/` page(s), and the glossary terms most directly relevant
   to the topic.

Scope rule: every page may reference any C++ symbol declared in its
manifest `watched_paths`. If a topic forces a citation outside those
paths, expand the page's `watched_paths` in the manifest rather than
guessing, and call out the change in the page text.

Forbidden in name-resolution pages: parsing details that belong in
[../pipeline/02-parse-ast.md](../pipeline/02-parse-ast.md); IR-level
information about how decl-refs are lowered (that belongs in
[../pipeline/04-ast-to-ir.md](../pipeline/04-ast-to-ir.md) and
[../cross-cutting/ir-instructions.md](../cross-cutting/ir-instructions.md));
copying the per-class field lists already covered in
[../ast-reference/](../ast-reference) — link those pages instead.

## IR-reference family contract

The pages under `docs/generated/design/ir-reference/` are an exhaustive
per-family catalog of every opcode declared in
[../../../../source/slang/slang-ir-insts.lua](../../../../source/slang/slang-ir-insts.lua).
They sit between the narrative survey in
[../cross-cutting/ir-instructions.md](../cross-cutting/ir-instructions.md)
(which covers schema, flag bits, module versioning, and the "add an
opcode" workflow) and the lowering pipeline page in
[../pipeline/04-ast-to-ir.md](../pipeline/04-ast-to-ir.md) (which
covers *when* AST nodes turn into IR). The IR-reference pages explain
*which* opcodes exist, *what shape* they have, and *which AST node*
(if any) lowers to each.

A prompt that targets one of those pages (other than `index.md`) must
produce a document with the following sections, in order:

1. `# <Family>` title — for example `# Types`, `# Values`,
   `# Control Flow`, `# Decorations`.
2. `## Source` — one paragraph identifying the Lua entry range that
   owns the family (cite line numbers when they materially help), the
   matching `IRFoo` wrappers in
   [../../../../source/slang/slang-ir-insts.h](../../../../source/slang/slang-ir-insts.h),
   and the `slang-lower-to-ir.cpp` visitor(s) that produce these
   opcodes. Include
   [../../../../source/slang/slang-ir.h](../../../../source/slang/slang-ir.h)
   when the family relies on infrastructure declared there (op flags,
   `IRBuilder` helpers).
3. `## Family hierarchy` — a brief mermaid `flowchart TD` mirroring
   the Lua nesting for the family, so the reader can see e.g. that
   `BasicType` is a subrange of `Type`. Abstract intermediates only
   appear here.
4. `## Opcodes` — one or more tables, one row per concrete opcode in
   the family. Sub-tables (split by sub-group) are encouraged when the
   family is large. Every table has the following columns, in order:

   | Column | Content |
   | --- | --- |
   | Opcode | Backtick-wrapped Lua entry name; this is also the `kIROp_<name>` enum tag. |
   | C++ wrapper | `IRFoo` struct name (from `struct_name` or implicit). Use an em-dash (`—`) when no wrapper exists. |
   | Operands | Compact comma-separated operand names from the Lua entry, e.g. `elementType, count`. Use `(variadic)` for variadic ops and `—` for nullary. |
   | Flags | One-letter codes joined without separators: `H` hoistable, `P` parent, `G` global. Blank when none apply. |
   | AST origin | The AST class (`VarDecl`, `BinaryExpr` (`+`), ...) whose `visit*` in `slang-lower-to-ir.cpp` produces this opcode, or `(synthesized)` for IR-pass-introduced opcodes, or `—` for opcodes with no direct AST source (e.g. autodiff intermediates). |
   | Summary | One short clause; no markup beyond inline code. |

5. `## Notable opcodes` — short prose callouts (2-5 sentences each)
   for the half-dozen or so opcodes per family where the table form
   leaves out essential context. An opcode is "notable" if any of the
   following holds: its operand list is non-obviously meaningful
   (e.g. `loop` / `ifElse` with their join-target operands), it
   participates in a system that spans multiple families
   (`specialize`, `lookupWitness`, existentials), it is created
   primarily by IR passes rather than lowering (`Poison`,
   `defaultConstruct`, `Each`), or its naming is misleading. Each
   callout names the opcode in a sub-heading. Do not include code
   blocks.
6. `## See also` — bullet list linking the conventions page
   ([../cross-cutting/ir-instructions.md](../cross-cutting/ir-instructions.md)),
   peer IR-reference pages, the lowering pipeline page
   ([../pipeline/04-ast-to-ir.md](../pipeline/04-ast-to-ir.md)), the
   relevant `ast-reference/` page(s) for the AST origin column, the
   `docs/design/` rationale doc when one exists, and the glossary
   entries most directly relevant to the family.

Coverage rule: every concrete opcode entry in the Lua file that
belongs to the family must appear in the `## Opcodes` table. Abstract
parent entries (those that exist only to group their children, e.g.
`BasicType`, `TerminatorInst`) appear only in `## Family hierarchy`.
If an opcode straddles two families, list it in the more specific
family and cross-link from the other.

Forbidden in IR-reference pages: design rationale (use
[../../design/ir.md](../../design/ir.md)), pass-by-pass behavior
descriptions (use [../pipeline/05-ir-passes.md](../pipeline/05-ir-passes.md)),
copying the schema description already covered in
[../cross-cutting/ir-instructions.md](../cross-cutting/ir-instructions.md),
and tutorial-style explanations of compiler theory (use the
[../glossary.md](../glossary.md)).

## Target-pipeline page contract

The pages under `docs/generated/design/target-pipelines/` document, for
one specific code-generation target, the **ordered** sequence of IR
passes that runs from end of AST lowering to the final emitted
artifact. They sit between the unordered per-pass catalog in
[../pipeline/05-ir-passes.md](../pipeline/05-ir-passes.md) and the
backend-emit overview in
[../pipeline/06-emit.md](../pipeline/06-emit.md): the catalog page
explains *what* every pass is and groups them by topic; the
target-pipeline pages explain *when* each pass runs for one target,
*which gate selects it*, and *what loops* iterate it.

A prompt that targets one of those pages must produce a document
with the following sections, in order:

1. `# <Target> Target Pipeline` title — for example
   `# SPIR-V Target Pipeline`, `# HLSL Target Pipeline`.
2. One-paragraph intro identifying the target's `CodeGenTarget`
   value, any precondition switch (e.g. `shouldEmitSPIRVDirectly()`
   for SPIR-V direct emit), and how this page complements
   [../pipeline/05-ir-passes.md](../pipeline/05-ir-passes.md).
3. `## Source` — link list for the orchestrator
   ([../../../../source/slang/slang-emit.cpp](../../../../source/slang/slang-emit.cpp))
   and the target-specific files (per-target emit, per-target
   legalization, any shared files invoked from a target-only branch).
   Cite the line numbers of the relevant entry points
   (`linkAndOptimizeIR`, the per-target `emit*ForEntryPoints*`,
   the per-target `legalize*`).
4. `## High-level phase diagram` — a small mermaid `flowchart TD`
   (≤ 8 nodes) linking the named phases the page covers; this is the
   reader's roadmap.
5. `## Phase <N>: <name>` — one section per phase. The number of
   phases is chosen by the target's natural decomposition (3-5 is
   typical). Each phase section contains:
   - A short paragraph (1-3 sentences) naming the source range it
     covers (e.g. "lines 1172-1710 of `slang-emit.cpp`").
   - A mermaid `flowchart TD` showing every pass call that runs for
     this target in this phase. Diagram conventions:
     - Every `SLANG_PASS(passFunc, ...)` call is its own rectangular
       node labeled with the pass name.
     - Conditional gates are diamond nodes labeled with the gate
       expression; the `true` arm runs the gated passes, the `false`
       arm falls through.
     - Loops are drawn as back-edges labeled with the loop condition
       and bound (e.g. `while changed && i < 8`).
     - Branches that cannot fire for this target are filtered out
       (passes gated on a sibling `CodeGenTarget::*` value).
     - Downstream binary tools (e.g. spirv-link, spirv-val,
       spirv-opt for SPIR-V) carry a `(downstream)` prefix.
     - `validateIRModuleIfEnabled` calls are summarized in a comment
       under the diagram rather than appearing as nodes.
   - A companion ordered table with one row per pass node in the
     diagram, columns:

   | Column | Content |
   | --- | --- |
   | # | 1-based order within the phase. |
   | Pass | Backtick-wrapped pass function name. |
   | File | Markdown link to the `.cpp` file that defines the pass. |
   | Gate | The condition (if any) that selects this pass; backtick-wrap the C++ expression. Use `(always)` when unconditional. |
   | Notes | Optional one-clause clarification (e.g. flag values passed, second invocation, etc.); blank when none needed. |

6. `## Conditional gates` — a single table consolidating every gate
   expression the phase diagrams reference, with the passes it
   controls. Group rows by gate kind: `requiredLoweringPassSet.*`
   flags first, then `targetCompilerOptions.*` / option-set
   toggles, then capability checks and environment variables.
7. `## Loops in the pipeline` — short prose describing each
   iterative pass (outer + inner loops, the iteration bound, the
   fixed-point condition). Include the loop bounds verbatim from
   the source.
8. `## Notable passes` — paragraph callouts (2-5 sentences each)
   for the passes whose target-specific semantics are not obvious
   from a row in the phase table: passes invoked multiple times,
   passes with target-specific configuration objects, passes
   deferred to a target legalization step rather than
   `linkAndOptimizeIR`, and any downstream binary chain.
9. `## See also` — bullet list linking
   [../pipeline/04-ast-to-ir.md](../pipeline/04-ast-to-ir.md),
   [../pipeline/05-ir-passes.md](../pipeline/05-ir-passes.md),
   [../pipeline/06-emit.md](../pipeline/06-emit.md),
   [../cross-cutting/targets.md](../cross-cutting/targets.md),
   [../ir-reference/index.md](../ir-reference/index.md), and the
   user-facing target documentation under `docs/user-guide/` when
   one exists.

Coverage rule: every `SLANG_PASS(passFunc, ...)` call in
`linkAndOptimizeIR` reachable for the target (and any
target-specific entry-point function the page covers) must appear
in exactly one phase table. Calls in branches that cannot fire for
the target are omitted; the prose acknowledges the filter.

Forbidden in target-pipeline pages: per-pass behavioral
explanations that already live in
[../pipeline/05-ir-passes.md](../pipeline/05-ir-passes.md) (link
those rows instead of duplicating them); design rationale that
belongs in `docs/design/`; tutorial-style introductions to the
target language (use the user-guide).

## Target-pipeline index contract

The page `docs/generated/design/target-pipelines/index.md` is a
navigation hub for the per-target pipeline pages. It is **not** a
target page — it does not document any pass. A prompt that targets
the index must produce a document with the following sections, in
order:

1. `# Target Pipelines` title.
2. One-paragraph intro stating that each peer page documents one
   target's ordered IR-pass + downstream-tool sequence and pointing
   the reader at [../pipeline/05-ir-passes.md](../pipeline/05-ir-passes.md)
   for the unordered topical catalog.
3. `## Pages` — bullet list linking to every peer page in
   `target-pipelines/`, with a one-clause summary per page
   (e.g. "SPIR-V via the direct-emit path",
   "HLSL source plus DXIL / DXBytecode downstream").
4. `## Shared shape` — references the **Target-pipeline page
   contract** and explains the four-phase decomposition once so
   readers do not have to re-read it on every page. Identifies
   `linkAndOptimizeIR` (`slang-emit.cpp` line ~892) as the shared
   orchestrator.
5. `## Cross-target comparison` — a single table summarizing the
   five targets, columns: **Target**, **CodeGenTarget enum values**,
   **Phase C entry**, **Phase D emitter**, **Downstream tools**,
   **Loops** (Y/N + a short note such as
   `simplifyIRForSpirvLegalization (8x16)`).
6. `## Filtering rules` — short paragraph reminding the reader that
   each target page filters out switch arms gated on a sibling
   target; a glance at one page should not be mistaken for the
   global ordering.
7. `## See also` — bullets to
   [../pipeline/04-ast-to-ir.md](../pipeline/04-ast-to-ir.md),
   [../pipeline/05-ir-passes.md](../pipeline/05-ir-passes.md),
   [../pipeline/06-emit.md](../pipeline/06-emit.md),
   [../cross-cutting/targets.md](../cross-cutting/targets.md), and
   [../ir-reference/index.md](../ir-reference/index.md).

Forbidden in the index: per-pass details, code snippets from the
target pages, or duplicated phase tables. The index references
target pages by link; it never copies their content.

## Pre-link mandatory-pass page contract

The page `docs/generated/design/pipeline/04b-pre-link-passes.md`
documents the ordered, per-translation-unit IR pass sequence that
runs inside `generateIRForTranslationUnit`
([slang-lower-to-ir.cpp](../../../../source/slang/slang-lower-to-ir.cpp)
line ~14386) **before** the module IR is cached on the
`Module` and pulled into `linkAndOptimizeIR` by `linkIR`. The
shape mirrors the **Target-pipeline page contract** above with
adjustments: no downstream tools, no per-target dispatch, no
`SLANG_PASS` macro — the calls in this region are plain function
calls of the form `passName(module)`. A prompt that targets this
page must produce a document with:

1. Title and one-paragraph intro that establishes this is the
   per-module pre-link pipeline (one invocation per
   `TranslationUnitRequest`), distinguishes it from the
   post-link `linkAndOptimizeIR` pipeline documented on
   `target-pipelines/`, and notes that the pipeline is
   target-agnostic — the same passes run for every shader
   target.
2. `## Source` — direct links to
   [slang-lower-to-ir.cpp](../../../../source/slang/slang-lower-to-ir.cpp)
   `generateIRForTranslationUnit` (line ~14386) and the cited
   pass-implementation files.
3. `## High-level phase diagram` — one Mermaid `flowchart TD`
   showing the four phases below feeding into the cached
   `m_irModule` on the `Module`.
4. Four `## Phase <N>` sections, each with its own Mermaid
   `flowchart TD` and an ordered table (`# / Pass / File / Gate / Notes`).
   The phases are:
   - **Phase A — AST walk and IR emission.** From `IRModule::create`
     through `lowerFrontEndEntryPointToIR` for every entry point,
     `ensureAllDeclsRec` for every module-decl member, the global
     hashed-string-literal aggregate (`kIROp_GlobalHashedStringLiterals`),
     and the optional `NVAPISlotModifier` decoration. Roughly
     lines 14408-14514.
   - **Phase B — Mandatory pre-optimization transformations.**
     `prelinkIR`, `lowerErrorHandling`, `lowerDefer`,
     `synthesizeBitFieldAccessors`, `lowerExpandType`, and
     `insertDebugValueStore` (gated on `debugInfoLevel >= Standard`).
     Roughly lines 14536-14563.
   - **Phase C — Mandatory optimization passes.** `constructSSA`,
     `applySparseConditionalConstantPropagation`,
     `simplifyCFG` and `peepholeOptimize` (gated on
     `!minimumOptimizations`), per-function `eliminateDeadCode`,
     optional `invertLoops`, and the
     `performMandatoryEarlyInlining` fixed-point loop. Roughly
     lines 14569-14650.
   - **Phase D — Non-essential validation, stripping, and
     finalization.** The `shouldRunNonEssentialValidation` block,
     the stripping block (with the obfuscation-source-map gate),
     `validateIRModuleIfEnabled`, and
     `module->buildMangledNameToGlobalInstMap`. Roughly lines
     14652-14771.
5. `## Conditional gates` — table grouped as in the
   target-pipeline contract:
   - Option-set toggles (`shouldPerformMinimumOptimizations`,
     `shouldRunNonEssentialValidation`, `shouldObfuscateCode`,
     `shouldHaveSourceMap`, `CompilerOptionName::LoopInversion`,
     `CompilerOptionName::TraceCoverage`, `getDebugInfoLevel`).
   - Context predicates (`sink->getErrorCount() != 0` early-exit,
     `ExperimentalModuleAttribute`, `NVAPISlotModifier`).
   - Per-translation-unit predicates
     (`SlangLanguageVersion::SLANG_LANGUAGE_VERSION_2025`).
6. `## Loops in the pipeline` — single subsection. The pre-link
   pipeline has **exactly one** loop: the `for(;;)`
   mandatory-early-inlining fixed-point at lines 14624-14650.
   Document the outer loop's termination condition and the
   per-iteration body. Explicitly state that no other loop
   exists.
7. `## Notable passes` — focused callouts for `prelinkIR`,
   `lowerErrorHandling`, `lowerDefer`,
   `synthesizeBitFieldAccessors`, `lowerExpandType`,
   `performMandatoryEarlyInlining` (and the surrounding loop),
   `stripFrontEndOnlyInstructions` (what "front-end-only" means;
   how obfuscation interacts), `obfuscateModuleLocs`.
8. `## Adjacent constructs` — three short subsections (no
   diagrams; one paragraph each):
   - `prelinkIR` — what it imports
     (`[unsafeForceInlineEarly]` and `externalSymbolsToPrelink`)
     and how it relates to the link step later inside
     `linkAndOptimizeIR`.
   - `SpecializedComponentTypeIRGenContext::process` — pointer to
     [slang-lower-to-ir.cpp](../../../../source/slang/slang-lower-to-ir.cpp)
     line ~14783; note that it produces a small
     "specialized component" IR module and does **not** run the
     mandatory-optimization passes.
   - `TargetProgram::createIRModuleForLayout` — pointer to the
     full deep dive in
     [04c-layout-ir.md](../../pipeline/04c-layout-ir.md);
     one-sentence summary of what it produces.
9. `## See also` — bullets to
   [../../pipeline/03-semantic-check.md](../../pipeline/03-semantic-check.md),
   [../../pipeline/04-ast-to-ir.md](../../pipeline/04-ast-to-ir.md),
   [../../pipeline/05-ir-passes.md](../../pipeline/05-ir-passes.md),
   [../../pipeline/04c-layout-ir.md](../../pipeline/04c-layout-ir.md),
   [../../ir-reference/index.md](../../ir-reference/index.md),
   and the target-pipeline index.

Important caveat for the prompt to flag: the pre-link passes use
plain function calls (`constructSSA(module)`,
`eliminateDeadCode(func, dceOptions)`, etc.), **not** the
`SLANG_PASS(...)` macro that wraps post-link passes. The
generated tables must not imply `SLANG_PASS` lineage.

## Quality checklist

Before considering the document done, verify:

- [ ] Front-matter present with all required keys.
- [ ] Every workspace-relative link resolves.
- [ ] Document under the size cap (in bytes of UTF-8).
- [ ] No speculative claims about code that does not exist.
- [ ] Cross-references to dependency docs use relative paths.
- [ ] No emojis, no editorial commentary, no copied prose.
