//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:08+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/types-struct.md#description
//META: doc_section_digest=0b091f4f0ba7cb2a07817dc55025aaaaadee5292438bfc88fba1ab02ee5d3878
//META: purpose=When an initializer list does not provide a value for a non-static field that has a default initializer, the default is used.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a field's default initializer applies when the initializer list stops before that field.
// Defaults has x=99, y (none), z=77; initializing with {1,2} supplies x and y but not z.
// Buffer output confirms z retains its default value of 77. Emission tests confirm the struct
// and its initializer are emitted on all backends.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry computeMain -stage compute

struct Defaults
{
    int x = 99;
    int y;
    int z = 77;
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1,1,1)]
void computeMain()
{
    // Initializer list provides x and y but not z; z should fall back to 77
    Defaults d = { 1, 2 };
    outputBuffer[0] = d.x;
    outputBuffer[1] = d.y;
    outputBuffer[2] = d.z;
}

// CHECK: 1
// CHECK: 2
// CHECK: 77

// HLSL: struct Defaults_{{[0-9_]*}}
// HLSL: int x_{{[0-9_]*}}
// HLSL: int y_{{[0-9_]*}}
// HLSL: int z_{{[0-9_]*}}

// GLSL: struct Defaults_{{[0-9_]*}}
// GLSL: int x_{{[0-9_]*}}
// GLSL: int y_{{[0-9_]*}}
// GLSL: int z_{{[0-9_]*}}

// METAL: struct Defaults_{{[0-9_]*}}
// METAL: int x_{{[0-9_]*}}
// METAL: int y_{{[0-9_]*}}
// METAL: int z_{{[0-9_]*}}

// WGSL: struct Defaults_{{[0-9_]*}}
// WGSL: x_{{[0-9_]*}}
// WGSL: y_{{[0-9_]*}}
// WGSL: z_{{[0-9_]*}}

// CUDA: struct Defaults_{{[0-9_]*}}
// CUDA: int x_{{[0-9_]*}}
// CUDA: int y_{{[0-9_]*}}
// CUDA: int z_{{[0-9_]*}}

// CPP: struct Defaults_{{[0-9_]*}}
// CPP: x_{{[0-9_]*}}
// CPP: y_{{[0-9_]*}}
// CPP: z_{{[0-9_]*}}
