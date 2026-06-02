//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:08+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/types-struct.md#constructor
//META: doc_section_digest=508a36baffb3b3fb0a209db200b9264ba975f5e186451437b5114720d356e55e
//META: purpose=Multiple `__init` constructors may be defined; overload resolution picks the most appropriate one based on initializer list argument count and types.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that constructor overload resolution picks the matching `__init` by argument count.
// Three instantiations use zero, one, and two-argument initializer lists. Buffer output confirms
// each object's fields reflect the values assigned by the selected overload. Emission tests
// confirm all three overloads are emitted on every backend.

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

    __init(int _a)
    {
        a = _a;
        b = 2;
    }

    __init(int _a, int _b)
    {
        a = _a;
        b = _b;
    }
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1,1,1)]
void computeMain()
{
    TestClass obj1;          // calls __init()
    TestClass obj2 = { 42 }; // calls __init(int)
    TestClass obj3 = { 42, 43 }; // calls __init(int,int)

    outputBuffer[0] = obj1.a;
    outputBuffer[1] = obj1.b;
    outputBuffer[2] = obj2.a;
    outputBuffer[3] = obj2.b;
    outputBuffer[4] = obj3.a;
    outputBuffer[5] = obj3.b;
}

// CHECK: 1
// CHECK: 2
// CHECK: 42
// CHECK: 2
// CHECK: 42
// CHECK: 43

// Three constructor overloads (x24init_0 = no-arg, x24init_1 = one-arg, x24init_2 = two-arg).
// HLSL: struct TestClass_{{[0-9_]*}}
// HLSL-DAG: TestClass_x24init_{{[0-9_]*}}()
// HLSL-DAG: TestClass_x24init_{{[0-9_]*}}(int
// HLSL-DAG: TestClass_x24init_{{[0-9_]*}}(int {{.*}}, int

// GLSL: struct TestClass_{{[0-9_]*}}
// GLSL-DAG: TestClass_x24init_{{[0-9_]*}}()
// GLSL-DAG: TestClass_x24init_{{[0-9_]*}}(int
// GLSL-DAG: TestClass_x24init_{{[0-9_]*}}(int {{.*}}, int

// METAL: struct TestClass_{{[0-9_]*}}
// METAL-DAG: TestClass_x24init_{{[0-9_]*}}()
// METAL-DAG: TestClass_x24init_{{[0-9_]*}}(int
// METAL-DAG: TestClass_x24init_{{[0-9_]*}}(int {{.*}}, int

// WGSL: struct TestClass_{{[0-9_]*}}
// WGSL-DAG: fn TestClass_x24init_{{[0-9_]*}}() ->
// WGSL-DAG: fn TestClass_x24init_{{[0-9_]*}}( {{.*}} : i32) ->
// WGSL-DAG: fn TestClass_x24init_{{[0-9_]*}}( {{.*}} : i32,

// CUDA: struct TestClass_{{[0-9_]*}}
// CUDA-DAG: TestClass_x24init_{{[0-9_]*}}()
// CUDA-DAG: TestClass_x24init_{{[0-9_]*}}(int
// CUDA-DAG: TestClass_x24init_{{[0-9_]*}}(int {{.*}}, int

// CPP: struct TestClass_{{[0-9_]*}}
// CPP-DAG: TestClass_x24init_{{[0-9_]*}}()
// CPP-DAG: TestClass_x24init_{{[0-9_]*}}(int32_t
// CPP-DAG: TestClass_x24init_{{[0-9_]*}}(int32_t {{.*}}, int32_t
