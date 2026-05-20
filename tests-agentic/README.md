# Agentic Test Suite

This directory contains an agent-generated, agent-maintained test suite
that mirrors the LLM-generated architectural documentation in
[`../docs/llm-generated/`](../docs/llm-generated/). Each test in this
tree is anchored to a documentation claim, and the suite is rebuilt
when the docs change.

The suite is **additive** — it does not replace the hand-written
`tests/` suite, and it runs **nightly only**, not per PR.

## What lives here

| Subtree             | Purpose                                                                      |
| ------------------- | ---------------------------------------------------------------------------- |
| `architecture/`     | Tests anchored to `docs/llm-generated/architecture/*.md`                     |
| `pipeline/`         | Tests anchored to the compilation-pipeline docs (lex → emit)                 |
| `syntax-reference/` | Tests anchored to the syntax / grammar / keywords docs                       |
| `ast-reference/`    | Tests anchored to the AST node reference                                     |
| `ir-reference/`     | Tests anchored to the IR opcode reference                                    |
| `name-resolution/`  | Tests anchored to scoping / lookup / overload-resolution docs                |
| `cross-cutting/`    | Tests anchored to diagnostics, IR instruction set, targets, etc.             |
| `target-pipelines/` | Tests anchored to per-target (SPIR-V/HLSL/Metal/WGSL/CUDA) end-to-end docs   |
| `_meta/`            | Pipeline infrastructure: manifest, prompts, schemas, freshness state, driver |

The directory structure mirrors `docs/llm-generated/` exactly: for
every `docs/llm-generated/<section>/<doc>.md`, there is one bundle at
`tests-agentic/<section>/<doc>/` (no `.md` suffix on the directory).
This is a hard rule enforced by the manifest.

## How a bundle is shaped

Each bundle directory contains:

```
<bundle>/
├── BUNDLE.md          # YAML front-matter + claims/tests index
├── <test>.slang       # Test file with a //META header + a //TEST directive
├── <test>.expected    # Optional checked-in expected output
└── ...
```

Every `.slang` file begins with a `//META` block of `key=value` lines
declaring `doc_ref` (the documentation anchor the test is derived
from), `doc_section_digest` (used to detect doc drift), `intent`
(functional / expansion / regression / negative), and the usual
generation provenance. See
[`_meta/prompts/_common.md`](_meta/prompts/_common.md) for the full
contract.

## Tests derive from docs — coverage does not

The single load-bearing rule for this suite:

> **Every test is anchored to a documentation claim. Coverage data is a
> signal of where to focus doc-driven expansion, never a target to
> chase line-by-line.**

When the expansion loop (Phase E) is triggered, the agent receives the
bundle's source doc and the instruction "this area is under-tested" —
it does **not** receive uncovered source-line numbers. If documented
behavior cannot reach uncovered code, the agent writes that down as a
doc gap, not as a synthesized test. See
[`_meta/prompts/_expand.md`](_meta/prompts/_expand.md) for the
expansion contract.

## Trust model

- The source code is authoritative.
- The hand-written `tests/` suite is authoritative for the behaviors
  it covers.
- The docs in [`../docs/llm-generated/`](../docs/llm-generated/) are
  the spec this suite checks against. They may drift from source; the
  doc-side regeneration loop closes that drift.
- Tests under `tests-agentic/` are valid only to the extent that their
  cited docs are valid. A failing agentic test means one of:
  doc says X but compiler does not-X (compiler bug **or** doc bug),
  or the test is wrong (regenerate).

## Running

Phase A (the current state) is scaffold only — there are no bundles
yet. Drive the framework with:

```bash
python3 tests-agentic/_meta/regenerate.py list           # 51 bundle keys
python3 tests-agentic/_meta/regenerate.py list-stale     # all "missing"
python3 tests-agentic/_meta/regenerate.py lint           # 0 errors
python3 tests-agentic/_meta/regenerate.py show <bundle>
```

Bootstrap generation (Phase B1), slang-test wiring (Phase B2),
cross-link (Phase C), review/remediate (Phase D), and the expansion
loop (Phase E) land in subsequent PRs. See
[`_meta/regenerate.md`](_meta/regenerate.md) for the operator
workflow.

## Reporting an issue

A failing test should not be hand-edited. Instead:

- If the **doc** is wrong, file a prompt-improvement task against
  `docs/llm-generated/_meta/prompts/<...>.md`.
- If the **prompt** that produced the bundle is wrong, file an issue
  against `tests-agentic/_meta/prompts/<bundle>.md`.
- If the **manifest** is wrong (missing a watched path, wrong source
  doc), edit `tests-agentic/_meta/manifest.yaml`.
- If the **compiler** is wrong, file it against the source as usual.
- Then re-run `regenerate.py mark-fresh <bundle>` after regeneration.
