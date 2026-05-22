# Common Rules for tests-agentic Generation

These rules apply to every per-section prompt under this directory. The
per-section prompt extends or overrides them; if there is a conflict, the
per-section prompt wins for that bundle.

## What you are producing

You are producing a **test bundle**: a directory at
`tests-agentic/<bundle-key>/` containing:

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
source_doc: docs/llm-generated/<...>.md
source_doc_digest: <sha256 of source_doc file at source_commit>
warning: "Auto-generated. May drift from source. Do not edit by hand."
---
```

Compute `watched_paths_digest` and `source_doc_digest` with
`python3 tests-agentic/_meta/regenerate.py digest <bundle>`.

## README.md body structure

```markdown
# Tests for <bundle-key>

## Intent

One short paragraph: which doc this bundle exercises, and the coverage
strategy (e.g. "one positive + one negative per claim in sections
3.1–3.5").

## Functional coverage

| Claim                                                                          | Intent     | Anchor                                                          | Tests                                                                  |
| ------------------------------------------------------------------------------ | ---------- | --------------------------------------------------------------- | ---------------------------------------------------------------------- |
| The more specialized generic wins when both candidates are equally accessible. | functional | [#overload-resolution](../../docs/llm-generated/<doc>.md#overload-resolution) | [`overload-prefer-specialized.slang`](overload-prefer-specialized.slang) |
| ...                                                                            | ...        | ...                                                             | ...                                                                    |

## Doc gaps observed

| Anchor                                                    | Kind            | Gap                                                                                   | Suggested addition                                          |
| --------------------------------------------------------- | --------------- | ------------------------------------------------------------------------------------- | ----------------------------------------------------------- |
| [#basic-scalar-types](../../docs/llm-generated/<doc>.md#basic-scalar-types) | missing-surface | The doc lists `IntPtr` and `UIntPtr` as opcodes but does not name a Slang surface construct that produces them. | A "user-level surface" column per row, or a note that these are host-side only. |
| ...                                                       | ...             | ...                                                                                   | ...                                                         |

If there are no gaps, omit the section. Do not write a placeholder row.

## Untested coverable claims

| Anchor                                                    | Backend            | Claim                                                                                | Why untested                                                 |
| --------------------------------------------------------- | ------------------ | ------------------------------------------------------------------------------------ | ------------------------------------------------------------ |
| [#dxr-ray-tracing](../../docs/llm-generated/<doc>.md#dxr-ray-tracing) | gpu-dxr            | The `closesthit` entry point lowers to a DXR `[shader("closesthit")]` HLSL function. | Agent runtime has no GPU; CI nightly has D3D12 + DXR support. |
| ...                                                       | ...                | ...                                                                                  | ...                                                          |

If every documented claim already has a test in the Coverage table,
omit this section.

## Untested claims

| Claim                                                                                                                                       | Reason            | Anchor                                                    | Why untested                                                                                            |
| ------------------------------------------------------------------------------------------------------------------------------------------- | ----------------- | --------------------------------------------------------- | ------------------------------------------------------------------------------------------------------- |
| The doc describes the `serialize(serializer, value)` template pattern that compiler internals use to thread values through serialization.   | needs-unit-test   | [#serialize-pattern](../../docs/llm-generated/<doc>.md#serialize-pattern) | A C++ idiom invoked from compiler-internal code; no slangc CLI surface reaches it. A C++ unit test against the serializer types could verify it. |
| ...                                                                                                                                         | ...               | ...                                                       | ...                                                                                                     |

If every documented claim is either tested or coverable, omit this
section.
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
  bundle directory, formatted `` [`filename.slang`](filename.slang) ``.
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
when the next revision of `docs/llm-generated/<doc>.md` is generated.
That means each row must be self-contained enough for an agent who
has never seen this bundle to know which doc section to edit and what
to add.

- **Anchor** is a markdown link to the doc section the gap is about,
  using the same anchor shape as the Coverage table. If the gap spans
  multiple sections, pick the most specific anchor and mention the
  others in the Gap cell.
- **Kind** is one value from the controlled vocabulary:
  | Kind                     | When to use                                                                                                       |
  | ------------------------ | ----------------------------------------------------------------------------------------------------------------- |
  | `missing-example`        | Doc names a claim but does not include a minimal example shader that exercises it.                                |
  | `missing-surface`        | Doc names an IR / AST / internal construct but does not name the user-level syntax or builtin that produces it.   |
  | `undocumented-behavior`  | Observed compiler behavior is real and reachable but the doc is silent about it (no claim, no caveat, no warning).|
  | `cascading-only-mention` | Doc describes a diagnostic or behavior that is always shadowed in practice by an earlier diagnostic or pass.      |
  | `ambiguous-claim`        | Doc claim has more than one reasonable interpretation; tests cannot anchor to it without making a guess.          |
  | `drift-from-source`      | Observed compiler behavior contradicts what the doc says (e.g., doc says "lowers to `select`" but actually emits `ifElse`). |
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

**Untested-coverable table rules:**

The `## Untested coverable claims` section is the **next-wave
priority list**. Each row is a documented claim the bundle does
**not** have a test for because the agent runtime (Claude Code,
Codex, etc.) cannot validate it locally — but CI (nightly,
GPU-equipped) or a developer's command line can. An expansion wave
that does have access to the relevant runner is expected to walk
this table and write tests for it.

- **Anchor**: markdown link to the doc section, same shape as the
  other tables.
- **Backend**: controlled vocabulary naming the runner / capability
  needed:

  | Backend                | Meaning                                                              |
  | ---------------------- | -------------------------------------------------------------------- |
  | `gpu-dxr`              | D3D12 + DXR ray-tracing entry points (`closesthit`/`anyhit`/`miss`/`raygen`/`intersection`/`callable`). |
  | `gpu-mesh-shader`      | Mesh / amplification entry points.                                   |
  | `gpu-dxc-dxil`         | DXC binary; `-target dxil` / DXBytecode output.                      |
  | `gpu-fxc-dxbc`         | fxc binary; DXBytecode legacy output.                                |
  | `gpu-cuda`             | nvrtc / PTX / OptiX runtime.                                         |
  | `gpu-metal-toolchain`  | Apple `metal` compiler / `.metallib` toolchain.                      |
  | `gpu-wgsl-tint`        | Tint downstream invocation (`-target wgsl-spirv`).                   |
  | `gpu-spirv-tools`      | spirv-link / spirv-val / spirv-opt downstream chain.                 |
  | `gpu-non-compute`      | Vertex / fragment / hull / domain / geometry / tessellation stages.  |
  | `gpu-bindless`         | Bindless / `NonUniformResourceIndex` capability.                     |
  | `gpu-cooperative`      | Cooperative matrix / cooperative vector capability.                  |
  | `gpu-vulkan-extension` | A Vulkan extension or capability set not enabled in the bundle.      |
  | `gpu-cross-api-flag`   | Vulkan-cross-API option flags (`VulkanInvertY`, `VulkanUseDxPositionW`, etc.). |
- **Claim**: one to three sentences describing what the test would
  verify, written in the voice of a future test agent — concrete
  enough that the agent can write the test from this row alone.
- **Why untested**: one sentence naming the runner constraint
  (typically "Agent runtime has no GPU; CI / local machine does.").

**Untested-claims table rules:**

The `## Untested claims` section is the **bucket of doc claims that
cannot be tested by `slang-test` directly**. Each row names a claim
and the kind of test harness that *would* verify it (C++ unit test,
multi-file test, CLI test, etc.) — or, when no harness exists, why
the claim is unobservable through any test directive. The doc-regen
workflow does **not** consume this table; it exists to make the
suite's coverage boundary visible to readers, and to surface
candidates for unit-test / CLI-test campaigns.

Columns: `Claim | Reason | Anchor | Why untested`. The Claim is the
primary content; Anchor is a supporting pointer.

- **Claim**: one to three sentences naming what the doc states.
  Lead with the claim because that's the substance — readers and
  downstream tooling want to know *what* before *why*.
- **Reason**: controlled vocabulary naming what is needed (when
  testable elsewhere) or why the claim is unreachable:

  | Reason                    | Meaning                                                                                                                                                                              |
  | ------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
  | `needs-unit-test`         | The claim is about C++ behavior (an `ISession` method, a `serialize(...)` template pattern, an internal helper) that has no slangc CLI surface but could be tested by a C++ unit test (in `tools/slang-unit-test/`). |
  | `needs-multi-file-test`   | The claim is observable only across 2+ `.slang` files (cross-module `import`, `__include` / `__implementing`, `-embed-downstream-ir` artefacts, `.slang-module` linking). The single-file `//TEST` directive cannot stage the input. |
  | `needs-cli-test`          | The claim is about how `slangc` is invoked rather than what it compiles (help output, exit codes, env-var-gated paths like `SLANG_RUN_SPIRV_VALIDATION`, flag-mapping tables, argument-parsing edge cases). A wrapper script invoking `slangc` directly is what would verify it. |
  | `link-stage-only`         | The claim is about a `(synthesized)` opcode / pass-output that does not exist at this bundle's `pipeline_stage`. Test belongs in the bundle whose pipeline stage matches.            |
  | `out-of-bundle`           | The claim is about behavior already covered in a sibling bundle. (Name the sibling in the Why cell.)                                                                                  |
  | `deprecated`              | The CLI flag or feature is deprecated and will not be tested.                                                                                                                         |
  | `process-doc`             | The claim is about a contributor walkthrough / process documentation, not a compiler behavior.                                                                                        |
  | `internal-source-fact`    | The claim is an implementation fact (C++ class hierarchy, field names, parser-callback names) with no user-observable consequence.                                                    |
  | `compile-time-toggle`     | The claim is about a preprocessor define or build-time flag baked into the binary. Not observable at run-time.                                                                        |
  | `requires-external-tool`  | The claim could be verified but requires a non-Slang tool (`unzip`, hex dumper, etc.) the runner does not include.                                                                    |
  | `implementation-detail`   | The claim is about internal compiler choice (pass ordering count, hoistability decisions) with no test-directive that reveals it.                                                     |
- **Anchor**: markdown link to the doc section that makes the claim.
- **Why untested**: one to two sentences elaborating the Reason —
  what specifically blocks the test, and (for `needs-*` rows) what
  shape of test would verify it.

## Per-test `//META` block

Every `.slang` file must begin with:

```slang
//META: generated=true
//META: model=<your model identifier>
//META: generated_at=<ISO 8601 timestamp, UTC>
//META: source_commit=<git HEAD when you ran>
//META: doc_ref=docs/llm-generated/<path>.md#<anchor>
//META: doc_section_digest=<sha256 of the cited section's text>
//META: purpose=<one-line summary, in plain English>
//META: intent=<functional | expansion | regression | negative>
//META: pipeline_stage=<lex | preprocess | parse | check | lower | ir-pass | layout | link | emit | runtime>
//META: warning=Auto-generated. May drift from source. Do not edit by hand.
```

After the `//META` block, add 1–3 short comment lines in plain English
describing the test in human terms. Then the test directive(s).

`doc_section_digest` is the SHA-256 of the text of the cited doc
section, computed as the body lines from the heading whose id is the
anchor up to but not including the next same-or-higher-level heading.
The agent can compute this directly from the doc file at
`source_commit`.

## Test directives you may use

The bundle is run by `slang-test`. **CI has full runner access**
(GPU, DXC, nvrtc, Apple toolchain, etc.); the agent runtime that
generates tests may not. The agent commits any test that has a
`//TEST` (or `//DIAGNOSTIC_TEST`) directive and at least one CHECK
pattern, even if the agent cannot execute the directive locally.
CI catches behavioral failures.

Allowed directives (any of these — pick what fits the claim):

| Directive                                                                | When to use                                                          |
| ------------------------------------------------------------------------ | -------------------------------------------------------------------- |
| `//TEST:SIMPLE(filecheck=CHECK):-target hlsl`                            | Compile to HLSL text and FileCheck the emitted code.                 |
| `//TEST:SIMPLE(filecheck=CHECK):-target glsl`                            | Compile to GLSL text and FileCheck the emitted code.                 |
| `//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm`                       | Compile to SPIR-V assembly text and FileCheck the emitted code.      |
| `//TEST:SIMPLE(filecheck=CHECK):-target metal`                           | Compile to Metal text and FileCheck the emitted code.                |
| `//TEST:SIMPLE(filecheck=CHECK):-target wgsl`                            | Compile to WGSL text and FileCheck the emitted code.                 |
| `//TEST:SIMPLE(filecheck=CHECK):-target cuda`                            | Compile to CUDA C++ text and FileCheck the emitted code.             |
| `//TEST:SIMPLE(filecheck=CHECK):-target cpp`                             | Compile to CPU C++ text and FileCheck the emitted code.              |
| `//TEST:SIMPLE(filecheck=CHECK):-target dxil`                            | Compile to DXIL bytecode and FileCheck the emitted code (CI-only).   |
| `//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):`                                  | Verify a diagnostic is emitted with the expected text/severity/code. |
| `//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type` | CPU compute kernel; verify the buffer values.                        |
| `//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type`  | Vulkan compute kernel; verify the buffer values (CI-only).           |
| `//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-dx12 -output-using-type` | D3D12 compute kernel; verify the buffer values (CI-only).            |
| `//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type` | CUDA compute kernel; verify the buffer values (CI-only).             |
| `//TEST:INTERPRET(filecheck=CHECK):`                                     | Run under `slangi` (byte-code interpreter).                          |
| `//TEST:SIMPLE...-dump-ir...`                                            | Inspect IR via FileCheck patterns.                                   |

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
CI validates it. Record nothing extra; the `gpu-*` Backend tags in
the `## Untested coverable claims` table are only used when no test
has been written yet.

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

#### `-dump-ir` directive (when observing IR instructions)

- Combine `-dump-ir` with **`-target <text-target>`** AND **`-o /dev/null`**.
  Without `-target` the compile stops early; without `-o /dev/null` the
  target text mixes with IR on stdout and FileCheck fails. With both,
  IR goes to stdout and target text is discarded.
- **Constant folding** collapses literal-arithmetic before IR can be
  observed. To observe `add(%a, %b)` in IR, pass operands as `uniform`
  function parameters (or read them from a buffer) rather than as
  literal integers.
- **Trivial locals are eliminated** during initial lowering. The `var`
  opcode survives in IR when the local is a struct accessed by field
  address, but a plain `int x = a;` is collapsed.

#### `DIAGNOSTIC_TEST` directive (for negative / "is rejected" claims)

- The `non-exhaustive` flag is an **argument** to the directive's
  argument list, not a directive prefix:
  `//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):` — not
  `//DIAGNOSTIC_TEST(non-exhaustive):SIMPLE(diag=CHECK):`.
- The runner **rejects** `non-exhaustive` when all diagnostics in the
  test happen to be matched. Omit the flag in that case.
- The runner sometimes emits **two diagnostics for a single
  user-visible error** — a short form and a span form (e.g. the
  span form repeats the same message with full type substitutions
  like `vector<int,3>`). Use `non-exhaustive` and match just the
  short form. Always copy the diagnostic text **verbatim** from
  the "Suggested annotations" output — even a single token mismatch
  (`entry point` vs `entrypoint`) will fail the test and waste an
  iteration round.
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
   `tests-agentic/_meta/manifest.yaml` (e.g. to add a `watched_path` or
   raise the size cap),

and then re-run regeneration. Hand-edited tests are an anti-pattern;
the lint pass flags any file whose `//META: generated=true` is paired
with a recent commit on the file that the operator cannot justify.
