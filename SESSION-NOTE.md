# slang#12829 — session state

Branch: fix/issue-12829 (worktree /workspace/agent/wt-slang-12829), base master 28c755b09d.

## Done
- Fix applied: `source/slang/slang-check-overload.cpp` — gated the generic-param-count block behind
  `if (!isSlang202cOrLater(this))` + rewrote the misleading comment (2196-2199), kept the 3 TODOs.
- Tests added:
  - `tests/language-feature/generics/generic-overload-count-tiebreak.slang` (behavioral, legacy+202c COMPARE_COMPUTE -cpu)
  - `tests/language-feature/generics/generic-overload-count-tiebreak-ambiguous.slang` (DIAGNOSTIC_TEST -std 202c)
- Plan: /workspace/agent/reports/slang-12829.md (has verified pre-fix baseline table).
- Submodules initialized in worktree; configured with `cmake --preset default`.
- Build running in background → build.log (waiter task b8uhod11m watches for BUILD_EXIT=).

## Pre-fix baselines (prebuilt slangc), all verified:
- IFloat/vectorN: legacy&202c → 1 ; post-fix 202c expected ambiguous
- vector3/vectorN: legacy&202c → 3 ; post-fix 202c expected ambiguous
- OverloadRank(1) on vector: legacy&202c → 1 ; post-fix 202c expected 2 (fall-through)
- non-generic vs generic: both → 10 ; must stay 10 in 202c

## ⛔ HOLD (2026-08-29 ~00:59Z) — DO NOT PUSH / OPEN PR
Maintainer (tangent-vector, via slang-triager msg id=4) found the naive Approach A regresses
`tests/language-feature/operator-overload/builtin-operator-fastpath-glsl.slang`: core + glsl expose
OVERLAPPING GENERIC operator overloads (matrix `*`, vector `==`/`!=`) that become AMBIGUOUS in 202c
once the count block is gated off — because the generic early-return at slang-check-overload.cpp:2420-2426
skips scope-rank AND OverloadRank for generic-vs-generic ties (NO fallback tie-breaker). Maintainer moved
his spike PR #12830 back to draft. Scope is now an OPEN language-design question (restructure/rank builtin
overload families first, or wait for a principled specificity relation). Branch preserved intact.
Reported HOLD to parent (msg id=11) and to triager (msg id=9).

## Investigation DONE (2026-08-29 ~01:10Z) — findings sent to triager (msg id=15) + parent (id=17)
Build succeeded (BUILD_EXIT=0). Reproduced glsl regression on the Approach-A build. KEY FINDING:
the ambiguous ties in builtin-operator-fastpath-glsl.slang are **glsl-vs-glsl at IDENTICAL
OverloadRank(15)** (both candidates in glsl.meta.slang, differ only by generic-param count), NOT
core(−3/−2)-vs-glsl(15). OverloadRank IS already reachable for these fully-specialized generics
(probe: differing ranks resolve, equal ranks ambiguous) ⇒ the 2420-2426 early-return is NOT the
blocker ⇒ "rank-first for generics" does NOT fix it. Needs a core-module/language-design change.
Full write-up: reports/slang-12829-glsl-regression.md. Learning appended to /workspace/shared.

My ambiguity diagnostic test PASSES. Behavioral test FAILS due to a HARNESS bug (COMPARE_COMPUTE
routes -std/-D to render-test which rejects them, and auto-synthesizes cuda variants) — needs
rework IF the approach survives redesign; deferred.

## Next steps (BLOCKED until scope decision)
- Pure hold. Do NOT run push/PR steps below until maintainer/parent resolves the language-design scope.
- If approach survives: fix the behavioral test harness invocation (use DIAGNOSTIC_TEST or a
  proper multi-file variant; -std/-D must reach slangc, not render-test).

(Deferred) push/PR plan once/if unblocked:
1. run both new tests; finalize CHECK lines against real diagnostic text.
2. broader regression: slang-test tests/language-feature/generics/ AND tests/language-feature/operator-overload/
3. ./extras/formatting.sh ; /code-review medium
4. commit, push origin, draft PR vs master, label "pr: non-breaking", trigger ci.yml.
5. peer review to slang-reviewer BEFORE Fix Report to parent.

Label confirmed available: "pr: non-breaking".
