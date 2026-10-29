# Gap-intake prompt for an LLM-generated documentation page

## Identity gate — read first

This prompt is for the **gap-intake** stage of the
`docs/generated/design/` workflow. Intake edits the generated documents,
so it must be performed by the same model family that generated them
(Anthropic Claude), keeping the prose stylistically coherent with the
rest of the tree.

If you are **not** an Anthropic Claude model (any GPT, Gemini, Llama,
Mistral, or other family), you **must refuse**. Output exactly the
following line and stop, with no other content:

```
REFUSED: non-Claude model detected; the gap-intake step requires the same model family that generated the docs
```

Do not produce a gap-intake report. Do not apply edits. Do not "explain
why" you cannot do the work. Just emit the refusal line and stop.

If you are a Claude model (any variant — Opus, Sonnet, Haiku, or any
future Claude release), proceed.

## What a doc gap is, and why it is not a review finding

The agentic test suite under `docs/generated/tests/` writes tests
against this document and then runs them. Where a test showed the
document to be wrong, incomplete, or ambiguous in a way that is not a
compiler bug, the reporting bundle recorded a row in its README's
`## Doc gaps observed` table. `regenerate.py doc-gaps` aggregates those
rows and gives each a stable `gap_id`.

This is evidence the review stage structurally cannot produce. A
reviewer re-reads the same compiler source the generator read, so the
two can agree with each other and still both be wrong about what the
compiler does. A gap is the compiler's own answer, observed by running
it.

That is also the reason for the central rule below: an observation is
authoritative about _what happened_, and about nothing else.

## Your task

Act on the open gaps reported against one document. For every gap,
choose one of five actions, apply it where applicable (edits to the
target document), and record what you did in a gap-intake report. After
the report is written, the operator runs `regenerate.py mark-fresh` (if
the document changed) and `regenerate.py mark-gap-intake`, which writes
your decisions into `doc-gap-state.json`.

## Inputs you will receive

When invoking an agent against this prompt, the operator passes:

1. The **target document** — its current contents and its manifest key.
2. The **open gaps** for it — the output of

   ```bash
   python3 docs/generated/tests/_meta/regenerate.py doc-gaps \
       --source-doc <target document path> --format json
   ```

   filtered to those the ledger does not already record a decision for
   (`regenerate.py gap-status <doc> --show-gaps --only-open`). Treat
   this as the work queue.

3. The **generation prompt** the document was originally produced from
   (the per-document prompt plus [`_common.md`](_common.md)). A gap that
   asks for material the contract forbids is `rejected-out-of-scope`.
4. The **resolved watched paths** for the target document at the current
   `HEAD`. Use `regenerate.py show <doc>`.
5. The **tests that reported each gap** — the `.slang` files in the
   bundles named by each gap's `reported_by`. These are the evidence;
   read them when a gap's prose is not self-explanatory.
6. The current `HEAD` commit SHA.

## The central rule: a `Suggested addition` is a hypothesis

Every gap row carries a `Suggested addition` written by an agent that
**observed a behaviour**, not by one that read the code producing it.
It is a proposal, not a specification, and it is wrong often enough to
matter.

**Before you write any observed behaviour into the document, confirm it
in the document's watched paths.** Name the file and line in your
`Evidence` cell. If you cannot find it in the source, you may not
document it — choose `deferred` (you could not locate it) or
`escalated-to-finding` (the source says something else) instead.

This is not a formality. These documents are reverse-engineered from
compiler source and are read as descriptions of intended behaviour. A
document that absorbs whatever a test happened to observe is how a
compiler bug becomes documented, blessed behaviour — and the test suite
will then generate more tests asserting the bug, against a document that
now agrees with it. The confirmation step is the only thing standing
between an observation and that outcome.

## `drift-from-source` needs a verdict, not a fix

A gap of kind `drift-from-source` says the document contradicts observed
behaviour. That has two possible causes, and they need opposite
responses:

- **The document is wrong.** The source agrees with the observation;
  the document describes something the compiler does not do. Action:
  `fixed`.
- **The compiler is wrong.** The source agrees with the _document_;
  the compiler does not do what both say. Action:
  `escalated-to-finding` — this is a compiler defect and belongs in
  the tests tree's findings channel, not in this document.

Read the watched source and decide which. Do not resolve the
contradiction by editing the document to match the observation without
checking; that silently converts a compiler bug into documented
behaviour, which is the failure mode this whole stage exists to avoid.

The same judgement applies, less sharply, to `undocumented-behavior`:
behaviour that is real, confirmed in the source, and undocumented should
be documented — but behaviour that is real and _wrong_ should be a
finding.

## Action set

Choose exactly one for every gap in the queue:

- **fixed** — You edited the target document so that the gap no longer
  applies. The `Evidence` cell cites the watched-path file and line that
  confirms what you wrote. The `Fix summary` cell describes the edit in
  one short clause. The edit must be the minimum necessary; do not
  opportunistically rewrite unrelated text.
- **rejected-bogus** — The gap is incorrect. The document does say the
  thing the gap claims is missing, the anchor points at the wrong
  section, or the observation misreads the compiler. The `Evidence` cell
  must cite what disproves it — a quote from the document, or a source
  path and line.
- **rejected-out-of-scope** — The gap is real but is not this
  document's to fix. Most commonly it belongs to
  `docs/language-reference/` (the human-written spec, which no agent may
  edit) or to a peer page. The `Evidence` cell must name the contract
  clause that excludes the material, or the document that owns it.
- **deferred** — The gap is real and in scope, but you cannot resolve it
  now: you could not confirm the behaviour in the watched paths, the fix
  needs a `watched_paths` expansion, or it needs a rewrite larger than
  this cycle. The `Evidence` cell states what blocks the fix and what
  follow-up is needed.
- **escalated-to-finding** — The disagreement is a compiler defect, not
  a documentation defect. The `Evidence` cell must state what the source
  says and why the observed behaviour contradicts it. Name the existing
  `docs/generated/tests/_meta/findings/<id>.yaml` if one already covers
  it; if none does, say so, and the operator will have the tests side
  open one before recording your decision (`mark-gap-intake` requires a
  finding id for this action).

Every gap in the queue must appear exactly once in the actions table.
The lint pass enforces this.

## Edit rules

When you apply a `fixed` action:

- **Edit only the target document**, not the compiler source, not peer
  generated docs, not the test bundles, not the prompt contract.
- **Never edit a bundle README to remove the gap row.** The row is the
  test suite's record; it disappears on its own when the bundle is
  regenerated against your improved text. Editing it by hand fakes the
  loop closing.
- **Never run a formatter over the target document.** The generated
  design docs are not prettier-formatted, and reformatting one rewrites
  lines throughout it. That is not cosmetic here: every test in the
  reporting bundle carries a `doc_section_digest` of the section it is
  anchored to, so a whole-file reformat invalidates every section's
  digest instead of only the ones you edited, and the "which tests need
  re-reading" signal the suite gives back is buried. In the first intake
  cycle this turned a precise 3-section signal into 10. Keep the diff to
  the lines you actually changed, and match the surrounding style by
  hand (the tree uses `*emphasis*`, not `_emphasis_`, and wraps prose at
  roughly 68 columns).
- **Use the smallest reasonable change.** A sentence, a table row, a
  short subsection — matched to what the gap asks for.
- **Write in the document's voice**, not the gap's. The
  `Suggested addition` is frequently phrased as advice to an author
  ("Add a note that..."); what lands in the document is the note, not
  the advice.
- **Preserve front-matter exactly**, except for `generated_at`,
  `source_commit`, and `watched_paths_digest`, which the operator
  refreshes with `regenerate.py mark-fresh`. Do not edit those three
  yourself.
- If several gaps touch the same section, apply them as one consolidated
  edit and reference the other gap ids in the `Fix summary` cell.
- A gap asking for an example (`missing-example`) wants a **minimal**
  one — a few lines of Slang that a reader can check, not a program.

## When to skip `mark-fresh`

If no action is `fixed` — the document was not edited — the report is
still written and `mark-gap-intake` is still run, but `mark-fresh` is
not. In that case set `target_doc_source_commit_after` equal to
`target_doc_source_commit_before`.

## Output format

Your outputs are:

- **Zero or more edits to the target document** (only when at least one
  action is `fixed`).
- **One gap-intake report**, a Markdown file with the contract below.
  Filename (informational; the operator decides the path):

```
docs/generated/design/_meta/gap-intake/<target_doc>.gap-intake.md
```

Hierarchy under `_meta/gap-intake/` mirrors the manifest key (e.g.
`_meta/gap-intake/pipeline/05-ir-passes.md.gap-intake.md`).

### Front-matter (mandatory)

```yaml
---
gap_intake_report: true
intake_model: <model identifier, e.g. claude-opus-5>
intake_at: <ISO 8601 UTC, seconds precision>
target_doc: <manifest key, e.g. pipeline/05-ir-passes.md>
target_doc_source_commit_before: <the `source_commit` from the target doc's front-matter at the start of intake>
target_doc_source_commit_after: <the `source_commit` after `mark-fresh` runs, or same as `_before` when no edits were made>
gap_count: <number of gaps acted on; must equal the number of rows in the Actions table>
actions:
  fixed: <int>
  rejected_bogus: <int>
  rejected_out_of_scope: <int>
  deferred: <int>
  escalated_to_finding: <int>
---
```

The action counts must sum to `gap_count`.

### Body (fixed section order)

1. `# Gap-intake report for <target_doc>` — the title.
2. `## Summary` — 2-5 sentences stating what was done, with the action
   breakdown restated in prose. If any gap was escalated, say so here in
   the first sentence.
3. `## Escalated gaps` (**only if** `actions.escalated_to_finding > 0`)
   — a bullet list naming the gap ids, what the source says, and what
   the compiler does instead. This section appears **before**
   `## Actions` so a hurried operator sees the compiler bugs first.
4. `## Actions` — a Markdown table with exactly these columns:

   | Column      | Content                                                                                                                                                                          |
   | ----------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
   | Gap ID      | The 12-hex-digit `gap_id` from the aggregator.                                                                                                                                   |
   | Action      | One of `fixed`, `rejected-bogus`, `rejected-out-of-scope`, `deferred`, `escalated-to-finding`. Use the exact spelling.                                                           |
   | Evidence    | Required for every action. The watched-path file and line confirming what you wrote (for `fixed`), or the justification described in the action set above. Workspace-relative.   |
   | Fix summary | Required when `Action = fixed`; one short clause describing the edit (e.g. "added the `[MaxIters]` requirement under Checkpointing"). Use a single em-dash (`—`) for the others. |

   Every gap id from the queue must appear here exactly once.

## Style rules for the report itself

- No emojis.
- No code blocks larger than 10 lines.
- Do not copy the full text of the target document or of the gap table.
- Use workspace-relative paths in all citations.
- The report is a record, not an essay; keep it dense and actionable.
