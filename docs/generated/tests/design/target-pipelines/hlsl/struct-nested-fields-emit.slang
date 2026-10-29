//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#phase-d-hlsl-emit-and-downstream-tools
//META: doc_section_digest=c7c97cf6f5a8dc2bac27a37c7292d84bc70e3e1764ddec58018711cf9fd606ce
//META: purpose=Expansion: a struct that contains another user struct as a field is emitted with both struct declarations preserved on HLSL.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// HLSL's shared C-like emitter walks user-struct definitions
// transitively. A struct containing another user struct must
// emit both `struct` declarations.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

struct Inner {
    float a;
    int b;
};

struct Outer {
    Inner i;
    int tag;
};

RWStructuredBuffer<Outer> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    Outer o;
    o.i.a = float(tid.x);
    o.i.b = int(tid.x);
    o.tag = int(tid.x) + 100;
    outBuf[tid.x] = o;
}

// CHECK-DAG: struct Inner_{{[0-9]+}}
// CHECK-DAG: struct Outer_{{[0-9]+}}
