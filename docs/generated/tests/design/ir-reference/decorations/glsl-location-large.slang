//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/decorations.md#layout-and-binding
//META: doc_section_digest=928ba5f8c333b9ec8b6e55397dc87060a4d672e8f7a0cc913acb7f02d7fea184
//META: purpose=Boundary - a large vk::location index (63) flows through into glslLocation(63 : Int) unchanged.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the upper end of the location range is not clamped or
// reinterpreted on the way into the decoration. 63 is the largest location
// index a Vulkan implementation is required to accept; the CHECK pins the
// exact integer operand.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<float> buf;

struct V
{
    [vk::location(63)] float u : TEXCOORD0;
};

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID, V v)
{
    buf[0] = v.u + float(tid.x);
}

// CHECK: [glslLocation(63 : Int)]
