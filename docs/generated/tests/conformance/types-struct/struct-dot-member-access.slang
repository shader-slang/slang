//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:08+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/types-struct.md#accessing-members-and-nested-types
//META: doc_section_digest=2755043638ff4d80438587ffde5a02e52e4e6ff81f851f26621452c5c5afad95
//META: purpose=Static and non-static struct members and nested types are accessed using the dot `.` operator; `::` is deprecated for static member access.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `.` accesses non-static fields, static fields, nested types, and member functions.
// Four buffer slots confirm instance field, type-qualified static, chained-dot nested static, and
// method return all resolve correctly. Emission tests confirm the struct and method survive on
// all text-emit backends.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry computeMain -stage compute

struct TestStruct
{
    int a;
    static const int b = 5;

    struct NestedStruct
    {
        static const int c = 6;
    }

    int getA()
    {
        return this.a;
    }
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1,1,1)]
void computeMain()
{
    TestStruct obj = { };
    obj.a = 42;

    // Non-static field via dot
    outputBuffer[0] = obj.a;
    // Static const via type name dot
    outputBuffer[1] = TestStruct.b;
    // Nested static via chained dots
    outputBuffer[2] = TestStruct.NestedStruct.c;
    // Member function via dot
    outputBuffer[3] = obj.getA();
}

// CHECK: 42
// CHECK: 5
// CHECK: 6
// CHECK: 42

// HLSL: struct TestStruct_{{[0-9_]*}}
// HLSL: int a_{{[0-9_]*}}
// HLSL: TestStruct_getA_{{[0-9_]*}}

// GLSL: struct TestStruct_{{[0-9_]*}}
// GLSL: int a_{{[0-9_]*}}
// GLSL: TestStruct_getA_{{[0-9_]*}}

// METAL: struct TestStruct_{{[0-9_]*}}
// METAL: int a_{{[0-9_]*}}
// METAL: TestStruct_getA_{{[0-9_]*}}

// WGSL: struct TestStruct_{{[0-9_]*}}
// WGSL: a_{{[0-9_]*}}
// WGSL: fn TestStruct_getA_{{[0-9_]*}}

// CUDA: struct TestStruct_{{[0-9_]*}}
// CUDA: int a_{{[0-9_]*}}
// CUDA: TestStruct_getA_{{[0-9_]*}}

// CPP: struct TestStruct_{{[0-9_]*}}
// CPP: a_{{[0-9_]*}}
// CPP: TestStruct_getA_{{[0-9_]*}}
