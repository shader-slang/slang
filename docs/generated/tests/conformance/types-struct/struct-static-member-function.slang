//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:08+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/types-struct.md#static-member-function
//META: doc_section_digest=8afbecf75d617dcea4fff9698c69139804aa496835e7f4949f20e79ddf202cec
//META: purpose=A static member function may be invoked without an object instance; it emits as a free function (no this parameter) in HLSL output.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a static member function is callable without an instance and emits as a named
// function in HLSL. TestStruct.getB() is called in the entry point; CHECK confirms the function
// name and a return statement appear in the emitted output.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

struct TestStruct
{
    static int b = 5;

    static int getB()
    {
        return b;
    }
}

RWStructuredBuffer<int> output;

[numthreads(1,1,1)]
void main()
{
    output[0] = TestStruct.getB();
}

// CHECK: getB
// CHECK: return
