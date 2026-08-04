//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00Z
//META: source_commit=60db9042207670b39b0524118887efacb7a3d42d
//META: doc_ref=docs/generated/design/ir-reference/decorations.md#layout-and-binding
//META: doc_section_digest=53c33072d342ae9430710df507cdde301b5918b46f7119412b00900374c8e222
//META: purpose=Boundary - vk::location(0) at the lower edge produces glslLocation(0:Int).
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary - the doc records glslLocation with operand
// `location: IRIntLit`. 0 is the lower edge; assert it is encoded
// exactly as `glslLocation(0 : Int)` rather than dropped or
// reinterpreted.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<float> buf;

struct V
{
    [vk::location(0)] float u : TEXCOORD0;
};

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID, V v)
{
    buf[0] = v.u + float(tid.x);
}

// CHECK: [glslLocation(0 : Int)]
