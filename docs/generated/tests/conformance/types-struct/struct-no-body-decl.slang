//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:08+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/types-struct.md#description
//META: doc_section_digest=0b091f4f0ba7cb2a07817dc55025aaaaadee5292438bfc88fba1ab02ee5d3878
//META: purpose=A no-body struct declaration (forward declaration) declares a struct type's existence and allows it to be used in type expressions before the full definition.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a no-body (forward) struct declaration followed by a full definition compiles.
// ForwardDeclared is declared without a body then defined with `int x`; CHECK confirms both
// the type name and field appear in HLSL output.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

// Forward declaration (no body)
struct ForwardDeclared;

// Full definition follows
struct ForwardDeclared
{
    int x;
}

RWStructuredBuffer<int> output;

[numthreads(1,1,1)]
void main()
{
    ForwardDeclared fd;
    fd.x = 55;
    output[0] = fd.x;
}

// CHECK: ForwardDeclared
// CHECK: x
