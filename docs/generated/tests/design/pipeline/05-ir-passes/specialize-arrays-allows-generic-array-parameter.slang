//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#specialization-and-generics
//META: doc_section_digest=4af16ce22c9a66bc00390c870ab8fa13bdbb68c7437a11e64d86f8f9c02cdbe4
//META: purpose=The specialize driver resolves a `let N : int` array-size generic parameter so the emit reflects the concrete N and no generic shape survives.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that generic-parameter substitution resolves an array-size
// generic. `sumN<let N : int>` is called with N = 4; the CHECK asserts
// the entry point is emitted and no `sumN` symbol survives, which only
// holds once the generic was specialized to a concrete N and inlined.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry main

uniform RWStructuredBuffer<int> buf;

[ForceInline]
int sumN<let N : int>(int data[N])
{
    int s = 0;
    [ForceUnroll] for (int i = 0; i < N; ++i) s += data[i];
    return s;
}

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int arr[4] = { int(tid.x), int(tid.y), int(tid.z), 0 };
    buf[0] = sumN<4>(arr);
}

// HLSL: void main
// HLSL-NOT: sumN
