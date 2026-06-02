//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:02:06+00:00
//META: source_commit=9c1a6a00ef413932805da5b813465a7a9d517fb9
//META: doc_ref=docs/language-reference/types-interface.md#description
//META: doc_section_digest=2ed61530bd334a97114c08a34024c5fb9ccf09620d7a60bd6a64f9d345223014
//META: purpose=A struct conforming to an interface with a method and a static const member emits correctly to HLSL: the struct type and method are present, the static const is inlined.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Pins the HLSL emission for a basic struct-interface conformance pattern:
// struct method, static const data member.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

interface IShape
{
    float area();
    static const int dims;
}

struct Square : IShape
{
    static const int dims = 2;
    float side;
    float area() { return side * side; }
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main(RWStructuredBuffer<float> buf)
{
    Square sq;
    sq.side = 3.0f;
    buf[0] = sq.area();
    buf[1] = float(Square.dims);
}

// CHECK: struct Square
// CHECK: Square_area
// CHECK: numthreads
