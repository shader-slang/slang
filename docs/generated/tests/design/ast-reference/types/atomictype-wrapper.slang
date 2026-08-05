//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/types.md#nodes
//META: doc_section_digest=c7f55cb42aaa9e03df513fb2e3ff5bbcf380f2c09432d750a2aa9c55dfc56801
//META: purpose=An AtomicType wraps an element type; an `Atomic<int>` buffer element performs atomic updates rather than a plain read-modify-write.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the atomic wrapper type changes how its element is
// updated: `buf[0].add(3)` on an Atomic<int> element must lower to the
// backend's atomic add rather than a load/add/store triple. CHECK
// matches InterlockedAdd in the emitted HLSL, which a plain int element
// would not produce.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry computeMain -stage compute

RWStructuredBuffer<Atomic<int> > buf;

//CHECK: InterlockedAdd
[numthreads(1, 1, 1)]
void computeMain()
{
    buf[0].add(3);
}
