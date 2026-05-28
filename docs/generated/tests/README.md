# Agentic Test Suite

This directory contains an agent-generated, agent-maintained test suite
that mirrors the LLM-generated architectural documentation in
[`../design/`](../design/). Each test in this
tree is anchored to a documentation claim, and the suite is rebuilt
when the docs change.

The suite is **additive** — it does not replace the hand-written
`tests/` suite, and it runs **nightly only**, not per PR.

**Where to look next:**

- [`INDEX.md`](INDEX.md) — bundle-by-bundle navigation table with per-bundle test counts and links to each bundle's `README.md`.
- [`_meta/regenerate.md`](_meta/regenerate.md) — operator workflow.
- [`_meta/prompts/_common.md`](_meta/prompts/_common.md) — universal rules every generation agent inherits.

## Layout

| Subtree             | Purpose                                                                                |
| ------------------- | -------------------------------------------------------------------------------------- |
| `architecture/`     | Tests anchored to `docs/generated/design/architecture/*.md`                            |
| `pipeline/`         | Tests anchored to the compilation-pipeline docs (lex → emit)                           |
| `syntax-reference/` | Tests anchored to the syntax / grammar / keywords docs                                 |
| `ast-reference/`    | Tests anchored to the AST node reference                                               |
| `ir-reference/`     | Tests anchored to the IR opcode reference                                              |
| `name-resolution/`  | Tests anchored to scoping / lookup / overload-resolution docs                          |
| `cross-cutting/`    | Tests anchored to diagnostics, IR instruction set, targets, etc.                       |
| `target-pipelines/` | Tests anchored to per-target (SPIR-V/HLSL/Metal/WGSL/CUDA) end-to-end docs             |
| `_meta/`            | Pipeline infrastructure: manifest, prompts, schemas, freshness + findings state, driver |

The directory structure mirrors `docs/generated/design/` exactly: for
every `docs/generated/design/<section>/<doc>.md`, there is one bundle at
`docs/generated/tests/<section>/<doc>/` (no `.md` suffix on the directory).
This is a hard rule enforced by the manifest.

## Bundle shape

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

When the expansion loop runs (Phase E; planned, not yet implemented),
the agent receives the bundle's source doc and the instruction "this
area is under-tested" — it does **not** receive uncovered source-line
numbers. If documented behavior cannot reach uncovered code, the agent
writes that down as a doc gap, not as a synthesized test. See
[`_meta/prompts/_expand.md`](_meta/prompts/_expand.md) for the
expansion contract.

## Phase status

The framework rolls out across several phases. The list below records
what is implemented today versus what is scaffolded or planned:

| Phase                  | Description                                                              | Status                                              |
| ---------------------- | ------------------------------------------------------------------------ | --------------------------------------------------- |
| A                      | Framework scaffold (driver, schemas, prompts, manifest)                  | implemented                                         |
| B1                     | Bootstrap generation across 47 bundles                                   | implemented                                         |
| B1.5 / B1.6 / B1.7     | Boundary expansion, coverage-driven refinement, diagnostics-catalog sweep | implemented                                         |
| B2                     | `slang-test -test-dir docs/generated/tests` wired into a nightly job     | planned                                             |
| C                      | Cross-link pass — bundles consume each other's READMEs                   | planned                                             |
| D                      | Review + remediation against a non-Claude model                          | scaffolded (schemas, prompts, state file in place)  |
| E                      | Coverage-driven expansion loop                                           | scaffolded (driver subcommands stub the data flow)  |
| F                      | Structured compiler-bug findings + operator-driven filing                | implemented                                         |

## Trust model

- The source code is authoritative.
- The hand-written `tests/` suite is authoritative for the behaviors
  it covers.
- The docs in [`../design/`](../design/) are
  the spec this suite checks against. They may drift from source; the
  doc-side regeneration loop closes that drift.
- Tests under `docs/generated/tests/` are valid only to the extent that their
  cited docs are valid. A failing agentic test means one of:
  doc says X but compiler does not-X (compiler bug **or** doc bug),
  or the test is wrong (regenerate).

## Running the suite

Two complementary checks gate the suite.

**Lint is the primary contract.** It validates that each `.slang` file
has a matching `//META` block and a `//TEST` directive with at least
one `CHECK` line, that every finding YAML conforms to the finding
schema, and that bundle READMEs carry the required front-matter and
section structure. Lint runs without `slangc`; agents commit
lint-clean bundles.

```bash
python3 docs/generated/tests/_meta/regenerate.py lint
```

**`slang-test` is the runtime check.** It compiles every `.slang` file
against the targets each test declares. CI nightly runs this once
Phase B2 lands; locally it is optional.

```bash
./build/Release/bin/slang-test -use-test-server -server-count 4 -test-dir docs/generated/tests
```

Backends the runner doesn't have (GPU runtimes, `dxc.exe`, Apple
toolchain, nvrtc, etc.) are ignored, not failed.

## Driving the framework

```bash
python3 docs/generated/tests/_meta/regenerate.py list             # bundle keys
python3 docs/generated/tests/_meta/regenerate.py list-stale       # bundles whose source doc / watched paths drifted
python3 docs/generated/tests/_meta/regenerate.py lint             # structural lint
python3 docs/generated/tests/_meta/regenerate.py show <bundle>    # manifest entry + resolved files
python3 docs/generated/tests/_meta/regenerate.py index --write    # regenerate INDEX.md
python3 docs/generated/tests/_meta/regenerate.py doc-gaps         # aggregated doc-gap rows, grouped by source doc
python3 docs/generated/tests/_meta/regenerate.py coverage-gaps <bundle> --from <report>.txt
python3 docs/generated/tests/_meta/regenerate.py findings list             # Phase F: pending compiler-bug findings
python3 docs/generated/tests/_meta/regenerate.py findings show <id>        # render finding as issue body
python3 docs/generated/tests/_meta/regenerate.py findings file <id>        # gh issue create + set project fields
python3 docs/generated/tests/_meta/regenerate.py findings dup <id> --of N  # mark finding as dup-of existing issue
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

## Compiler bug findings

When generation surfaces what looks like a compiler defect (SIGSEGV,
missing diagnostic, wrong codegen, regression, catalog drift), the
generation agent writes a structured **finding** to
`_meta/findings/<id>.yaml`. Findings are committed alongside the
tests — they are the audit trail from "agent saw this" to "issue
filed."

Filing is operator-driven and explicit. The agent never calls `gh`;
that boundary is enforced by the
[`_meta/prompts/_common.md`](_meta/prompts/_common.md) contract.

```bash
python3 docs/generated/tests/_meta/regenerate.py findings list             # what's pending
python3 docs/generated/tests/_meta/regenerate.py findings show <id>        # render the issue body
python3 docs/generated/tests/_meta/regenerate.py findings file <id>        # gh issue create + set project fields
python3 docs/generated/tests/_meta/regenerate.py findings dup <id> --of N  # mark dup-of existing issue
```

Filed issues carry the `Test Agent Finding` label and land in the
`Slang-All` project (#10) with `Status=Todo`, `Priority=P2`,
`Release=Unplanned`, `Size=XS (~1d)`, `Estimate=1`. The live queue:

[Open `Test Agent Finding` issues on shader-slang/slang](https://github.com/shader-slang/slang/issues?q=is%3Aissue%20state%3Aopen%20label%3A%22Test%20Agent%20Finding%22)

`findings show` is the dedup gate: the operator inspects the rendered
issue body and searches the tracker before invoking `findings file`.
A finding whose citation does not resolve on disk is rejected by lint
and refuses to render — this prevents the rendered issue from
referencing files that don't exist.

## Reporting an issue

A failing test should not be hand-edited. Instead:

- If the **doc** is wrong, file a prompt-improvement task against
  `docs/generated/design/_meta/prompts/<...>.md`.
- If the **prompt** that produced the bundle is wrong, file an issue
  against `docs/generated/tests/_meta/prompts/<bundle>.md`.
- If the **manifest** is wrong (missing a watched path, wrong source
  doc), edit `docs/generated/tests/_meta/manifest.yaml`.
- If the **compiler** is wrong, write a finding YAML to
  `_meta/findings/<id>.yaml` per the schema, then have the operator
  triage and file via `regenerate.py findings file <id>`. See
  "Compiler bug findings" above.
- Then re-run `regenerate.py mark-fresh <bundle>` after regeneration.
