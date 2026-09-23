---
gap_intake_report: true
intake_model: "claude-opus-5[1m]"
intake_at: 2026-08-11T16:30:18Z
target_doc: syntax-reference/keywords-and-builtins.md
target_doc_source_commit_before: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_source_commit_after: ec47ea72b6aa5fefc3b36f8a780dbd3ecf5b1f6e
gap_count: 11
actions:
  fixed: 11
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated_to_finding: 0
---

# Gap-intake report for syntax-reference/keywords-and-builtins.md

## Summary

Nothing was escalated: no observation in this queue contradicted the
watched source, and every gap turned out to be the document omitting a
form the parser or a meta-module states outright. All eleven were fixed,
none rejected and none deferred. The queue was dominated by one shape of
complaint — a table cell naming a parse callback and nothing else — so
most fixes are a single paragraph or bullet list appended to the table
that gives the form each callback actually reads, derived from the
callback bodies in `source/slang/slang-parser.cpp` and cross-checked
against real usages in the `*.meta.slang` sources. Two
fixes are deliberately narrower than the gap asked: `1d8770e795d5`
(which targets accept `SV_WaveIndex`) and `9a1a7ad263e3` (a
user-observable-effect column for the simple modifiers) both hinge on
files outside `watched_paths`, so the document now states the part the
watched source proves and routes the rest to the page that owns it
rather than asserting an unbacked target list.

Operator follow-ups, none blocking:

1. `source/slang/slang-ir-legalize-varying-params.cpp` is the single
   source of truth for which targets accept a system-value semantic
   (`SystemValueSemanticName::WaveIndex` at `:4333` for Metal and
   `:4970` for WGSL). Adding it to `watched_paths` would let the
   `SV_WaveIndex` bullet name the accepting targets outright instead of
   linking the pass. Same file would settle any future `SV_*` gap.
2. Possible compiler enhancement, not a defect: `WaveGetWaveIndex()`
   lowers to `SubgroupId` on SPIR-V
   (`source/slang/hlsl.meta.slang:17320-17325`), yet the `SV_WaveIndex`
   semantic has no GLSL/SPIR-V case in the varying-param legalizer, so
   the entry-point spelling is rejected there while the function form
   works. Nothing in the watched source promises the semantic on those
   targets, so this is a missing mapping rather than drift; recorded
   here in case the tests side wants it as an enhancement request.
3. The document's line-number citations are stale by roughly +8 in the
   `slang-parser.cpp` regions below line ~7000 and +19 above line
   ~9600, measured against the watched files at
   `ec47ea72b6aa5fefc3b36f8a780dbd3ecf5b1f6e` (identical to `HEAD` for
   all seven watched paths). This intake deliberately left every
   existing number untouched and added no new ones, so `mark-fresh`
   will bless the drift; re-deriving the table is a remediation-pass
   job, not a gap-intake one.
4. The `-experimental-feature` half of `0ff3440b9c31` needed no edit:
   `source/slang/slang-options.cpp:1222-1225` declares the option with
   a `nullptr` value spec, so it is a plain on/off flag and the
   document's existing "gated by `-experimental-feature`" already is
   the exact invocation. The fact was left out of the document because
   `slang-options.cpp` is outside `watched_paths`.

## Actions

| Gap ID | Action | Evidence | Fix summary |
| --- | --- | --- | --- |
| f848c4fac6e9 | fixed | `source/slang/slang-parser.cpp:10281-10383` — `shared` / `volatile` / `coherent` / `restrict` / `readonly` / `writeonly` read no tokens (`shared` branches on `allowGLSLInput` at `:10287`, `volatile` builds both nodes and diagnoses by language version at `:10302-10318`); `:10545-10550` `hitAttributeEXT` likewise; `:10385-10501` `layout` reads a parenthesized `name` / `name = expr` list; `:10104-10279` and `:10552-10680` give the per-modifier argument shapes, with optional parentheses at `:10123`, `:10138`, `:10178`, `:10641`. Spellings cross-checked in `source/slang/glsl.meta.slang:7630`, `:10018`, `source/slang/hlsl.meta.slang:17302-17303`, `:26138`, `source/slang/core.meta.slang:4160`. | added two paragraphs after the table: the six keywords that take no arguments and why their callbacks exist, then the argument shape of every row that does, including the four whose parentheses are optional |
| 1b40bd3f3dc7 | fixed | `source/slang/hlsl.meta.slang:27609-27610` declares `static const __ResourceDescriptorHeapType ResourceDescriptorHeap = {};` and the sampler equivalent; `:27591-27607` the two `__subscript`s; `:27450-27456` the generated `__implicit_conversion __init(UntypedResourceHandle h)` on every heap-castable resource type (`:27384-27390` for textures). Bundle test `builtin-resource-descriptor-heap-multitarget.slang:36` uses `RWStructuredBuffer<uint> heapBuf = ResourceDescriptorHeap[0];`. | split the descriptor-heap material into its own bullet naming `ResourceDescriptorHeap` / `SamplerDescriptorHeap` as the `static const` globals, and gave the assignment form that recovers the concrete resource type |
| 1d8770e795d5 | fixed | `source/slang/hlsl.meta.slang:15-16` declares `SV_WaveIndex` / `SV_GroupIndex` as semantics on the hidden `in` globals `__builtinWaveIndex` / `__builtinGroupIndex`, with no `[require(...)]`; `:17302-17351` declares `WaveGetWaveIndex()` with `[require(cuda_glsl_hlsl_metal_spirv_wgsl, subgroup_workgroup_index)]` and a `__target_switch` arm per target. The accept/reject list per target lives in `source/slang/slang-ir-legalize-varying-params.cpp` (`:4333`, `:4970`), outside `watched_paths`, so it is linked rather than asserted. | rewrote the wave-builtin sentence to separate the capability-gated function from the two ungated system-value semantics, named varying-param legalization as what decides their per-target availability, and pointed at `WaveGetWaveIndex()` as the portable spelling |
| 0ff3440b9c31 | fixed | `source/slang/slang-parser.cpp:4329-4367` documents and implements both `__constraint` forms (`== ` equality, `:` subtype) plus the `interface IDerived : IBase { __constraint DataType == This; }` example; `:4370-4382` shows `__associatedfunc` is a type expression followed by a decl name; `:4244-4298` shows `__func_extension` reads a syntax-decl-dispatched target expression, a modern param list, an optional `throws`, an optional `->`, and a body. Usages: `source/slang/core.meta.slang:724` and `source/slang/hlsl.meta.slang:31682-31688`. | added a three-bullet list after the decl table giving the written form of each, each anchored to a real core-module usage |
| 80abe4cc4205 | fixed | `source/slang/slang-parser.cpp:3255-3269` reads `(`, comma-separated `ParseTypeExp` list, `)`, a mandatory `->`, and a result type; `:3470-3474` is the type-specifier lookahead; `:1662-1670` accepts `functype F` as a generic parameter. Usage `source/slang/hlsl.meta.slang:28710`. Bundle test `decl-functype-type-specifier.slang` passes `addOne` to a `functype(int) -> int` parameter. | gave the `functype(<parameter types>) -> <result type>` spelling, a core-module usage, the fact that such a parameter accepts a function name, and the `functype F` generic-parameter form; callability of a function-typed value is decided in the checker and is deliberately not claimed |
| bbc3d4c97d7b | fixed | `source/slang/slang-parser.cpp:6441-6455` — `ParseClass` reads a required name, an optional inheritance clause, and a body, and nothing else; `:6370-6439` — `ParseStruct` additionally handles a synthesized name for the anonymous form (`:6400-6408`), `parseOptGenericDecl` (`:6409`), the `= T` alias form (`:6416-6429`), and the body-less form (`:6430-6434`). | added a paragraph contrasting the two at the parse level, noting that a generic aggregate must be a `struct`, and that the parser leaves every other consequence of the choice to later stages |
| 131453a3ff0f | fixed | `source/slang/slang-parser.cpp:3141-3236` (`fwd_diff` / `bwd_diff` / `__apply` / `__func_as_type` one operand, `__dispatch_kernel` three), `:7996-8001` (`__return_val` none), `:8030-8090` (`sizeof` / `alignof` one plus optional data layout, `countof` exactly one), `:8092-8168` (pack queries one, `__shapePermute` / `__shapeReduce` two, `__shapeConcat` / `__shapeSwap` / `__packBranch` three), `:8170-8194` (`__getAddress`, `__floatAsInt`), `:8196-8212` (`try` / `no_diff` take a bare leaf expression), `:9705-9720` (`new` over a postfix expression). `Std140DataLayout` exists at `source/slang/hlsl.meta.slang:45`. | added a bullet list after the table giving the operand count and parenthesization of every row, grouped so the rows that share a shape share a line |
| a2d9ef39a8a4 | fixed | `source/slang/slang-parser.cpp:5205-5285` gives the `syntax <name> [: <class>] [= <existing>];` grammar and the alias behaviour (`:5249-5259`); `:5543-5609` gives the `attribute_syntax [<Name>(<param> : <Type>...)] : <class>;` grammar. Both resolve the class through `ASTBuilder::findSyntaxClass` (`:5227`, `:5587`; declared `source/slang/slang-ast-builder.h:847-855`). Real declarations at `source/slang/core.meta.slang:28` and `:128`; a tree-wide search for the `syntax X = Y;` alias form over `*.slang` finds no use. | added a paragraph giving both grammars with core-module examples, and the reason redefinition is core-module-only in practice: the class name must already exist in the compiler, so the extension point is a spelling, not a node |
| aa48d92eed58 | fixed | `source/slang/slang-parser.cpp:2505-2546` — `isReservedKeywordName` covers exactly `struct`, `class`, `enum`, `typealias`, `typedef` and only warns; its comment states that almost every other keyword is contextual and shadowable. `:5890` is the parser's one inspection of `gl_` (GLSL interface-block redeclaration, replaced with an `EmptyDecl`); no occurrence of `SV_` anywhere in the parser. `:10733-10742` registers only the `__`-prefixed `__init` / `__subscript` / `__include`. `source/slang/glsl.meta.slang:117` shows `gl_Position` is an ordinary declaration. Bundle test `reserved-init-subscript-include-plain-identifiers.slang` calls user functions named `init` / `subscript` / `include`. | replaced the one-line closing with the actual enforcement story: prefixes are advisory with no diagnostic, the single `gl_` special case, the `KeywordUsedAsName` warning and its five spellings, and the bare `init` / `subscript` / `include` illustration |
| 9a1a7ad263e3 | fixed | `source/slang/slang-parser.cpp:10698-10708` — every simple-modifier row uses the `getSyntaxClass<...>()` overload whose callback is `parseSimpleSyntax` (`:5198-5202`), which constructs the node and reads nothing. The user-observable effects the gap asks for are owned by `docs/generated/design/ast-reference/modifiers.md` (its `### Matrix layout modifiers` at `:201-214` covers `row_major` / `column_major`) and would otherwise require `slang-check-*.cpp` / `slang-emit-*.cpp`, none of which are in this page's `watched_paths`. | stated that the table is a parse-time binding only, and routed the user-observable effect of each node class to `../ast-reference/modifiers.md`, which owns it; the effect column itself is intentionally not duplicated here |
| fb6c0e3e7b12 | fixed | `source/slang/slang-parser.cpp:6862-6906` — `parseCompileTimeForStmt` reads the loop variable, the literal tokens `in` and `Range`, and one or two range expressions; `:6611-6717` `__stage_switch` shares `parseTargetSwitchStmtImpl` with `__target_switch` and resolves case labels via `findCapabilityName`; `:6719-6735` `__intrinsic_asm "<text>"` plus optional args; `:6737-6760` `__GPU_FOREACH` with the literal `LAMBDA` token (the source states the form verbatim at `:6739-6741`); `:7620-7650` `__requireCapability(<cap>, ...);`. Bundle test `statement-compile-time-for.slang` uses `$for(i in Range(4))`; `source/slang/hlsl.meta.slang:17312` and `:17330-17337` show the `__intrinsic_asm` and `__stage_switch` forms in use. | put the `$for(i in Range(N))` header (including the two-argument `Range(begin, end)`) in the `for` row, and added a bullet list giving the fixed form of each compiler-internal statement keyword |
