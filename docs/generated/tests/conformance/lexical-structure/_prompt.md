# Prompt: docs/generated/tests/conformance/lexical-structure/

See [`_common.md`](_common.md) for universal rules, the
source-of-truth hierarchy, and the parallel-trees policy.

## Target

Bundle at
`docs/generated/tests/conformance/lexical-structure/`,
anchored to
[`docs/language-reference/lexical-structure.md`](../../../../language-reference/lexical-structure.md).

The doc covers source units, encoding, whitespace, escaped line
breaks, comments, the compilation phases, identifiers, and integer
literals (floating / string / character sections are marked "not
yet complete").

## High-value claims

- **Comment behaviour**: block comments do not nest; line comments
  end at next newline (or EOF); block comments may contain line
  breaks; unterminated block comments are an error.
- **Identifier grammar**: starts with letter or `_`, then letters/
  digits/underscores; lone `_` is reserved.
- **Integer-literal lexical details**: underscores in the digit run
  are ignored (`1_000_000` == `1000000`); octal-NOT-supported (per
  this doc — contradicts `expressions-literal.md`, record as a
  doc-gap and avoid testing the contradicted surface here).
- **Whitespace / line-break**: tab and space are both horizontal
  whitespace; CRLF and LF and CR are all line breaks.

## What NOT to test here

- Integer-literal suffix → type rules (already covered in
  `language-reference/expressions-literal`). Avoid duplication.
- Boolean literal grammar (covered in `expressions-literal`).
- Floating / string / character literal grammar — doc sections
  marked "not yet complete"; record as untested-claim until the
  doc fills in.
