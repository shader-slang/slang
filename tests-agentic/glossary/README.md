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
to `## Out of scope` below). This bundle picks the subset of terms
whose one-paragraph definitions imply a behavior observable via the
slangi interpreter, the diagnostic stream, or text-mode target
emission, and adds one focused test per such term.

The glossary has a single content section `#terms` that contains
every entry, so every test's `doc_ref` and `doc_section_digest`
reference that one anchor; the `purpose` line names the specific
term being verified.

## Claims enumerated

| Claim ID | Term                            | Anchor                                                            | Claim (one line)                                                                                            | Tests                                          |
| -------- | ------------------------------- | ----------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------------- | ---------------------------------------------- |
| C-01     | entry point                     | [#terms](../../docs/llm-generated/glossary.md#terms)              | A function plus a pipeline stage defines a kernel that the runner dispatches.                               | [`entry-point-stage-flag.slang`](entry-point-stage-flag.slang)                 |
| C-02     | lexer                           | [#terms](../../docs/llm-generated/glossary.md#terms)              | The lexer strips `//` and `/* */` comments as trivia before later stages see them.                          | [`lexer-comments-stripped.slang`](lexer-comments-stripped.slang)                |
| C-03     | preprocessor                    | [#terms](../../docs/llm-generated/glossary.md#terms)              | `#define` macro substitution happens before parsing, so a defined identifier is replaced with its value.    | [`preprocessor-define-substitution.slang`](preprocessor-define-substitution.slang)       |
| C-04     | parser                          | [#terms](../../docs/llm-generated/glossary.md#terms)              | The parser produces an AST that the rest of the pipeline can execute; nested `if`/`else` parses correctly.  | [`parser-recursive-descent-parses-if.slang`](parser-recursive-descent-parses-if.slang)     |
| C-05     | name resolution                 | [#terms](../../docs/llm-generated/glossary.md#terms)              | Identifier uses bind to the declarations they refer to, taking scope into account.                          | [`name-resolution-binds-local.slang`](name-resolution-binds-local.slang)            |
| C-06     | shadowing                       | [#terms](../../docs/llm-generated/glossary.md#terms)              | A name defined in an inner scope hides a same-named name from an outer scope.                               | [`shadowing-inner-hides-outer.slang`](shadowing-inner-hides-outer.slang)            |
| C-07     | overload resolution             | [#terms](../../docs/llm-generated/glossary.md#terms)              | Among candidates sharing a name, the better-matching one is preferred.                                      | [`overload-resolution-picks-best.slang`](overload-resolution-picks-best.slang)         |
| C-08     | type inference                  | [#terms](../../docs/llm-generated/glossary.md#terms)              | An omitted type on `var` is inferred from the initializer.                                                  | [`type-inference-var-from-int.slang`](type-inference-var-from-int.slang)            |
| C-09     | DiagnosticSink                  | [#terms](../../docs/llm-generated/glossary.md#terms)              | The compiler routes diagnostics through a sink; an undefined identifier produces an "undefined identifier". | [`diagnostic-undeclared-identifier.slang`](diagnostic-undeclared-identifier.slang)       |
| C-10     | inlining                        | [#terms](../../docs/llm-generated/glossary.md#terms)              | A call is replaced with a copy of the callee's body; the inlined form produces the same observable result.  | [`inlining-call-disappears.slang`](inlining-call-disappears.slang)               |
| C-11     | monomorphization, specialization | [#terms](../../docs/llm-generated/glossary.md#terms)             | Generic functions are specialized to concrete type-argument copies.                                         | [`monomorphization-generic-instantiation.slang`](monomorphization-generic-instantiation.slang) |
| C-12     | target                          | [#terms](../../docs/llm-generated/glossary.md#terms)              | A target selects a code-generation format; `-target hlsl` produces HLSL text.                               | [`target-hlsl-emits-hlsl.slang`](target-hlsl-emits-hlsl.slang)                 |
| C-13     | target                          | [#terms](../../docs/llm-generated/glossary.md#terms)              | A target selects a code-generation format; `-target glsl` produces GLSL text.                               | [`target-glsl-emits-glsl.slang`](target-glsl-emits-glsl.slang)                 |
| C-14     | interpreter (slangi)            | [#terms](../../docs/llm-generated/glossary.md#terms)              | The byte-code interpreter runs a compiled program end-to-end.                                               | [`interpreter-runs-byte-code.slang`](interpreter-runs-byte-code.slang)             |

## Tests in this bundle

| File                                           | Intent     | Doc anchor |
| ---------------------------------------------- | ---------- | ---------- |
| [`entry-point-stage-flag.slang`](entry-point-stage-flag.slang)                 | functional | `#terms`   |
| [`lexer-comments-stripped.slang`](lexer-comments-stripped.slang)                | functional | `#terms`   |
| [`preprocessor-define-substitution.slang`](preprocessor-define-substitution.slang)       | functional | `#terms`   |
| [`parser-recursive-descent-parses-if.slang`](parser-recursive-descent-parses-if.slang)     | functional | `#terms`   |
| [`name-resolution-binds-local.slang`](name-resolution-binds-local.slang)            | functional | `#terms`   |
| [`shadowing-inner-hides-outer.slang`](shadowing-inner-hides-outer.slang)            | functional | `#terms`   |
| [`overload-resolution-picks-best.slang`](overload-resolution-picks-best.slang)         | functional | `#terms`   |
| [`type-inference-var-from-int.slang`](type-inference-var-from-int.slang)            | functional | `#terms`   |
| [`diagnostic-undeclared-identifier.slang`](diagnostic-undeclared-identifier.slang)       | negative   | `#terms`   |
| [`inlining-call-disappears.slang`](inlining-call-disappears.slang)               | functional | `#terms`   |
| [`monomorphization-generic-instantiation.slang`](monomorphization-generic-instantiation.slang) | functional | `#terms`   |
| [`target-hlsl-emits-hlsl.slang`](target-hlsl-emits-hlsl.slang)                 | functional | `#terms`   |
| [`target-glsl-emits-glsl.slang`](target-glsl-emits-glsl.slang)                 | functional | `#terms`   |
| [`interpreter-runs-byte-code.slang`](interpreter-runs-byte-code.slang)             | functional | `#terms`   |

## Doc gaps observed

(none) — the glossary is a lookup aid that points at peer documents
for the contract surface; in-bundle gaps would be better reported
against those peer documents.

## Out of scope

The glossary defines many terms whose one-paragraph entries do not
imply an externally observable behavior reachable via slangc on a
no-GPU runner. These were considered and dropped after one or more
attempts to find a clean observable; they belong to their respective
peer-document bundles (or to no test at all).

- **Internal C++ types / helpers with no slangc surface:**
  `abstract syntax tree`, `ASTBuilder`, `IRBuilder`, `IRInst`,
  `IROp`, `IRFunc`, `IRModule`, `IRDecoration`, `decoration`,
  `parent instruction`, `hoistable instruction`,
  `terminator instruction`, `block parameter`,
  `single static assignment (SSA)`, `control-flow graph`,
  `dominator`, `dataflow analysis`, `decl-ref`,
  `lookup breadcrumb`, `lookup mask`, `lookup options`,
  `lookup result`, `linkage`, `session`, `scope`, `source-loc`,
  `FIDDLE`, `intermediate representation` (the concept; specific IR
  text is target-specific and unstable for FileCheck).
- **Serialization back-ends, not user-visible by default:**
  `fossil format`, `RIFF container`.
- **IR / type-system vocabulary owned by other bundles:**
  `existential type`, `witness table`, `differential pair`,
  `capability atom`, `profile`, `layout IR module`,
  `mandatory optimization pass`, `target legalization driver`,
  `target intrinsic`, `core module`, `prelude`, `module`
  (the `IModule` C++ object; `import` semantics belong to
  the language-reference module bundle), `translation unit`
  (the multi-file flag belongs to architecture/overview),
  `syntax-decl`, `partial generic application`,
  `transparent member`, `two-stage parsing`,
  `recursive descent` (the technique; the parsing behavior is
  covered by `parser-recursive-descent-parses-if.slang`),
  `dead-code elimination` (the IR pass is real but no contract-stable
  text artifact ties to it through `-dump-ir`),
  `conversion cost` (the numeric scoring is internal; the
  observable consequence — implicit-conversion rejection — is
  owned by `name-resolution/overload-resolution`).
- **Already covered by the user-facing visibility bundle:**
  `visibility`.
