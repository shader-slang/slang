//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:08+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/types-struct.md#accessing-members-and-nested-types
//META: doc_section_digest=2755043638ff4d80438587ffde5a02e52e4e6ff81f851f26621452c5c5afad95
//META: purpose=The doc's `TestStruct` example with static/non-static members, static/non-static functions, and nested struct emits valid HLSL.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// This exercises the full `TestStruct` example from the doc's "Accessing Members and Nested Types"
// section. The struct has a field, static field, static const field, nested struct, non-static
// member function (read-only and mutating), and static member functions.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -entry main -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry main -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry main -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry main -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry main -stage compute

struct TestStruct
{
    int a;
    static int b = 5;
    static const int c = 6;

    struct NestedStruct
    {
        static int c = 6;
        int d;
    }

    int getA()
    {
        return this.a;
    }

    [mutating] int incrementAndReturnA()
    {
        a = a + 1;
        return a;
    }

    static int getB()
    {
        return b;
    }

    static int incrementAndReturnB()
    {
        b = b + 1;
        return b;
    }
}

RWStructuredBuffer<int> output;

[numthreads(1,1,1)]
void main()
{
    TestStruct obj = { };
    obj.a = 42;
    output[0] = obj.getA();
    output[1] = obj.incrementAndReturnA();
    output[2] = TestStruct.getB();
    output[3] = TestStruct.incrementAndReturnB();
}

// CHECK: TestStruct
// CHECK: getA
// CHECK: incrementAndReturnA

// GLSL emits TestStruct with member function names.
// GLSL: struct TestStruct_{{[0-9]+}}
// GLSL: TestStruct_getA_{{[0-9]+}}
// GLSL: TestStruct_incrementAndReturnA_{{[0-9]+}}

// SPIRV inlines function bodies; the struct is not emitted as a type but
// we can assert that the result values (int_42 stored at index 0, OpIAdd for
// incrementAndReturn) appear in the module.
// SPV: %int_42
// SPV: OpIAdd

// Metal emits TestStruct with member function names.
// METAL: struct TestStruct_{{[0-9]+}}
// METAL: TestStruct_getA_{{[0-9]+}}
// METAL: TestStruct_incrementAndReturnA_{{[0-9]+}}

// WGSL emits TestStruct with member function names.
// WGSL: struct TestStruct_{{[0-9]+}}
// WGSL: TestStruct_getA_{{[0-9]+}}
// WGSL: TestStruct_incrementAndReturnA_{{[0-9]+}}

// CUDA emits TestStruct with member function names.
// CUDA: struct TestStruct_{{[0-9]+}}
// CUDA: TestStruct_getA_{{[0-9]+}}
// CUDA: TestStruct_incrementAndReturnA_{{[0-9]+}}

// CPP emits TestStruct with member function names.
// CPP: struct TestStruct_{{[0-9]+}}
// CPP: TestStruct_getA_{{[0-9]+}}
// CPP: TestStruct_incrementAndReturnA_{{[0-9]+}}
