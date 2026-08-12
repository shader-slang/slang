//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:08+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/types-struct.md#d3d-constant-buffer-layout
//META: doc_section_digest=ebd30924bf777b67b6ac3c520785d923dfbd7247c96b9f0e8010016332107ae9
//META: purpose=Inside a struct body, `This` refers to the enclosing struct type itself and can be used in method return types and local variable declarations.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `This` inside a struct body resolves to the enclosing struct type.
// withValue() declares its return type and local variable as `This`; the buffer output
// confirms the returned instance carries the expected value. Emission tests confirm
// the struct is preserved with the member function on all text-emit backends.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry computeMain -stage compute

struct MyStruct
{
    int value;

    // `This` used as return type -- returns a new instance
    This withValue(int v)
    {
        This result;
        result.value = v;
        return result;
    }
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1,1,1)]
void computeMain()
{
    MyStruct s;
    s.value = 0;
    MyStruct s2 = s.withValue(99);
    outputBuffer[0] = s2.value;
}

// CHECK: 99

// HLSL: struct MyStruct_{{[0-9_]*}}
// HLSL: int value_{{[0-9_]*}}
// HLSL: MyStruct_{{[0-9_]*}} MyStruct_withValue_{{[0-9_]*}}

// GLSL: struct MyStruct_{{[0-9_]*}}
// GLSL: int value_{{[0-9_]*}}
// GLSL: MyStruct_{{[0-9_]*}} MyStruct_withValue_{{[0-9_]*}}

// METAL: struct MyStruct_{{[0-9_]*}}
// METAL: int value_{{[0-9_]*}}
// METAL: MyStruct_{{[0-9_]*}} MyStruct_withValue_{{[0-9_]*}}

// WGSL: struct MyStruct_{{[0-9_]*}}
// WGSL: value_{{[0-9_]*}}
// WGSL: fn MyStruct_withValue_{{[0-9_]*}}

// CUDA: struct MyStruct_{{[0-9_]*}}
// CUDA: int value_{{[0-9_]*}}
// CUDA: MyStruct_withValue_{{[0-9_]*}}

// CPP: struct MyStruct_{{[0-9_]*}}
// CPP: value_{{[0-9_]*}}
// CPP: MyStruct_withValue_{{[0-9_]*}}
