# Agentic Test Suite

This directory contains an agent-generated, agent-maintained test suite
that mirrors the LLM-generated architectural documentation in
[`../docs/generated/design/`](../docs/generated/design/). Each test in this
tree is anchored to a documentation claim, and the suite is rebuilt
when the docs change.

The suite is **additive** — it does not replace the hand-written
`tests/` suite, and it runs **nightly only**, not per PR.

**Where to look next:**

- [`INDEX.md`](INDEX.md) — bundle-by-bundle navigation table with per-bundle test counts and links to each bundle's `README.md`.
- [`_meta/regenerate.md`](_meta/regenerate.md) — operator workflow.
- [`_meta/prompts/_common.md`](_meta/prompts/_common.md) — universal rules every generation agent inherits.

## What lives here

| Subtree             | Purpose                                                                      |
| ------------------- | ---------------------------------------------------------------------------- |
| `architecture/`     | Tests anchored to `docs/generated/design/architecture/*.md`                     |
| `pipeline/`         | Tests anchored to the compilation-pipeline docs (lex → emit)                 |
| `syntax-reference/` | Tests anchored to the syntax / grammar / keywords docs                       |
| `ast-reference/`    | Tests anchored to the AST node reference                                     |
| `ir-reference/`     | Tests anchored to the IR opcode reference                                    |
| `name-resolution/`  | Tests anchored to scoping / lookup / overload-resolution docs                |
| `cross-cutting/`    | Tests anchored to diagnostics, IR instruction set, targets, etc.             |
| `target-pipelines/` | Tests anchored to per-target (SPIR-V/HLSL/Metal/WGSL/CUDA) end-to-end docs   |
| `_meta/`            | Pipeline infrastructure: manifest, prompts, schemas, freshness state, driver |

The directory structure mirrors `docs/generated/design/` exactly: for
every `docs/generated/design/<section>/<doc>.md`, there is one bundle at
`docs/generated/tests/<section>/<doc>/` (no `.md` suffix on the directory).
This is a hard rule enforced by the manifest.

## How a bundle is shaped

Each bundle directory contains:

```
<bundle>/
├── README.md          # YAML front-matter + claims/tests index
├── <test>.slang       # Test file with a //META header + a //TEST directive
├── <test>.expected    # Optional checked-in expected output
└── ...
```

Every `.slang` file begins with a `//META` block of `key=value` lines
declaring `doc_ref` (the documentation anchor the test is derived
from), `doc_section_digest` (used to detect doc drift), `intent`
(`functional` / `boundary` / `negative` / `stress` / `expansion` /
`regression`), and the usual generation provenance. See
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
- The docs in [`../docs/generated/design/`](../docs/generated/design/) are
  the spec this suite checks against. They may drift from source; the
  doc-side regeneration loop closes that drift.
- Tests under `docs/generated/tests/` are valid only to the extent that their
  cited docs are valid. A failing agentic test means one of:
  doc says X but compiler does not-X (compiler bug **or** doc bug),
  or the test is wrong (regenerate).

## Running the suite

The full suite is run by `slang-test`:

```bash
./build/Release/bin/slang-test -use-test-server -server-count 4 -test-dir docs/generated/tests
```

This runs every `.slang` file in the tree. Backends that the runner
doesn't have (GPU runtimes, `dxc.exe`, Apple toolchain, nvrtc, etc.)
are ignored, not failed. CI nightly is expected to lift the ignored
set by providing the missing toolchains.

## Driving the framework

```bash
python3 docs/generated/tests/_meta/regenerate.py list           # bundle keys
python3 docs/generated/tests/_meta/regenerate.py list-stale     # bundles whose source doc / watched paths drifted
python3 docs/generated/tests/_meta/regenerate.py lint           # structural lint
python3 docs/generated/tests/_meta/regenerate.py show <bundle>  # manifest entry + resolved files
python3 docs/generated/tests/_meta/regenerate.py index --write  # regenerate INDEX.md
python3 docs/generated/tests/_meta/regenerate.py doc-gaps       # aggregated doc-gap rows, grouped by source doc
python3 docs/generated/tests/_meta/regenerate.py coverage-gaps <bundle> --from <report>.txt
```

See [`_meta/regenerate.md`](_meta/regenerate.md) for the full
operator workflow.

## Bundle README structure

Every bundle's `README.md` carries YAML front-matter (`generated_at`,
`source_commit`, `watched_paths_digest`, `source_doc_digest`) and
four canonical sections, in this order:

1. `## Intent` — a paragraph on which doc this bundle exercises and
   the coverage strategy.
2. `## Functional coverage` — `Claim | Intent | Anchor | Tests` table.
   One row per documented claim that has a test; the Claim cell
   matches the test's `//META: purpose=...` line verbatim.
3. `## Untested claims` — `Claim | Reason | Anchor | Why untested`
   table. Doc claims without a test yet. The `Reason` controlled
   vocabulary classifies each into a test-harness alternative
   (`needs-unit-test` / `needs-multi-file-test` / `needs-cli-test`),
   a runner-capability alternative (`gpu-dxr`, `gpu-cuda`,
   `gpu-metal-toolchain`, …), or a terminal category
   (`link-stage-only`, `out-of-bundle`, `deprecated`, …).
4. `## Doc gaps observed` — `Anchor | Kind | Gap | Suggested
   addition` table. The feedback channel into doc regeneration. Rows
   are aggregated by `regenerate.py doc-gaps` and consumed by the
   doc-regen workflow.

Empty sections are filled with `NA` rather than omitted. Bundle-
specific sections (e.g., `## Sibling-bundle overlap`,
`## Catalog coverage`, `## Codes dropped`) may appear after the four
canonical headings.

## Reporting an issue

A failing test should not be hand-edited. Instead:

- If the **doc** is wrong, file a prompt-improvement task against
  `docs/generated/design/_meta/prompts/<...>.md`.
- If the **prompt** that produced the bundle is wrong, file an issue
  against `docs/generated/tests/_meta/prompts/<bundle>.md`.
- If the **manifest** is wrong (missing a watched path, wrong source
  doc), edit `docs/generated/tests/_meta/manifest.yaml`.
- If the **compiler** is wrong, file it against the source as usual.
- Then re-run `regenerate.py mark-fresh <bundle>` after regeneration.
