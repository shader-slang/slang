//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:08+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/types-struct.md#nonstatic-member-function
//META: doc_section_digest=c90f11708cb380fb674a4708aaf5743e5a1f31c69b6cf4d7b4a599ffc35791bb
//META: purpose=A member function declared with `[mutating]` may write to non-static data members; without it, write access is disallowed.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `[mutating]` grants write access to non-static fields inside a member function.
// incrementAndReturnA() adds 1 to `a` and returns the result; buffer output asserts both the
// return value and the field itself reflect the mutation. Emission tests confirm the mutating
// method is emitted with the correct inout/pointer qualifier on all backends.

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

    [mutating] int incrementAndReturnA()
    {
        a = a + 1;
        return a;
    }
}

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0], stride=4)
RWStructuredBuffer<int> outputBuffer;

[numthreads(1,1,1)]
void computeMain()
{
    TestStruct obj;
    obj.a = 10;
    int result = obj.incrementAndReturnA();
    outputBuffer[0] = result;
    outputBuffer[1] = obj.a;
}

// CHECK: 11
// CHECK: 11

// HLSL emits mutating method with inout qualifier.
// HLSL: struct TestStruct_{{[0-9_]*}}
// HLSL: int TestStruct_incrementAndReturnA_{{[0-9_]*}}(inout TestStruct_{{[0-9_]*}} this_{{[0-9_]*}})

// GLSL emits mutating method with inout qualifier.
// GLSL: struct TestStruct_{{[0-9_]*}}
// GLSL: int TestStruct_incrementAndReturnA_{{[0-9_]*}}(inout TestStruct_{{[0-9_]*}} this_{{[0-9_]*}})

// Metal emits mutating method with pointer qualifier.
// METAL: struct TestStruct_{{[0-9_]*}}
// METAL: int TestStruct_incrementAndReturnA_{{[0-9_]*}}(TestStruct_{{[0-9_]*}} thread* this_{{[0-9_]*}})

// WGSL emits mutating method with ptr<function,...> qualifier.
// WGSL: struct TestStruct_{{[0-9_]*}}
// WGSL: fn TestStruct_incrementAndReturnA_{{[0-9_]*}}( this_{{[0-9_]*}} : ptr<function, TestStruct_{{[0-9_]*}}>) -> i32

// CUDA: struct TestStruct_{{[0-9_]*}}
// CUDA: TestStruct_incrementAndReturnA_{{[0-9_]*}}(TestStruct_{{[0-9_]*}} * this_{{[0-9_]*}})

// CPP: struct TestStruct_{{[0-9_]*}}
// CPP: TestStruct_incrementAndReturnA_{{[0-9_]*}}(TestStruct_{{[0-9_]*}} * this_{{[0-9_]*}})
