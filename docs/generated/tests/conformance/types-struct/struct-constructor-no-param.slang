//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:08+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/types-struct.md#constructor
//META: doc_section_digest=508a36baffb3b3fb0a209db200b9264ba975f5e186451437b5114720d356e55e
//META: purpose=A user-defined no-parameter `__init` constructor is invoked on plain `TestClass obj;` instantiation; members take the constructor-assigned values.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that declaring an object without an initializer list invokes the no-parameter `__init`.
// obj1 is declared bare; the constructor sets a=1 and b=2. Buffer output confirms those values
// are live after construction. Emission tests verify the constructor is emitted on all backends.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry computeMain -stage compute

struct TestClass
{
    int a, b;

    __init()
    {
        a = 1;
        b = 2;
    }
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1,1,1)]
void computeMain()
{
    TestClass obj1;
    outputBuffer[0] = obj1.a;
    outputBuffer[1] = obj1.b;
}

// CHECK: 1
// CHECK: 2

// HLSL: struct TestClass_{{[0-9_]*}}
// HLSL: TestClass_{{[0-9_]*}} TestClass_x24init_{{[0-9_]*}}()

// GLSL: struct TestClass_{{[0-9_]*}}
// GLSL: TestClass_{{[0-9_]*}} TestClass_x24init_{{[0-9_]*}}()

// METAL: struct TestClass_{{[0-9_]*}}
// METAL: TestClass_{{[0-9_]*}} TestClass_x24init_{{[0-9_]*}}()

// WGSL: struct TestClass_{{[0-9_]*}}
// WGSL: fn TestClass_x24init_{{[0-9_]*}}() -> TestClass_{{[0-9_]*}}

// CUDA: struct TestClass_{{[0-9_]*}}
// CUDA: TestClass_x24init_{{[0-9_]*}}()

// CPP: struct TestClass_{{[0-9_]*}}
// CPP: TestClass_x24init_{{[0-9_]*}}()
