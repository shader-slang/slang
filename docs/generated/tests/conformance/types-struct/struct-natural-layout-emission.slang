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

// Verifies that non-static fields appear in declaration order in emitted HLSL, reflecting
// natural layout. Mixed has int x, float y, int z; CHECK asserts all three field names appear
// in the emitted struct in that order.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry main -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry main -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry main -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry main -stage compute

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

// GLSL emits struct Mixed with int x, float y, int z in declaration order.
// GLSL: struct Mixed_{{[0-9]+}}
// GLSL: x_{{[0-9]+}}
// GLSL: y_{{[0-9]+}}
// GLSL: z_{{[0-9]+}}

// Metal emits struct Mixed with all three fields in order.
// METAL: struct Mixed_{{[0-9]+}}
// METAL: x_{{[0-9]+}}
// METAL: y_{{[0-9]+}}
// METAL: z_{{[0-9]+}}

// WGSL emits struct Mixed with all three fields.
// WGSL: struct Mixed_{{[0-9]+}}
// WGSL: x_{{[0-9]+}}
// WGSL: y_{{[0-9]+}}
// WGSL: z_{{[0-9]+}}

// CUDA emits struct Mixed with int x, float y, int z.
// CUDA: struct Mixed_{{[0-9]+}}
// CUDA: x_{{[0-9]+}}
// CUDA: y_{{[0-9]+}}
// CUDA: z_{{[0-9]+}}

// CPP emits struct Mixed with all three fields.
// CPP: struct Mixed_{{[0-9]+}}
// CPP: x_{{[0-9]+}}
// CPP: y_{{[0-9]+}}
// CPP: z_{{[0-9]+}}
