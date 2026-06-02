//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:08+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/types-struct.md#description
//META: doc_section_digest=0b091f4f0ba7cb2a07817dc55025aaaaadee5292438bfc88fba1ab02ee5d3878
//META: purpose=A struct with non-static fields emits as a struct type with those fields on all text-emit backends.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Emission test: verifies that a Slang struct with int fields
// is emitted as a struct type containing those fields on HLSL and GLSL.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL_CHECK):-target glsl -entry main -stage compute

struct Point
{
    int x;
    int y;
}

RWStructuredBuffer<int> output;

[numthreads(1,1,1)]
void main()
{
    Point p;
    p.x = 3;
    p.y = 7;
    output[0] = p.x + p.y;
}

// CHECK: struct Point
// CHECK: int x
// CHECK: int y

// GLSL_CHECK: struct Point
