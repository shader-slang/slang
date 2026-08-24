//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=1195a752790819415908e083ce9656a353d4764f2fab38cc5aa27d92f184c486
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
