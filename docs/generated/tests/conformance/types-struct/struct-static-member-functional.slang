//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:08+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/types-struct.md#description
//META: doc_section_digest=0b091f4f0ba7cb2a07817dc55025aaaaadee5292438bfc88fba1ab02ee5d3878
//META: purpose=Static data members are allocated from global storage; they are shared across all instances and accessed via the type name (not an instance).
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a `static` data member is allocated from global storage and emits as a
// module-scope static variable in HLSL. TestStruct.b is initialised to 5 and read in the
// entry point; CHECK confirms a static variable with value int(5) appears in the output.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

struct TestStruct
{
    int a;
    static int b = 5;
}

RWStructuredBuffer<int> output;

[numthreads(1,1,1)]
void main()
{
    output[0] = TestStruct.b;
}

// CHECK: static int b_{{[0-9]+}} = int(5)
