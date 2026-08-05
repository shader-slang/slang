//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#aggtypedecl-structdecl-classdecl-enumdecl
//META: doc_section_digest=ee81272d01f69d54c75b75fc6518d2ad8af9fdb4a6eed0176bdf0fe82ee19e5c
//META: purpose=A StructDecl survives to each text-emit backend as a named struct type rather than being scalarized away.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the aggregate type declared in Slang is still an aggregate in
// the emitted target text. The same two-field struct is compiled to HLSL,
// GLSL and CUDA; each prefix's CHECK pins the `struct Point` declaration in
// that backend's output, so a backend that flattened the aggregate would
// fail its own directive.

//TEST:SIMPLE(filecheck=CHECK_HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=CHECK_GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=CHECK_CUDA):-target cuda -entry main -stage compute

struct Point { int x; int y; }

//TEST_INPUT: set outBuf = out ubuffer(data=[0 0], stride=4)
RWStructuredBuffer<int> outBuf;

//CHECK_HLSL: struct Point
//CHECK_GLSL: struct Point
//CHECK_CUDA: struct Point
[shader("compute")]
[numthreads(1,1,1)]
void main()
{
    Point p;
    p.x = 7;
    p.y = 8;
    outBuf[0] = p.x + p.y;
}
