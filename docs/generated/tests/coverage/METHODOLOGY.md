# Coverage-gap suite — methodology (white-box characterization tests)

This is the **third** generated-test tree, a peer of `conformance/` and
`design/`, and it is deliberately different from both. Read this before adding
anything here.

## What this tree is — and is not

These are **white-box characterization tests**. They are selected by _coverage
gap_ (uncovered compiler code) and they assert _what slangc currently does_,
not what the spec says it should do.

- `conformance/` asserts the **language reference** (a failure = spec-vs-compiler drift).
- `design/` asserts the **generated design docs** (a failure = behavioural regression in codified behaviour).
- `coverage/` asserts **observed current behaviour of otherwise-untested code** (a failure = the code changed; triage decides if that's a fix or a regression).

This tree exists for one reason: the doc-anchored flow plateaus below the
coverage we want because the docs do not describe every reachable branch. This
tree targets those branches directly. It is the **one place where reading a
coverage report and the compiler source to choose a test is allowed** — the
exact opposite of [`_expand.md` § The hard rule](../_meta/prompts/_expand.md).
Everywhere else, that rule still holds.

## The standing risk: do not codify bugs as "expected"

Because these tests pin _current_ output, a test written against a buggy branch
locks the bug in. Mitigation is mandatory:

- **Pin the real observed output** (run slangc, copy verbatim — never invent).
- If you cannot confirm the behaviour is _correct_ (no spec, or unsure), tag the
  test `//META: characterization-unverified=true` so a human/spec review can
  revisit it. Pinning unverified behaviour is allowed **only** with this tag.
- **A crash / abort / internal-error / malformed output is a finding, never a
  test** — file it under `_meta/findings/` per
  [`_common.md` § Reporting suspected compiler bugs](../_meta/prompts/_common.md).
- Where a language-reference claim _does_ cover the behaviour, prefer writing
  the test in `conformance/` instead — this tree is for the genuinely
  undocumented reachable code.

## Triage first — only target reachable gaps

Most uncovered lines are **not** worth (or possible) to target. Before writing
tests, classify each gap:

| Class                                                                                         | Action                                                                                     |
| --------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------ |
| **reachable from the slangc / slangi CLI**                                                    | target it — this is the suite's job                                                        |
| **runner-gated** (needs a GPU / DXC / nvrtc / Metal toolchain)                                | write the test with `//META: requires-tool=...`; CI validates, local runs report `ignored` |
| **defensive / unreachable** (asserts on impossible states, `default:` on exhaustive switches) | do **not** target — record why in the bundle README `## Unreachable gaps`                  |
| **dead code** (no caller reachable from any entry)                                            | do **not** target — a candidate finding for removal, not a test                            |

The triage map (what is uncovered and _why_) is itself a deliverable — it tells
us the real reachable ceiling.

## Per-test contract

Each `.slang` (or CLI/unit harness) test must:

- drive real input through `build/.../slangc` (or `slangi`) and **compile/run cleanly**;
- pin the **observed** output (`filecheck` / `filecheck-buffer`) or diagnostic (`diag`), copied verbatim from real output; tight on the distinguishing token, wild on incidental ids/registers;
- carry the standard `//META` block **plus**:
  - `//META: intent=characterization`
  - `//META: covers=source/slang/<file>.cpp` (the source file/area the test is for — the white-box target; this replaces the `doc_ref` requirement, which does not apply here)
  - `//META: characterization-unverified=true` when correctness is unconfirmed (see above).

`doc_ref` is **not** required in this tree. This tree is **exempt from the
emission fan-out gate** and from the doc-anchor lint; its own lint requires
`intent=characterization`, a `covers=` target, and at least one matcher.

## How this tree is generated

The sweep that produced these bundles is
[`_meta/workflows/whitebox-coverage.js`](../_meta/workflows/whitebox-coverage.js).
It hard-codes the reachable source-area clusters chosen by gap triage (one per
bundle) together with the line-coverage figure that motivated each, and runs one
agent per cluster. It is kept in the corpus so this tree is reproducible rather
than only described: re-running it is what regenerates these bundles.

Unlike the doc-anchored trees, these bundles carry `role: coverage` in
`_meta/manifest.yaml` and declare **no** `source_doc` — only `watched_paths`,
which are the compiler sources they characterize. A change to those sources is
the only thing that marks a coverage bundle stale.

## Bundle layout

Organised by the source area being covered, e.g.
`coverage/emit/`, `coverage/legalize/`, `coverage/parser/`. Each bundle
has a `README.md` with: `## Intent`, `## Functional coverage` (claim cell =
what the test pins + the `covers=` target), `## Unreachable gaps`, and
`## Doc gaps observed` (when a reachable behaviour _should_ have been documented,
feed it back to the doc tree). Findings go to `_meta/findings/` as usual.
