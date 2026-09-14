//META: generated=true
//META: model=claude-opus-4-8[1m]
//META: generated_at=2026-06-11T17:00:00Z
//META: source_commit=ef1068b5485e09b3a7afadba2e25f9541e29af42
//META: doc_ref=docs/generated/design/pipeline/04c-layout-ir.md#global-scope-type-layout
//META: doc_section_digest=e885f1a92beacc0053db90ae24923135a05a098bfc3c282c2580a971645fcec9
//META: purpose=Verifies a float3/float2/float struct lays out at offsets 0/16/24 under std140 vector power-of-two alignment but 0/12/20 under scalar layout natural packing.
//META: intent=characterization
//META: covers=source/slang/slang-type-layout.cpp
//META: pipeline_stage=layout
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Pins the vector-alignment rule in GLSLBaseLayoutRulesImpl::GetVectorLayout
// (std140 rounds a vector's alignment up to the next power of two of its
// size) against the scalar layout path selected by -force-glsl-scalar-layout
// (natural packing, no power-of-two bump). For `S { float3 a; float2 b;
// float c }`: under std140 the `float3` forces 16-byte alignment so `b`
// lands at 16 and `c` at 24; under scalar layout `b` packs at 12 and `c` at
// 20. The emitted type name flips from `S_std140` to `S_natural`. Verbatim
// from spirv-asm emit on both paths.
//TEST:SIMPLE(filecheck=STD140):-target spirv-asm -entry main -stage compute
//TEST:SIMPLE(filecheck=SCALAR):-target spirv-asm -entry main -stage compute -force-glsl-scalar-layout

struct S { float3 a; float2 b; float tail; }
ConstantBuffer<S> cb;
RWStructuredBuffer<float> outp;
[numthreads(1, 1, 1)]
void main()
{
    outp[0] = cb.a.x + cb.b.x + cb.tail;
}

// std140: float3 forces 16-byte alignment; float2 at 16, tail at 24.
//STD140-DAG: OpMemberDecorate %S_std140 0 Offset 0
//STD140-DAG: OpMemberDecorate %S_std140 1 Offset 16
//STD140-DAG: OpMemberDecorate %S_std140 2 Offset 24
// scalar layout: natural packing; float2 at 12, tail at 20.
//SCALAR-DAG: OpMemberDecorate %S_natural 0 Offset 0
//SCALAR-DAG: OpMemberDecorate %S_natural 1 Offset 12
//SCALAR-DAG: OpMemberDecorate %S_natural 2 Offset 20
