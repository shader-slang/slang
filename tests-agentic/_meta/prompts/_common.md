# Common Rules for tests-agentic Generation

These rules apply to every per-section prompt under this directory. The
per-section prompt extends or overrides them; if there is a conflict, the
per-section prompt wins for that bundle.

## What you are producing

You are producing a **test bundle**: a directory at
`tests-agentic/<bundle-key>/` containing:

- exactly one `BUNDLE.md` with YAML front-matter and a claims/tests index;
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
"doc-gap" finding in `BUNDLE.md` under `## Doc gaps observed`.

## BUNDLE.md front-matter

Every `BUNDLE.md` must begin with:

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

## BUNDLE.md body structure

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

| Directive                                                                | When to use                                                                            |
| ------------------------------------------------------------------------ | -------------------------------------------------------------------------------------- |
| `//TEST:SIMPLE(filecheck=CHECK):`                                        | Compile and FileCheck the textual output (HLSL/GLSL/MSL/WGSL/CUDA/SPIR-V-asm/IR-dump). |
| `//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):`                                  | Verify a diagnostic is emitted with the expected text/severity/code.                   |
| `//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type` | CPU compute kernel; verify the buffer values.                                          |
| `//TEST:INTERPRET(filecheck=CHECK):`                                     | Run under `slangi` (byte-code interpreter).                                            |
| `//TEST:SIMPLE...-dump-ir...`                                            | Inspect IR via FileCheck patterns.                                                     |

Do **not** use any directive that requires a real GPU (Vulkan, D3D12,
Metal, WGSL runtime, OptiX, etc.). If a behavior can only be exercised
on a GPU, document it as a doc-gap-like note in `BUNDLE.md` under
`## Out of scope (no-GPU runner)` and skip it.

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
