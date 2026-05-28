# Prompt: glossary.md

See [_common.md](_common.md) for universal rules.

## Target

Produce `docs/generated/design/glossary.md` — a single-page,
alphabetically-ordered glossary of compiler-internals vocabulary used
throughout the rest of this tree and in the Slang source.

Audience: a new contributor reading the other generated documents or the
source itself who needs a one-paragraph anchor for an unfamiliar term.

This document is **not** the same as
[docs/language-reference/glossary.md](../../language-reference/glossary.md),
which is the user-facing language/execution-model glossary (threads,
dispatch, observable behavior). Do not redefine terms that already live
in that glossary; cross-link to it from the preamble and otherwise stay
out of its lane.

## Two term classes

Each entry must be tagged with exactly one class:

- `[Slang]` — terms whose meaning is defined by this project and which a
  general compiler engineer would not know from compiler theory alone.
  Examples: `IRDecoration`, `hoistable instruction`, `syntax-decl`,
  `capability atom`, `FIDDLE`, `fossil format`, `RIFF container`,
  `translation unit` (as used here), `decl-ref`, `lower-to-IR`,
  `prelude`, `IRBuilder`, `witness table`.
- `[General]` — terms drawn from general compiler theory that the
  project uses. Examples: `abstract syntax tree`, `single static
  assignment`, `dominator`, `control-flow graph`, `dataflow analysis`,
  `dead-code elimination`, `inlining`, `monomorphization`, `lexer`,
  `parser`, `recursive descent`, `name resolution`, `type inference`,
  `intermediate representation`, `preprocessor`.

If a term is borderline (e.g. `existential type` — general PL theory but
Slang uses it in a specific way), prefer `[Slang]` and explain the
project-specific shading inside the definition.

## Required structure

1. `# Compiler-Internals Glossary` (title).
2. `## Conventions` — short preamble (3-5 sentences) explaining the two
   classes, the `See:` / `External:` link style, and pointing readers at
   [../language-reference/glossary.md](../language-reference/glossary.md)
   for language-runtime terms.
3. `## Terms` — flat A-Z list. Use a sub-heading per letter only if the
   final list has more than ~30 entries; otherwise just stream the
   entries.
4. `## Cross-reference index` — a compact table at the bottom that pairs
   each peer doc with the terms it introduces, so the glossary is
   reachable both alphabetically and topically. Example shape:

   ```markdown
   | Peer document | Terms it introduces |
   | --- | --- |
   | [pipeline/04-ast-to-ir.md](pipeline/04-ast-to-ir.md) | IRBuilder, lower-to-IR, hoistable instruction, ... |
   ```

## Entry shape

Each entry is a definition-list-style block:

```markdown
**term name** `[Slang|General]`
: One short paragraph (2-4 sentences) defining the term in this
  project's usage. Cite at most one concrete code anchor if it
  materially clarifies the definition.

  See: [peer-doc.md](path/to/peer-doc.md)
  External: https://en.wikipedia.org/wiki/...   (only for [General], optional)
```

Rules:

- `See:` is **mandatory** and must link to a peer doc under
  `docs/generated/design/` (workspace-relative). The linter rejects
  unresolved paths.
- `External:` is **optional** and only applies to `[General]` entries.
  Use Wikipedia or another canonical reference (e.g. an ISO spec, a
  classic paper) when one fits. If no good external reference exists,
  omit the line — do not fabricate links.
- Do not include code blocks inside an entry. A single inline
  `` `Identifier` `` is fine.
- Do not write tutorial-style content. The glossary is for lookup; the
  peer doc behind `See:` is where readers go for depth.

## Term selection

You must cover at least the following floor. Add more as the watched
paths warrant; omit only if the term turns out not to be used in the
watched docs or source.

Slang-specific floor:

`ASTBuilder`, `capability atom`, `core module`, `decl-ref`,
`DiagnosticSink`, `entry point` (in the compile-pipeline sense, distinct
from the runtime sense in the language-reference glossary), `existential
type`, `FIDDLE`, `fossil format`, `hoistable instruction`, `IRBuilder`,
`IRDecoration`, `IRFunc`, `IRInst`, `IRModule`, `linkage`, `lookup
result`, `lower-to-IR`, `module` (in the compile sense), `prelude`,
`profile`, `RIFF container`, `session`, `source-loc`, `specialization`,
`syntax-decl`, `target`, `translation unit`, `two-stage parsing`,
`witness table`.

General-theory floor:

`abstract syntax tree`, `control-flow graph`, `dataflow analysis`,
`dead-code elimination`, `dominator`, `inlining`, `intermediate
representation`, `lexer`, `monomorphization`, `name resolution`,
`parser`, `preprocessor`, `recursive descent`, `single static assignment
(SSA)`, `type inference`.

## Forbidden content (in addition to _common.md)

- Do not redefine terms that already appear in
  [../language-reference/glossary.md](../language-reference/glossary.md).
  If a term in that doc also has a compiler-internals meaning, omit it
  here and rely on the preamble cross-link.
- Do not include per-function or per-class reference documentation.
- Do not include compiler **flags** or command-line options
  ([../command-line-slangc-reference.md](../command-line-slangc-reference.md)
  is the home for those).

## Quality checklist (in addition to the universal one)

- [ ] Every entry is tagged with exactly one of `[Slang]` or `[General]`.
- [ ] Every entry has a `See:` link that resolves to a peer doc under
      `docs/generated/design/`.
- [ ] Every `External:` link, when present, is an `https://` URL.
- [ ] No entry duplicates a term already defined in
      [../language-reference/glossary.md](../language-reference/glossary.md).
- [ ] The `## Cross-reference index` table covers every peer doc listed
      in the manifest entry for `glossary.md`.
- [ ] Document under the 100 KB size cap.
