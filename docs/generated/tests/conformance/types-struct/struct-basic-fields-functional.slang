//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:08+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/types-struct.md#description
//META: doc_section_digest=0b091f4f0ba7cb2a07817dc55025aaaaadee5292438bfc88fba1ab02ee5d3878
//META: purpose=A struct declared with non-static fields allocates one copy of each field per instance; fields are read and written via the dot operator.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A struct with two int fields. Each instance owns separate storage for both fields.
// Reading and writing obj.x and obj.y exercises basic field access via the dot operator.
// Buffer output confirms the stored values; emission tests confirm the struct is preserved
// on all text-emit backends.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry computeMain -stage compute

struct Point
{
    int x;
    int y;
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1,1,1)]
void computeMain()
{
    Point p;
    p.x = 3;
    p.y = 7;
    outputBuffer[0] = p.x;
    outputBuffer[1] = p.y;
}

// CHECK: 3
// CHECK: 7

// HLSL: struct Point_{{[0-9_]*}}
// HLSL: int x_{{[0-9_]*}}
// HLSL: int y_{{[0-9_]*}}

// GLSL: struct Point_{{[0-9_]*}}
// GLSL: int x_{{[0-9_]*}}
// GLSL: int y_{{[0-9_]*}}

// METAL: struct Point_{{[0-9_]*}}
// METAL: int x_{{[0-9_]*}}
// METAL: int y_{{[0-9_]*}}

// WGSL: struct Point_{{[0-9_]*}}
// WGSL: x_{{[0-9_]*}}
// WGSL: y_{{[0-9_]*}}

// CUDA: struct Point_{{[0-9_]*}}
// CUDA: int x_{{[0-9_]*}}
// CUDA: int y_{{[0-9_]*}}

// CPP: struct Point_{{[0-9_]*}}
// CPP: x_{{[0-9_]*}}
// CPP: y_{{[0-9_]*}}
