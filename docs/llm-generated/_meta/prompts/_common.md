# Common contract for every generated document

Every prompt under this directory inherits the rules below. Individual
prompts only describe the structure and content specific to their target
document.

## Inputs you will receive

When invoking an agent against one of these prompts, pass at minimum:

1. The list of **watched paths** for the target document, expanded to the
   actual files at the current commit. Use
   `regenerate.py show <doc>` to obtain this list.
2. The contents of any documents listed under `depends_on` for the target
   document, as additional context.
3. The target document's manifest key (e.g. `pipeline/05-ir-passes.md`),
   which is also the workspace-relative path under `docs/llm-generated/`.
4. The current `HEAD` commit SHA (for the front-matter `source_commit`
   field) and the current `watched_paths_digest`
   (`regenerate.py digest <doc>`).

## Mandatory front-matter

Every generated document must begin with:

```yaml
---
generated: true
model: <model identifier>
generated_at: <ISO 8601 timestamp, UTC, seconds precision>
source_commit: <full git SHA>
watched_paths_digest: <hex sha256 from regenerate.py digest>
warning: "Auto-generated. May drift from source. Do not edit by hand."
---
```

Everything below the closing `---` is the body of the document.

## Universal style rules

- Use Markdown headings (`#`, `##`, ...). The first heading must be `#`
  and is the document title.
- When you cite a source file, use a markdown link with the
  workspace-relative path, e.g. `[slang-parser.cpp](../../../source/slang/slang-parser.cpp)`.
  Every such link must resolve at `source_commit`.
- When you reference a sibling LLM-generated doc, link relatively, e.g.
  `[../pipeline/05-ir-passes.md](../pipeline/05-ir-passes.md)`.
- Do not copy verbatim prose from `docs/design/`. You may cite those
  documents as further reading.
- Do not invent file paths, function names, or symbols. If unsure, omit
  the claim.
- Prefer concrete, code-anchored statements ("the `Lexer::lexToken`
  method in [slang-lexer.cpp](../../../source/compiler-core/slang-lexer.cpp)
  classifies tokens by ...") over abstract claims ("the lexer is
  efficient and clean").
- Use mermaid diagrams when they materially clarify structure. Follow
  the project's mermaid-syntax conventions: no spaces in node IDs,
  no explicit colors, escape special characters in labels with quotes.

## Universal content rules

- Stay within the size cap shown in the manifest. If the natural content
  is larger, summarize and split off detail into separate docs (and note
  that the cap should be raised in the manifest as a follow-up).
- The first paragraph of the body must say, in plain language, what the
  document covers and who its intended reader is.
- Do not duplicate large excerpts of source code. Cite a few essential
  signatures and link the rest.
- If you cannot find the information needed for a section, write a short
  paragraph explaining that the information was not located in the
  watched paths, and propose which additional paths should be added to
  the manifest.

## Forbidden content

- Per-function reference documentation that belongs in Doxygen.
- Speculative roadmap items not present in the source.
- Editorial opinions about the codebase ("this is well-designed",
  "this is hacky"). Stick to descriptive claims.
- Verbatim text from `docs/design/`, `docs/user-guide/`, the language
  reference, or upstream third-party documentation.

## Quality checklist

Before considering the document done, verify:

- [ ] Front-matter present with all required keys.
- [ ] Every workspace-relative link resolves.
- [ ] Document under the size cap (in bytes of UTF-8).
- [ ] No speculative claims about code that does not exist.
- [ ] Cross-references to dependency docs use relative paths.
- [ ] No emojis, no editorial commentary, no copied prose.
