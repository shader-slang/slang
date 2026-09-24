# Slice 209: preserve CUDA hardware active-mask semantics

This ExecPlan follows `.agent/PLANS.md`. The NVVM workflow explicitly requires this completed
plan and its report to be committed; raw evidence remains under ignored `build/`.

## Purpose and Observable Result

`WaveGetConvergedMask()` and `WaveGetConvergedMulti()` must read the currently executing CUDA
lanes without naming nonparticipating lanes in a full-mask synchronization. Existing implicit
aggregate shuffles must use the CUDA prelude's raw read followed by ballot of those lanes.
This is backend parity with CUDA; it does not promise logical reconvergence from a hardware mask.

## Progress

- [x] 2026-09-24: Read workflow, accepted 208 evidence, build skill, and producer/consumer trace.
- [x] 2026-09-24: Select full checkpoint and document scope before implementation.
- [x] 2026-09-24: Provider-owned sideeffect/convergent PTX operation compiles; full host build completed.
- [x] 2026-09-24: Both LLVM dialects and actual PTX validated. Exact final fixtures compile with
      old emitter full-mask ballots; revised aggregate structural unit fails as expected. No old GPU
      dispatch. Final restored compiler/provider hashes exactly match passing prototype.
- [x] 2026-09-24: Typed closure and raw-versus-ballot composition implemented; focused 9/9,
      runtime 4/4, units 475/475 (one existing Windows skip), toolkit 18/18 pass.
- [x] 2026-09-24: All final gates and four explicit SM50/60 compile/assembly probes pass.
- [x] 2026-09-24: Exact accepted 208 comparison has zero old deltas; 53 failures and 4 resolved
      histories retained. Self-review/durable records complete; ownership returns to parent for acceptance.

- [x] 2026-09-24: Parent independently accepted all final code/evidence for local commit.

## Surprises and Discoveries

`WaveGetActiveMask()` is a distinct logical operation handled by `slang-ir-synthesize-active-mask`;
its existing full-mask entry ballot is intentional and stays unchanged. The faulty function
`_emitNVVMActiveMaskValue` instead implements exact `__activemask()` assembly from
`WaveGetConvergedMask/Multi`, as well as `_getActiveMask()` inside aggregate shuffle.
CUDA prelude `_getActiveMask` explicitly computes `__ballot_sync(__activemask(), true)` and
retains a historical TODO about logical mask tracking. LLVM 14 has no activemask intrinsic enum. Parent reviewed the finite provider PTX approach and
agreed to ABI 36/full checkpoint, preserving deferred FP64 admission.

## Decision Log

- 2026-09-24 worker: Existing correctness gap overrides rolling complex cadence (207/208/209
  wave-heavy). All six material cells compile; application runtime bindings/oracles remain absent.
- 2026-09-24 worker: Full checkpoint required for provider/catalog semantics and ABI impact;
  preserve all 452 frozen identities and accepted 94 discovery identities in all three modes.
- 2026-09-24 worker: Raw mask tests must permit scheduler-dependent subsets; a source branch alone
  does not guarantee an exact converged mask. Exact operation/PTX inspection proves elimination
  of illegal full-mask ballot. Keep logical-mask synthesis unchanged.

## Outcomes and Retrospective

Parent accepted the completed full checkpoint after independent code and evidence review. All 1585 prior correct cells
are refreshed, six additions pass, all 53 unresolved failures remain; 1644 fresh / 1591 correct, no
missing/duplicate cells or old outcome-field deltas. No material runtime claim. Parent owns the local commit. Last full checkpoint is now 209 and
implementation cadence resets from 1 to 0.

## Context and Current Pipeline

For `if (lane < 16) mask = WaveGetConvergedMask();`, `hlsl.meta.slang` produces a canonical
zero-argument unsigned-i32 `GenericAsm("__activemask()")`. `_resolveNVVMAggregateWaveOperation`
validates this exact helper; `_initializeNVVMActiveMaskStep` currently requests ballot(uint,bool),
and `_emitNVVMActiveMaskValue` supplies (-1,true). PTX vote.sync requires all non-exited named
lanes to participate, which bypassing upper lanes need not do. The valid producer needs no repair;
the operation boundary must represent a hardware mask. `_waveShuffleMultiple(_getActiveMask(),...)`
requires a distinct raw-mask-then-ballot composition before existing scalar shuffles.

## Scope and Non-Goals

Correct the two existing raw-mask families and existing implicit aggregate shuffle semantics.
No FP64 feature expansion, min/max, vector shuffle, quad reconvergence, KernelContext, constant
precision, material runtime assumptions, driver change, commit or push. Old corpus sources/oracles
are immutable. Preserve all 53 open failure histories and four resolved transitions from 208.

## Architecture and Invariants

Add a zero-operand unsigned-i32 hardware-mask semantic only if no existing exact operation exists.
Provider owns fixed PTX instruction emission, with optimizer attributes preserving control dependence.
Raw intrinsic emits exactly the read; implicit aggregate helper composes it with ballot(mask,true).
Preflight closure lists both operations before provider discovery. Typed catalog rejects invalid
signature shapes. No AST/IR syntax rebuilding, arbitrary graph walks, or fallback is allowed.

## Interfaces and Dependencies

Internal builder ABI 36 replaces ABI 35, semantic catalog and LLVM 14 provider; CUDA 12.9.2,
L4 SM89 executing target 80. The external contract is PTX `activemask.b32`, introduced ISA 6.2,
with scheduler-dependent membership; vote.sync requires named non-exited lanes to participate.
References: https://docs.nvidia.com/cuda/archive/12.9.1/parallel-thread-execution/index.html and
https://docs.nvidia.com/cuda/archive/12.9.1/cuda-c-programming-guide/index.html.

## Milestones

1. Prototype fixed provider PTX read, require appropriate no-hoist attributes, validate LLVM 7
   serialization and libNVVM/PTX assembly. Discard if semantics or optimizer contract remains unclear.
2. Add focused raw scalar/vector and implicit shuffle coverage with independent invariants and
   exact closure/PTX assertions. Capture old compiler failure before change; ordinary unsafe runtime
   passes do not prove correctness.
3. Build with `source build/nvvm-loop/slice-203-env.sh` then
   `cmake --build --preset releaseWithDebugInfo --parallel 4 --target slangc slang-test render-test test-server`.
4. Final focused gates, smoke, units (two servers), toolkit, frozen 452 × 3, discovery 94 × 3 plus additions
   (at most 100 identities), complex 6 cells; sequential suites, at most four CPU workers overall.

## Validation and Acceptance

Raw paths: `build/nvvm-loop/slice-209-before` and `slice-209-after`. Follow exact commands in WORKFLOW.
Run GPU smoke before expensive GPU suites. All new runtime fixtures execute NVRTC O3/NVVM O0/O3;
all old logical-mask and wave coverage is preserved. Compare identities, classification, return code,
full execution_counts, diagnostic and canonical_shape against accepted 208 cumulative outcomes.
No missing/duplicate cells; additions are separate; historical 53 failures remain visible.
Final tested source, compiler/provider hashes and ABI must match after last source edits.

## Failure and Recovery

Stop GPU dispatch on device loss. Preserve rejected runtime/IR probes as excluded evidence, never
change old oracle to hide failure. Revert unprincipled implementation if provider contract cannot be
proved. Escalate consequential uncertainty to parent, documenting exact evidence.

## Artifacts and Hand-Off

Create report.slice-209-active-mask.md, runtime-validation.slice-209.json, census/discovery summaries,
and STATUS handoff. Include helper inventory/input-shape audit, exact delta counts, final source and
binary hashes. No binaries or raw logs tracked. Parent reviews then commits accepted slice.

### Final-source evidence

Compiler SHA256: `483db7465914c1626c8fd427f425eebbcd04e8f996a26ba031dec65c0893a231`.
Provider SHA256: `ae0e7859f6062a69f191cace4ae8d96340c074815866808c5b43467f41144372`, ABI 36.
Source base: `d54288b4d9483bd3d6a3036453fa7079321fd163` plus exact hashes in the raw provenance.
The scoped revert restores only the emitter from that base, retaining final ABI/catalog/provider;
its identity is explicitly distinct from accepted 208 binaries. `final-fixtures/` contains final
source hashes and old PTX. Exact raw reads disappear and full-mask ballots return at O0/O3.
Restoring the final emitter reproduces identical final compiler/provider hashes.

NVRTC O3/direct O0/direct O3 raw PTX read counts are 8/2/8 with no ballots. Implicit matrix helper
counts are 3/1/3 raw reads and 3/1/3 ballots with dynamic observed masks. NVRTC and direct O3 PTX both
retain actual lane < 16 branches and a lane 31-only branch around singleton reads. This validates
the runtime membership oracle without assuming whole-source-branch convergence.

A provisional new fixture used whole-matrix equality, which CUDA source emission does not provide.
The final new oracle compares both rows explicitly; no old source/oracle or compiler support changed.
Raw rejected fixture/log evidence remains excluded.

### Completed self-review and handoff

No new production helper or fallback exists. The existing active-mask recipe initializer/emitter
now selects the exact hardware snapshot; the implicit aggregate recipe explicitly owns its ballot.
Provider finite inline assembly is the canonical implementation boundary because LLVM 14 has no
intrinsic enum; sideeffect/convergent and both-dialect/actual-PTX checks preserve the contract.
Canonical producer shapes were correct; no syntax or semantic representation is reconstructed.
The old full-mask mapping fails the exact structural test and PTX inspection. Logical mask synthesis
and deferred FP64 implicit admission remain unchanged. Parent independently audited PTX/hash and
full-ledger preservation. Parent accepted slice 209 as the latest implementation and full checkpoint;
the implementation cadence is now 0.

Unrelated formatter whitespace in legacy emitter-unit sections was reversed after the corpus
gates. The intended changed unit function bytes remained identical. Only its unit artifact changed,
and the full 475-unit suite was rerun. Compiler/provider/runtime source hashes stayed identical.
All final source and artifact hashes are rechecked by the result generator.
