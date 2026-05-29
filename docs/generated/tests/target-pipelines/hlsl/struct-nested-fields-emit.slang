//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#phase-d-hlsl-emit-and-downstream-tools
//META: doc_section_digest=03d6fe7b8f4502160d35cd7e0f89a3145a7dfe7a6680c44cb1092265f9fe4de7
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
