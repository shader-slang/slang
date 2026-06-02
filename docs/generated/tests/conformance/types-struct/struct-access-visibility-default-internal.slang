//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:08+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/types-struct.md#description
//META: doc_section_digest=0b091f4f0ba7cb2a07817dc55025aaaaadee5292438bfc88fba1ab02ee5d3878
//META: purpose=Members declared with `public` visibility are accessible from outside the struct; default member visibility is `internal`.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `public` members are accessible from outside the struct and appear in emitted HLSL.
// Both the explicit `public` field and the default-visibility field are written and summed in the
// entry point; CHECK confirms both field names survive to HLSL output.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

public struct MyStruct
{
    public int publicField;
    int defaultField;  // internal by default
}

RWStructuredBuffer<int> output;

[numthreads(1,1,1)]
void main()
{
    MyStruct s;
    s.publicField = 10;
    s.defaultField = 20;
    output[0] = s.publicField + s.defaultField;
}

// CHECK: publicField
// CHECK: defaultField
