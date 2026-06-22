# Agentic test creation pipeline

How a documentation file becomes a verified, maintained test bundle in
`docs/generated/tests/`, and how the suite feeds back into the
documentation. This is the operator/agent's map of the system; the
normative rules live in [`prompts/_common.md`](prompts/_common.md),
[`prompts/_claims.md`](prompts/_claims.md), and
[`CAMPAIGN.md`](CAMPAIGN.md).

```
 SOURCE DOCS                              CONFIG / PROMPTS
 ───────────                              ────────────────
 docs/language-reference/*.md  (spec) ┐   manifest.yaml  (source_doc + watched_paths
 docs/generated/design/*.md (design) ┤                   + depends_on + coverage_targets)
                                      │   <bundle>/_prompt.md      (per-bundle strategy)
                                      │   prompts/_common.md       (universal rules)
                                      │   prompts/_claims.md       (claim methodology)
                                      │   CAMPAIGN.md              (orchestration loop)
                                      ▼
   ┌─────────────────────────── GENERATION  (per bundle) ───────────────────────────┐
   │ 1 triage doc         → normative? ≥3 testable claims? else → § Skipped docs      │
   │ 2 enumerate claims   → flat numbered list of every documented assertion          │
   │ 3 map claims→tests   → × dimensions (basic/corner/boundary/combinations/types/   │
   │                         back-ends); functional+emission pairs; all-target emit   │
   │ 4 write .slang       → //META (doc_ref, purpose, intent, digest) + //TEST + CHECK │
   │ 5 verify-as-you-go   → slang-test / regenerate.py verify                          │
   │ 6 findings           → suspected compiler bug → _meta/findings/<id>.yaml          │
   │ 7 doc gaps           → doc≠compiler but not a bug → README ## Doc gaps observed   │
   │ 8 bundle README      → front-matter + Claims + Functional/Untested + Doc gaps     │
   │ 9 lint               → structural contract must pass                              │
   └──────────────────────────────────────────────────────────────────────────────────┘
                                      ▼
 OUTPUTS
 ───────
   conformance/<doc>/ , design/<area>/<doc>/   (.slang tests + README.md + _prompt.md)
   _meta/findings/*.yaml      (committed audit trail of suspected compiler bugs)
   _meta/expected-failures.txt (known-failing tests, with tracking links)
   INDEX.md                   (regenerated navigation)
                                      ▼
                       NIGHTLY CI  (slang-test -test-dir docs/generated/tests)
                                      │
   FEEDBACK CHANNELS  ◄───────────────┴──────────────────────────────────────►
   • ## Doc gaps observed  → regenerate.py doc-gaps   → documentation regeneration
   • findings/*.yaml        → regenerate.py findings file → compiler tracking issues
   • source/watched digests → regenerate.py list-stale → regenerate drifted bundles
   • lang-ref-coverage / expansion-candidates → choose what to generate next
```

## 1. Inputs — where the doc material comes from

The suite is **doc-anchored**: every test traces to one documentation
assertion. Two source trees feed it, by role:

| Source tree                    | Drives                 | Authority                                                                                                        |
| ------------------------------ | ---------------------- | ---------------------------------------------------------------------------------------------------------------- |
| `docs/language-reference/*.md` | `conformance/` bundles | Authoritative human-written spec. A failing test is a spec-vs-compiler signal.                                   |
| `docs/generated/design/*.md`   | `design/` bundles      | LLM-reverse-engineered from compiler source; may codify bugs. A failing test is a regression in known behaviour. |

Each bundle is bound to its inputs by `_meta/manifest.yaml`:

- **`source_doc`** — the one doc the bundle's claims derive from; its
  sha256 (`source_doc_digest`) is tracked so doc edits invalidate the
  bundle.
- **`watched_paths`** — compiler source files that, when changed, also
  mark the bundle stale (`watched_paths_digest`).
- **`depends_on`** — sibling bundles whose READMEs are context.
- **`coverage_targets`** — slangc source the bundle is responsible for
  exercising (used only to _rank_ under-coverage, never fed line-by-line
  into generation).

The per-bundle **`_prompt.md`** (co-located in the bundle dir) records
how that doc's claims were extracted. Shared prompts —
`_common.md` (universal rules), `_claims.md` (claim methodology) — and
`CAMPAIGN.md` (the batch protocol) complete the input set.

## 2. Stages — inside generation (per bundle)

1. **Triage** the doc: is it normative? If it is pure intro/glossary,
   `> TODO`, or has fewer than ~3 testable claims, skip it and record the
   reason in `CAMPAIGN.md § Skipped docs`.
2. **Enumerate claims** ([`_claims.md` §1](prompts/_claims.md)): a flat
   numbered list of every normative, testable assertion. This list — not
   a test count — is the bundle's target.
3. **Map claims → tests** along the dimensions the doc commits to (basic
   / corner / boundary / combinations / types / back-ends). Observable
   claims get a functional + emission pair; target-dependent claims emit
   on **every feasible back-end** (HLSL/GLSL/SPIR-V/Metal/WGSL/CUDA/CPP).
4. **Write `.slang` files**: one per (claim × dimension), each with a
   `//META` block (`doc_ref` anchor, `purpose`, `intent`,
   `doc_section_digest`, provenance), `//TEST` directives, and `CHECK`
   lines that encode what the doc says (not merely what the compiler
   emits).
5. **Verify as you write**: `slang-test <file>` or
   `regenerate.py verify <bundle>` (GPU/toolchain-absent tests report
   `ignored`, not failed).
6. **Findings**: when a test reveals a suspected compiler bug, write a
   structured `_meta/findings/<id>.yaml` and add the test to
   `expected-failures.txt` (referencing the pending finding). Agents
   never file issues.
7. **Doc gaps**: when the doc and compiler disagree but it is not a
   compiler bug (fabricated/ambiguous/missing claim), record a row in the
   README's `## Doc gaps observed`.
8. **Bundle README**: front-matter (timestamps, commit, digests) plus the
   claim-bearing sections (`## Claims`, `## Functional coverage`,
   `## Untested claims`, `## Doc gaps observed`).
9. **Lint**: `regenerate.py lint <bundle>` enforces the structural
   contract — every `.slang` has a `//META` block and a `//TEST` with at
   least one `CHECK`; finding YAMLs match the schema; READMEs carry the
   required sections. Agents commit lint-clean bundles; lint runs without
   `slangc`.

## 3. Outputs

- **Bundle directories** under `conformance/` and `design/`, each with
  `.slang` tests, a `README.md`, and a co-located `_prompt.md`.
- **`_meta/findings/*.yaml`** — committed records of suspected compiler
  defects (the audit trail from "agent saw this" to "issue filed").
- **`_meta/expected-failures.txt`** — tests that fail because of a filed
  (or pending) compiler bug, kept out of the CI failure gate.
- **`INDEX.md`** — regenerated bundle index (`regenerate.py index --write`).
- **The nightly CI signal** — `slang-test -test-dir docs/generated/tests`
  runs the whole suite; it is additive and does not gate per-PR.

## 4. User (operator) interaction points

The pipeline is operator-driven at these gates — everything else is
autonomous:

- **Doc selection / campaign launch** — which docs to cover, in what
  order ([`CAMPAIGN.md`](CAMPAIGN.md)).
- **Skip decisions** — a doc deemed unsuitable is recorded in
  `CAMPAIGN.md § Skipped docs` with a rationale so it is not
  re-litigated.
- **Findings triage** — a human reviews `_meta/findings/` and files the
  worthwhile ones as tracking issues (`regenerate.py findings file <id>`),
  de-duping shared root causes (`findings dup`). Agents never call `gh`.
- **Doc-gap review** — `## Doc gaps observed` rows are the human's queue
  for improving the docs.
- **Expected-failures upkeep** — after triage, pending-finding references
  in `expected-failures.txt` are replaced with tracking-issue URLs.
- **PR review** — bundles land as commits; review is the usual PR gate.

## 5. Feedback loops into documentation (and the compiler)

The suite is designed to _improve its own inputs_:

- **Doc gaps → docs.** `regenerate.py doc-gaps` aggregates every
  `## Doc gaps observed` row by source doc; the doc-regeneration workflow
  consumes them to fix `docs/generated/design/` and to flag
  language-reference gaps. A test anchored to the spec that fails is the
  honest signal that the spec or the compiler is wrong.
- **Findings → compiler.** Triaged findings become tracking issues; fixes
  remove the matching `expected-failures.txt` entry.
- **Drift detection → regeneration.** `source_doc_digest` and
  `watched_paths_digest` make a bundle _stale_ when its doc or watched
  compiler source changes; `regenerate.py list-stale` lists them and
  `mark-fresh` clears the flag after regeneration.
- **Coverage signals → what's next.** `regenerate.py lang-ref-coverage`
  (per-file spec coverage %) and `expansion-candidates` /
  `coverage-gaps` (under-exercised `coverage_targets`) tell the campaign
  which docs and bundles to grow — always doc-driven, never aimed at a
  specific uncovered source line.
- **Prompt improvement.** When a bundle is systematically wrong, the fix
  is to the bundle's `_prompt.md` (or the shared prompts), followed by
  regeneration — not a hand-edit of the generated tests.

## Driver command reference

`python3 docs/generated/tests/_meta/regenerate.py <cmd>`:

| Command                                  | Role in the pipeline                                                  |
| ---------------------------------------- | --------------------------------------------------------------------- |
| `list` / `list-stale`                    | enumerate bundles / those whose source or watched paths drifted       |
| `digest <bundle>`                        | compute `watched_paths_digest` + `source_doc_digest` for front-matter |
| `show <bundle>`                          | resolved manifest entry + files                                       |
| `lint [bundle]`                          | structural contract (the agent's commit gate)                         |
| `verify [bundle]`                        | run `slang-test`; reports passed / ignored / expected-fail / FAILED   |
| `index --write`                          | regenerate `INDEX.md`                                                 |
| `lang-ref-coverage`                      | spec coverage odometer                                                |
| `expansion-candidates` / `coverage-gaps` | rank under-coverage (ranking only)                                    |
| `doc-gaps`                               | aggregate `## Doc gaps observed` rows for doc regeneration            |
| `mark-fresh <bundle>`                    | clear the stale flag after regenerating                               |
| `findings list/show/file/dup`            | triage + file suspected-compiler-bug findings                         |
