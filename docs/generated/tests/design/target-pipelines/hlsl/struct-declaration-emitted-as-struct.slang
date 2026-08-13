//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#phase-d-hlsl-emit-and-downstream-tools
//META: doc_section_digest=4e5f7ab56dedeffe7b1e374800617516f1d9e429f63231ad5fa4dd15419d06d1
//META: purpose=A user-declared struct keeps its `struct Name { ... };` shape on HLSL emit (CLikeSourceEmitter convention).
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// HLSL is a C-like target so the shared CLikeSourceEmitter base
// class emits `struct Data_<N> { ... };` for user structs.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

struct Data { float a; int b; };
RWStructuredBuffer<Data> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    Data d;
    d.a = float(tid.x);
    d.b = int(tid.x);
    outBuf[tid.x] = d;
}

// CHECK: struct Data_{{[0-9]+}}
// CHECK: float a_{{[0-9]+}};
// CHECK: int b_{{[0-9]+}};
// CHECK: RWStructuredBuffer<Data_{{[0-9]+}}{{.*}}> outBuf_{{[0-9]+}}
