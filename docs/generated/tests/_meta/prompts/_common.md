# Common Rules for docs/generated/tests Generation

These rules apply to every per-section prompt under this directory. The
per-section prompt extends or overrides them; if there is a conflict, the
per-section prompt wins for that bundle.

## What you are producing

You are producing a **test bundle**: a directory at
`docs/generated/tests/<bundle-key>/` containing:

- exactly one `README.md` with YAML front-matter and a claims/tests index;
- N `.slang` test files, each with a `//META` block at the top;
- optionally, `.expected` outputs alongside their `.slang` files.

The bundle key matches the manifest key (e.g. `pipeline/03-semantic-check`).
The corresponding source documentation is named by the manifest's
`source_doc` field — read it first, then read everything it links to.

## The single most important rule

**Every test must be anchored to a documentation claim.** Each test's
`//META: doc_ref=...` field points at a real `path#anchor` inside the
bundle's `source_doc` (or a secondary doc the per-section prompt
explicitly allows). The test's `purpose` field paraphrases the claim
being verified.

### Source-of-truth hierarchy

When a behaviour is described in more than one place, **the language
reference outranks the generated design docs**.

1. **`docs/language-reference/*.md`** — the human-written language
   reference manual. Describes what Slang behaviour _should be_. This
   is the **authoritative spec**. Anchor to the language reference
   whenever it covers the claim you are testing.
2. **`docs/generated/design/*.md`** — the LLM-generated architectural
   docs. Reverse-engineered from the compiler source and therefore
   liable to codify bugs as if they were intentional behaviour.
   Anchor here **only when the language reference is silent** on the
   specific behaviour (the language reference is a work-in-progress
   and incomplete; the design-doc fallback path stays load-bearing).
3. **Compiler source** — never the test's primary citation. See
   `## Reporting suspected compiler bugs` for how to handle
   spec-vs-compiler disagreements you observe.

If you find a behaviour described in **both** the language reference
and a generated-design doc, and the two disagree, anchor the test to
the **language reference** and let the test fail. The failure is the
signal: the compiler matches one of them and not the other, and the
human triage step decides whether the spec or the compiler is wrong.
Write a `drift-from-source` doc-gap row in the bundle README naming
both citations so the next regeneration of either doc consumes it.

### Anchor format

The `//META: doc_ref=` value is a single `path#anchor` pointing at the
authoritative doc for that one claim. Examples:

```
//META: doc_ref=docs/language-reference/expressions-literal.md#integer-literal-expressions
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#name-binding
```

The path must exist; the lint pass validates this. If the anchor
fragment does not match a heading in the file, the test still passes
lint but the citation is fragile and reviewers will catch it.

### Where the test lives — role-based trees (`spec/` + `regression/`)

The framework uses **two parallel test trees** with distinct roles.
Each bundle's `_prompt.md` is co-located in the bundle directory.

| Tree                                                                                                                                                                              | Bundles' `source_doc` points at                         | Role                                                                                             |
| --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------- | ------------------------------------------------------------------------------------------------ |
| `docs/generated/tests/spec/`                                                                                                                                                      | `docs/language-reference/*.md` (the human-written spec) | **Spec conformance.** A failing test is a spec-vs-compiler signal.                               |
| `docs/generated/tests/regression/<area>/` (areas: `pipeline/`, `ast-reference/`, `ir-reference/`, `syntax-reference/`, `name-resolution/`, `cross-cutting/`, `target-pipelines/`) | `docs/generated/design/*.md` (LLM-derived design docs)  | **Regression coverage.** A failing test is a behavioural regression in known-codified behaviour. |

The two trees are intentionally allowed to overlap on the same
compiler surface — they verify _different_ properties (spec
conformance vs. behavioural regression). When a `spec/` test fails and
its `regression/` counterpart passes, that IS the spec-vs-compiler
drift signal the suite is designed to surface; file a finding.

**Where to write a new test:**

1. If the language reference describes the claim → put the test in
   `docs/generated/tests/spec/<doc-name>/` (creating the bundle if
   absent). `<doc-name>` mirrors the lang-ref filename without the
   `.md` suffix.
2. Otherwise → put the test in the appropriate existing `regression/`
   bundle.
3. If you find the same surface described in both, write a test in
   **both** trees. They are doing different jobs.

This keeps coverage of the language reference trivially measurable
per-bundle, lets the bundle path tell a reviewer the failure
semantics at a glance, and removes the per-test ambiguity an "anchor
tag" approach would introduce.

### Campaign mode (autonomous batch generation)

When an operator runs an autonomous-generation campaign across many
lang-ref docs at once, the **bug-finding flow is local-only**:

- Write the structured finding YAML under
  `docs/generated/tests/_meta/findings/<id>.yaml` exactly as the
  schema requires.
- Run `regenerate.py lint` to validate the YAML.
- Add the affected test path to
  `docs/generated/tests/_meta/expected-failures.txt` with a `#`
  comment naming the _local_ finding ID (not yet a tracking-issue
  URL — that comes later).
- **Do NOT** run `regenerate.py findings file <id>` during campaign
  mode. The YAML stays in `_meta/findings/` (not `_meta/findings/filed/`).
  A later human-led triage pass reviews the pending findings, edits
  them if needed, files the ones worth filing, and de-dupes any that
  share root cause.

The expected-failures-lint enforces a tracking-issue-style link
above every entry block; during campaign mode you can satisfy this
with the pending finding's filename:

```
# Pending: docs/generated/tests/_meta/findings/slangi-foo-bar.yaml
# (will become a tracking issue at human-triage time)
docs/generated/tests/spec/.../some-test.slang
```

For the full claim-driven generation methodology a campaign session
follows, see [`_claims.md`](_claims.md) and [`../CAMPAIGN.md`](../CAMPAIGN.md).

This preserves the bug signal without generating a flood of GitHub
issues during a single-session burst.

You will be asked at times to "increase coverage" or to "expand" a
bundle. **You must never** in that case:

- read or be shown a coverage report;
- look at uncovered source-line numbers;
- write a test aimed at exercising a specific line, branch, or function
  of `slangc`;
- invent a test whose behavior is not described by the source doc.

The right move when an expansion pass is requested is: re-read the
source doc more carefully, find behaviors that are mentioned but
under-represented in the current bundle, and add tests for those. If a
claim isn't in the doc, _don't write a test for it_ — instead, record a
"doc-gap" finding in `README.md` under `## Doc gaps observed`.

## README.md front-matter

Every `README.md` must begin with:

```yaml
---
generated: true
model: <your model identifier>
generated_at: <ISO 8601 timestamp, UTC>
source_commit: <git HEAD when you ran>
watched_paths_digest: <sha256 from regenerate.py digest>
source_doc: docs/generated/design/<...>.md
source_doc_digest: <sha256 of source_doc file at source_commit>
warning: "Auto-generated. May drift from source. Do not edit by hand."
---
```

Compute `watched_paths_digest` and `source_doc_digest` with
`python3 docs/generated/tests/_meta/regenerate.py digest <bundle>`.

## README.md body structure

Every bundle README has exactly these four sections, in this order:

1. `## Intent`
2. `## Functional coverage`
3. `## Untested claims`
4. `## Doc gaps observed`

If a section has nothing to record, write `NA` as its body. Do not
omit the heading. Bundle-specific sections (e.g. `## Sibling-bundle
overlap`, `## Catalog coverage`, `## Codes dropped`) may appear after
the four canonical sections but never between them.

```markdown
# Tests for <bundle-key>

## Intent

One short paragraph: which doc this bundle exercises, and the coverage
strategy (e.g. "one positive + one negative per claim in sections
3.1–3.5").

## Functional coverage

| Claim                                                                          | Intent     | Anchor                                                               | Tests                                                                    |
| ------------------------------------------------------------------------------ | ---------- | -------------------------------------------------------------------- | ------------------------------------------------------------------------ |
| The more specialized generic wins when both candidates are equally accessible. | functional | [#overload-resolution](../../../design/<doc>.md#overload-resolution) | [`overload-prefer-specialized.slang`](overload-prefer-specialized.slang) |
| ...                                                                            | ...        | ...                                                                  | ...                                                                      |

## Untested claims

| Claim                                                                                                                                     | Reason          | Anchor                                                           | Why untested                                                                                                                                     |
| ----------------------------------------------------------------------------------------------------------------------------------------- | --------------- | ---------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------ |
| The doc describes the `serialize(serializer, value)` template pattern that compiler internals use to thread values through serialization. | needs-unit-test | [#serialize-pattern](../../../design/<doc>.md#serialize-pattern) | A C++ idiom invoked from compiler-internal code; no slangc CLI surface reaches it. A C++ unit test against the serializer types could verify it. |
| The `closesthit` entry point lowers to a DXR `[shader("closesthit")]` HLSL function.                                                      | gpu-dxr         | [#dxr-ray-tracing](../../../design/<doc>.md#dxr-ray-tracing)     | Agent runtime has no GPU; CI nightly has D3D12 + DXR support.                                                                                    |
| ...                                                                                                                                       | ...             | ...                                                              | ...                                                                                                                                              |

## Doc gaps observed

| Anchor                                                             | Kind            | Gap                                                                                                             | Suggested addition                                                              |
| ------------------------------------------------------------------ | --------------- | --------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------- |
| [#basic-scalar-types](../../../design/<doc>.md#basic-scalar-types) | missing-surface | The doc lists `IntPtr` and `UIntPtr` as opcodes but does not name a Slang surface construct that produces them. | A "user-level surface" column per row, or a note that these are host-side only. |
| ...                                                                | ...             | ...                                                                                                             | ...                                                                             |
```

**Functional-coverage table rules:**

- One row per documented claim. The **Claim** cell paraphrases what the
  test verifies in one plain-English sentence; it must match the test's
  `//META: purpose=...` line verbatim.
- **Intent** uses the controlled vocabulary `functional | boundary |
negative | expansion | regression` and matches the test's
  `//META: intent=...` field. When multiple tests with different
  intents share one claim row, list them comma-separated
  (`functional, boundary`).
- **Anchor** is a markdown link to the cited section of the source doc:
  `[#anchor-name](<relative-path-to-doc>#anchor-name)`. Use the same
  anchor that appears in the tests' `//META: doc_ref=...` fields.
- **Tests** is a comma-separated list of clickable filenames in the
  bundle directory, formatted ``[`filename.slang`](filename.slang)``.
  Never repeat a claim across rows — if multiple tests verify the same
  claim (identical purpose), group them in the Tests cell of a single
  row.
- Rows are sorted by anchor, then by claim text, so claims under the
  same doc section cluster.

The lint pass verifies that every `.slang` file in the bundle directory
appears in the Functional coverage table's Tests column.

**Doc-gap table rules:**

The `## Doc gaps observed` section is the **feedback channel into doc
regeneration**. The downstream doc-regen workflow aggregates every
bundle's gap rows (grouped by `source_doc`) and uses them as input
when the next revision of `docs/generated/design/<doc>.md` is generated.
That means each row must be self-contained enough for an agent who
has never seen this bundle to know which doc section to edit and what
to add.

- **Anchor** is a markdown link to the doc section the gap is about,
  using the same anchor shape as the Coverage table. If the gap spans
  multiple sections, pick the most specific anchor and mention the
  others in the Gap cell.
- **Kind** is one value from the controlled vocabulary:
  | Kind | When to use |
  | ------------------------ | ----------------------------------------------------------------------------------------------------------------- |
  | `missing-example` | Doc names a claim but does not include a minimal example shader that exercises it. |
  | `missing-surface` | Doc names an IR / AST / internal construct but does not name the user-level syntax or builtin that produces it. |
  | `undocumented-behavior` | Observed compiler behavior is real and reachable but the doc is silent about it (no claim, no caveat, no warning).|
  | `cascading-only-mention` | Doc describes a diagnostic or behavior that is always shadowed in practice by an earlier diagnostic or pass. |
  | `ambiguous-claim` | Doc claim has more than one reasonable interpretation; tests cannot anchor to it without making a guess. |
  | `drift-from-source` | Observed compiler behavior contradicts what the doc says (e.g., doc says "lowers to `select`" but actually emits `ifElse`). |
- **Gap** is one to three sentences naming what the doc lacks, in the
  voice of a reader of the doc. Quote the doc literally when the gap
  is about its exact wording. Do **not** describe internal compiler
  details — restrict yourself to observations a doc-only reader could
  make plus your own test observations.
- **Suggested addition** is one to three sentences proposing concrete
  doc edit (e.g., "Add a one-line `user surface` column to the
  Storage-only floating-point table with the syntax that declares
  each opcode"). The doc-regen agent reads this verbatim, so write
  it as an actionable instruction, not a question.

Rows from multiple bundles that report the same anchor + kind are
merged by the aggregator (`regenerate.py doc-gaps`), so independent
co-reports are fine — they are useful evidence.

**Untested-claims table rules:**

The `## Untested claims` section is the **bucket of doc claims that
do not yet have a test in this bundle**. Each row names a claim and
why no test exists — either the kind of test harness that _would_
verify it (C++ unit test, multi-file test, CLI test), the runner /
capability that _would_ unblock it (GPU runtime, DXC, etc.), or, for
truly terminal cases, why no upgrade unblocks it. The doc-regen
workflow does **not** consume this table; it exists to make the
suite's coverage boundary visible to readers, and to surface
candidates for follow-up campaigns.

Columns: `Claim | Reason | Anchor | Why untested`. The Claim is the
primary content; Anchor is a supporting pointer.

- **Claim**: one to three sentences naming what the doc states.
  Lead with the claim because that's the substance — readers and
  downstream tooling want to know _what_ before _why_.
- **Reason**: controlled vocabulary naming what is needed (when
  testable elsewhere) or why the claim is unreachable. Three families:

  Test-harness alternatives — testable, just not via slang-test `//TEST`:

  | Reason                  | Meaning                                                                                                                                                                                                                |
  | ----------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
  | `needs-unit-test`       | The claim is about C++ behavior (an `ISession` method, a `serialize(...)` template pattern, an internal helper) that has no slangc CLI surface but could be tested by a C++ unit test (in `tools/slang-unit-test/`).   |
  | `needs-multi-file-test` | The claim is observable only across 2+ `.slang` files (cross-module `import`, `__include` / `__implementing`, `-embed-downstream-ir` artefacts, `.slang-module` linking).                                              |
  | `needs-cli-test`        | The claim is about how `slangc` is invoked (help output, exit codes, env-var-gated paths like `SLANG_RUN_SPIRV_VALIDATION`, flag-mapping tables). A wrapper script invoking `slangc` directly is what would verify it. |

  Runner-capability alternatives — testable with a different runner /
  toolchain. CI nightly typically lifts these; the agent runtime
  cannot validate them locally.

  | Reason                 | Meaning                                                                                                           |
  | ---------------------- | ----------------------------------------------------------------------------------------------------------------- |
  | `gpu-dxr`              | D3D12 + DXR ray-tracing entry points (`closesthit` / `anyhit` / `miss` / `raygen` / `intersection` / `callable`). |
  | `gpu-mesh-shader`      | Mesh / amplification entry points.                                                                                |
  | `gpu-dxc-dxil`         | DXC binary; `-target dxil` / DXBytecode output.                                                                   |
  | `gpu-fxc-dxbc`         | fxc binary; DXBytecode legacy output.                                                                             |
  | `gpu-cuda`             | nvrtc / PTX / OptiX runtime.                                                                                      |
  | `gpu-metal-toolchain`  | Apple `metal` compiler / `.metallib` toolchain.                                                                   |
  | `gpu-wgsl-tint`        | Tint downstream invocation (`-target wgsl-spirv`).                                                                |
  | `gpu-spirv-tools`      | spirv-link / spirv-val / spirv-opt downstream chain.                                                              |
  | `gpu-non-compute`      | Vertex / fragment / hull / domain / geometry / tessellation stages.                                               |
  | `gpu-bindless`         | Bindless / `NonUniformResourceIndex` capability.                                                                  |
  | `gpu-cooperative`      | Cooperative matrix / cooperative vector capability.                                                               |
  | `gpu-vulkan-extension` | A Vulkan extension or capability set not enabled in the bundle.                                                   |
  | `gpu-cross-api-flag`   | Vulkan-cross-API option flags (`VulkanInvertY`, `VulkanUseDxPositionW`, etc.).                                    |
  | `gpu-other`            | General GPU-runtime requirement without a specific tag above.                                                     |

  Terminal — no harness or runner upgrade unblocks:

  | Reason                   | Meaning                                                                                                                            |
  | ------------------------ | ---------------------------------------------------------------------------------------------------------------------------------- |
  | `link-stage-only`        | The claim is about a `(synthesized)` opcode / pass-output that does not exist at this bundle's `pipeline_stage`.                   |
  | `out-of-bundle`          | The claim is about behavior already covered in a sibling bundle. (Name the sibling in the Why cell.)                               |
  | `deprecated`             | The CLI flag or feature is deprecated and will not be tested.                                                                      |
  | `process-doc`            | The claim is about a contributor walkthrough / process documentation, not a compiler behavior.                                     |
  | `internal-source-fact`   | The claim is an implementation fact (C++ class hierarchy, field names, parser-callback names) with no user-observable consequence. |
  | `compile-time-toggle`    | The claim is about a preprocessor define or build-time flag baked into the binary. Not observable at run-time.                     |
  | `requires-external-tool` | The claim could be verified but requires a non-Slang tool (`unzip`, hex dumper, etc.) the runner does not include.                 |
  | `implementation-detail`  | The claim is about internal compiler choice (pass ordering count, hoistability decisions) with no test-directive that reveals it.  |

- **Anchor**: markdown link to the doc section that makes the claim.
- **Why untested**: one to two sentences elaborating the Reason —
  what specifically blocks the test, and (for `needs-*` / `gpu-*`
  rows) what shape of test would verify it.

If the table is empty (no untested claims at all), write `NA` as the
section body.

## Per-test `//META` block

Every `.slang` file must begin with:

```slang
//META: generated=true
//META: model=<your model identifier>
//META: generated_at=<ISO 8601 timestamp, UTC>
//META: source_commit=<git HEAD when you ran>
//META: doc_ref=docs/generated/design/<path>.md#<anchor>
//META: doc_section_digest=<sha256 of the cited section's text>
//META: purpose=<one-line summary, in plain English>
//META: intent=<functional | boundary | negative | stress | expansion | regression>
//META: pipeline_stage=<lex | preprocess | parse | check | lower | ir-pass | layout | link | emit | runtime>
//META: warning=Auto-generated. May drift from source. Do not edit by hand.
```

Optional keys:

```slang
//META: requires-tool=<comma-separated list>
```

After the `//META` block, add 1–3 short comment lines in plain English
describing the test in human terms. Then the test directive(s).

`doc_section_digest` is the SHA-256 of the text of the cited doc
section, computed as the body lines from the heading whose id is the
anchor up to but not including the next same-or-higher-level heading.
The agent can compute this directly from the doc file at
`source_commit`.

### `requires-tool` (optional)

Some tests need a downstream binary that is not part of the Slang
build itself: `dxc` for DXIL, `nvrtc` for CUDA PTX, the Apple
`metal` compiler / `metallib` toolchain, and so on. When such a
binary is missing locally, the test cannot run; without a gate, it
reports as **FAILED** instead of being correctly skipped, polluting
the verify output and (eventually) the nightly CI signal.

Declare the prerequisite explicitly:

```slang
//META: requires-tool=dxc
```

Allowed values (comma-separated when more than one is needed):

| Value             | What it gates                                                                    |
| ----------------- | -------------------------------------------------------------------------------- |
| `dxc`             | DXC binary for `-target dxil` / DXIL-assembly.                                   |
| `fxc`             | Legacy fxc.exe for `-target dxbc`.                                               |
| `nvrtc`           | NVRTC / CUDA toolchain for `-target cuda` runtime, `-target ptx`, OptiX runtime. |
| `metal-toolchain` | Apple `metal` compiler / `metallib` (Mac-only).                                  |
| `spirv-tools`     | spirv-link / spirv-val / spirv-opt downstream chain.                             |
| `tint`            | Tint downstream (`-target wgsl-spirv`).                                          |

`regenerate.py verify` honors `requires-tool`: when a required
binary is not on `$PATH`, the test is reported as `ignored`
(backend not available locally — CI validates) rather than executed.
CI nightly runs with all tools provisioned, so the test still runs
there.

Do **not** use `requires-tool` to silence tests that are merely
failing — that is the job of the suite-level expected-failure list
([see below](#suite-level-expected-failures)). `requires-tool` is
for environmental gating only.

## Test directives you may use

The bundle is run by `slang-test`. **CI has full runner access**
(GPU, DXC, nvrtc, Apple toolchain, etc.); the agent runtime that
generates tests does not. CI is the safety net for the directives
you cannot execute locally — **not** a substitute for verifying the
ones you can. You must run `regenerate.py verify <bundle>` (see
[`## Verify before committing`](#verify-before-committing) below)
on every bundle you touch and fix every failing test before commit.
A test that fails locally and ships to CI wastes nightly runs and
buries genuine compiler bugs in the failure noise.

Allowed directives (any of these — pick what fits the claim):

| Directive                                                                 | When to use                                                          |
| ------------------------------------------------------------------------- | -------------------------------------------------------------------- |
| `//TEST:SIMPLE(filecheck=CHECK):-target hlsl`                             | Compile to HLSL text and FileCheck the emitted code.                 |
| `//TEST:SIMPLE(filecheck=CHECK):-target glsl`                             | Compile to GLSL text and FileCheck the emitted code.                 |
| `//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm`                        | Compile to SPIR-V assembly text and FileCheck the emitted code.      |
| `//TEST:SIMPLE(filecheck=CHECK):-target metal`                            | Compile to Metal text and FileCheck the emitted code.                |
| `//TEST:SIMPLE(filecheck=CHECK):-target wgsl`                             | Compile to WGSL text and FileCheck the emitted code.                 |
| `//TEST:SIMPLE(filecheck=CHECK):-target cuda`                             | Compile to CUDA C++ text and FileCheck the emitted code.             |
| `//TEST:SIMPLE(filecheck=CHECK):-target cpp`                              | Compile to CPU C++ text and FileCheck the emitted code.              |
| `//TEST:SIMPLE(filecheck=CHECK):-target dxil`                             | Compile to DXIL bytecode and FileCheck the emitted code (CI-only).   |
| `//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):`                                   | Verify a diagnostic is emitted with the expected text/severity/code. |
| `//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type`  | CPU compute kernel; verify the buffer values.                        |
| `//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type`   | Vulkan compute kernel; verify the buffer values (CI-only).           |
| `//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-dx12 -output-using-type` | D3D12 compute kernel; verify the buffer values (CI-only).            |
| `//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type` | CUDA compute kernel; verify the buffer values (CI-only).             |
| `//TEST:INTERPRET(filecheck=CHECK):`                                      | Run under `slangi` (byte-code interpreter).                          |
| `//TEST:SIMPLE...-dump-ir...`                                             | Inspect IR via FileCheck patterns.                                   |

**Required for every directive:** at least one `// CHECK:` (or
`/*CHECK:` block) somewhere in the file that the directive's
matcher will consume. A `//TEST` directive with no corresponding
CHECK pattern is rejected by lint — such a test runs but verifies
nothing.

If a claim is genuinely unobservable through any `slang-test`
directive even with full runner access (e.g., the claim is about a
C++ template pattern with no CLI surface), record it in the
`## Untested claims` table with the appropriate `Reason` tag. If the
claim is observable but the test would only be validatable in CI
(needs a GPU runner / DXC / nvrtc / etc.), still write the test —
CI validates it. The `gpu-*` Reason tags in `## Untested claims` are
used only when no test has been written for the claim yet; once a
test exists, the claim moves to `## Functional coverage`.

### Exercise as many backends as the claim allows

A single test file may carry **multiple `//TEST` directives**, one per
line at the top of the file. `slang-test` runs each directive
independently and any failure fails the file. **The default is to test
the claim against every feasible backend, not just one:**

- For a claim that is **target-independent** (lexer / parser /
  early-semantic-check), one or two directives is enough — adding more
  is repetition of identical pre-backend behavior. Use `INTERPRET` as
  the primary.
- For a claim that is **target-dependent** (IR passes, legalization,
  emit behaviors, capability gates), add a `//TEST` directive for
  _every_ feasible text-emit target where the claim is observable:
  HLSL, GLSL, SPIR-V (asm), Metal, WGSL, CUDA, CPP. If different
  targets produce different emitted text, use distinct `CHECK`
  patterns per directive (see existing tests under `tests/` for
  examples; slang-test selects the pattern block matching the
  directive).

This is how the suite gains backend coverage. A single-target test
masks per-target regressions; a multi-target test catches them.

### Pressure the compiler — boundary coverage is mandatory

**One smoke test per claim is the floor, not the ceiling.** For
every documented claim, instantiate the relevant boundary axes
below. Boundary tests carry `//META: intent=boundary`; stress
patterns carry `//META: intent=stress`. The lint rejects bundles
where the source doc touches a mandatory axis without at least one
matching test.

#### Mandatory axes

If the source doc claim mentions or implies any of these, the
bundle MUST include boundary tests for them:

**Integer types** (`int`, `uint`, `int8/16/32/64`, `uint8/16/32/64`):

- `0` (lower edge)
- documented `MIN` (e.g. `int(-2147483648)`, `int8(-128)`)
- documented `MAX` (e.g. `uint(0xFFFFFFFF)`, `int(2147483647)`)
- `MAX + 1` (overflow) — assert wrap for unsigned; assert documented
  behavior (UB / saturate / wrap) for signed
- literal too-large-for-type (negative test: documented error fires)

**Float types** (`float`, `double`, `half`):

- `0.0` and `-0.0` (distinct bit patterns)
- smallest positive normal
- smallest positive denormal / subnormal (if doc admits)
- largest finite
- `+inf`, `-inf`
- `NaN`
- a value that exposes documented rounding behavior

**Buffer / array / pointer access** (`RWStructuredBuffer`,
`ByteAddressBuffer`, `ConstantBuffer`, `ParameterBlock`,
`Texture*`, fixed-size array, runtime array, pointer types):

- index `0` (lower edge)
- index `N-1` where `N` is the buffer/array bound
- index `N` (one-past-end) — assert documented behavior (silent
  ignore / clamp / diagnostic / UB)
- index `-1` (negative — where signed; covers underflow)
- runtime-bounded vs static-bounded access
- zero-length buffer (where allowed)
- aliasing two refs to the same storage (write through one, read
  through another)

**Diagnostics** (any documented error / warning):

- one negative test per documented diagnostic code, citing the
  exact code (`E####`) and message text **verbatim** from
  "Suggested annotations".
- if the documented input has multiple shapes (expression-level
  vs declaration-level vs statement-level), test each shape.

#### Encouraged axes

These are not lint-enforced, but skip them only with a
`## Untested claims` rationale:

- **Vector / matrix**: 1-element / smallest documented dim, max
  documented dim, mixed-type construction, swizzle of length 1
  vs full.
- **Arrays**: empty (if allowed), `[1]`, max documented size.
- **Generics / variadics**: 0 type args, 1 arg, "many" args
  (>= 8), recursive instantiation depth `>= 3`, pack of length
  0 / 1 / N.
- **Control flow**: empty body, deeply nested (>= 5 levels),
  fall-through, dead code after return, unreachable, all-paths-
  diverge.

#### Pattern guidance

- **One boundary value per file.** `slang-test` reports failures
  per file; isolating each boundary makes the failure signal
  precise. Don't pack 5 boundaries into one `COMPARE_COMPUTE` test.
- **File naming reflects the boundary**:
  `add-uint32-max-overflow.slang`, `buffer-write-index-zero.slang`,
  `float-divide-by-positive-zero.slang`. Avoid generic names like
  `boundary-1.slang`.
- **`purpose` field names the exact boundary**: "Verifies
  `uint(MAX) + uint(1)` wraps to `0` per documented unsigned wrap
  semantics." Don't repurpose smoke-test purpose strings.
- Boundary tests anchor to the same doc claim as their parent
  smoke test (same `doc_ref`). They are not separate claims —
  they are additional probes of the same claim.

### Lessons captured from earlier bundles

#### `slangi` / INTERPRET quirks

- `slangi` printf does **not** support `%s`. For string-typed claims,
  use `//TEST:SIMPLE(filecheck=CHECK):-target cpp` and FileCheck the
  emitted C++ source for the string literal.
- Slang prefers **constructor-style casts** (`int('A')`) over C-style
  casts (`(int)'A'`) for char-to-int. The C-style form returns 0 in
  the interpreter.
- `static const int x = N;` at file scope, then read in `main`, is the
  cleanest pattern for asserting compile-time-known values without
  buffer plumbing.
- **`slangi` cannot instantiate `class`** — `class C { __init... } new C(...)`
  crashes the interpreter. For class-reference-type / ref-accessor
  observations, use `-target hlsl` (or other text-emit) instead.
- **Lambda direct-invocation fails under INTERPRET.**
  `let f = (int x) => ...; f(7)` does not execute on the interpreter.
  Use the `IFunc`-constrained helper idiom from
  `tests/language-feature/lambda/lambda-in-let.slang` if you need to
  exercise lambdas on `slangi`.
- **`throw` / `catch` are NOT correctly evaluated under INTERPRET**
  — the catch handler appears to fire even on the success path. Use
  `//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type`
  for exception semantics.
- **`slangi` cannot host `cbuffer` or non-const `static` module
  globals.** The interpreter rejects with
  `unsupported global inst for vm bytecode emit`. For module-scope
  values, use `static const` only; for `ConstantBuffer<T>` claims,
  switch to `//TEST:SIMPLE(filecheck=CHECK):-target hlsl`.

#### FileCheck pattern gotchas

- **`[[...]]` is a FileCheck regex-variable reference**, not literal
  text. Metal `[[kernel]]`, HLSL `[[vk::binding(...)]]`, and Vulkan
  semantic annotations in emitted code cannot be checked with a
  literal `[[kernel]]` pattern — FileCheck reports
  `undefined variable: kernel`. Use a bare substring (e.g. `kernel`)
  or escape the brackets.
- **DCE strips locally-unused code before SPIR-V emit** (and most
  other backends). To observe a computed value in emitted text, the
  value must escape — write it to an `RWStructuredBuffer` parameter,
  return it from an entry point, or use it in a side-effecting
  printf/atomic. A pure-internal computation is removed before the
  CHECK can see it.
- **Identifier mangling varies per target.** A Slang local `a` may
  appear as `a_0` in HLSL, `globalParams_0.a_0` in CUDA via param-
  block lowering, or `__ldg(&...->a_0)` in CUDA via read-only buffer
  lowering. Use FileCheck wildcards like `a_{{[0-9]+}}` rather than
  literal `a_0`.
- **CUDA factors `__ldg(&uniform)` reads into temporaries**,
  splitting compound expressions on uniform operands. To observe a
  binary expression on CUDA, derive operands from thread/dispatch
  IDs (or locals holding them) rather than from `uniform` globals.
- **Metal `[[buffer(N)]]` indices are positional**, not driven by
  `vk::binding(...)` or HLSL `register(uN)`. Two struct fields bound
  via `vk::binding(7)` and `vk::binding(3)` may still emit as
  `[[buffer(0)]]` and `[[buffer(1)]]` on Metal. Don't assert
  binding-driven indices on Metal.
- **`CHECK` directives on the same source line must appear in
  textual order** of the patterns they match. When multiple emit
  markers appear on one line (e.g. Metal
  `[[kernel]] ... [[thread_position_in_grid]] ... [[buffer(N)]]`),
  use `// CHECK-SAME:` for additional matches following the initial
  `// CHECK:`.
- **Default to `CHECK-DAG` for any test with more than one CHECK
  line.** FileCheck's plain `CHECK:` cursor advances monotonically:
  a `CHECK: error` followed by `CHECK: E30011` cannot match
  `error[E30011]: msg` (the second pattern's text is _behind_ the
  cursor after the first match). Use `CHECK-DAG:` for unordered
  matches, `CHECK:` only when source-text order is the property you
  are verifying. This single rule prevents the most common
  agent-generated FileCheck failure: ordered CHECKs against tokens
  the compiler emits on one line.
- **DIAGNOSTIC_TEST CHECK lines** without a caret are also
  unordered substring matches; the same `CHECK-DAG` discipline
  applies if you write multiple of them. With carets, position
  anchors the match — order between caret-anchored CHECKs is not
  enforced by the runner, but mixing caret-anchored and substring
  CHECKs against the same line is fragile; prefer caret-anchored.

#### Pipeline observation points — `-dump-ir` vs target emit vs `-dump-ir-after`

The Slang pipeline lowers a Slang program through several IR phases:
frontend lowering → generic specialization → SSA → optimization → **target
legalization** → final text emit. Different test claims are observable at
different points. Picking the wrong observation point is the single most
common cause of "the doc says X, my CHECK looks for X, the test fails."

**`-dump-ir`** prints the IR snapshot **before target legalization**.
What you see: opcodes that exist in the platform-neutral IR, decorations
attached in the frontend, generic specializations after monomorphization.
What you do **NOT** see: any opcode synthesized inside `slang-ir-legalize-*`
passes. That includes:

- OptiX-specific: `getOptiXPayloadRegister`, `setOptiXPayloadRegister`,
  `getOptiXSbtDataPointer`, `getOptiXRayPayloadPtr`,
  `getOptiXHitAttribute`. (Synthesized in `slang-ir-legalize-varying-params.cpp`.)
- SPIR-V-bindless: `SPIRVLoadDescriptorFromHeap`, `SPIRVResourceHeap`,
  `SPIRVSamplerHeap`, `SPIRVLoadTexelPointerFromHeap`. (Synthesized in
  SPIR-V target legalization.)
- Vulkan-RT: `GetVulkanRayTracingPayloadLocation`. (Synthesized in
  SPIR-V target legalization.)
- HLSL-DXR shape transforms run inside the HLSL legalizer.

When the IR-reference doc lists an opcode under a target-specific
section ("OptiX-side", "SPIR-V bindless", "Vulkan raytracing", etc.),
**do not anchor the test to `-dump-ir`** — the opcode is not in that
snapshot. Anchor it to the final text emit instead:

- For CUDA / OptiX: check `optixGetPayload_N(...)`, `optixSetPayload_N(...)`,
  `optixGetAttribute_N(...)` in the CUDA C++ text.
- For SPIR-V: check `OpCapability RuntimeDescriptorArray`,
  `__slang_resource_heap`, `OpCapability RayTracingKHR`,
  `OpDecorate %p Location N` in the SPIR-V assembly.
- For HLSL DXR: check the actual closesthit/anyhit signature in the
  emitted HLSL text.

**Target text emit** (`-target <text-target>` without `-dump-ir`) is
the right observation point for anything documented as "the compiler
emits …" — HLSL/GLSL/SPIRV-asm/Metal/WGSL/CUDA/CPP/Torch text. This is
also what users see, so it is the most stable contract.

**Combining `-dump-ir` with target emit** requires both
`-target <text-target>` and `-o /dev/null`. Without `-target` the
compile stops early; without `-o /dev/null` the target text mixes
with IR on stdout and FileCheck fails. With both, IR goes to stdout
and the target text is discarded.

**Other -dump-ir hazards** that the agent must guard against:

- **Constant folding** collapses literal-arithmetic before IR can be
  observed. To observe `add(%a, %b)` in IR, pass operands as `uniform`
  function parameters (or read them from a buffer) rather than as
  literal integers. The same defense works against an emit-stage test
  for an optimization pass: see [Defeat the optimizer](#defeat-the-optimizer)
  below.
- **Trivial locals are eliminated** during initial lowering. The `var`
  opcode survives in IR when the local is a struct accessed by field
  address, but a plain `int x = a;` is collapsed.
- **Phase names for `-dump-ir-after`** — if you genuinely need a
  later snapshot (e.g. to observe what a specific IR pass produced),
  use `-dump-ir-after <passName>` and name the pass exactly as it
  appears in `source/slang/slang-ir-*.cpp` (e.g. `lowerGenerics`).
  Beware: legalization passes that depend on the active target are
  internal; their snapshots are unstable across compiler revisions
  and tests anchored to them tend to bit-rot. Prefer the final text
  emit when available.

#### Defeat the optimizer

The compiler aggressively folds constants and DCEs unused values.
A test that uses literal inputs to verify an optimization pass or
emit behavior will often fail because the pass had nothing to do.

- **Use `uniform`-fed inputs** when the test is for an optimization
  pass or any IR-pass that operates on runtime values. Example: a
  test for `invertYOfPositionOutput` (negates Y under `-fvk-invert-y`)
  with the literal vertex `float4(0.0, 1.0, 0.0, 1.0)` will not see
  `OpFNegate` in the SPIR-V — the constant folder pre-computes the
  negation. Read the Y component from a `uniform float` instead.
- **Use entry-point parameters or dispatch IDs**, not module-scope
  literals, for any value the pass needs to see at runtime.
- **Sink intermediate values into an `RWStructuredBuffer`** so DCE
  cannot remove them.
- **Defeat compile-time literal recognition** when the value matters:
  on CUDA `__ldg(&uniform)` reads, the optimizer can re-fold loads
  into temporaries (see "CUDA factors `__ldg(&uniform)` reads into
  temporaries" above). Use thread/dispatch IDs as operands when the
  CHECK must observe a binary op on the result.

#### Tests must compile cleanly first

A test that does not compile is not a CHECK failure — it is a
broken test. `verify` reports the compiler's error message; **fix
the compile failure before tuning any CHECK**. Common invalid-shader
pitfalls observed in earlier bundles:

- **`SV_Position` cannot decorate an array directly.** A geometry-
  shader input `triangle float4 verts[3] : SV_Position` rejects
  with E30701. Wrap each input vertex in a struct that carries the
  semantic, then pass an array of structs:
  ```slang
  struct GsInput { float4 position : SV_Position; }
  void main(triangle GsInput verts[3], ...) { ... }
  ```
  The same rule applies to `point`, `line`, and tessellation input
  patches.
- **`[raypayload]` on the payload struct is required** for HLSL DXR
  legalization passes (`legalizeEmptyRayPayloadsForHLSL`,
  `replaceLocationIntrinsicsWithRaytracingObject`) to fire. Without
  the decoration, the payload parameter is dropped during lowering
  and the closesthit/anyhit signature emits without it.
- **Entry-point stage capabilities are checked at type-check time.**
  If the core module excludes a target from the capability set for
  a given intrinsic (e.g. `NonUniformResourceIndex` requires
  `cpp_cuda_glsl_hlsl_spirv` and so excludes Metal), the test
  rejects with E36107 before any emit happens. This is usually a
  doc-bug; see [Record a doc-gap](#record-a-doc-gap-when-the-doc-claims-a-surface-the-compiler-rejects)
  below.

#### Record a doc-gap when the doc claims a surface the compiler rejects

If `verify` shows the compiler rejecting the **input the doc
proposes** (E00017 "unknown command-line option", E36107
"unavailable features in entry point", and similar), the right
response is **not** to silently commit a failing test or shoehorn a
workaround. The right response is to record a `drift-from-source`
row in `## Doc gaps observed` in the bundle README, classified by
the controlled `Kind` vocabulary in this file. Examples of doc-bugs
we have already surfaced:

- The SPIR-V doc names `-validate-spirv` as the CLI flag enabling
  spirv-val; no such flag exists. Only `SLANG_RUN_SPIRV_VALIDATION`
  (env var) and `-skip-spirv-validation` are real surfaces.
- The Metal doc claims `NonUniformResourceIndex` is a pass-through;
  the core module excludes Metal from its capability set, so the
  intrinsic does not even compile.

Recording the doc-gap is more valuable than working around the
fabrication: the next doc regeneration consumes the row and fixes
the source-of-truth.

#### Use the real CLI flag spellings, not C++ enum names

The design docs occasionally use internal C++ enum names where the
user-visible surface is different. The agent must use the CLI
spelling — what `slangc -h` accepts — not the internal name.

Observed cases:

- `CUDAHeader` (C++) is `-target cuh` (CLI). Not `cuda-header`,
  which slangc rejects.
- `CUDASource` is `-target cuda`.
- `DXIL` is `-target dxil`; the assembly form is `-target dxil-asm`.
- `SPIRV` is `-target spirv`; the assembly form is `-target spirv-asm`.
- `HostCPP` is `-target host-cpp`; the executable form is
  `-target host-executable`.

When the doc names a target / option using a C++ enum or class
identifier, the test must still use the CLI spelling. If unsure,
run `./build/Release/bin/slangc -h` (or check
`source/slang/slang-options.cpp`).

Similarly, IR opcode names in `-dump-ir` output can differ between
the C++ class (`IRStreamOutputTypeDecoration`) and the dump-printer
name (`streamOutputTypeLayout`); when the test fails because the
expected substring isn't there, check the actual `-dump-ir` output
for the dump-printer spelling.

#### `DIAGNOSTIC_TEST` directive (for negative / "is rejected" claims)

- The `non-exhaustive` flag is an **argument** to the directive's
  argument list, not a directive prefix:
  `//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):` — not
  `//DIAGNOSTIC_TEST(non-exhaustive):SIMPLE(diag=CHECK):`.
- **Default to omitting `non-exhaustive`.** The runner errors with
  `Unnecessary 'non-exhaustive': All N diagnostic(s) were matched
by annotations` when the flag is set but every emitted diagnostic
  was matched — and this is the single most common cause of FAILED
  results during `verify`. Write your CHECK annotations, run
  `verify`, and add `non-exhaustive` **only** if the verify output
  shows unmatched diagnostics left over after your CHECKs consume
  the expected ones. Do not add it defensively.
- The runner sometimes emits **two diagnostics for a single
  user-visible error** — a short form and a span form (e.g. the
  span form repeats the same message with full type substitutions
  like `vector<int,3>`). When verify shows this, add `non-exhaustive`
  and match just the short form. Always copy the diagnostic text
  **verbatim** from the "Suggested annotations" output — even a
  single token mismatch (`entry point` vs `entrypoint`) will fail
  the test and waste an iteration round.
- The `//CHECK:` annotation must appear on the source line
  **immediately following** the offending source line — not several
  lines downstream. The matcher is position-based; placement matters.
- Type-mismatch diagnostics span a range (`^^^^`, multi-caret), not
  a single caret. Copy the exact width from "Suggested annotations".
- The runner's "Suggested annotations" output (when a test fails) is
  the **source of truth** for exact line + column positions. Hand-
  counting carets is unreliable; copy from suggested annotations.
- **Caret `^` placement** in `//CHECK:` requires the caret column to
  be `>= 10` (the `//CHECK:` prefix consumes columns 1–9). For tokens
  in columns 1–9, use the block-comment form: `/*CHECK: ... */`.
- **Some diagnostics attach to unexpected lines** (e.g. missing-return
  attaches to the function signature line, not the return statement).
  Look at the actual compiler output before placing annotations.

## Verify before committing

After you finish writing or editing tests, run:

```bash
python3 docs/generated/tests/_meta/regenerate.py lint <bundle>
python3 docs/generated/tests/_meta/regenerate.py verify <bundle>
```

`verify` runs `slang-test -test-dir docs/generated/tests` filtered to
the bundle and reports three buckets:

- **passed** — the directive ran and its CHECK patterns matched. Good.
- **ignored** — the runner doesn't have the backend the directive
  requested (`-target dxil` with no DXC, GPU APIs without a driver,
  Apple toolchain on Linux, etc.). The directive is still committed;
  CI's nightly job has those backends and will validate. Ignored is
  not a failure.
- **FAILED** — the directive ran and the CHECK patterns did not match,
  OR the diagnostic-test runner objected (e.g. `Unnecessary
'non-exhaustive'`, line-count mismatch, message-text mismatch).
  **Every FAILED test must be fixed before commit.**

The two most common causes of a FAILED result on first verify are:

1. **`DIAGNOSTIC_TEST` with `non-exhaustive` set when the compiler
   emits exactly the diagnostics you annotated.** The runner enforces
   "don't say there might be more diagnostics when there aren't" — see
   the [DIAGNOSTIC_TEST](#diagnostic_test-directive-for-negative--is-rejected-claims)
   section. Default to omitting the flag; only add it after a verify
   run shows leftover unmatched diagnostics.
2. **CHECK lines that don't match the actual output.** Often the
   compiler emits the same information in a slightly different form
   than you guessed (extra modifiers, different precision suffix,
   token reordered). Re-read `actual-output` from the verify output,
   then either edit the CHECK line to match what the compiler
   actually says, or rewrite the test to ask a question that has a
   stable answer.

If a test fails for a reason that looks like a real compiler bug (not
a test bug), file a finding (see
[`## Reporting suspected compiler bugs`](#reporting-suspected-compiler-bugs-findings)).
A finding lets you ship the bundle without the broken test; do not
ship a known-failing test just to "let CI flag it" — that pollutes
the failure signal.

For runners that have **none** of the backends a directive targets,
verify-ignored is the right answer; ship the test and CI validates
it. For runners that have **some** of the backends, the test runs
against the subset the runner does have; verify uses that subset.

### Suite-level expected failures

A test that fails because of a **filed compiler bug** (i.e. there is
a finding under `_meta/findings/filed/` and a tracking issue) should
be listed in `docs/generated/tests/_meta/expected-failures.txt` so
the nightly job stays green while the fix is in flight. Format:

```
# One path per line. Comments start with #.
# Always include the tracking issue link as a comment above the path.
# Remove the entry when the underlying bug is fixed.

# https://github.com/shader-slang/slang/issues/11375 — slangi VM constants OOB
docs/generated/tests/regression/ast-reference/expressions/logic-and-short-circuit.slang
docs/generated/tests/regression/ast-reference/expressions/logic-or-short-circuit.slang
```

`regenerate.py verify` honors this file: matching tests are reported
under a separate `expected-fail` bucket (not `FAILED`) and do not
count against the exit code. The nightly job's pass/fail gate is
**only** the unexpected-failure count.

This list is **only** for failures attributable to a filed compiler
bug (or a comparable cross-component issue with a tracking link).
It is not a place to park flaky tests, test-runner bugs, or
genuinely broken tests — those need real fixes, not silencing. A
line without a tracking-issue comment fails lint.

## Slang command line — quick reminders

- Slang uses **single-dash** multi-char options: `-target spirv`, not
  `--target spirv`. `-stage compute`, `-entry main`, `-o out.spv`.
- For SPIR-V text output use `-target spirv-asm`.
- The output of any test must FileCheck deterministically — avoid
  emitting timestamps, paths, or address-style values in the patterns
  you check against.

## Quality bar

- Every `.slang` file must compile under `slangc` at the target it
  declares. The lint pass cannot run `slangc` itself, but the operator
  spot-checks before `mark-fresh`. A test that fails to compile is
  considered broken and must be regenerated.
- Tests are independent: do not depend on filename ordering or on
  shared state from another file in the same bundle.
- No two tests should exercise the same claim with literally identical
  shader bodies. If two tests are aimed at the same anchor, they should
  differ in some meaningful axis (target, parameter shape, edge value).
- File names are kebab-case and end with `.slang`. They should be
  readable summaries of the test's purpose, e.g.
  `overload-prefer-specialized.slang`, not `test1.slang`.

## Hand-edits

The contract is that this bundle is fully reproducible from the source
doc + this prompt + the watched paths. If a human wants to "fix" a
test, the right move is to either:

1. file a prompt-improvement task against this prompt file, or
2. file a doc-improvement task against the bundle's `source_doc`, or
3. file a manifest-improvement task against
   `docs/generated/tests/_meta/manifest.yaml` (e.g. to add a `watched_path` or
   raise the size cap),

and then re-run regeneration. Hand-edited tests are an anti-pattern;
the lint pass flags any file whose `//META: generated=true` is paired
with a recent commit on the file that the operator cannot justify.

## Reporting suspected compiler bugs (findings)

While writing tests you may observe `slangc` doing something that looks
wrong: a segfault, a missing diagnostic that the doc claims should
fire, codegen that contradicts a cited claim, a regression versus an
older `slangc` revision. **Do not** open GitHub issues, attempt fixes,
or paste these observations into your scratch notes. Record a
structured **finding** instead. A separate triage pass (run by the
operator via `regenerate.py findings file`) reviews findings, edits
them as needed, and files the actionable ones.

When to write a finding:

- `slangc` crashes (SIGSEGV, internal compiler error, abort)
- A documented diagnostic does not fire on input that should trigger it
- A `DIAGNOSTIC_TEST` row in the catalog goes silent
- Generated code is observably wrong against the cited expectation
- A `//TEST` previously passing now fails after a `slangc` update

When **not** to write a finding:

- The doc claim itself seems wrong → record a doc gap in the bundle
  `README.md` under `## Doc gaps observed` instead
- Your test is malformed and `slangc` correctly rejects it → fix the test
- You are merely uncertain about expected behavior → re-read the doc;
  if still uncertain, record a doc gap, not a finding

### Where the finding lives

One YAML file per finding, named after the finding's `id`:

```
docs/generated/tests/_meta/findings/<id>.yaml
```

`<id>` is a kebab-case slug; pick something descriptive and unique,
e.g. `pipeline-04b-defer-lower-sigsegv`. The filename must match the
`id` field exactly (the lint pass enforces this). After the operator
files the finding as an issue, the file is moved to
`docs/generated/tests/_meta/findings/filed/`.

### Required fields

The full schema is at
`docs/generated/tests/_meta/schema/finding.schema.json`. Required:

```yaml
schema_version: 1
id: <kebab-case slug; matches filename>
bundle: <bundle key that surfaced the finding, e.g. pipeline/04b-pre-link-passes>
suspected_kind: sigsegv | wrong-diagnostic | wrong-codegen | regression | catalog-drift | doc-claim-overstated
observed_at: <ISO 8601 timestamp, UTC>
evidence:
  command: <exact slangc/slang-test invocation that triggers the failure>
  source_slang: <workspace-relative path to the .slang you were writing>
  observed_summary: <single line: what happened>
expected:
  claim: <one sentence stating the correct behavior>
  citation_kind: spec | sibling-test | older-slangc | doc
  citation: <path / commit ref / spec section pointing to the source of truth>
provenance:
  agent_model: <your model identifier>
  source_commit: <slangc git HEAD when observed>
  doc_anchor: <docs/generated/design/<file>.md#<section> the test was anchored to>
```

Optional (use when relevant):

```yaml
scope: spirv | hlsl | metal | wgsl | cuda | cpp | diagnostics | parser | ir | core-module
title: <explicit issue title, ≤80 chars; otherwise derived>
evidence:
  exit_code: <process exit code, e.g. 139 for SIGSEGV>
  stderr_tail: <last lines of stderr; single line, may include \n escapes>
agent_self_assessment:
  confidence: low | medium | high
  alternatives_considered:
    - <something you ruled out>
```

### What you must NOT do

- Do **not** call `gh` or otherwise file issues yourself.
- Do **not** attempt to minimize the repro. The operator does this
  during triage. Your `evidence.source_slang` points at the full
  `.slang` you were writing; that is enough.
- Do **not** populate `evidence.minimized_repro` — that field is
  reserved for the operator.
- Do **not** speculate about the root cause. Stick to observation:
  what you ran, what the doc / spec / sibling test says should happen,
  what actually happened. No "this is probably in the specialization
  pass" hand-waving.
- Do **not** cite the design doc (`citation_kind: doc`) when a
  stronger source is available. The design docs are themselves
  LLM-generated; prefer the spec, an existing hand-written test under
  `tests/`, or output from an older `slangc` revision.
- Whatever you cite, **the citation must resolve**. For
  `citation_kind: doc`, the path must exist under `docs/`. For
  `citation_kind: sibling-test`, the path must exist under `tests/`.
  For `citation_kind: spec`, the path must exist under
  `external/spec/`. The lint pass rejects findings whose citation
  path is not present on disk — do not invent paths to support a
  claim. If you cannot ground the expected behavior in a real path,
  cite the bundle's `source_doc` (`citation_kind: doc`) and mark
  `agent_self_assessment.confidence: low`, or skip writing the
  finding entirely.

### Ground rule

A finding is **evidence**, not a conclusion. If you can't fill the
required fields honestly, don't write the finding — file a doc gap
or skip the test.
