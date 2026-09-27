# Array copies expose a preexisting padding-loss bug

## Motivation

Slice279 corrects stores with direct nested struct fields, while preserving opaque array stores to
avoid expanding potentially huge arrays. General integer arrays were already admitted. This valid case
therefore needed separate evidence:

```slang
struct Child { uint16_t first; uint last; }
struct Cell { uint16_t first; Child child; }
typedef Cell Payload[3];
[noinline]
void assignOut(out Payload destination, Payload source)
{
    destination = source;
}
```

Distinct initialized field values must survive this copy. Each Cell has size12, with scalar fields at
byte offsets0,4,8; the child.first field must not move into padding at offset2.

## Proposed solution

This is accepted research with open correctness defects, not a compiler fix. Three frozen fixtures
exercise root arrays of flat padded records, a guarded record containing such an array, and arrays of
nested records. Each executes65536low16-bit patterns at NVRTC O3/NVVM O0/O3 with an independent full
buffer oracle. An isolated out-copy diagnostic removes correlations between earlier failing operations.

| Shape                        | NVRTC O3 | NVVM O0 | NVVM O3 |
| ---------------------------- | -------- | ------- | ------- |
| Root array of flat records   | Pass     | Pass    | Pass    |
| Guarded record with array    | Pass     | Pass    | Pass    |
| Root array of nested records | Wrong46  | Pass    | Wrong14 |

The numbers are failing-check masks; every correct buffer is `[0,123,0,456]`. The isolated copy returns
`[2,123,0,456]` in both optimized modes and passes at NVVM O0. Mask2 identifies destination child.first;
all source fields remain correct. The preserved277 compiler also fails the original nested probe
(NVRTC46/NVVM O3=46), proving this is preexisting. The disappearance of mask32 on279 is consistent with its direct element-store correction; canonical
element stores are visible in PTX. Whole-array copies remain affected.

## Change summary

No compiler, provider, harness or corpus inventory changes. This report, completed plan and
[structured evidence](research-evidence.slice-280.json) retain the nine focused outcomes, isolated
three-cell diagnostic, three-cell277 control, negative oracle control and unchanged identities.
STATUS/HANDOFF/HISTORY point to the new finding. Raw sources, commands, outputs, CUDA/LLVM/PTX and
attempt history remain under `build/nvvm-array-record-stores280`.

All37 runtime artifacts,8 qualified sources,2 configurations,576main inputs and22pins match279 before
and after research. Full279's1740 outcomes (1703correct/37unresolved/20resolved histories), native suites
and material evidence remain inherited. These new failures are outside that main corpus and remain
explicitly open. Full279/targeted233/implementation cadence0 are unchanged.

## Concepts and vocabulary

A whole-array store writes an LLVM aggregate value through a typed pointer. Nested padding separates
fields according to canonical type layout; it is not an extra field or a value the oracle may read.
The stage mask records which checks failed in one sequence. Absence of a bit cannot independently prove
an operation correct when an earlier failure may have corrupted its inputs.

## Process report

The source uses ordinary initialized integer records, bounded indices0..2 and independent source/
destination patterns. General copyable-type admission recursively accepts both records and arrays.
The emitted NVVM LLVM retains `[3 x {i16,{i16,i32}}]` stores at choose/replace/assignOut, align4.
O3 PTX loads child.first from canonical source offsets4/16/28 but emits adjacent16-bit stores at
0/12/24, placing those values at2/14/26. Generated CUDA uses ordinary `FixedArray<Cell,3>` copy
assignment; its NVRTC PTX exhibits the same incorrect offset conversion. The evidence locates the
break downstream of valid emitted code; it does not identify a particular internal optimizer pass.
The producer should not invent a different semantic representation to compensate.

Independent review strengthened the wrapper oracle before execution: prefix/tail now depend on input,
and are checked immediately after element replacement. The first run encountered warning30081 because
uint checker results were used as boolean conditions. All nine native failures and actual buffers are
retained; explicit `!=0` comparisons produce the clean seven-pass/two-mismatch run. No warning attempt
is counted as passing. Deliberately omitting the whole assignment produces the predicted failing mask8.

The original sequence's out-copy check passed despite bad emitted stores: earlier corruption and
stale destination values masked the error. A separate diagnostic initializes a fresh source and distinct
destination, copies once, and directly checks every field on both sides. Its two optimized failures
confirm the defect independently. The original nine results remain unchanged; absence of stage64 is
not reported as qualification. A separate reused-context reviewer and root independently inspected
source/oracle, output evidence and offsets; the author worked in a fresh context.

Limits: length3, runtime seed0, exhaustive low16-bit patterns only; fixed upper integer sentinels.
No arbitrary-array, substandard-array, performance or material-runtime conclusion. Next is a bounded
correction prototype that preserves layout without uncontrolled array expansion, with explicit
NVRTC and NVVM obligations. The development loop continues; Slack remains skipped.
