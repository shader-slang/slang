//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:08+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/types-struct.md#natural-layout
//META: doc_section_digest=380aa59bd382916defff5c41e4ce069cc263f0377fbd8a35da604e427f5858d7
//META: purpose=A struct with mixed int/float fields emits with those fields in declaration order (non-static members allocated sequentially in struct).
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says natural layout: "The data is laid out in order of: Parent types, Non-static data members."
// "Offset of the first data item is 0; offset of the Nth item is offset+size of previous item
// rounded up to the alignment of the item."
// We verify the field ordering is preserved in emitted HLSL.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

struct Mixed
{
    int x;
    float y;
    int z;
}

RWStructuredBuffer<int> output;

[numthreads(1,1,1)]
void main()
{
    Mixed m;
    m.x = 1;
    m.y = 2.0f;
    m.z = 3;
    output[0] = m.x + int(m.y) + m.z;
}

// CHECK: struct Mixed
// CHECK: int x
// CHECK: float y
// CHECK: int z
