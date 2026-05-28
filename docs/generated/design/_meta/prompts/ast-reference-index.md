# Prompt: ast-reference/index.md

See [_common.md](_common.md) for the universal rules. This page is the
navigation entry point for the AST reference subtree; the per-page
**AST reference family contract** in `_common.md` does not directly
apply.

## Target

Produce `docs/generated/design/ast-reference/index.md` — a short
navigation document that introduces the AST reference subtree, names
the families and their owning headers, and links every family page.

Audience: a developer who just opened
`docs/generated/design/ast-reference/` and needs to pick the right page.

## Required structure

1. `# AST Reference` title.
2. Two-paragraph intro: what this subtree is, who it is for, how it
   relates to [../pipeline/02-parse-ast.md](../pipeline/02-parse-ast.md)
   (the "how the AST is built" page) and
   [../syntax-reference/grammar.md](../syntax-reference/grammar.md)
   (the grammar).
3. `## Family taxonomy` — a mermaid diagram showing the abstract
   roots (`NodeBase`, `SyntaxNode`, `DeclBase`, `Decl`,
   `ModifiableSyntaxNode`, `Stmt`, `Expr`, `Modifier`, `Val`, `Type`)
   exactly as declared in
   [../../../../source/slang/slang-ast-base.h](../../../../source/slang/slang-ast-base.h).
   Each node in the diagram that has a dedicated family page should
   point at it via a sibling bullet list under the diagram (do not
   use mermaid `click`).
4. `## Pages` — a table:

   ```markdown
   | Page | Family root | Owning header | Approx. concrete classes |
   | --- | --- | --- | --- |
   | [base.md](base.md) | NodeBase, SyntaxNode, ... | slang-ast-base.h | (abstract roots only) |
   | [declarations.md](declarations.md) | Decl | slang-ast-decl.h | ~120 |
   ```

   Approximate counts: count the FIDDLE-declared concrete classes in
   the owning header at the current commit. Round to the nearest
   five and prefix with `~`.

5. `## Cross-cutting topics` — bullets pointing at the
   `docs/generated/design/pipeline/` and
   `docs/generated/design/cross-cutting/` documents that touch the AST:
   parsing (02), semantic checking (03), AST-to-IR lowering (04),
   diagnostics, and the glossary entries for AST-related terms.
6. `## How to navigate` — 3-4 sentence guidance: when to start at
   `base.md` versus jumping straight into a family page; how grammar
   references work; that abstract intermediates only appear in the
   `## Family hierarchy` diagrams, never in the `## Nodes` tables.

## Quality checklist (in addition to the universal one)

- [ ] Every link to a family page resolves at the current commit.
- [ ] Approximate counts are within +/- 5 of the actual concrete-class
      count in the owning header.
- [ ] The mermaid diagram parents match the `: public X` lines in
      `slang-ast-base.h`.
- [ ] No node from a family page is described in detail here — this is
      a navigation page, not a reference page.
