# Prompt: tests-agentic/cross-cutting/diagnostics-catalog/

See [`_common.md`](_common.md) for universal rules. Those rules apply.

## Target

Produce a comprehensive catalog of `DIAGNOSTIC_TEST` files — one test
per diagnostic code that the Slang compiler can emit — at
`tests-agentic/cross-cutting/diagnostics-catalog/`.

This bundle is the **systematic negative-test sweep** the
agentic-tests plan §15.3 calls for. It is unusual within the framework
because its source-of-truth is the structured LUA / macro-style
diagnostic catalogs in the compiler source, not a narrative
documentation file.

## Why this bundle is allowed under the doc-anchoring contract

The framework's universal rule is: tests derive from documentation,
not from source code. The compiler's diagnostic catalogs
(`source/slang/slang-diagnostics.lua` and the
`*-diagnostic-defs.h` headers in `source/compiler-core/`) are
**structured spec sources**: each entry is a declarative tuple of
`(code, severity, name, message)` that defines the diagnostic
contract the compiler offers to users. They are not narrative prose,
but they are not arbitrary implementation source either — they are
the canonical enumeration of "what diagnostics this compiler emits."

For this bundle, the catalog files themselves count as the spec.
Each test anchors `doc_ref` to the catalog entry (the LUA `err()`
call or `DIAGNOSTIC()` macro line) rather than to a narrative
section in `docs/llm-generated/cross-cutting/diagnostics.md` —
though the narrative doc is also referenced for general framing.

## Inputs available to you

- **`tests-agentic/_meta/diagnostics-catalog/catalog.txt`** — full
  catalog snapshot (695 codes) with severity / name / source /
  message columns. Authoritative.
- **`tests-agentic/_meta/diagnostics-catalog/uncovered-bucket-<N>.txt`**
  — the subset of uncovered codes assigned to your generation
  bucket. Work only from this list; the operator will dispatch
  five parallel agents, one per bucket, so don't try to claim
  codes outside yours.
- The narrative doc:
  `docs/llm-generated/cross-cutting/diagnostics.md` — for framing
  and existing-anchor reference.
- The compiler source you may consult **only to verify a code is
  reachable from a minimum-reproduction input** (not to mine
  behaviors the catalog doesn't already specify).

## What to produce

For each uncovered code in your bucket, produce **one `.slang` test**
that triggers that diagnostic from a minimum-reproduction input.

**Per-test contract (extends `_common.md`):**

```slang
//META: generated=true
//META: model=<your model id>
//META: generated_at=<ISO 8601 UTC>
//META: source_commit=<HEAD>
//META: doc_ref=source/slang/slang-diagnostics.lua    ← or the appropriate -defs.h
//META: doc_section_digest=<sha256 of the catalog-entry line>
//META: purpose=Fires diagnostic E<code> (<name>) — <message>
//META: intent=negative
//META: pipeline_stage=<lex | parse | check | lower | ir-pass | emit | link>
//META: catalog_code=<code>
//META: catalog_name=<name from catalog>
//META: warning=Auto-generated. May drift from source. Do not edit by hand.
```

Note the new field `catalog_code` (and `catalog_name`). They are
catalog-specific and ignored by the existing lint, but make grep
trivial: `grep "catalog_code=30019" tests-agentic` finds the test
for code 30019.

**The test body is the minimum input that fires the diagnostic:**

```slang
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

// <minimum reproduction here>

// CHECK: E<code>
```

Use `non-exhaustive` when the compiler emits more than one
diagnostic in response (it often does); match just your target code.
Use the exact code (`E30019`, `W41024`, etc.) — slang-test will
match by code.

## File-naming convention

`<code>-<kebab-case-name>.slang`. Examples:

- `30019-argument-type-mismatch.slang`
- `15302-include-recursion.slang`
- `41024-name-shadow-warning.slang`
- `38000-no-entry-point-found.slang`

This makes the bundle browseable by code and by name.

## Hard rules

- **Stay within your bucket.** Don't write tests for codes outside
  the bucket file you were given. Other agents are handling other
  buckets in parallel.
- **Don't synthesize code that doesn't fire the diagnostic.** If
  you can't find a minimum input that fires a specific code after 2
  attempts, **drop it**. List dropped codes under
  `## Codes dropped (could not reach from minimum input)` in
  `README.md` with a one-line reason ("internal diagnostic with no
  user trigger" / "requires multi-file test" / "requires API
  surface not available to slangc CLI" / etc.).
- **Don't lock in buggy behavior.** If the compiler emits something
  other than what the catalog message says, drop the code; do not
  write a test that asserts the buggy output.
- **One test per code.** Multiple codes in one test file would
  defeat the catalog's `grep`-ability.
- **All categories of severity are in scope** — errors, warnings,
  notes, info. `intent=negative` for all of them (we are testing
  that the compiler emits the right diagnostic).

## README.md structure

The first agent to land in this directory creates README.md; later
agents in the same wave **append** to its tables. Each agent owns
its bucket's rows. Sections:

1. Front-matter (per `_common.md`).
2. `## Intent` — paragraph explaining this is the catalog sweep
   bundle, what each test is, and that this is the agentic-tests
   plan §15.3 deliverable.
3. `## Catalog coverage` — small table:
   ```
   | Bucket | Codes in bucket | Tests added | Codes dropped |
   ```
4. `## Tests in this bundle` — one row per test, sorted by code:
   `| <code>-<name>.slang | E<code> | <severity> | <pipeline stage> |`
5. `## Codes dropped (could not reach from minimum input)` — one
   row per dropped code with the drop reason.

## Quality checklist

- [ ] Every test reports `100% of tests passed (1/1)` under
      `build/Release/bin/slang-test`.
- [ ] The CHECK pattern names the exact diagnostic code (`E30019`,
      not just `error:`).
- [ ] `non-exhaustive` is used iff the compiler emits diagnostics
      other than the target one.
- [ ] File name is `<code>-<kebab-name>.slang`.
- [ ] `catalog_code` matches the file's code.
- [ ] Test body is **minimum** — no extraneous functions, no
      unrelated constructs.
- [ ] Codes that cannot be triggered from a single `.slang` file
      are dropped (not synthesized with hacks).

## Do NOT

- mark-fresh (the operator handles it).
- commit.
- modify other agents' tests (each bucket is independent).
- write tests for codes outside your assigned bucket.
- include source-line content from the compiler's implementation
  (only catalog entries).
