---
generated: true
model: claude-opus-4-7
generated_at: 2026-05-20T15:30:00+00:00
source_commit: 330c9a8d807b9f9352e4754f466d1244ae681cff
watched_paths_digest: a6d3a5d9b8d3bbba776f567c8131a79827870f0fa5f74cc44705956d0835d743
source_doc: docs/llm-generated/glossary.md
source_doc_digest: 7d9c229f3f832284dbcebab4fb569e7390a48952bebdf93502dce7fbaecf411a
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Tests for glossary

## Intent

Tests verify the **compiler-internals vocabulary** defined in
[`docs/llm-generated/glossary.md`](../../docs/llm-generated/glossary.md).
The glossary is a lookup aid: most of its entries describe internal
data structures or IR families that have no direct slangc surface
and are therefore not testable through the agentic suite (those go
to `## Untested claims` below). This bundle picks the subset of terms
whose one-paragraph definitions imply a behavior observable via the
slangi interpreter, the diagnostic stream, or text-mode target
emission, and adds one focused test per such term.

The glossary has a single content section `#terms` that contains
every entry, so every test's `doc_ref` and `doc_section_digest`
reference that one anchor; the `purpose` line names the specific
term being verified.

## Functional coverage

| Claim | Intent | Anchor | Tests |
| --- | --- | --- | --- |
| DiagnosticSink: the compiler emits diagnostics through a sink; a use of an undeclared identifier produces an "undefined identifier" error, observable as text on the compile output. | negative | [#terms](../../docs/llm-generated/glossary.md#terms) | [`diagnostic-undeclared-identifier.slang`](diagnostic-undeclared-identifier.slang) |
| entry point: a function selected by `-entry NAME -stage STAGE` runs as the kernel; the byte-code interpreter dispatches `main` and produces its observable output. | functional | [#terms](../../docs/llm-generated/glossary.md#terms) | [`entry-point-stage-flag.slang`](entry-point-stage-flag.slang) |
| inlining: a call instruction is replaced with a copy of the callee's body, so the inlined computation produces the same observable result as the un-inlined form. | functional | [#terms](../../docs/llm-generated/glossary.md#terms) | [`inlining-call-disappears.slang`](inlining-call-disappears.slang) |
| interpreter (slangi): the `slangi` byte-code interpreter runs a compiled program end-to-end; its `printf` output is the user-visible evidence that compilation + lowering + bytecode-emit + execution all succeeded. | functional | [#terms](../../docs/llm-generated/glossary.md#terms) | [`interpreter-runs-byte-code.slang`](interpreter-runs-byte-code.slang) |
| lexer: the lexer converts a source character stream into a token stream, stripping `//` line comments and `/* ... */` block comments as trivia so they are not visible to later stages. | functional | [#terms](../../docs/llm-generated/glossary.md#terms) | [`lexer-comments-stripped.slang`](lexer-comments-stripped.slang) |
| monomorphization / specialization: a generic function is replaced with concrete copies per type argument set, so the emitted target code carries no generic type parameter and computes the concrete result. | functional | [#terms](../../docs/llm-generated/glossary.md#terms) | [`monomorphization-generic-instantiation.slang`](monomorphization-generic-instantiation.slang) |
| name resolution: identifier uses are bound to their declarations taking scope into account; a use of `n` inside a function body refers to the function's parameter, and an outer free identifier resolves to the corresponding declaration at module scope. | functional | [#terms](../../docs/llm-generated/glossary.md#terms) | [`name-resolution-binds-local.slang`](name-resolution-binds-local.slang) |
| overload resolution: among candidates sharing a name, the one whose parameter type matches the argument without a conversion is preferred over the one requiring an implicit conversion. | functional | [#terms](../../docs/llm-generated/glossary.md#terms) | [`overload-resolution-picks-best.slang`](overload-resolution-picks-best.slang) |
| parser: the parser consumes tokens and produces an AST that the checker / lowerer can execute; a nested `if`/`else` statement parses into a control-flow tree that selects the matching branch at run time. | functional | [#terms](../../docs/llm-generated/glossary.md#terms) | [`parser-recursive-descent-parses-if.slang`](parser-recursive-descent-parses-if.slang) |
| preprocessor: the preprocessor handles `#define` macro substitution on the lexer's token stream before parsing, so an identifier defined as a constant is replaced with that constant in later uses. | functional | [#terms](../../docs/llm-generated/glossary.md#terms) | [`preprocessor-define-substitution.slang`](preprocessor-define-substitution.slang) |
| shadowing: a name defined in an inner scope hides a same-named name from an outer scope for the duration of that inner scope. | functional | [#terms](../../docs/llm-generated/glossary.md#terms) | [`shadowing-inner-hides-outer.slang`](shadowing-inner-hides-outer.slang) |
| target: a `TargetRequest` selects a code-generation format; passing `-target hlsl` produces HLSL text containing the entry-point function. | functional | [#terms](../../docs/llm-generated/glossary.md#terms) | [`target-hlsl-emits-hlsl.slang`](target-hlsl-emits-hlsl.slang) |
| target: switching `-target` switches the emitted code-generation format; `-target glsl` yields GLSL with a `#version` directive and a `main()` function. | functional | [#terms](../../docs/llm-generated/glossary.md#terms) | [`target-glsl-emits-glsl.slang`](target-glsl-emits-glsl.slang) |
| type inference: an omitted type on a `var` initialized from an int literal is inferred to `int`; overload resolution then picks the int overload. | functional | [#terms](../../docs/llm-generated/glossary.md#terms) | [`type-inference-var-from-int.slang`](type-inference-var-from-int.slang) |

## Doc gaps observed

(none) — the glossary is a lookup aid that points at peer documents
for the contract surface; in-bundle gaps would be better reported
against those peer documents.

## Untested claims

| Claim | Reason | Anchor | Why untested |
| --- | --- | --- | --- |
| **Serialization back-ends, not user-visible by default:** `fossil format`, `RIFF container`. | (unclassified) | (unspecified) | Reason and explanation to be refined by the next regeneration. |
| **Internal C++ types / helpers with no slangc surface:** `abstract syntax tree`, `ASTBuilder`, `IRBuilder`, `IRInst`, `IROp`, `IRFunc`, `IRModule`, `IRDecoration`, `decoration`, `parent instruction`, `hoistable instruction`, `terminator instruction`, `block parameter`, `single static assignment (SSA)`, `control-flow graph`, `dominator`, `dataflow analysis`, `decl-ref`, `lookup breadcrumb`, `lookup mask`, `lookup options`, `lookup result`, `linkage`, `session`, `scope`, `source-loc`, `FIDDLE`, `intermediate representation` (the concept; specific IR text is target-specific and unstable for FileCheck). | needs-unit-test | [#astbuilder](../../docs/llm-generated/glossary.md#astbuilder) | No slangc CLI surface reaches this. A C++ unit test in `tools/slang-unit-test/` could exercise the relevant compiler internals directly. |
| **IR / type-system vocabulary owned by other bundles:** `existential type`, `witness table`, `differential pair`, `capability atom`, `profile`, `layout IR module`, `mandatory optimization pass`, `target legalization driver`, `target intrinsic`, `core module`, `prelude`, `module` (the `IModule` C++ object; `import` semantics belong to the language-reference module bundle), `translation unit` (the multi-file flag belongs to architecture/overview), `syntax-decl`, `partial generic application`, `transparent member`, `two-stage parsing`, `recursive descent` (the technique; the parsing behavior is covered by `parser-recursive-descent-parses-if.slang`), `dead-code elimination` (the IR pass is real but no contract-stable text artifact ties to it through `-dump-ir`), `conversion cost` (the numeric scoring is internal; the observable consequence — implicit-conversion rejection — is owned by `name-resolution/overload-resolution`). | out-of-bundle | [#profile](../../docs/llm-generated/glossary.md#profile) | Covered by a sibling bundle; see the appropriate `tests-agentic/<sibling>/` directory. |
| **Already covered by the user-facing visibility bundle:** `visibility`. | out-of-bundle | [#visibility](../../docs/llm-generated/glossary.md#visibility) | Covered by a sibling bundle; see the appropriate `tests-agentic/<sibling>/` directory. |
