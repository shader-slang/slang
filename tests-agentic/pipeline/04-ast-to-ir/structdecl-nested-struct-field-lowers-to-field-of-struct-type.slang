//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T18:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/llm-generated/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=7d43267959b0c547516266beea43e6e96622d37270bfab810d9819a48f90812b
//META: purpose=A StructDecl containing another StructDecl as a field lowers to an outer IRStructType whose `field(...)` row names the inner IRStructType; a nested member assignment lowers to chained `get_field_addr` instructions.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary: struct-field-type axis — a struct-typed field. Both the
// inner and outer StructDecls satisfy the doc's "StructDecl ->
// IRStructType with IRStructField children" row; the inner type's
// name appears as the type operand of the outer's field row.
// `o.inner.x = a` lowers to chained `get_field_addr`.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

struct Inner { int x; }
struct Outer { Inner inner; int z; }

uniform int a;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    Outer o;
    o.inner.x = a;
    o.z = a + 1;
    buf[0] = o.inner.x + o.z;
}

// CHECK-LABEL: struct %Outer
// CHECK: field(%inner, %Inner)
// CHECK: field(%z, Int)
// CHECK-LABEL: func %main
// CHECK: get_field_addr(%{{.*}}, %inner)
// CHECK: get_field_addr(%{{.*}}, %x)
