//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:08+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/types-struct.md#description
//META: doc_section_digest=0b091f4f0ba7cb2a07817dc55025aaaaadee5292438bfc88fba1ab02ee5d3878
//META: purpose=`static const` data members must have a default initializer; the value is accessible at compile time via the type name.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc states: "`static const` data members must have a default initializer."
// A static const int with value 6 should be accessible via type name and
// produce the constant in emitted code.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

struct TestStruct
{
    static const int c = 6;
}

RWStructuredBuffer<int> output;

[numthreads(1,1,1)]
void main()
{
    output[0] = TestStruct.c;
}

// CHECK: int(6)
