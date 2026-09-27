# Reject the typed-copy helper as a general array correction

## Motivation

Prototype281's small `noinline optnone` NVVM store helper preserves nested-array layout, but a provider
store must also handle constructed/selected values, earlier snapshots and underaligned pointers. Its
aggregate call arguments could grow with array size even when the authored helper stays small.

## Proposed solution

The broader small correctness gate passes, but the general helper is rejected under the frozen size
gate. No production change or arbitrary size cutoff is selected.

| Frozen gate                             | Result                              |
| --------------------------------------- | ----------------------------------- |
| Small ordinary store, NVVM O0 / O3      | Correct / wrong mask18              |
| Small typed helper, NVVM O0 / O3        | Both correct                        |
| 17-element helper, O0 / O3 compile-only | Both compile                        |
| 65,536-element helper, O0 compile-only  | Timeout at120seconds, process124    |
| 65,536-element helper, O3               | Not run after the size-gate failure |

The small oracle is `[0,123,0,456]`, over65536low16-bit patterns at runtime seed0. It observes an earlier
load after every source field changes, a completely constructed value, both branches of an aggregate
phi, actual byte-offset1 storage and four surrounding canaries. The timeout was bounded by120seconds
and4GiB virtual memory; it does not prove that compilation could never finish with more resources.

## Change summary

Only this report, the completed plan, [structured evidence](research-evidence.slice-282.json) and
navigation change. Raw prototypes/runners/results remain under `build/nvvm-ssa-copy-boundary282`.
Eight requested cells retain exact disposition: three correct GPU, one wrong-output control, two
successful compile-only, one timeout and one explicitly not run. No accepted artifacts were replaced;
37runtime/8source/2config/576input/22pin identities remain exact279.
Full279/targeted233/cadence0 and1740corpus outcomes remain inherited. Research280's open optimized
array-copy defects remain open; this experiment does not alter their failure history.

## Concepts and vocabulary

An SSA snapshot keeps a value from its original load time. A phi selects between complete SSA values.
Alignment is a guarantee supplied to a memory operation; a typed pointer can still require align1
when its actual byte address is misaligned. A call's aggregate parameter materialization is separate
from the number of instructions authored in the helper body.

## Process report

The canonical three-element payload contains nine scalar leaves at per-element offsets0/4/8,
stride12. Every constructed leaf is inserted before use. The earlier load occurs before source
mutation and is then stored independently; both the old and new values are checked. The phi receives
that earlier SSA value and an independently constructed pattern, so it does not read previously
corrupted destination memory. Each branch executes per pattern.

A40-byte align4 byte allocation supplies the real offset1 destination. The36-byte payload occupies
bytes1..36; canaries at0/37/38/39 are checked after each store. Typed accesses declare align1, and a
pointer-low-bits check confirms the intended address. No semantic check reads internal padding. The
helper succeeds on both optimization levels; ordinaryO3 fails child.first in its aligned snapshot and
constructed destinations (mask18). There is no malformed producer representation to repair.

The larger cases reuse281's loaded-value snapshot/copy kernel with only array capacity and count-bound
changes. From3 to17 elements, O3 PTX instruction count grows242→578: helper21→19, caller replace46→215,
and kernel120→289. This exposes caller-side aggregate argument expansion despite a small helper.
At65536elements, O0 produced no completed PTX/result before the120-second timeout. The process group
was terminated and the O3 cell explicitly retained as not run under the frozen stopping rule. No
retry, threshold or fallback conceals this failure; a sampled memory observation is not peak usage.

Root and separate reused-context independent review audit source shape, full output, caller/helper
PTX, exact identities and the timeout/stop rule. Results qualify only the concrete small value cases;
large cases have no GPU execution, and no NVRTC helper or production correction is claimed.

Next, test a cheaper counterfactual: weakening the whole-array store's alignment annotation while
keeping an actually aligned destination, without introducing a new call boundary. The ordinary
misaligned path supplies a lead, not proof that this change corrects general aligned stores. Preserve
canonical values and establish independent before/after evidence before any implementation.
