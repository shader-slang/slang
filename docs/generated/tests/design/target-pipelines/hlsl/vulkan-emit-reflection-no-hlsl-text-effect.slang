//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#option-set-toggles
//META: doc_section_digest=65ae1152b296463b8293b7591a32f87fe87ce08c5528a74ea4488483c5c3c5b7
//META: purpose=`-fspv-reflect` turns on the pass that adds user-type-name decorations even on the HLSL pipeline, but those decorations are read only by the SPIR-V emitter, so the emitted HLSL text is unchanged and carries no user-type marker.
//META: intent=negative
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies a documented no-op: the option is accepted on an HLSL
// compile and reaches its pass, yet leaves nothing behind in the
// text. The test compiles a named user struct through a constant
// buffer — the shape whose user type name the decoration would
// record — with the option on, then asserts the ordinary emission
// is intact and that no `UserType` marker (the spelling the SPIR-V
// backend emits from the same decoration) appears anywhere.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute -fspv-reflect

struct MyStruct
{
    float4 a;
};
ConstantBuffer<MyStruct> gCB;
RWStructuredBuffer<float4> outBuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = gCB.a;
}

// CHECK-NOT: UserType
// CHECK: struct MyStruct_{{[0-9]+}}
// CHECK: cbuffer gCB_{{[0-9]+}} : register(b0)
// CHECK-NOT: UserType
