//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:08+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/types-struct.md#description
//META: doc_section_digest=0b091f4f0ba7cb2a07817dc55025aaaaadee5292438bfc88fba1ab02ee5d3878
//META: purpose=An anonymous struct (no identifier) is assigned a unique compiler-generated name; the variable declared with it has its fields accessible via dot notation.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that an anonymous struct declared inline as a variable type compiles and its fields
// are accessible via dot notation. Buffer output confirms both values; emission tests confirm
// the anonymous struct is emitted with a generated name on all text-emit backends.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry computeMain -stage compute

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1,1,1)]
void computeMain()
{
    // Anonymous struct inline variable declaration
    struct { int a; int b; } obj;
    obj.a = 42;
    obj.b = 15;
    outputBuffer[0] = obj.a;
    outputBuffer[1] = obj.b;
}

// CHECK: 42
// CHECK: 15

// HLSL: struct SLANG_anonymous
// HLSL: int a_{{[0-9_]*}}
// HLSL: int b_{{[0-9_]*}}

// GLSL: struct SLANG_anonymous
// GLSL: int a_{{[0-9_]*}}
// GLSL: int b_{{[0-9_]*}}

// METAL: struct SLANG_anonymous
// METAL: int a_{{[0-9_]*}}
// METAL: int b_{{[0-9_]*}}

// WGSL: struct SLANG_anonymous
// WGSL: a_{{[0-9_]*}}
// WGSL: b_{{[0-9_]*}}

// CUDA: struct SLANG_anonymous
// CUDA: int a_{{[0-9_]*}}
// CUDA: int b_{{[0-9_]*}}

// CPP: struct SLANG_anonymous
// CPP: a_{{[0-9_]*}}
// CPP: b_{{[0-9_]*}}
