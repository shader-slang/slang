# Characterize local substandard record-array admission

Status: independently accepted research; closed with this local commit. Development loop stopped.
No production change.

## Motivation

Qualified FP8/BF16 record values do not imply record-array support. Consider these valid source roles:

```slang
struct Cell
{
    uint16_t before;
    FloatE4M3 a;
    FloatE5M2 b;
    BFloat16 scalar;
    uint16_t after;
};
typedef Cell Pair[2];
struct Wrapper { uint head; Cell values[2]; uint tail; };
[noinline] void assignOut(out Pair destination, Pair source) { destination = source; }
```

Compare a local `Cell values[2]` accessed through a host-dependent index, the root Pair through
noinline initialization/copy helpers, and equivalent guarded Wrapper helpers. An integer-only Wrapper
with the same field widths separates the array structure from the substandard leaf admission.

## Proposed solution

Characterize accepted285 without widening admission. All three mixed sources pass NVRTC O3. NVVM
O0/O3 reject the local variable or mutable helper parameter. The integer control passes all three
modes. Six correct GPU cells produce 22 exact words; two existing negative units pass. Three additional
O0 captures retain Slang IR and repeat the same rejections, producing no LLVM/PTX or GPU evidence.

| Mixed source role      | NVVM O0/O3 E52017 shape                               |
| ---------------------- | ----------------------------------------------------- |
| Local indexing         | `var`                                                 |
| Root-array helper      | `helper function parameter: OutParam<Array<Cell, 2>>` |
| Guarded-wrapper helper | `helper function parameter: OutParam<Wrapper>`        |

One deliberate integer guard corruption produces the independently predicted `[32,0,65536,9321]`
and fails its unchanged zero-mask oracle. This is an expected failure, not a passing shader.

## Change summary

Only this report, the bounded plan, [structured evidence](research-evidence.slice-296.json) and navigation
are retained. All 18 obligations and the interrupted-at-cell13 history remain explicit. Sources,
versions, runners, full buffers and IR logs stay under `build/nvvm-record-arrays296`. No compiler,
provider, maintained test, corpus inventory or classifier changes; full293 remains inherited.

## Concepts and vocabulary

**Admission role** distinguishes internal values/local mutable references from resource, shared,
readonly and exported interfaces. **Physical storage** is a separate representation proof, not a grant
of these roles. **Natural layout metadata** describes rule0 on the captured Slang IR; it is not emitted
CUDA allocation or provider layout evidence. **Unsupported compilation** executes a failed harness
obligation but provides no GPU correctness result.

## Process report

`visitAggTypeDecl` builds canonical records; `visitVarDecl` creates local storage, and
`_lowerInfoFromFuncParameters` wraps source `out` parameters with `OutParam`. Final O0 Slang IR retains
`Ptr(Array(Cell,2))` and dynamic `getElementPtr` in the local case. Root and wrapper cases retain both
noinline helpers, the source whole-value load, the call and `store(destination,source)`. These are
intentional source representations, not malformed types needing a producer repair.

`_validateNVVMFunction` rejects the local variable because no existing physical, copyable, helper or
resource local-root predicate accepts it. `_validateNVVMHelperTarget` rejects the two OutParam shapes
through `_isSupportedNVVMHelperParameterType`. The dedicated substandard proof recursively accepts
record fields but not arrays; general helper/copyable predicates exclude the FP8/BF16 leaves and also
authorize broader storage roles. Widening those general predicates would exceed this slice. Later
array layout, element provenance and provider-store gates are not exercised by these early refusals.
No new production helper, fallback or special case exists to audit.

Every GPU fixture visits all 65,536 low16 patterns across two elements. Integer expressions derived
from the host seed and element index independently check every field; raw bitcasts preserve the
encodings without floating arithmetic. Copy destinations begin with complementary patterns, and
source preservation is checked separately. Wrapper guards are input-dependent. Output masks start at
nonzero sentinels, completion counters increment in the loop, and all output words are inspected.
Coverage is correlated, not Cartesian. Calculated Cell/Pair/Wrapper CUDA sizes and alignments are
8/2,16/2,24/4; captured matching Cell/Wrapper decorations use Natural rule0 and do not prove CUDA layout.
BF3/BF4 physical component arrays and existing BF16 array metadata queries remain separate roles.

Pre-execution review replaced runner v1 to separate compiler-only classification from native counts
and reject additional non-E52017 compiler errors. The v2 run completed 13 obligations but stopped on
the corruption control: the maintained classifier's earlier NVRTC error heuristic matched `nvrtc-o3`
in the source path and consumed the FileCheck error. Its later FileCheck matcher already recognizes
that message. A path-only substitution in a saved-log copy changes the classification to runtime-mismatch.
Root and independent review adjudicated the exact saved buffer/log/counts; original failed/infrastructure
records remain unchanged. Unexecuted v3 initially misattributed the cause to FileCheck wording; reviewed
v4 corrects that account and runs only the five unrun obligations. No completed cell was rerun.

Before/after checks preserve 100 layout entries,37 runtime artifacts,11 qualified sources,2 configurations,
576 main inputs and22 pins. Six focused GPU passes, nine unsupported compile obligations, two units and
one expected corruption rejection account for all18. A fresh author and separate fresh reviewer inspect
sources, oracle, execution and IR. Final independent acceptance is complete. The user requested
stopping after296; the loop is stopped at this slice closeout. A separate path-sensitive classifier repair and required full checkpoint are queued,
not started. Record-array admission, provider285 behavior, material/runtime claims and
full293/targeted233/cadence0 remain unchanged.
