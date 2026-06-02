//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:58:38+00:00
//META: source_commit=a01e79278cf0be9772deb8b2be52ee87687697a3
//META: doc_ref=docs/language-reference/declarations.md#enumeration-types
//META: doc_section_digest=4b116a159e7885ebdd4cd361d9d70a300c658321193492133928c90bc17abd3e
//META: purpose=Enumeration cases are referred to as if they were static members of the enumeration type (e.g., `Color.Red`).
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says: "An enumeration case is referred to as if it were a `static`
// member of the enumeration type (e.g., `Color.Red`)."

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry computeMain -stage compute

enum Color { Red = 0, Green = 3, Blue = 4 }
RWStructuredBuffer<int> output;
[numthreads(1, 1, 1)]
void computeMain(uint3 id : SV_DispatchThreadID)
{
    int r = Color.Red;
    int g = Color.Green;
    output[0] = r;
    output[1] = g;
}

// CHECK-DAG: 0
// CHECK-DAG: 3
