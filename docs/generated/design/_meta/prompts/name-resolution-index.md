# Prompt: name-resolution/index.md

See [_common.md](_common.md) for the universal rules. This page is the
navigation entry point for the name-resolution subtree; the per-page
**Name-resolution family contract** in `_common.md` does not directly
apply.

## Target

Produce `docs/generated/design/name-resolution/index.md` — a short
navigation document that introduces the name-resolution subtree, names
the topics each peer page covers, and links them.

Audience: a developer who just opened
`docs/generated/design/name-resolution/` and needs to pick the right page.
A typical reader is investigating an ambiguous-overload diagnostic,
implementing a new lookup rule, or modifying visibility behavior.

## Required structure

1. `# Name Resolution` title.
2. Two- to three-paragraph intro:
   - What this subtree is. State explicitly that it describes the
     *algorithmic rules* for resolving identifiers, in contrast to the
     parsing flow ([../pipeline/02-parse-ast.md](../pipeline/02-parse-ast.md))
     and the high-level semantic-checking flow
     ([../pipeline/03-semantic-check.md](../pipeline/03-semantic-check.md)).
   - Who it is for. Typical readers: contributors modifying lookup,
     visibility, or overload-resolution rules; new contributors trying
     to understand a name-resolution-related diagnostic.
   - How the pages relate to each other. `scopes.md` is the
     foundation; `lookup.md` uses scopes; `visibility.md` filters
     lookup results; `overload-resolution.md` ranks the survivors.
3. `## Pages` — a table:

   ```markdown
   | Page | Topic | Primary source |
   | --- | --- | --- |
   | [scopes.md](scopes.md) | The `Scope` data structure and how parsing builds the scope chain | slang-ast-base.h, slang-parser.cpp |
   | [lookup.md](lookup.md) | The unqualified and member lookup algorithm; masks, options, shadowing | slang-lookup.{h,cpp} |
   | [visibility.md](visibility.md) | `public` / `internal` / `private` modifiers and the visibility filter | slang-ast-modifier.h, slang-check-decl.cpp |
   | [overload-resolution.md](overload-resolution.md) | Candidate filtering and conversion-cost ranking | slang-check-overload.cpp |
   ```

4. `## Flow diagram` — a small mermaid flowchart showing how an
   unqualified identifier becomes a fully resolved `DeclRef`:

   ```
   identifier -> scope walk -> raw lookup results -> visibility filter -> overload resolution -> DeclRef
   ```

   Label each transition with the page that owns it.
5. `## Where this fits in the pipeline` — 2-3 sentences linking
   [../pipeline/02-parse-ast.md](../pipeline/02-parse-ast.md) (which
   builds the scope chain), [../pipeline/03-semantic-check.md](../pipeline/03-semantic-check.md)
   (which orchestrates lookup), and [../ast-reference/base.md](../ast-reference/base.md)
   (which documents `DeclRef`, `Scope`, `LookupResult` shapes).
6. `## Related glossary terms` — bullets pointing at the relevant
   entries in [../glossary.md](../glossary.md): `decl-ref`,
   `lookup result`, `name resolution`, plus the new entries:
   `scope`, `shadowing`, `lookup mask`, `lookup options`,
   `lookup breadcrumb`, `transparent member`, `visibility modifier`,
   `overload resolution`, `conversion cost`,
   `partial generic application`.

## Quality checklist (in addition to the universal one)

- [ ] Every peer page is named in the `## Pages` table and reachable
      from at least one other section.
- [ ] The flow diagram is consistent with the topic split described in
      the intro.
- [ ] No node from a peer page is explained in depth here — this is a
      navigation page, not a reference page.
- [ ] All glossary cross-references resolve to entries in
      [../glossary.md](../glossary.md).
