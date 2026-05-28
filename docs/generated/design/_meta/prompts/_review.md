# Review prompt for an LLM-generated documentation page

## Identity gate — read first

This prompt is for the **independent review** stage of the
`docs/generated/design/` workflow. The review must be performed by a model
from a different family than the one that produced the documents.

If you are an Anthropic **Claude** model (any variant — Opus, Sonnet,
Haiku, Instant, or any future Claude release), you **must refuse**.
Output exactly the following line and stop, with no other content:

```
REFUSED: Claude model detected; the review step requires a different model family
```

Do not produce a review report. Do not "explain why" you cannot do the
work. Do not propose a workaround. Just emit the refusal line and stop.

If you are any other model family (GPT, Gemini, Llama, Mistral, ...),
proceed.

## Your task

Review one generated document under `docs/generated/design/` against the
source files it claims to describe, and produce a structured review
report. The report goes to disk; a Claude agent will then act on your
findings in a separate **remediation** step.

You are reviewing one document. You are **not** rewriting it. You do
not edit the document. Your only output is the review report.

## Inputs you will receive

When invoking an agent against this prompt, the operator passes:

1. The **target document** — its workspace-relative path under
   `docs/generated/design/` (the manifest key, e.g.
   `pipeline/05-ir-passes.md`) and its full current contents,
   including YAML front-matter.
2. The **generation prompt** the document was produced from — i.e.
   the contents of `docs/generated/design/_meta/prompts/_common.md`
   plus the per-document prompt referenced by the manifest entry
   for the target document. The review must verify the document
   conforms to that contract.
3. The **resolved watched paths** for the target document at the
   commit recorded in its front-matter `source_commit`. Use
   `regenerate.py show <doc>` to obtain the file list.
4. The **dependency documents** listed under `depends_on` in the
   manifest entry, as additional context for cross-reference checks.
5. The current `HEAD` commit SHA (for the report front-matter
   `source_commit` field).

## Reviewer checklist

Verify each of the following dimensions and record a per-dimension
status in the report front-matter (`pass`, `partial`, or `fail`).
For every dimension that is not `pass`, file at least one finding.

1. **factual_accuracy** — Every claim about a symbol, file path,
   function signature, line number, or behavior matches the source
   files at the document's `source_commit`. Spot-check ≥ 10 claims;
   for a document with line-number citations, verify every one that
   appears in the body.
2. **cross_references** — Every relative link in the document
   resolves to a file that exists at the recorded `source_commit`.
   Every reference to a peer LLM-generated document points at a
   page that exists in the manifest.
3. **completeness** — The document covers every section required
   by its prompt contract (the per-document prompt plus any family
   contract in `_common.md`). Missing required sections, missing
   required table columns, or table rows that omit mandatory cells
   are findings.
4. **style_consistency** — The document obeys the universal style
   rules in `_common.md`: no emojis, no editorial commentary, no
   verbatim copying from `docs/design/` or other handwritten docs,
   markdown links use workspace-relative paths, mermaid diagrams
   follow project conventions.
5. **source_alignment** — Where the document summarizes source
   behavior, the summary is supported by the source files. Flag
   claims that the source actively contradicts; flag claims that
   the source does not support either way (speculation).
6. **front_matter_validity** — The YAML front-matter contains every
   required key (`generated`, `model`, `generated_at`,
   `source_commit`, `watched_paths_digest`, `warning`). The
   `watched_paths_digest` matches what `regenerate.py digest <doc>`
   would produce at `source_commit` (you do not need to recompute
   it; flag only obvious mismatches such as a non-hex value or a
   missing field).

## Severity

Every finding carries one of:

- **critical** — The document is actively misleading: it asserts a
  fact the source contradicts, or its instructions would cause a
  reader to take a wrong action. Code generation, build-system, or
  ABI claims that are wrong are typically critical.
- **major** — A required section is missing or substantially
  incomplete; a cited symbol does not exist; a link is dangling.
- **minor** — A line number is off by more than a few lines; a
  cited file path uses the wrong directory; the prose includes a
  small but verifiable inaccuracy.
- **nit** — Pure-style issues that do not change reader
  understanding: wording choices, redundant phrases, capitalization
  consistency. Use sparingly.

A finding **must** include evidence — a citation of the source file
(workspace-relative path, ideally with a line number) and, where
practical, a one-line quote of the contradicting text. A
recommendation is required and must be specific enough that a
remediator can act on it without re-reading the source themselves.

## Out of scope for review

- Do **not** rewrite or paraphrase the document. The remediator
  performs all edits.
- Do **not** propose new sections beyond what the prompt contract
  already requires.
- Do **not** flag absent material that the prompt's "Forbidden
  content" / "Out of scope" rules explicitly exclude.
- Do **not** flag design choices made by the prompt contract itself
  (e.g. four-phase target-pipeline structure). If the contract is
  wrong, file an out-of-band issue against the prompt rather than
  the document.
- Do **not** review another generated document while reviewing this
  one, even if you spot drift; per-doc reviews are independent.

## Output format

Your single output is a Markdown file matching this contract.
Filename (informational; the operator decides the path):

```
docs/generated/design/_meta/reviews/<target_doc>.review.md
```

Hierarchy under `_meta/reviews/` mirrors the manifest key (e.g.
`_meta/reviews/pipeline/05-ir-passes.md.review.md`).

### Front-matter (mandatory)

```yaml
---
review_report: true
reviewer_model: <model identifier, e.g. gpt-5-2026-01-15>
reviewed_at: <ISO 8601 UTC, seconds precision, e.g. 2026-05-15T18:00:00+00:00>
target_doc: <manifest key of the reviewed document>
target_doc_source_commit: <the `source_commit` from the reviewed doc's front-matter>
target_doc_watched_paths_digest: <the `watched_paths_digest` from the reviewed doc's front-matter>
source_commit: <HEAD SHA at review time>
checklist:
  factual_accuracy: pass | partial | fail
  cross_references: pass | partial | fail
  completeness: pass | partial | fail
  style_consistency: pass | partial | fail
  source_alignment: pass | partial | fail
  front_matter_validity: pass | partial | fail
finding_count: <total number of rows in the Findings table>
severity_breakdown:
  critical: <int>
  major: <int>
  minor: <int>
  nit: <int>
---
```

The `severity_breakdown` counts must sum to `finding_count`.

### Body (fixed section order)

1. `# Review report for <target_doc>` — the title.
2. `## Summary` — 2-5 sentences stating the overall verdict and the
   single most important finding (if any).
3. `## Items checked` — bullet list summarizing what was actually
   verified. Be concrete: "verified 24 line-number citations against
   `slang-emit.cpp`", "resolved all 17 relative links", etc.
4. `## Findings` — a Markdown table with exactly these columns:

   | Column | Content |
   | --- | --- |
   | ID | Sequential `F-001`, `F-002`, ...; unique within the report. |
   | Severity | One of `critical`, `major`, `minor`, `nit`. |
   | Location | A heading anchor or line range inside the target doc (e.g. `## Phase B`, lines 110-115). |
   | Description | One short paragraph. Be precise; quote the offending text in backticks. |
   | Evidence | The source file (with line number) that proves the finding, or a peer doc reference. |
   | Recommendation | A specific, actionable fix (e.g. "change line N from X to Y", "delete the row about Z", "add the section `## Foo`"). |

   If there are no findings, write `(no findings)` on a single line
   immediately below the section heading and omit the table.

5. `## No-issues notes` (**optional**) — 3-5 bullets calling out
   specific things you verified that look good. Include only if it
   adds signal beyond `## Items checked`.

## Style rules for the review report itself

- No emojis.
- No code blocks larger than 10 lines; quote only what is needed
  for evidence.
- Do not copy the full text of the document being reviewed.
- Keep total length proportional to the document's complexity;
  reports under 4 KB are typical, reports over 16 KB are unusual.
- Use workspace-relative paths in all source citations
  (`source/slang/slang-emit.cpp` rather than absolute paths).
- Do not include suggestions in `## Items checked`; suggestions
  belong in finding `Recommendation` cells.
