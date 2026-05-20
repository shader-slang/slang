//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00Z
//META: source_commit=60db9042207670b39b0524118887efacb7a3d42d
//META: doc_ref=docs/llm-generated/ir-reference/decorations.md#layout-and-binding
//META: doc_section_digest=53c33072d342ae9430710df507cdde301b5918b46f7119412b00900374c8e222
//META: purpose=Boundary - a `: packoffset(c2.y)` modifier lowers to a packoffset decoration with (registerOffset, componentOffset) = (2, 1).
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary - the doc records packoffset with operands
// (registerOffset, componentOffset). Component letters x/y/z/w
// encode to 0/1/2/3; assert the dotted form `c2.y` decomposes
// into separate register and component integers in the dump.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

cbuffer MyCB
{
    float4 a : packoffset(c0);
    float4 b : packoffset(c2.y);
};

uniform RWStructuredBuffer<float> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = a.x + b.y + float(tid.x);
}

// CHECK-DAG: [packoffset(0 : Int, 0 : Int)]
// CHECK-DAG: [packoffset(2 : Int, 1 : Int)]
