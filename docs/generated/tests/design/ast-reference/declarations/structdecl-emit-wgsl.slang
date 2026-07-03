//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#aggtypedecl-structdecl-classdecl-enumdecl
//META: doc_section_digest=60c3e2924d17c23b1f3bc412881a4c5e71f81efe23ecdd996e436f61429ac8c8
//META: purpose=A StructDecl is emitted as a struct type by each text-emit backend; the struct keyword and field names appear in the output.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies a Slang StructDecl reaches the WGSL backend as a WGSL struct
// declaration. The emitted WGSL text declares `struct Point_0` with both
// `i32` fields in `name : type` form; CHECK pins the struct keyword and each
// mangled field name, proving the aggregate type survives to WGSL emission.

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
