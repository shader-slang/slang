//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:30:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=7d43267959b0c547516266beea43e6e96622d37270bfab810d9819a48f90812b
//META: purpose=A StructDecl lowers to an `IRStructType` parent instruction containing `IRStructField` children.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A two-field struct lowers to a `struct %Pair` parent with two
// `field(%<key>, <Type>)` rows — the doc's "StructDecl → IRStructType
// with IRStructField children" claim.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

struct Pair { int x; int y; }

uniform int a;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    Pair p;
    p.x = a;
    p.y = a + 1;
    buf[0] = p.x + p.y;
}

// CHECK-LABEL: struct %Pair
// CHECK: field({{.*}}, Int)
// CHECK: field({{.*}}, Int)
