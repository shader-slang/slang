//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:08+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/types-struct.md#constructor
//META: doc_section_digest=508a36baffb3b3fb0a209db200b9264ba975f5e186451437b5114720d356e55e
//META: purpose=Without a user-provided constructor, aggregate initialization fills non-static fields in order from the initializer list; extra unspecified fields are default-initialized.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies aggregate initialization fills fields in declaration order and default-initializes
// fields omitted from the list. p1 uses a full three-element list; p2 uses a two-element list
// leaving z unspecified. Buffer output confirms the assigned values; emission tests confirm
// the struct is preserved across all text-emit backends.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry computeMain -stage compute

struct Plain
{
    int x;
    int y;
    int z;
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1,1,1)]
void computeMain()
{
    Plain p1 = { 10, 20, 30 };
    Plain p2 = { 5, 6 };
    outputBuffer[0] = p1.x;
    outputBuffer[1] = p1.y;
    outputBuffer[2] = p1.z;
    outputBuffer[3] = p2.x;
    outputBuffer[4] = p2.y;
    outputBuffer[5] = p2.z; // default-initialized to 0
}

// CHECK: 10
// CHECK: 20
// CHECK: 30
// CHECK: 5
// CHECK: 6
// CHECK: 0

// HLSL: struct Plain_{{[0-9_]*}}
// HLSL: int x_{{[0-9_]*}}
// HLSL: int y_{{[0-9_]*}}
// HLSL: int z_{{[0-9_]*}}

// GLSL: struct Plain_{{[0-9_]*}}
// GLSL: int x_{{[0-9_]*}}
// GLSL: int y_{{[0-9_]*}}
// GLSL: int z_{{[0-9_]*}}

// METAL: struct Plain_{{[0-9_]*}}
// METAL: int x_{{[0-9_]*}}
// METAL: int y_{{[0-9_]*}}
// METAL: int z_{{[0-9_]*}}

// WGSL: struct Plain_{{[0-9_]*}}
// WGSL: x_{{[0-9_]*}}
// WGSL: y_{{[0-9_]*}}
// WGSL: z_{{[0-9_]*}}

// CUDA: struct Plain_{{[0-9_]*}}
// CUDA: int x_{{[0-9_]*}}
// CUDA: int y_{{[0-9_]*}}
// CUDA: int z_{{[0-9_]*}}

// CPP: struct Plain_{{[0-9_]*}}
// CPP: x_{{[0-9_]*}}
// CPP: y_{{[0-9_]*}}
// CPP: z_{{[0-9_]*}}
