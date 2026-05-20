//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:09:57Z
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/ast-reference/declarations.md#aggtypedecl-structdecl-classdecl-enumdecl
//META: doc_section_digest=a5782ff90acc1f9bd796853a7eeead0f64a6539af34d0a231454f705284208e7
//META: purpose=A StructDecl is emitted as a struct type by each text-emit backend; the struct keyword and field names appear in the output.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a Slang StructDecl reaches each text-emit backend as
// the expected struct shape. Different backends spell the
// declaration slightly differently, so each //TEST line has its own
// CHECK label.

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
