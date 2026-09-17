# Expected-failures triage — index

> Prose triage reports for `_meta/expected-failures.txt`. One file per
> investigated entry, kept next to the findings they feed so the reasoning
> behind a finding (or behind a decision _not_ to file one) is recoverable.
> Filed compiler defects live in `_meta/findings/`; this directory holds the
> analysis, including verdicts that produced no finding at all.

Scope: the 21 entries in `docs/generated/tests/_meta/expected-failures.txt`
as of commit `9aafdeb00d`, each re-run against a compiler built from that
tree (`build/Release/bin/slangc`, `2026.17.1-3-ga680b2b50f`).

Verdict legend:

- **stale-entry** — the test now passes; the entry should be deleted.
- **covered** — genuine compiler defect, already has a finding YAML.
- **new-finding** — genuine compiler defect, no finding existed; one is added.
- **test-bug** — the compiler is right and the test is wrong; fix the test.
- **doc-and-test-stale** — the compiler changed intentionally; both the test
  and a design-doc claim are out of date.

| #     | Entry                                                                              | Verdict                        | Detail                                               |
| ----- | ---------------------------------------------------------------------------------- | ------------------------------ | ---------------------------------------------------- |
| 01    | `conformance/types-pointer/pointer-arithmetic-spirv-emission.slang`                | **stale-entry**                | [01](01-pointer-arithmetic-stale-entry.md)           |
| 02    | `design/ir-reference/decorations/builtin-requirement-key-decoration.slang`         | **new-finding**                | [02](02-builtin-requirement-duplicate-decoration.md) |
| 03    | `design/cross-cutting/ir-instructions/decoration-builtin-requirement-key-ir.slang` | new-finding (same cause as 02) | [02](02-builtin-requirement-duplicate-decoration.md) |
| 04    | `design/ir-reference/differentiation/builtin-requirement-decoration.slang`         | new-finding (same cause as 02) | [02](02-builtin-requirement-duplicate-decoration.md) |
| 05    | `design/ast-reference/declarations/classdecl-reference-type.slang`                 | **test-bug**                   | [03](03-classdecl-reference-type-test-bug.md)        |
| 06    | `design/pipeline/04c-layout-ir/raytracing-callable-payload-rejected-on-cuda.slang` | **doc-and-test-stale**         | [04](04-cuda-callable-payload-now-supported.md)      |
| 07    | `design/pipeline/05-ir-passes/typeflow-report-dynamic-dispatch-sites.slang`        | not yet triaged                | [05](05-remaining-untriaged.md)                      |
| 08-13 | five `conformance/*` entries + `switchstmt-case-decl-used-in-later-case`           | **covered**                    | [06](06-already-covered.md)                          |
| 14-17 | `new-expr-constructor-args.slang.1/.2`, `refaccessor-property.slang.1/.2`          | **covered**                    | [06](06-already-covered.md)                          |
| 18-19 | `countof-static-array`, `countof-fixed-size-array`                                 | **covered**                    | [06](06-already-covered.md)                          |
| 20    | `design/target-pipelines/metal/append-buffer-params-carry-buffer-slots.slang`      | **covered**                    | [06](06-already-covered.md)                          |
| 21    | `design/ast-reference/values/builtinoperationintval-enum-operands-fold.slang`      | **covered**                    | [06](06-already-covered.md)                          |

## Summary

| Verdict                               | Count  |
| ------------------------------------- | ------ |
| covered by an existing finding        | 13     |
| new-finding (one defect, three tests) | 3      |
| test-bug                              | 1      |
| doc-and-test-stale                    | 1      |
| stale-entry                           | 1      |
| not yet triaged                       | 1      |
| **total**                             | **21** |

Actionable outcomes:

| #   | Action                                                                                                                        | Status   |
| --- | ----------------------------------------------------------------------------------------------------------------------------- | -------- |
| 1   | Land the new finding `builtin-requirement-decoration-added-twice` (02)                                                        | **done** |
| 2   | Fix the design-doc CUDA claim and retarget its test (04)                                                                      | **done** |
| 3   | ~~Delete the stale entry for `pointer-arithmetic-spirv-emission`~~ — **overturned**: it is a real codegen bug, now filed (01) | **done** |
| 4   | Fix the `classdecl-reference-type` test, then drop its entry (03)                                                             | **done** |
| 5   | Triage `typeflow-report-dynamic-dispatch-sites` (05)                                                                          | open     |
| 6   | Confirm the keyword-matched link in (06)                                                                                      | open     |
| 7   | Root-cause the duplicate-decoration finding                                                                                   | open     |

## A note on method

Two of these verdicts were already written down, in the comment blocks of
`expected-failures.txt` itself — the `classdecl-reference-type` fix ("make the
allocation live") and the whole CUDA callable analysis, including the
observation that the design doc had gone stale the same way. They were
rediscovered here rather than read.

The lesson is about the file's role: `expected-failures.txt` is a triage
source, not just a suppression list. Anyone regenerating docs or bundles should
read it first, and the doc-regeneration pass that shipped in `a680b2b50f`
would have caught the CUDA error if it had.
