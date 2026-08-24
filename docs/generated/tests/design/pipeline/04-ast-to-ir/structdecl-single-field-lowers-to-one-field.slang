//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=1195a752790819415908e083ce9656a353d4764f2fab38cc5aa27d92f184c486
//META: purpose=A StructDecl with exactly one scalar field lowers to an IRStructType carrying exactly one `field(...)` child — sibling of the empty (zero-field is not tested) and the eight-field boundary cases.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary: struct-field-count lower edge (one). The doc says
// "StructDecl -> IRStructType with IRStructField children". With a
// single field the struct still lowers — exactly one `field(...)`
// row.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

struct One { int v; }

uniform int x;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    One o;
    o.v = x;
    buf[0] = o.v;
}

// CHECK-LABEL: struct %One
// CHECK: field(%v, Int)
