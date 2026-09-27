# Qualify nested substandard records through dynamic dispatch

This bounded research/qualification ExecPlan follows `.agent/PLANS.md` and the committed NVVM plan
exception. The authorized loop remains active; skip Slack, no push/system changes. Root owns scope,
execution, acceptance and commits; one bounded author owns raw fixtures/driver, separate review
checks source/oracles/evidence. Preserve accepted285 binaries; no compiler implementation in this slice.

## Purpose and Observable Result

Connect two previously separate qualified domains:270 flat FP8/BF16 records through AnyValue dynamic
dispatch, and279 nested FP8/BF16 records through internal value/local transport. Demonstrate that
nested fields survive runtime-selected concrete interface implementations and generated packing/
unpacking with independent raw-bit expectations. A diagnostic change alone never establishes support.

## Progress

- [x] 2026-09-27: Research286 committed4cb05e2d7; reread WORKFLOW/STATUS. Accepted285 layout and
      sources are restored exactly; no production change from286. Candidate-screening frequency lead retained.
- [x] Author/reviewer freeze at most two focused sources, exact payload-layout contract and full oracle.
- [x] 2026-09-27: New bounded author nested_dynamic287 available; separate reused reviewer assigned. Live285 identity verifies100layout/37runtime/11source/2config/576input/22pins and device.
- [x] Freeze all source/runner/mode/control obligations before execution.
- [x] Run bounded cells serially, retain full buffers and actual dynamic IR/physical-layout evidence.
- [x] Review gaps or qualified scope; preserve identities, compact report/record/plan/navigation and local commit.

## Surprises and Discoveries

All six new cells and six neighbors pass. Both deliberate corruptions return the predicted masks.
Two captures retain runtime selection, mutation pack-back and the original snapshot payload. Natural
AnyValue size20 differs from CUDA size24; the canonical marshaller preserves both layouts correctly.

## Decision Log

- 2026-09-27, root: Return to prioritized FP8/dynamic language coverage after material research286.
  Limit this slice to qualification on accepted binaries; an actual failure gets its own bounded
  correction slice. No broad aggregate ABI/admission expansion or speculative compiler change.

## Outcomes and Retrospective

Twelve positive GPU cells, two expected oracle rejections and two IR captures completed without
timeout/retry. Accepted285 identities remain exact before/after. Independent final review accepts all evidence with no findings; formatting and local commit close
this slice. The main corpus stays580cases/576sources/1740cells with1703correct/37unresolved/20histories;
last full285/targeted233/cadence0 stays inherited. New focused sources remain outside the main manifest.

## Context and Current Pipeline

Existing `tests/compute/dynamic-dispatch-substandard-float.slang` uses `[anyValueSize(4)] IFoo`,
two registered concrete implementations and `createDynamicObject<IFoo>(runtimeID, packedPayload)`.
Generated pack/unpack helpers use canonical field keys and Natural byte packing; NVVM type lowering
allocates concrete local records according to CUDA layout.270 qualified flat records and BF2 component
addresses.279 recursively admitted nested local/value records;285 corrected nested integer array stores.
This slice tests nested records (not arrays) passing through the actual dynamic interface machinery.

## Scope and Non-Goals

At most one nested substandard fixture with two concrete conformers and one integer-only structural
control. Use existing E4M3/E5M2, scalarBF16/BF2 and integer leaves only; finite nested records, no
record arrays, BF3/BF4 whole-value admission, device/shared/readonly/exported signatures or arithmetic
semantics. No implementation, type equivalence, custom marshaller or new backend capability in287.
Do not let constant folding/static specialization replace the dynamic path being claimed.

## Architecture and Invariants

Preserve original canonical record fields and witness/interface machinery. Before execution, derive
and document the packed input schema from the existing AnyValue marshalling contract; independently
calculate Natural field offsets and CUDA local offsets. Do not infer an expected layout from observed
wrong output or confuse payload packing with local storage. Choose a small fixed capacity sufficient
for each conformer, with distinct integer guards around nested substandard values. Runtime-loaded
payload bits/type IDs must exercise both conformers; inspect final IR to prove dynamic selection and
packing/unpacking survive. If the source API cannot retain the intended path, record that boundary
rather than silently replacing it with static generic calls.

Raw-bit transport uses bit_cast and independent integer checks, no floating arithmetic/NaN comparison.
Cover all256 FP8 patterns and65536 BF16 encodings with deterministic distinct neighboring values;
state correlations and avoid claiming Cartesian coverage. Exercise both BF2 lanes and both registered
conformers. The checker must observe every payload field/guard independently, avoid reusing possibly
corrupted objects as expectations, and initialize output/destinations so omitted writes are visible.
If a dynamic mutating method can be included without expanding the schema, qualify pack-back as well;
otherwise explicitly scope results to unpack/read transport. Freeze this choice before execution.

## Interfaces and Dependencies

Raw root `build/nvvm-nested-dynamic287`. Focused source drafts and mirrored native directives stay
there for this research slice; promote a persistent native regression only under a subsequent explicit
bounded implementation/test slice if justified. Maintained focused runner from278/285 may be reused
without editing production corpus runners. Three modes: NVRTC O3, NVVM O0, NVVM O3, SM80. Native
Ubuntu/L4SM89/CUDA12.9.2/NVRTC12.9.86/LLVM14, driver580.126.09. Accepted285 actual compiler62469125,
providerABI42 af1661de/version301-g8fbf0f84e; current source HEAD4cb05e2d7 is not the compiled identity.
No build required. A future production rebuild must refresh version metadata and rebuild restored286
observer objects. Max4CPU workers, serial GPU/compiler cells, native2servers, all gates<=1800s.

## Milestones

1. Verify37runtime/11qualifiedsource/2config/576maininput/22pins against accepted285 and full100-entry
   restored layout. No source edits/builds. Record actual loaded binaries and selected material/source
   identities. Existing full285 results remain inherited, not rerun under a new label.
2. Author prepares at most two fixtures plus exact expected full output buffer, bit-pattern loops,
   packed field offsets/size/alignment, explicit type conformance IDs and intended dynamic call path.
   Include at most two deliberate payload/guard corruption controls at NVVM O0 to prove checker
   sensitivity; expected mismatches and exact masks must be calculated before running. Formatting
   must preserve directives and explicit struct terminators. Reviewer/root approve before execution.
3. Freeze six main obligations (2sources x3modes) if both sources are used, plus at most2negative
   oracle controls and up to2IR captures (one per actual source at NVVM O3). Runner must preserve
   each requested disposition, source/command/hash, actual complete output, classifier labels and
   emitted diagnostics. Each shader cell<=180s, batch<=1800s, jobs1, no retries. If adapters cannot
   support the authored payload contract, record the obstacle before expanding to another harness.
4. Execute existing flat dynamic-dispatch source unchanged at3modes and existing nested local fixture
   unchanged at3modes as six neighboring controls. Preserve every previous passing output. Freeze
   original directives/sidecars/full independent outputs; no main manifest change or baseline erasure.
5. Examine actual preflight/LLVM/linked IR where available. Require visible dynamic type selection,
   canonical nested pack/unpack helpers and correct physical local layout before claiming dynamic
   nested support. On failure, retain all cells and trace the first owning producer/consumer boundary;
   stop this slice at a concrete next reproducer/correction plan, without adding implementation here.
   Compile failure is not a GPU mismatch; skipped/missing execution is never a pass. Known unrelated
   failures remain separate, including NVRTC nested-array defect and large-array O3 timeout.
6. Root/independent audit exact full output and IR evidence, negative-oracle sensitivity, all cell
   dispositions and live unchanged285 identities. Complete compact five-part report, one structured
   evidence record, completed plan and short navigation. Format/diffcheck/local commit; continue loop.

## Validation and Acceptance

Qualified support requires actual executed outputs in all requested modes and independent field/guard
oracles, plus dynamic-path IR evidence. No change to compiler/shared ABI means inherited full285 is
sufficient if all identities remain exact. Record unsupported roles/limitations explicitly. A failing
new case is useful evidence but not an accepted feature; no diagnostic merely moving downstream
justifies a compiler change. No compile-speed, GPU-speed or material runtime claim.

## Failure and Recovery

No production source/layout mutation is intended. Preserve initial failures and unrun suffixes;
fixture syntax/oracle corrections get new frozen versions and retain prior attempted cells. Do not
silently retry, widen roles or overwrite original evidence. If unexpected code/build mutation occurs,
stop and restore verified285 before other work. User stop instructions take precedence.

## Artifacts and Hand-Off

Raw fixtures, independent oracle/layout notes, driver/freezes, outputs and IR belong under raw root.
Durable report.slice-287-nested-dynamic-records.md, research-evidence.slice-287.json, this completed
plan and navigation own outcomes. Independent evidence/IR review is accepted; immediate next action is the final formatted local commit.
Next bounded slice promotes the qualified probes into persistent native regressions without expanding
compiler admission or changing the main corpus.
