---
name: design-docs-incremental-update
description: Incrementally refresh the LLM-generated design docs under docs/generated/design/. Use when the user asks to update, refresh, or regenerate the generated design documentation after source changes. Runs the three-stage operator loop defined in docs/generated/design/_meta/regenerate.md - (1) regenerate stale docs (Claude), (2) direct the user to run the non-Claude review, (3) apply remediation (Claude) - with explicit confirmation before stages 1 and 3.
allowed-tools:
  - Bash
  - Read
  - Grep
  - Glob
  - Edit
  - Write
---

# Generated Design Docs - Incremental Update

This skill drives the incremental refresh workflow for
`docs/generated/design/`, as specified in
[regenerate.md](../../../docs/generated/design/_meta/regenerate.md).
That document is the source of truth; this skill is the per-invocation
runbook. Re-read `regenerate.md` if anything below is ambiguous.

## Reference paths

All driver commands run from the repository root.

- Workflow spec: `docs/generated/design/_meta/regenerate.md`
- Driver script: `docs/generated/design/_meta/regenerate.py`
- Manifest: `docs/generated/design/_meta/manifest.yaml`
- Common generation contract:
  `docs/generated/design/_meta/prompts/_common.md`
- Per-doc generation prompts:
  `docs/generated/design/_meta/prompts/<name>.md`
- Review prompt (non-Claude only):
  `docs/generated/design/_meta/prompts/_review.md`
- Remediation prompt (Claude only):
  `docs/generated/design/_meta/prompts/_remediate.md`

## The three stages

Run the stages in order. Pause between stages and confirm with the
user where indicated.

| # | Stage                   | Performed by                       | Confirm first |
| - | ----------------------- | ---------------------------------- | ------------- |
| 1 | Incremental regenerate  | This agent (Claude)                | **Yes**       |
| 2 | Review                  | User, with a non-Claude model      | n/a (direct)  |
| 3 | Remediation             | This agent (Claude)                | **Yes**       |

If you are not a Claude model, refuse stages 1 and 3 - both belong to
the Claude family per the workflow's soft-refusal contract. The
driver's `mark-fresh` / `mark-remediated` commands will refuse to
record an entry under a non-Claude `model` field.

## Stage 1 - Incremental regeneration

### 1a. Survey

```bash
python3 docs/generated/design/_meta/regenerate.py list-stale
```

Collect every row tagged `stale` or `missing`. These are the docs to
regenerate.

Order the work in dependency order: if any of the listed docs has a
`depends_on` entry that is also stale, regenerate the dependency
first. Use `regenerate.py show <doc>` to inspect the manifest entry
(it prints the prompt path, the resolved watched files, and the
`depends_on` list).

If no docs are stale or missing, report that and proceed directly to
stage 2 (there is still potentially something to review).

### 1b. Confirm

Present to the user, then **wait for confirmation**:

- The ordered list of docs you will regenerate (manifest keys).
- The model id you will record on each doc (e.g. `claude-opus-4.7`).
  This must match the model actually doing the generation.

Do not begin regenerating until the user confirms.

### 1c. Regenerate each doc

For each stale/missing doc, in dependency order:

1. **Gather inputs.**
   - Run `python3 docs/generated/design/_meta/regenerate.py show <doc>`.
     This prints the manifest entry, the prompt path, the resolved
     watched files, and `depends_on`.
   - Read the previous version of the doc at
     `docs/generated/design/<doc>` if it exists (a `missing` doc has
     no prior version).
   - Run `git diff <previous source_commit> -- <watched paths>` to
     surface the changes that drove staleness. The previous
     `source_commit` is in the doc's existing front-matter.
   - Read each `depends_on` peer at its current location under
     `docs/generated/design/`.
2. **Read the prompts.**
   - The per-doc prompt at the path printed by `show <doc>`.
   - `docs/generated/design/_meta/prompts/_common.md` (inherited
     contract).
3. **Capture commit + digest.**
   ```bash
   git rev-parse HEAD
   python3 docs/generated/design/_meta/regenerate.py digest <doc>
   ```
4. **Produce the doc.** Write the regenerated body to
   `docs/generated/design/<doc>` (manifest key relative to that
   directory). Prefer a focused edit over a full rewrite when a
   previous version exists and the watched-paths diff is small.
   Always emit the mandatory YAML front-matter from `_common.md`:
   - `generated: true`
   - `model: claude-opus-4.7` (or your actual Claude model id)
   - `generated_at:` ISO 8601 UTC, seconds precision, `Z` suffix
   - `source_commit:` the SHA from step 3
   - `watched_paths_digest:` the digest from step 3
   - `warning: "Auto-generated. May drift from source. Do not edit by hand."`
5. **Lint.**
   ```bash
   python3 docs/generated/design/_meta/regenerate.py lint <doc>
   ```
   If lint reports a structural **error** (missing front-matter key,
   broken link, etc.), re-run the regeneration with the lint output
   in the context. **Do not hand-edit the doc to silence lint.**
   Size-cap **warnings** are advisory; mention them in the wrap-up
   but do not block on them.
6. **Mark fresh.**
   ```bash
   python3 docs/generated/design/_meta/regenerate.py mark-fresh <doc> \
       --model claude-opus-4.7
   ```

### 1d. Wrap up

Show the user:

- The list of docs regenerated.
- Any lint warnings (e.g. size-cap exceedances).
- The output of a final `regenerate.py list-stale` (rows you
  touched should no longer appear as stale).

## Stage 2 - Direct the user to perform the review

The review **cannot** be done by this agent. The workflow requires a
non-Claude model for independent verification, and both the review
prompt and the driver's `mark-reviewed` gate enforce this softly.

Emit the following hand-off block to the user verbatim (substituting
the actual list of docs needing review), then stop and wait.

````
Stage 2 (Review) is yours to drive. The driver does not call any
agent; you run the non-Claude reviewer agent out of band.

1. Pick a non-Claude model (any GPT, Gemini, Llama, or Mistral
   variant works). The review prompt has a hard refusal banner for
   Claude models.

2. List docs needing review:

       python3 docs/generated/design/_meta/regenerate.py review-status

   Rows tagged `unreviewed` or `review-stale` are pending. The docs
   I just regenerated will show `review-stale`.

3. For each pending doc:

   a. Open `docs/generated/design/_meta/prompts/_review.md`.
   b. Show the reviewer agent:
      - the target document (with its YAML front-matter),
      - its per-doc generation prompt (path from
        `regenerate.py show <doc>`) plus
        `docs/generated/design/_meta/prompts/_common.md`,
      - the resolved watched files at the doc's recorded
        `source_commit`,
      - any `depends_on` peer documents.
   c. Save the agent's report to **exactly**
      `docs/generated/design/_meta/reviews/<doc>.review.md`,
      preserving the manifest-key directory hierarchy
      (e.g. `_meta/reviews/pipeline/05-ir-passes.md.review.md`).
      If a report already exists at that path from a prior review
      cycle, **overwrite it in place** - do not create a new
      subdirectory (date-stamped, model-stamped, run-stamped, or
      otherwise) and do not append a suffix to the filename. The
      ledger only knows the canonical path; reports written
      elsewhere are invisible to the driver. The previous report's
      content is preserved in git history.
   d. Lint and record:

          python3 docs/generated/design/_meta/regenerate.py lint <doc>
          python3 docs/generated/design/_meta/regenerate.py mark-reviewed <doc>

      `mark-reviewed` refuses if `reviewer_model` contains
      `claude` or `anthropic`.

4. When `review-status` shows every relevant doc as
   `reviewed-pending-remediation`, come back and ask me to run
   stage 3 (Remediation).
````

## Stage 3 - Remediation

### 3a. Confirm

When the user signals the reviews are complete:

```bash
python3 docs/generated/design/_meta/regenerate.py review-status
```

Identify every row with `reviewed-pending-remediation`. Show that
list to the user and **wait for confirmation** before remediating.

Special case: if a row is still `review-stale`, the doc was
regenerated *after* its last review. Flag this - the user must
re-run the review on that doc before remediation can happen. Skip
the row.

### 3b. Remediate each doc

For each `reviewed-pending-remediation` doc:

1. **Gather inputs.**
   - The target document at `docs/generated/design/<doc>`.
   - The review report at
     `docs/generated/design/_meta/reviews/<doc>.review.md`.
   - The per-doc generation prompt (path from `show <doc>`) plus
     `docs/generated/design/_meta/prompts/_common.md` (to verify
     each finding aligns with the contract before fixing).
   - The resolved watched files at the current `HEAD`
     (`regenerate.py show <doc>` again).
2. **Read** the remediation prompt at
   `docs/generated/design/_meta/prompts/_remediate.md`. For every
   finding ID in the review's `## Findings` table, choose exactly
   one action from `_remediate.md` (`fixed`, `rejected-bogus`,
   `rejected-out-of-scope`, `deferred`, `escalated`).
3. **Apply edits** to the target document for every `fixed`
   action. Keep edits minimal - do not opportunistically rewrite
   unrelated text.
4. **Write the remediation report** to
   `docs/generated/design/_meta/remediations/<doc>.remediation.md`,
   preserving the manifest-key directory hierarchy. The report
   must match the contract in `_remediate.md` (front-matter,
   `## Actions` table covering every finding ID exactly once).
5. **Refresh the digest if the doc changed:**
   ```bash
   python3 docs/generated/design/_meta/regenerate.py mark-fresh <doc> \
       --model claude-opus-4.7
   ```
   Skip this step if every action was a non-edit
   (`rejected-*`, `deferred`, or `escalated`).
6. **Lint and record.**
   ```bash
   python3 docs/generated/design/_meta/regenerate.py lint <doc>
   python3 docs/generated/design/_meta/regenerate.py mark-remediated <doc>
   ```
   `mark-remediated` refuses if `remediator_model` does not contain
   `claude` or `anthropic`.

### 3c. Wrap up

Run `regenerate.py review-status` and report:

- Docs that are now `remediated` (cycle complete).
- Any `deferred` or `escalated` findings the user needs to follow
  up on (cite the remediation report).
- Whether any doc still shows a non-`remediated` status, and why.

## Pitfalls

- **Never hand-edit a generated doc to silence a lint error.**
  Re-prompt yourself with the lint output included; structural
  drift is a generation-quality signal.
- **Never edit the review or remediation reports by hand** either;
  re-generate them through the corresponding agent prompt.
- **Driver scope.** `regenerate.py` does not call any LLM, commit,
  push, or auto-edit documents. All writes happen through the
  agent (you); all bookkeeping happens through the driver. Do not
  shortcut the bookkeeping commands (`mark-fresh`,
  `mark-reviewed`, `mark-remediated`) - they update
  `freshness.json` and `review-state.json`.
- **Manifest gaps.** If you discover a watched_paths_digest did not
  flip when a relevant source file changed, extend
  `docs/generated/design/_meta/manifest.yaml` rather than editing
  the generated doc. See "Lessons from the first end-to-end
  exercise" in `regenerate.md`.
- **Model id discipline.** The id you pass to `--model` must match
  the model performing the step (the soft-refusal gate keys off
  it).
- **Dependency order.** Architecture and overview docs come before
  the pipeline / family pages that depend on them. When in doubt,
  check the `depends_on` lists from `regenerate.py show <doc>`.
