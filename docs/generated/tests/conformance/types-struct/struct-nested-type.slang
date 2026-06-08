//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:08+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/types-struct.md#description
//META: doc_section_digest=0b091f4f0ba7cb2a07817dc55025aaaaadee5292438bfc88fba1ab02ee5d3878
//META: purpose=A nested type declared inside a struct body is enclosed within the struct's name scope and accessed via the outer struct name with the dot operator.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a type nested inside a struct is accessed by chaining the outer struct name
// with the dot operator. TestStruct.NestedStruct is used as a return type of a helper function
// to prevent constant folding. Buffer output confirms d=55; emission tests confirm the nested
// struct name (mangled as TestStruct_NestedStruct) is preserved on all backends.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry computeMain -stage compute

struct TestStruct
{
    struct NestedStruct
    {
        int d;
    }
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<int> outputBuffer;

TestStruct.NestedStruct makeNested(int v)
{
    TestStruct.NestedStruct n;
    n.d = v;
    return n;
}

[numthreads(1,1,1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    // Non-constant input prevents constant folding
    TestStruct.NestedStruct obj = makeNested((int)tid.x + 55);
    outputBuffer[0] = obj.d;
}

// CHECK: 55

// Nested struct emitted with flattened name TestStruct_NestedStruct.
// HLSL: struct TestStruct_NestedStruct_{{[0-9_]*}}
// HLSL: int d_{{[0-9_]*}}

// GLSL: struct TestStruct_NestedStruct_{{[0-9_]*}}
// GLSL: int d_{{[0-9_]*}}

// METAL: struct TestStruct_NestedStruct_{{[0-9_]*}}
// METAL: int d_{{[0-9_]*}}

// WGSL: struct TestStruct_NestedStruct_{{[0-9_]*}}
// WGSL: d_{{[0-9_]*}}

// CUDA: struct TestStruct_NestedStruct_{{[0-9_]*}}
// CUDA: int d_{{[0-9_]*}}

// CPP: struct TestStruct_NestedStruct_{{[0-9_]*}}
// CPP: d_{{[0-9_]*}}
