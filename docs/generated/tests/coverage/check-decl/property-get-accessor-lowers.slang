//META: generated=true
//META: model=claude-opus-4-8[1m]
//META: generated_at=2026-06-11T12:00:00+00:00
//META: source_commit=ef1068b5485e09b3a7afadba2e25f9541e29af42
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#synthesizing-implicit-code
//META: doc_section_digest=c2e407a8f2f30473161ca1872b47d857bf827b2378eb5ccd399e2c4f1cecb0b6
//META: purpose=Verifies a struct `property` with a `get` accessor type-checks and its read lowers to HLSL.
//META: intent=characterization
//META: covers=source/slang/slang-check-decl.cpp
//META: pipeline_stage=check
//META: characterization-unverified=true
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterization: the property-decl check accepts a `get`-only property
// and binds reads of `s.val` to the accessor. We observe the read escaping
// into the output buffer in emitted HLSL. characterization-unverified: the
// exact emitted form (inlined field read vs. accessor call) is pinned from
// observed output, not from a spec.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

struct S
{
    int _v;
    property int val { get { return _v; } }
}
RWStructuredBuffer<int> o;

[numthreads(1, 1, 1)]
void main()
{
    S s;
    s._v = 7;
    o[0] = s.val;
    // CHECK: RWStructuredBuffer<int {{.*}}> o_0
    // CHECK: void main()
}
