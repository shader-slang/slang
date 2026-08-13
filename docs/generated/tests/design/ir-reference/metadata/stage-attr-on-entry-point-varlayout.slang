//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/metadata.md#attr-family
//META: doc_section_digest=2da76dd4726862c0040f126154c48b23ed2237c18f88f175659ae9d45c2fec5d
//META: purpose=The stage attribute tags an entry-point varLayout with a single pipeline-stage integer literal.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the stage tag and its arity. The entry point's parameter var layout
// carries the attribute; the CHECK pins the closing parenthesis after one
// operand, so an attribute with a different operand count would not match, and
// the captured id is resolved back to a `stage` definition from within the
// var layout that owns it.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = int(tid.x);
}

// CHECK: = varLayout(%{{[0-9]+}}, %{{[0-9]+}}, %[[STAGE:[0-9]+]])
// CHECK: %[[STAGE]]{{.*}}= stage({{[0-9]+}} : Int)
