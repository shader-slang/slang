---
generated: true
model: claude-opus-4-7
generated_at: 2026-06-01T15:00:00+00:00
source_commit: d25453d7f0b4867db4cb5eabf34fb6cd088cf596
watched_paths_digest: e2a8ce71db0094ec32afc0973c06c065ba0b9981e03e492197a7d6862571f9f1
source_doc: docs/language-reference/lexical-structure.md
source_doc_digest: 966a201fda28c775172186dbde06cb14891d584c984af5c90890d8c08ff9e353
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Tests for language-reference/lexical-structure

## Intent

Tests verify the lexer-level claims in the **language reference** at
[`docs/language-reference/lexical-structure.md`](../../../../language-reference/lexical-structure.md):
whitespace and line-break definitions, comment behaviour
(line-comment-terminates-at-newline; block-comments-do-NOT-nest),
identifier grammar, escaped-line-break joining, and integer-literal
lexical details (underscore-as-digit-separator, no-octal).

Integer-literal *type-selection* claims (the suffix table) are not
duplicated here; they live in
`language-reference/expressions-literal`.

## Functional coverage

| Claim | Intent | Anchor | Tests |
| --- | --- | --- | --- |
| A line comment extends to the next line break; subsequent lines are not part of it. | functional | [#comments](../../../../language-reference/lexical-structure.md#comments) | [`line-comment-stops-at-newline.slang`](line-comment-stops-at-newline.slang) |
| Block comments do not nest: `/* /* */ x */` closes at the first `*/`, leaving `x */` outside the comment. | boundary | [#comments](../../../../language-reference/lexical-structure.md#comments) | [`block-comment-does-not-nest.slang`](block-comment-does-not-nest.slang) |
| Block comments may contain line breaks and embedded text without disturbing surrounding code. | functional | [#comments](../../../../language-reference/lexical-structure.md#comments) | [`block-comment-spans-lines.slang`](block-comment-spans-lines.slang) |
| Unterminated block comment is an error per the spec ("It is an error if a block comment that begins in a source unit is not terminated in that source unit"). | negative | [#comments](../../../../language-reference/lexical-structure.md#comments) | [`block-comment-unterminated-rejected.slang`](block-comment-unterminated-rejected.slang) |
| Identifiers may begin with an underscore and contain letters/digits/underscores in any subsequent position. | functional | [#identifiers](../../../../language-reference/lexical-structure.md#identifiers) | [`identifier-underscore-prefix.slang`](identifier-underscore-prefix.slang) |
| Integer literal digit-separator: underscores in the digit run are ignored — `1_000_000` equals `1000000`. | boundary | [#integer-literals](../../../../language-reference/lexical-structure.md#integer-literals) | [`integer-literal-underscore-ignored.slang`](integer-literal-underscore-ignored.slang) |
| An escaped line break (`\` immediately before a newline) is eliminated during phase 2 — code after the joined line continues as if no break were present. | boundary | [#escaped-line-breaks](../../../../language-reference/lexical-structure.md#escaped-line-breaks) | [`escaped-line-break-in-macro.slang`](escaped-line-break-in-macro.slang) |
| Tab (U+0009) and space (U+0020) are both horizontal whitespace and may appear between tokens. | functional | [#whitespace](../../../../language-reference/lexical-structure.md#whitespace) | [`whitespace-tab-and-space.slang`](whitespace-tab-and-space.slang) |


## Untested claims

| Claim | Reason | Anchor | Why untested |
| --- | --- | --- | --- |
| Lone `_` identifier is reserved. | (unclassified) | [#identifiers](../../../../language-reference/lexical-structure.md#identifiers) | The spec says `_` "is reserved by the language and must not be used by programs". The compiler's diagnostic for this is non-trivial to pin without a specific test pattern; deferred. |
| Floating-point literal grammar. | (unclassified) | [#floating-point-literals](../../../../language-reference/lexical-structure.md#floating-point-literals) | Section is marked "Note: This section is not yet complete." Test once doc fills in. |
| String literal grammar. | (unclassified) | [#string-literals](../../../../language-reference/lexical-structure.md#string-literals) | Section incomplete in the doc. |
| Character literal grammar. | (unclassified) | [#character-literals](../../../../language-reference/lexical-structure.md#character-literals) | Section incomplete in the doc. |
| Operators-and-punctuation lexical claims. | (unclassified) | [#operators-and-punctuation](../../../../language-reference/lexical-structure.md#operators-and-punctuation) | Section incomplete in the doc; operator *semantics* are tested in `language-reference/expressions-operators`. |
| Source-encoding claims (UTF-8 recommended). | internal-source-fact | [#encoding](../../../../language-reference/lexical-structure.md#encoding) | Spec uses `*may*` / `*should*`; no observable normative behaviour at the slangc CLI. |


## Doc gaps observed

| Anchor | Kind | Gap | Suggested addition |
| --- | --- | --- | --- |
| [#integer-literals](../../../../language-reference/lexical-structure.md#integer-literals) | ambiguous-claim | The doc says "Octal literals (radix 8) are not supported. A `0` prefix on an integer literal does *not* specify an octal literal as it does in C." But the sibling doc `docs/language-reference/expressions-literal.md` explicitly describes octal literals with deprecation semantics. The two language-reference docs **disagree** about whether octal literals exist at all. | Reconcile the two docs. Either remove the octal grammar production from `expressions-literal.md` (and the carve-out about `00`, `01` being octal) or remove the "not supported" claim from `lexical-structure.md` and replace it with a deprecation reference. |
| [#integer-literals](../../../../language-reference/lexical-structure.md#integer-literals) | ambiguous-claim | The doc lists integer-literal suffixes as lowercase `u` / `l` / `ll` / `ul` / `ull`. The sibling `expressions-literal.md` table covers both `u`/`U` and `l`/`L` (and `LL` / `ULL` etc.), case-insensitive. Pick one. | Align the two docs on case-sensitivity of suffixes. The compiler accepts both cases in practice, so `lexical-structure.md` should mention case-insensitivity explicitly. |
| [#comments](../../../../language-reference/lexical-structure.md#comments) | drift-from-source | The doc states "It is an error if a block comment that begins in a source unit is not terminated in that source unit." The compiler does reject such input, but the diagnostic that fires is the generic `E20001 — unexpected end of file, expected '}'`, not a comment-specific message. The error-naming is implementation-detail-leaky. | Either implement a comment-specific diagnostic (e.g. `E10042 — unterminated block comment`) or note in the doc that the rejection surface is the trailing-token diagnostic of whatever construct ate the unterminated comment. |
