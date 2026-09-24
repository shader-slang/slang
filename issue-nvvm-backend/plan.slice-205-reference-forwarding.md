# Forward mutable helper parameters through the established NVVM pointer ABI

This ExecPlan follows `.agent/PLANS.md`. The NVVM maintainer exception requires committing the
completed plan/report with this bounded slice; the parent owns acceptance and the local commit.

## Purpose and Observable Result

Support an `out Payload` passed to a mutating method, as in the unchanged material's
`mx_layer_bsdf(..., out BSDF result)` calling `result.set_layer(...)`. Demonstrate independent
expected GPU output for nested forwarding, scalar and aggregate values, and repeated access to the
same storage at NVRTC O3 and NVVM O0/O3.

## Progress

- [x] 2026-09-24: Read workflow, status, accepted204 evidence and native Linux build skill; clean
      branch `nvvm-backend`, base `dd30f64a7672f345c3f47a0c1b434d5cb3debb28`.
- [x] 2026-09-24: Choose the material-driven reference boundary over independent wave transport.
- [x] 2026-09-24: Record bounded scope and full-checkpoint trigger before implementation.
- [x] 2026-09-24: Both final fixtures pass NVRTC and reject Out->Borrow independently at NVVM
      O0/O3 before; identical fixtures pass 6/6 after. Final IR records nested Borrow->Out forwarding.
- [x] 2026-09-24: Removed two redundant Ptr restrictions; no new helper or type/storage rule.
- [x] 2026-09-24: Full checkpoint complete: 1,617 fresh cells, 1,556 correct and 61 known
      failures; all 1,611 previous exact outcomes/counts/diagnostics preserved. Six complex cells replayed.
- [x] 2026-09-24: Report, manifest, outcome TSVs, design facts and STATUS prepared for parent
      integration review; parent owns acceptance and local commit.

## Surprises and Discoveries

The local copyable/helper pointer classifiers already accept exact single-operand `Ptr<T>`,
`OutParam<T>` and `BorrowInOutParam<T>` with generic address space. Call admission nevertheless
requires the argument to be `Ptr`, excluding forwarding between those canonical parameter roles.
The separate direct-resource struct classifier is unchanged. Pointer/descriptor-containing
helper-value aggregates remain inside the existing local helper storage domain; the second fixture
proves its pointer-bearing case independently.

## Decision Log

- 2026-09-24: Preserve frontend parameter roles; `addArg` directly forwards an address for
  Out/BorrowInOut/BorrowIn and `_lowerInfoFromFuncParameters` intentionally constructs distinct
  direction wrappers. These are valid source roles, not malformed or competing type identities.
- 2026-09-24: Reuse existing exact classifiers and `isTypeEqual` for element identity. Do not create
  structural equivalence, admit arbitrary pointers, alter alias attributes or physical layouts.
- 2026-09-24: Shared helper ABI admission requires a full checkpoint under WORKFLOW. Reuse accepted
  slice 204 as before evidence, subject to source/toolchain hash verification; do not repeat it.

## Outcomes and Retrospective

The focused contract passes in both canonical storage families. Runtime 4/4, focused 14/14, units 473/473 plus one platform skip and toolkit 18/18 pass.
Both sample_buffer direct complex cells now compile/assemble; both eval_buffer direct cells reject
the independent boolean-element pointer boundary. The final full corpus preserves all 1,611 prior cells exactly and adds six correct runtime cells.
No missing/duplicate keys, diagnostic changes or inherited runtime outcomes remain. The final
source/tool hash recheck passes. Parent acceptance and local commit are the next action.

## Context and Current Pipeline

`_lowerInfoFromFuncParameters` builds `OutParam<Payload>` for the outer output and
`BorrowInOutParam<Payload>` for mutable `this`. `addArg` obtains the existing address and passes
it without manufacturing a new value. `_isSupportedNVVMHelperArgument` should admit the wrapper
change only within existing canonical mutable local storage classes, retaining exact element type.
NVVM type lowering already maps these classes to the same typed generic pointer; call emission
must forward the address unchanged. The frontend owns initialization and source passing semantics.

## Scope and Non-Goals

One mutable local parameter forwarding feature. No expansion of the separate direct-resource
struct classifier, arbitrary reference compatibility, shared/global/device address-space coercion, readonly access relaxation,
physical layout changes, provider ABI changes or shader rewrites. No next-feature investigation.
Material support checks remain compile/assembly only: bindings and output oracles are absent.

## Architecture and Invariants

Source direction wrappers express allowed source operations. They are not independent pointee
layouts or permission to change address space/aliasing. Exact canonical local classifiers establish
storage, the existing parameter role checks establish mutable callee roles, and `isTypeEqual`
establishes pointee identity. Derived copyable pointers retain their existing acceptance, without
admitting additional derived layouts or wrapper spellings.

## Interfaces and Dependencies

Only `_isSupportedNVVMHelperArgument` is expected to change. Provider ABI 35 is unchanged. Native
RelWithDebInfo tools/provider, CUDA 12.9.2, L4/SM89 device and target 80 match accepted204. Inspect and
source `build/nvvm-loop/slice-203-env.sh` (it overrides Debug paths from env.sh).

## Milestones

1. Add a disjoint eligible runtime fixture and before proof with independent expected output.
2. Retain only necessary argument-admission edits; audit each new helper/fallback/special case
   (expected inventory: two existing branch restrictions removed, no new production helper). Both
   the copyable aggregate and pointer-bearing helper aggregate have
   separate fail-before runtime sources, so each removed restriction has a direct removal proof.
3. Run relevant reference/aggregate and appropriate rejection boundaries, runtime gate, units,
   toolkit, complex cells, and complete frozen/discovery checkpoint sequentially.
4. Preserve portable outcome TSVs, hashes and failures; draft five-part report and STATUS.

## Validation and Acceptance

Fresh domain: all 452 frozen identities and all 85 old discovery identities plus two new selected
sources, each in NVRTC O3/NVVM O0/NVVM O3. Compare every old key/classification/return code/execution
count/diagnostic exactly with 204 (1,611 cells, 1,550 correct and 61 known failures), with no omissions or
duplicates. Additions counted separately. This final full run has no inherited runtime cells.
Focused reference/aggregate tests and negative element/layout/address-space boundaries must pass.
Run runtime 4 first, units 473 plus one platform skip (or explain additions), toolkit 18 and all six
complex cells. Stop GPU work on device loss. No concurrent GPU suites or performance sampling.

Build: `CMAKE_BUILD_PARALLEL_LEVEL=1 cmake --build --preset releaseWithDebugInfo --parallel 4
--target slangc slang-test render-test test-server`. Reuse prior validate.sh commands with slice 205
output paths and the new focused selections; maximum four corpus workers. Discovery always reads
its complete 50–100-source manifest before optional filters. Format changed files only.

## Failure and Recovery

Record rejection or output failures without concealing them. Diagnose only this boundary; revert
an unprincipled attempt if downstream representation differs. Preserve base and before fixture
hashes for removal proof. A full regression blocks acceptance. Keep partial evidence restartable.
No push, driver replacement, reboot or local commit by the worker.

## Artifacts and Hand-Off

Raw scripts/logs/IR under ignored `build/nvvm-loop/slice-205-before` and `slice-205-after`.
Durable `runtime-validation.slice-205.json`, `census.slice-205.tsv`,
`discovery-census.slice-205.tsv`, this plan, five-part report, design and STATUS. Parent independently
reviews exact deltas and final hashes before acceptance/local commit.

The initial combined fixture had unsupported `pointer += 1` syntax; it was corrected to ordinary
`pointer = pointer + 1` before recording final baseline hashes. Splitting the fixtures exposes each
branch independently. A before IR probe omitted `-stage compute`; its retry overlapped linker output
and could not load the compiler. Neither is runtime evidence. Final post-build IR successfully
corroborates unchanged producer roles; pre-change GPU logs supply the actual failure proof.

## Final Acceptance Evidence

`runtime-validation.slice-205.json` is authoritative. Runtime smoke 4/4; focused 14/14; units
473/473 plus one Windows-only skip; toolkit 18/18; frozen 449/438/438 correct over 452 identities;
discovery 77/77/77 over 87 identities. All 61 known failures are unchanged. Complex NVRTC entries
and both sample_buffer direct modes compile/assemble; only eval_buffer direct modes reject the
recorded boolean-element pointer shape. The previous accepted full checkpoint remains 204 until
parent review, which is complete; this full checkpoint resets cadence to zero. No next feature started.

Per-branch removal proof: both final fixtures independently pass NVRTC before and reject Out->Borrow
at NVVM O0/O3; the exact source hashes pass all six cells afterward. Existing negative pointer,
readonly access and aggregate layout cases pass their rejection oracles. Final helper inventory:
two existing branch restrictions removed, both retained after input-shape review; no added helper,
fallback, producer change, structural equivalence, pointer cast or alias attribute.

The first formatting attempts lacked the setup PATH. The final pinned-tool code formatting check
passed without source changes, so all runtime evidence still describes the final source. Design
formatting restored its untouched HEAD prefix to avoid historical table churn.

Parent integration review accepted both existing-branch changes, independent fail-before fixtures,
exact full preservation and final hashes. Slice 205 is the accepted full checkpoint; cadence is zero.
