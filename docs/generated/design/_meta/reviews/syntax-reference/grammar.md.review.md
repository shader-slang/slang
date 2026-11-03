---
review_report: true
reviewer_model: gpt-6
reviewed_at: 2026-11-03T06:39:47Z
target_doc: syntax-reference/grammar.md
target_doc_source_commit: 2518a942d59b1228fc8cc70a37739baab82b6002
target_doc_watched_paths_digest: 84e367782040254240d2878794cbff2815f0a6ccd2df6af1a236785a547a7918
source_commit: c63c2fbd5115a98b2b2469f43618671653d40bf1
checklist:
  factual_accuracy: fail
  cross_references: pass
  completeness: partial
  style_consistency: partial
  source_alignment: fail
  front_matter_validity: pass
finding_count: 8
severity_breakdown:
  critical: 3
  major: 2
  minor: 3
  nit: 0
---

# Review report for syntax-reference/grammar.md

## Summary

The document covers the required topics, but needs correction before tooling developers can rely on its grammar. Most importantly, several declaration productions omit the mandatory angle brackets surrounding generic parameters. The explanations of generic disambiguation and attribute lookup also contradict the implementation.

## Items checked

- Reviewed against the **current working tree**, which matches the document’s recorded source commit throughout `source/`; the source comparison showed no differences.
- Read `_review.md`, `_common.md`, and `syntax-grammar.md`, and ran `regenerate.py show syntax-reference/grammar.md`.
- Spot-checked more than 20 claims, including generic delimiters and lookahead, attribute parsing, function types, recursive declarators, literal subclasses, string concatenation, character literals, import-path decoding, struct aliases, parameter inference, experimental-feature gating, and compile-time loop checking.
- Verified all 18 cited source locations, counting the six individual `genericDepth` locations separately. The cited locations match the stated code.
- Resolved all 31 Markdown links, covering 13 distinct targets; confirmed all five linked generated pages appear in the manifest. Consulted dependency pages only for cross-reference checks.
- Checked required headings, production definitions and parser citations, front-matter keys and formats, and Liquid safety. Measured 61,943 bytes against the manifest’s 65,536-byte cap and the prompt’s separate 48 KB requirement.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | critical | Declarations, lines 180–200 and 300–326; Generics, line 890 | Multiple declaration rules use bare `GenericParams?`, although `GenericParams` contains only the comma-separated parameters. Consequently, the grammar omits `<` and `>` from ordinary generic structs, interfaces, enums, functions, constructors, subscripts, and function extensions. | `source/slang/slang-parser.cpp:1867` selects the generic path only upon `OpLess`; `ParseGenericDeclImpl` explicitly reads `OpLess` at line 1818 and `OpGreater` at line 1850. | Replace these uses with `('<' GenericParams '>')?`, consistently with the existing type-alias and extension productions, or introduce a separately defined bracketed parameter-list rule. |
| F-002 | critical | `<` disambiguation, lines 775–782 | The assertion that a semantic visitor “skips the speculative parse entirely” and that “Anything else is treated as a comparison” is false. Semantic checking can leave `baseKind` as `Unknown`, which still reaches speculative parsing. | `source/slang/slang-parser.cpp:3079–3177`: `baseKind` starts as `Unknown`; only recognized shapes change it. The comment near line 3137 explicitly describes an unknown `LetExpr`, such as `h.o.get<4>()`; the unknown path subsequently invokes `parseGenericApp(&newParser, base)`. | Explain the three outcomes: known generic commits, known non-generic returns the comparison base, and unknown falls through to speculative parsing and the follower-token test, even with a semantic visitor. |
| F-003 | critical | Attributes and decorations, lines 871–873 | “The same syntax-decl lookup the parser uses for keyword modifiers” incorrectly describes attribute resolution. Parsing constructs an unchecked attribute and defers lookup; attributes are subsequently resolved using an attribute-specific lookup mask. | `source/slang/slang-parser.cpp:1046–1062` explicitly says lookup of an `AttributeDecl` is deferred rather than performed like `SyntaxDecl` lookup. `source/slang/slang-check-modifier.cpp:163–179` implements `lookUpAttributeDecl` using `LookupMask::Attribute`. | State that parsing records the normalized name, arguments, and scope in `UncheckedAttribute`; semantic checking resolves the corresponding attribute declaration. Retain the explanation that `attribute_syntax` supplies attribute declarations. |
| F-004 | major | Notation and productions, especially lines 418–420, 448, 515, 669, and 832 | The grammar uses undefined non-terminals, despite declaring that UpperCamelCase names are defined in the document. Examples include `Operator`, `RegToken`, `CaseStmt`, `DefaultStmt`, and `ModifierArgs`. Other placeholders, such as `GenericSpecialization`, have only informal comments instead of definitions. | `docs/generated/design/_meta/prompts/syntax-grammar.md:75–85` requires verifiable productions and parser citations for every non-terminal. Implementations include `ParseDeclName` at `source/slang/slang-parser.cpp:1490`, `ParseSemantic` at line 4138, and `ParseCaseStmt`/`ParseDefaultStmt` at lines 6940–6960. | Define the missing rules with their parser functions. Reuse `GenericArgs` for `GenericSpecialization` if intended, and explicitly distinguish deliberately abbreviated token sequences from defined non-terminals. |
| F-005 | major | Productions throughout; examples at lines 223, 414, 655, 804, and 863 | The mandatory per-production implementation citations are incomplete. `TypeList`, `ArraySuffix`, `UnaryOp`, `ModifierList`, and `ArgList`, among others, lack their own parser-function attribution. A general link to the parser does not satisfy the explicit per-non-terminal requirement. | `docs/generated/design/_meta/prompts/syntax-grammar.md:77–85`: “When you state a production, parenthetically cite the parser function” and “Every non-terminal cites at least one parser function.” Relevant implementations include `ParseModifiers` at `source/slang/slang-parser.cpp:1279`, `parseDirectAbstractDeclarator` at line 2636, and `parsePrefixExpr` at line 10085. | Audit every production and attach its implementing function or explicit delegation chain, including helper rules and grouped alternatives. |
| F-006 | minor | Types, line 994 | The function-type production requires at least one parameter type, excluding the supported spelling `functype() -> int`. | `source/slang/slang-parser.cpp:3403–3416`: `parseFuncTypeExpr` checks for the closing parenthesis before parsing the first parameter. | Make the entire parameter-type list optional: `'functype' '(' (Type (',' Type)*)? ')' '->' Type`. |
| F-007 | minor | Variable / binding declarations, line 417 | `Declarator ::= '*'? ...` allows only one unparenthesized pointer prefix, whereas the parser recursively accepts multiple prefixes. | `source/slang/slang-parser.cpp:2784–2803`: after consuming `*`, `parseDeclarator` calls itself for the inner declarator at line 2796. | Express the pointer branch recursively, such as `Declarator ::= '*' Declarator` plus the direct-declarator alternative. |
| F-008 | minor | Whole document | At 61,943 bytes, the document exceeds the per-document prompt’s “under 48 KB” requirement, although it fits the subsequently enlarged manifest cap. | `docs/generated/design/_meta/prompts/syntax-grammar.md:96–98` explicitly requires length under 48 KB; `docs/generated/design/_meta/manifest.yaml:387–392` records the larger 65,536-byte cap. | Condense explanatory prose while preserving required productions and citations to meet the prompt’s limit. If the larger limit is intended to supersede it, reconcile that contract discrepancy before treating size compliance as complete. |