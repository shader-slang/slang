# Slice 209: preserve CUDA hardware active-mask semantics

## Motivation

Consider the following kernel fragment:

```slang
uint lane = WaveGetLaneIndex();
if (lane < 16)
{
    uint mask = WaveGetConvergedMask();
    outputBuffer[lane] = mask;
}
```

CUDA's `__activemask()` observes the hardware lanes executing that instruction. Direct NVVM
instead emitted `vote.sync.ballot.b32` with participation mask -1. Non-exited upper lanes that
bypass the branch need not participate, so the emitted operation violated the ballot contract.
A runtime pass under that contract cannot prove correctness. The hardware mask itself may be a
proper subset of the source branch's lanes because independent scheduling does not guarantee
source-level convergence.

## Proposed solution

Represent the hardware observation as a distinct zero-operand unsigned-i32 typed operation.
The LLVM 14 provider emits fixed `activemask.b32` inline assembly, marked `sideeffect` and
`convergent` so optimizer transformations preserve distinct observations and control dependence.
LLVM 14 has no corresponding intrinsic; no undocumented `llvm.*` declaration is invented.

Raw scalar and uint4 source intrinsics use that observation directly. Existing implicit matrix
shuffles reproduce the CUDA prelude's `_getActiveMask()` composition: read hardware mask, then
ballot true using that observed mask. The separate `WaveGetActiveMask()` logical-mask synthesis
pass stays unchanged. CUDA prelude's historical logical-mask-tracking TODO remains applicable.

## Change summary

- Builder ABI 36 and semantic catalog: append exact hardware-mask operation and signature.
- Provider: fixed hardware instruction; typed validation remains the operation boundary.
- Emitter: remove full-mask ballot substitution; explicitly represent the implicit helper ballot
  in both preflight closure and emission.
- Builder/emitter units and two CUDA fixtures: exact serialization/operand provenance and
  independent runtime membership/self-shuffle invariants, all three backend/optimization modes.
- Discovery manifest adds two distinct sources; old frozen/discovery oracles remain unchanged.
- Plan, result manifest, census tables, design and STATUS retain full-checkpoint evidence.

## Concepts and vocabulary

A hardware mask is the snapshot of lanes executing one instruction. A logical active mask is the
source-control-flow participation set propagated by Slang's synthesis pass. A participation mask
names the lanes required to execute a synchronized ballot or shuffle. These contracts differ.

## Process report

`hlsl.meta.slang` produces canonical `GenericAsm("__activemask()")` for
`WaveGetConvergedMask`, and a uint4 construction around the same read for `WaveGetConvergedMulti`.
`_resolveNVVMAggregateWaveOperation` validates their exact result and zero-parameter signatures.
Their producer is correct. `_initializeNVVMActiveMaskStep` previously selected a different semantic
operation; `_emitNVVMActiveMaskValue` supplied a full mask and true. The fix replaces this
incorrect boundary mapping with a typed hardware read.

For `WaveReadLaneAt(matrixValue, lane)`, lowering produces the existing OutParam aggregate helper
spelling `_waveShuffleMultiple(_getActiveMask(), $0, $1)`. Its valid matrix shape continues through
recursive scalar shuffles. The helper owns the additional ballot, so its operation recipe and
preflight now include that ballot explicitly. The ballot mask operand is the hardware read result,
not an invented full participation set. The source prelude remains the source of truth.

No new helper, fallback, structural equivalence, graph walk, syntax reconstruction or representation
repair was introduced. Existing helper changes survive because they implement valid canonical
producer shapes at their backend boundary. FP64 implicit aggregate admission remains deferred;
its old rationale is updated because this slice removes the mask defect but does not validate or
expand that feature. Full provider/ABI impact requires a full corpus checkpoint.

The raw runtime fixture checks self membership, excluded lanes from inspected divergent branches,
zero upper uint4 words and exact single-caller masks. It does not assert an exact mask for a whole
source branch. The implicit matrix fixture reads each caller's own lane, so the expected value is
independent of scheduling. Structural before/after evidence is decisive for eliminating the old
illegal ballot; ordinary old-runtime success is not counted as proof.

Full validation is complete; exact results and source/artifact identities are recorded in
`runtime-validation.slice-209.json`. Parent owns acceptance and commit. Complex cadence is explicitly overridden by this recorded correctness
issue; all six material cells already compile, and application runtime contracts remain absent.

### Final validation and preservation

| Gate                         | Result                                                                       |
| ---------------------------- | ---------------------------------------------------------------------------- |
| Focused                      | 9/9: six GPU cells plus strict provider, closure and unsupported-shape units |
| Runtime smoke                | 4/4                                                                          |
| NVVM/routing/reporting units | 475/475; one existing Windows-only skip                                      |
| Toolkit                      | 18/18 compile/assembly cells                                                 |
| Full frozen                  | 1356 fresh cells; 1333 correct, 23 unchanged failures                        |
| Full discovery               | 288 fresh cells; 258 correct, 30 unchanged failures                          |
| Complex materials            | 6/6 compile/assembly cells; no runtime claim                                 |
| Explicit lower targets       | SM50/SM60 x NVRTC/NVVM: four raw-mask compile/assembly cells                 |

All 1585 previously correct cells are freshly preserved. Six additions pass; all 53 existing
failures and four resolved slice 208 failure histories remain recorded. The 1644-cell checkpoint
contains 1591 correct cells with zero missing/duplicate cells and zero old changes in classification,
return code, full execution counts, diagnostic or canonical shape. Both corpus runners return2
for retained diagnostic failures; that exit is explicitly preserved rather than treated as all-green.
The two unchanged frozen sum/product fixes and all nine slice 208 additions remain correct.

The exact final fixtures were compiled at O0/O3 with only the emitter reverted to accepted 208
source. They emitted zero hardware reads and full-mask ballots; the final aggregate unit rejected
that old behavior. The provider/catalog remained ABI 36 during this isolated drill, so this is a
controlled emitter comparison, not a claim of executing accepted 208 binaries. No divergent baseline
GPU kernel was dispatched. Restoring the final emitter reproduced byte-identical final artifacts.

Final compiler SHA256:
`483db7465914c1626c8fd427f425eebbcd04e8f996a26ba031dec65c0893a231`.
Provider SHA256 (ABI 36):
`ae0e7859f6062a69f191cace4ae8d96340c074815866808c5b43467f41144372`.
Source base is `d54288b4d9483bd3d6a3036453fa7079321fd163` plus manifest hashes.
After reversing unrelated legacy unit-file formatter whitespace, only the unit artifact changed;
the full 475-unit suite was rerun. Compiler/provider/runtime fixture hashes remained unchanged,
so the complete runtime checkpoint remains applicable. No GPU loss, driver change, reboot, commit
or push occurred. Raw evidence lives under `build/nvvm-loop/slice-209-{before,after}`.

Parent accepted this full checkpoint as 209, resetting cadence from 1 to 0. Rolling 207/208/209
is wave-heavy because the existing correctness defect overrides complex cadence; the next selection
should reconsider complex work without inventing the absent material runtime contract.
