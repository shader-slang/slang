//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:08+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/types-struct.md#description
//META: doc_section_digest=0b091f4f0ba7cb2a07817dc55025aaaaadee5292438bfc88fba1ab02ee5d3878
//META: purpose=When a constructor is present, a non-`const` field's default initializer is overridden by the constructor's explicit assignment.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a constructor's explicit assignment overrides a field's default initializer.
// WithDefault.x has default 99, but __init(int) overwrites it; buffer output confirms the
// constructed value is 42, not 99. Emission tests verify the constructor is emitted everywhere.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry computeMain -stage compute

struct WithDefault
{
    int x = 99;

    __init(int v)
    {
        x = v;  // overrides the default initializer
    }
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1,1,1)]
void computeMain()
{
    WithDefault obj = { 42 };
    outputBuffer[0] = obj.x;
}

// CHECK: 42

// HLSL: struct WithDefault_{{[0-9_]*}}
// HLSL: WithDefault_{{[0-9_]*}} WithDefault_x24init_{{[0-9_]*}}(int

// GLSL: struct WithDefault_{{[0-9_]*}}
// GLSL: WithDefault_{{[0-9_]*}} WithDefault_x24init_{{[0-9_]*}}(int

// METAL: struct WithDefault_{{[0-9_]*}}
// METAL: WithDefault_{{[0-9_]*}} WithDefault_x24init_{{[0-9_]*}}(int

// WGSL: struct WithDefault_{{[0-9_]*}}
// WGSL: fn WithDefault_x24init_{{[0-9_]*}}( {{.*}} : i32) -> WithDefault_{{[0-9_]*}}

// CUDA: struct WithDefault_{{[0-9_]*}}
// CUDA: WithDefault_x24init_{{[0-9_]*}}(int

// CPP: struct WithDefault_{{[0-9_]*}}
// CPP: WithDefault_x24init_{{[0-9_]*}}(int
