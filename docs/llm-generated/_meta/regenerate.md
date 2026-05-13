# Regeneration Workflow

This document describes how to (re)generate the LLM-authored documents
under `docs/llm-generated/`. The pipeline is deliberately **operator-driven**:
the driver script tracks state, but document generation itself is an
out-of-band agent invocation that the operator performs.

## Prerequisites

- A Python 3.9+ interpreter (no third-party packages are required; PyYAML
  is used if available but is not mandatory).
- A working `git` and a clean checkout (`git status` should show no
  unintended changes).
- An AI agent capable of reading files in the workspace and writing a
  single markdown document. The pipeline is intentionally model-agnostic:
  any agent that can be pointed at the watched paths and the prompt
  template can be used.

## The driver

All commands run from the repository root.

```bash
python3 docs/llm-generated/_meta/regenerate.py <subcommand> [args...]
```

| Subcommand | Purpose |
| --- | --- |
| `list` | Print every doc in the manifest |
| `list-stale` | Classify each doc as `missing`, `stale`, or `fresh` |
| `digest <doc>` | Compute the current digest of a doc's watched paths |
| `show <doc>` | Show the manifest entry plus the resolved source files |
| `mark-fresh <doc> [--commit SHA] [--model NAME]` | Record a fresh entry |
| `lint [<doc>...]` | Structural linter (front-matter, link resolution, size cap) |

`<doc>` is the manifest key (e.g. `pipeline/05-ir-passes.md`), not the full
workspace path.

## Operator loop

### One-shot bootstrap (Phase 2)

When `docs/llm-generated/` is empty, every doc is `missing`:

```bash
python3 docs/llm-generated/_meta/regenerate.py list-stale
```

Run the agent once per doc, in dependency order (a doc that lists
`depends_on` other docs should be regenerated *after* its dependencies, so
that the agent can read them as additional context). For each doc:

1. Open the prompt template at
   `docs/llm-generated/_meta/<prompt-path>` (the path is shown by
   `regenerate.py show <doc>`).
2. Show the agent the watched files (use `regenerate.py show <doc>` to
   list them) plus any `depends_on` documents that have already been
   generated.
3. Ask the agent to produce the target document, including the front-matter
   block exactly as specified in the prompt template. The `source_commit`
   value should be the current `HEAD`. Compute
   `watched_paths_digest` with `regenerate.py digest <doc>`.
4. Save the agent's output to `docs/llm-generated/<doc>`.
5. Run `regenerate.py lint <doc>`. Fix structural issues (front-matter
   keys, broken links, size cap) by re-prompting the agent — never by
   hand-editing the doc.
6. Once lint passes, run
   `regenerate.py mark-fresh <doc> --model <model-id>` to update
   `freshness.json`.

### Incremental refresh (Phase 4)

When the source tree has moved on:

```bash
python3 docs/llm-generated/_meta/regenerate.py list-stale
```

For each doc reported as `stale`:

1. Optionally show the agent the previous version of the doc and a
   `git diff` of the watched paths since the recorded `source_commit` so
   that it can do a focused refresh rather than a full rewrite.
2. Regenerate the doc, lint, and `mark-fresh` exactly as in the bootstrap
   loop.

If multiple docs are stale, regenerate them in dependency order
(architecture docs before pipeline docs; primary docs before docs that
depend on them).

### Cross-link pass (Phase 3)

After every doc has been generated at least once, do a second pass over
the whole tree where each agent invocation also gets the previously
generated peer docs as context. This typically:

- aligns terminology (e.g. consistent use of "translation unit",
  "module", "compile request");
- inserts cross-references (`[parser pipeline](../pipeline/02-parse-ast.md)`);
- prunes redundant explanations that another doc already covers.

The cross-link pass is just a regular regeneration of every doc with an
expanded context, followed by `lint` and `mark-fresh`.

## Linter exit codes and policy

`lint` exits non-zero if any document has a structural **error**:

- Missing or malformed YAML front-matter
- Required front-matter keys missing (`generated`, `model`,
  `generated_at`, `source_commit`, `watched_paths_digest`, `warning`)
- A markdown link `[text](path)` whose path does not resolve

`lint` reports **warnings** (currently only "size exceeds cap") without
failing. A warning is a hint to either tighten the prompt to produce a
shorter doc, or to raise the cap deliberately in the manifest.

## Lessons from the first end-to-end exercise

The first time the loop is exercised end-to-end it tends to surface
**manifest gaps**: a doc was authored against information drawn from a
file that the manifest does not actually watch, so a real change to that
file does not flag the doc as stale. The recommended response is:

1. Add the missing path to the doc's `watched_paths` in `manifest.yaml`.
2. Re-run `regenerate.py list-stale`. The doc will now be flagged stale
   because the watched-paths digest changed.
3. If the doc's existing content still accurately reflects the file,
   `mark-fresh` it; otherwise regenerate first, then mark fresh.

A concrete example surfaced during the Phase 4 exercise: a temporary
edit to [source/compiler-core/slang-token-defs.h](../../../source/compiler-core/slang-token-defs.h)
flagged only `architecture/module-map.md` (whose `source/compiler-core/slang-*.h`
glob caught it). Both `pipeline/01-lex-preprocess.md` and
`syntax-reference/tokens.md` reverse-engineer their token catalogues from
that file, so the manifest was extended to watch it explicitly. The
expansion is recorded as a manifest entry, not as a hand-edit to the
generated doc.

## What the driver intentionally does NOT do

- It does not call any agent or LLM. Generation is operator-driven.
- It does not commit or push changes.
- It does not auto-edit generated documents. If lint fails, the operator
  re-runs the agent with the lint output included in the prompt context.

## Continuous integration

Wiring this into CI is a deliberate Phase 4 follow-up that has not been
enabled yet. The intended attachment point is a new workflow under
[../../../.github/workflows/](../../../.github/workflows/) that runs
`regenerate.py list-stale` on every PR touching watched paths and posts
a soft warning (not a hard failure) listing stale documents. The warning
should remind the PR author to schedule a documentation refresh, not
block the PR.

The reason for the soft-warning policy is that documentation drift is a
*reviewable* property of a change, not a *blocking* one — most source
changes do not warrant a same-PR doc refresh, especially if the change is
self-contained or temporary. Hard-failing PRs on doc staleness would
either incentivize hand-editing the generated docs (defeating the
experiment) or burn out reviewers with mechanical churn.

### Reference workflow (not enabled — for future hookup)

The existing
[../../../.github/workflows/check-toc.yml](../../../.github/workflows/check-toc.yml)
is the model: a non-blocking docs-hygiene check on PRs. A future
`check-llm-generated-docs.yml` would follow the same shape:

```yaml
# .github/workflows/check-llm-generated-docs.yml  -- NOT YET ENABLED
name: Check LLM-generated docs freshness (advisory only)
on:
  pull_request:
    branches: [master]
    paths:
      - 'source/**'
      - 'prelude/**'
      - 'include/**'
      - 'docs/llm-generated/**'
jobs:
  check-llm-doc-freshness:
    if: github.event.pull_request.draft != true
    runs-on: ubuntu-latest
    continue-on-error: true   # advisory: never blocks the PR
    steps:
      - uses: actions/checkout@v4
        with: { fetch-depth: 0 }
      - name: List stale LLM-generated docs
        run: |
          python3 docs/llm-generated/_meta/regenerate.py list-stale || true
      - name: Lint LLM-generated docs
        run: |
          python3 docs/llm-generated/_meta/regenerate.py lint
```

Notes for whoever enables this later:

- `continue-on-error: true` plus a non-zero exit from `list-stale` is
  what makes the check advisory rather than blocking. Removing that flag
  would convert it to a hard gate; do not do that without revisiting the
  policy above.
- The `lint` step *should* be a hard gate, because lint failures
  indicate structurally invalid docs (broken links, missing front-matter)
  rather than drift. It runs without `|| true`.
- The workflow does not regenerate anything. Regeneration remains
  operator-driven; CI only reports.
- No additional secrets or runners are needed; the driver has no
  third-party Python dependencies.
