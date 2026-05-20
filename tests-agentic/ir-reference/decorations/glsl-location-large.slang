//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00Z
//META: source_commit=60db9042207670b39b0524118887efacb7a3d42d
//META: doc_ref=docs/llm-generated/ir-reference/decorations.md#layout-and-binding
//META: doc_section_digest=53c33072d342ae9430710df507cdde301b5918b46f7119412b00900374c8e222
//META: purpose=Boundary - a large vk::location index (63) flows through into glslLocation(63:Int) unchanged.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary - 63 is at the documented Vulkan upper edge for input
// locations. Confirm the integer survives lowering into the
// glslLocation decoration's IRIntLit operand without clamping.

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
