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

## Claims enumerated

| Claim ID | Anchor               | Claim (one line)                                                               | Tests                               |
| -------- | -------------------- | ------------------------------------------------------------------------------ | ----------------------------------- |
| C-01     | #overload-resolution | The more specialized generic wins when both candidates are equally accessible. | `overload-prefer-specialized.slang` |
| ...      | ...                  | ...                                                                            | ...                                 |

## Tests in this bundle

| File                                | Intent     | Doc anchor             |
| ----------------------------------- | ---------- | ---------------------- |
| `overload-prefer-specialized.slang` | functional | `#overload-resolution` |
| ...                                 | ...        | ...                    |

## Doc gaps observed

(One bullet per gap. If none, write `(none)`. Each bullet should be a
single sentence naming the behavior that lacks a documented claim, with
a suggestion of where it could be added.)
```

The lint pass verifies that every test file appears in the **Tests in
this bundle** table and that every Claim ID appears in at least one
test's `//META: claim_ids` if you choose to use that field.

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

The bundle is run by `slang-test`. The agentic suite runs on CI runners
that have **no GPU**, so only directives that work without one are
allowed:

| Directive                                                                | When to use                                                          |
| ------------------------------------------------------------------------ | -------------------------------------------------------------------- |
| `//TEST:SIMPLE(filecheck=CHECK):-target hlsl`                            | Compile to HLSL text and FileCheck the emitted code.                 |
| `//TEST:SIMPLE(filecheck=CHECK):-target glsl`                            | Compile to GLSL text and FileCheck the emitted code.                 |
| `//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm`                       | Compile to SPIR-V assembly text and FileCheck the emitted code.      |
| `//TEST:SIMPLE(filecheck=CHECK):-target metal`                           | Compile to Metal text and FileCheck the emitted code.                |
| `//TEST:SIMPLE(filecheck=CHECK):-target wgsl`                            | Compile to WGSL text and FileCheck the emitted code.                 |
| `//TEST:SIMPLE(filecheck=CHECK):-target cuda`                            | Compile to CUDA C++ text and FileCheck the emitted code.             |
| `//TEST:SIMPLE(filecheck=CHECK):-target cpp`                             | Compile to CPU C++ text and FileCheck the emitted code.              |
| `//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):`                                  | Verify a diagnostic is emitted with the expected text/severity/code. |
| `//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type` | CPU compute kernel; verify the buffer values.                        |
| `//TEST:INTERPRET(filecheck=CHECK):`                                     | Run under `slangi` (byte-code interpreter).                          |
| `//TEST:SIMPLE...-dump-ir...`                                            | Inspect IR via FileCheck patterns.                                   |

Do **not** use any directive that requires a real GPU (Vulkan, D3D12,
Metal, WGSL runtime, OptiX, etc.). If a behavior can only be exercised
on a GPU, document it as a doc-gap-like note in `README.md` under
`## Out of scope (no-GPU runner)` and skip it.

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
`## Out of scope` rationale:

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
