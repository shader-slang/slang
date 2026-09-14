//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/resources-and-atomics.md#atomic-operations
//META: doc_section_digest=e0b71e7cc30a0e069a5833b7fdc7038f02b6ab982bbba6656983e3a65dcba7ce
//META: purpose=`Atomic<float>::and(v)` is rejected as a missing member — `'and' is not a member of 'Atomic<float>'` — rather than as a bad argument type, because the bitwise atomics live in an extension constrained to `IBitAtomicable`.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the interface split the atomics section describes: `and`,
// `or`, `xor`, `increment`, and `decrement` live behind an
// `IBitAtomicable` constraint that `float` does not satisfy, so name
// lookup never finds the member at all. The caret-anchored CHECKs span
// the member name and pin both the short "member not found" form and
// its long form; that is what distinguishes this from an
// argument-conversion failure, which would instead report a mismatched
// type on the `1.0` argument.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -stage compute -entry main

RWStructuredBuffer<Atomic<float>> abuf;
uniform RWStructuredBuffer<float> rwbuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int idx = (int)tid.x;
    rwbuf[idx] = abuf[idx].and(1.0);
//CHECK:                   ^^^ member not found
//CHECK:                   ^^^ 'and' is not a member of 'Atomic<float>'.
}
