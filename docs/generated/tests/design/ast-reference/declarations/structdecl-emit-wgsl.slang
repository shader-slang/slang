//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#aggtypedecl-structdecl-classdecl-enumdecl
//META: doc_section_digest=ee81272d01f69d54c75b75fc6518d2ad8af9fdb4a6eed0176bdf0fe82ee19e5c
//META: purpose=A StructDecl survives to each text-emit backend as a named struct type rather than being scalarized away.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the same aggregate-survives-emission claim on the WGSL backend,
// where struct members use `name : type,` syntax and `int` becomes `i32`.
// The CHECKs pin the struct declaration and both member lines.

//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry main -stage compute

struct Point { int x; int y; }

//TEST_INPUT: set outBuf = out ubuffer(data=[0 0], stride=4)
RWStructuredBuffer<int> outBuf;

//WGSL: struct Point_0
//WGSL: x_{{[0-9]+}} : i32,
//WGSL: y_{{[0-9]+}} : i32,
[shader("compute")]
[numthreads(1,1,1)]
void main()
{
    Point p;
    p.x = 7;
    p.y = 8;
    outBuf[0] = p.x + p.y;
}
