# Generated-suite coverage history

slangc compiler coverage of the **generated** test suite
(`docs/generated/tests/`) measured across the key commits in the suite's
history, plus this branch. All runs use the **same** coverage-instrumented
`libslang.so` (`build/RelWithDebInfo/`, clang source-based coverage) — only
the test suite is swapped (`git checkout <commit> -- docs/generated/tests/`),
so the numbers are directly comparable. Per-run profdata is preserved under
`build/coverage-generated-<label>/slang-test.profdata`; any report below can be
regenerated without re-running tests via
`tools/coverage/run-generated-coverage.sh --label <label> --report-only`.
The bootstrap/master/post-11421 runs were driven by
`tools/coverage/run-history.sh` and `run-history-11421.sh`.

## Data points

`covered = total_lines − missed_lines` (the second line-number in an llvm-cov
report is _missed_, not covered).

| Label                                  | Commit      | `.slang` files | Line cov   | Covered lines |
| -------------------------------------- | ----------- | -------------- | ---------- | ------------- |
| baseline (pre-regen, 4.7 design)       | `a8f6d4f7d` | —              | 53.10%     | 134,016       |
| suite-bootstrap                        | `5377f3e02` | 2,683          | 51.44%     | 129,831       |
| suite-master (fork `origin/master`)    | `eb0dba821` | 2,680          | 51.45%     | 129,877       |
| **suite-post-11421 (upstream master)** | `0486484fb` | 3,080          | **53.55%** | **135,156**   |
| this branch (step 3)                   | `43b6e3e8d` | 2,523          | 52.27%     | 131,922       |

## What changed between the commits

- **Bootstrap → fork master** (`5377f3e02` → `eb0dba821`): essentially no test
  change (−3 files, CI-wiring + expected-failures cleanup). Coverage flat
  (51.44% → 51.45%). The fork's `origin/master` does **not** contain PR #11421.
- **→ post-11421** (`0486484fb`, PR #11421): restructured the suite into
  `conformance/` + `design/` and **added** the language-reference conformance
  bundles. +400 files (2,680 → 3,080); +2.10pp (51.45% → 53.55%). This is the
  current upstream-master state and the **target to beat**. (It sits 6 commits
  ahead of the fork's `origin/master`.)
- **→ this branch** (`43b6e3e8d`): regenerated `design/` against the updated
  4.8 design docs (PR 11490 follow-up). The regen's recreate-to-doc-scope step
  **deleted 985 emission `.slang` files** (566 spirv-asm, 243 hlsl, 68 metal,
  61 glsl, 48 wgsl, 12 cuda, 1 torch) + ir-reference tests; backend fan-out +
  variant/deep expansion recovered ~1,257 lines. Net **52.27%** — still
  **1.28pp / 3,234 covered lines below** post-11421.

## Where the gap to post-11421 lives (per-file)

Per-file diff of slangc coverage, post-11421 minus this branch. The net
~3,039-line deficit is concentrated in emission + legalization backends —
exactly the territory of the deleted single-target tests:

```
 671  slang-emit-spirv.cpp           117  slang-ir-pytorch-cpp-binding.cpp
 640  slang-ir-glsl-legalize.cpp     106  slang-ir-metadata.cpp
 152  slang-ir-legalize-varying...    93  slang-emit-hlsl.cpp
 144  slang-ir-spirv-legalize.cpp     91  slang-emit-metal.cpp
 119  slang-emit-c-like.cpp           70  slang-ir-metal-legalize.cpp
                                      37  slang-emit-torch.cpp  (37→0)
```

This branch is ahead of post-11421 almost nowhere (≈97 lines, all jitter) — the
4.8 regen + expansion mostly **re-derived a subset** of the old coverage rather
than adding new ground. `slang-emit-torch.cpp` went 37→0: a single deleted test
was the only thing covering that file.

## Implication

To beat 53.55%, the deficit is the deleted emission depth (target-pipelines/\* +
ir-reference single-target tests), and the same emit-spirv / glsl-legalize /
spirv-legalize / varying-params files are the top under-covered hotspots **even
at the target**. Beating the target means either recovering that emission depth
(doc-enrichment so the design docs enumerate the per-type/per-op variants, or
restoring the deleted tests) and/or growing genuinely new doc-anchored depth on
top of the post-11421 state (the new `conformance/` tree, anchored to the
authoritative `docs/language-reference/` specs, has the most headroom).
