---
generated: true
model: claude-opus-4-8[1m]
generated_at: 2026-06-11T15:00:00+00:00
source_commit: ef1068b5485e09b3a7afadba2e25f9541e29af42
watched_paths_digest: 42f4b911f9253e6d36391e4734dc07282f032209ff1f8ab27d77fa319acb0e13
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Tests for coverage/parser

White-box characterization tests targeting under-exercised branches of the
recursive-descent parser in `source/slang/slang-parser.cpp` (~68% covered).
These pin the compiler's _current observed behaviour_ (not a spec). The
manifest `source_doc` is the parser source file itself; the white-box target is
also named in each test's `//META: covers=` field. The `//META: doc_ref` points
at the LLM-derived `pipeline/02-parse-ast.md` design doc only to satisfy the
shared `//META` shape — the authority for this tree is the source, per
`coverage/METHODOLOGY.md`.

## Intent

Drive syntax edge cases from the hint area — operator declarations, subscript
declarations, attribute placement, GLSL layout qualifiers, generic-argument
angle-bracket disambiguation, expression-statement forms, literal suffixes, and
parse-error recovery — through `slangc`/`slangi` and pin the observed result.
Parse-error paths use `//DIAGNOSTIC_TEST` (they validate locally even without
FileCheck); positive parse forms (the success side of operator overloading, the
`>>` split, the comma operator) use `//TEST:INTERPRET` so the interpreter
validates the value locally. Every diagnostic code and message below was copied
verbatim from a real `slangc` run at `source_commit`.

## Functional coverage

| Test                                                                                                     | What it pins                                                                                                                                                | covers=                       |
| -------------------------------------------------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------- | ----------------------------- |
| [`operator-decl-invalid-symbol.slang`](operator-decl-invalid-symbol.slang)                               | An `operator @` declaration with an unrecognized symbol is rejected with E20008 ("invalid operator '@'.").                                                  | source/slang/slang-parser.cpp |
| [`operator-overload-plus-lowers.slang`](operator-overload-plus-lowers.slang)                             | A user `operator+` parses, binds, and `a + b` evaluates to the overload's result (2+3 = 5) under the interpreter.                                           | source/slang/slang-parser.cpp |
| [`subscript-missing-return-type.slang`](subscript-missing-return-type.slang)                             | A `__subscript` with no `-> ReturnType` clause is rejected with E30901.                                                                                     | source/slang/slang-parser.cpp |
| [`empty-parentheses-expression.slang`](empty-parentheses-expression.slang)                               | An empty `()` in value position is rejected with E20005 ("empty parenthesis '()' is not a valid expression.").                                              | source/slang/slang-parser.cpp |
| [`unintended-empty-statement.slang`](unintended-empty-statement.slang)                                   | A lone `;` body for an `if` emits the W20101 (E20101) unintended-empty-statement warning and still compiles.                                                | source/slang/slang-parser.cpp |
| [`unexpected-body-after-semicolon.slang`](unexpected-body-after-semicolon.slang)                         | A `void foo(); {}` signature-plus-body is rejected with E20102 ("is this ';' a typo?").                                                                     | source/slang/slang-parser.cpp |
| [`decl-not-allowed-in-statement.slang`](decl-not-allowed-in-statement.slang)                             | A `namespace` declaration inside a function body is rejected with E30102 ("namespace is not allowed here.").                                                | source/slang/slang-parser.cpp |
| [`anonymous-scoped-enum.slang`](anonymous-scoped-enum.slang)                                             | An unnamed `enum class { ... }` is rejected with E32001 (anonymous scoped enum not allowed).                                                                | source/slang/slang-parser.cpp |
| [`glsl-layout-qualifier-unrecognized.slang`](glsl-layout-qualifier-unrecognized.slang)                   | An unknown identifier inside `layout(...)` is rejected with E31217.                                                                                         | source/slang/slang-parser.cpp |
| [`struct-bracket-attribute-placement-2026.slang`](struct-bracket-attribute-placement-2026.slang)         | A bracketed attribute list placed after `struct` is rejected with E31205 under `-std 2026`.                                                                 | source/slang/slang-parser.cpp |
| [`invalid-integer-literal-suffix.slang`](invalid-integer-literal-suffix.slang)                           | An integer literal with an unrecognized suffix (`5q`) is rejected with the invalid-suffix diagnostic (E39999).                                              | source/slang/slang-parser.cpp |
| [`invalid-float-literal-suffix.slang`](invalid-float-literal-suffix.slang)                               | A float literal with an unrecognized suffix (`1.0z`) is rejected with the invalid-suffix diagnostic (E39999); the compiler emits it twice (non-exhaustive). | source/slang/slang-parser.cpp |
| [`declaration-declares-nothing.slang`](declaration-declares-nothing.slang)                               | A type-only declaration with no declarator (`int;`) is rejected with E39999 ("declaration does not declare anything").                                      | source/slang/slang-parser.cpp |
| [`nested-generic-angle-bracket-disambiguation.slang`](nested-generic-angle-bracket-disambiguation.slang) | A trailing `>>` in `Array<Array<int,2>,3>` is split into two generic closers; the nested read returns 9 under the interpreter.                              | source/slang/slang-parser.cpp |
| [`comma-expression-statement.slang`](comma-expression-statement.slang)                                   | A comma operator at statement top level (`a=1, b=2;`) parses and both assignments run (a+b == 3) under the interpreter.                                     | source/slang/slang-parser.cpp |

## Unreachable gaps

| Gap                                                                                                                                                                 | Why not targeted                                                                                                                                                                                                                                                                       |
| ------------------------------------------------------------------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| SPIR-V inline-assembly parse diagnostics (`UnrecognizedSpirvOpcode`, `SpirvInstructionWithoutResultId`, `SpirvOperandRange`, `SpirvInstructionWithTooManyOperands`) | Reachable but reach a narrow `spirv_asm { ... }` embedded-assembly sub-grammar; deferred to keep this bundle on the named hint area (general syntax / operators / attributes / layout / literals / recovery). A dedicated spirv-asm-parser coverage bundle would fit them better.      |
| Target/capability/version parse diagnostics (`UnknownTargetName`, `UnknownCapability`, `UnknownLanguageVersion`, `InvalidSpirvVersion`, `InvalidCudaSmVersion`)     | Reachable only through `__target_switch` / capability-atom / version-pragma sub-grammars whose inputs are environment- and target-specific; out of scope for the syntax-edge-case hint.                                                                                                |
| `Unimplemented` parse sites                                                                                                                                         | Defensive markers for partially-parsed constructs; emitting them depends on internal feature flags rather than a stable surface input, so they are not a reliable characterization target.                                                                                             |
| `IntegerLiteralTooLarge` (E10012, legacy non-`E`-prefixed catalog format)                                                                                           | Reachable (a >64-bit literal triggers it), but it is emitted in the legacy diagnostic-catalog format (`file(line): error 10012:`) rather than the rich `E####` form, which the DIAGNOSTIC_TEST annotation matcher pins less reliably; left out to avoid a fragile text-position match. |

## Doc gaps observed

| Anchor                                                                         | Kind                  | Gap                                                                                                                                                                                                                                                                                   | Suggested addition                                                                                                                                                                                                      |
| ------------------------------------------------------------------------------ | --------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| [#modifier-parsing](../../../design/pipeline/02-parse-ast.md#modifier-parsing) | undocumented-behavior | The section describes modifier and attribute parsing generically but does not mention that a bracketed attribute list placed _after_ the `struct` keyword is version-gated: silently accepted before 2025, a deprecation warning (E31204) in 2025, and a hard error (E31205) in 2026. | Add a note (or a small version-by-behaviour table) under modifier parsing stating that post-keyword `struct [attr] Name` placement is deprecated in 2025 and removed in 2026, naming the diagnostic each version emits. |
| [#failure-modes](../../../design/pipeline/02-parse-ast.md#failure-modes)       | undocumented-behavior | The failure-modes section does not list the statement-context declaration restriction: a `namespace` (and similarly `import`) is accepted only at file/module scope and emits E30102 ("... is not allowed here.") when written inside a function body.                                | Add a row naming E30102 and the set of declaration kinds that are file/module-scope-only, so a reader knows which declarations the statement parser rejects.                                                            |
