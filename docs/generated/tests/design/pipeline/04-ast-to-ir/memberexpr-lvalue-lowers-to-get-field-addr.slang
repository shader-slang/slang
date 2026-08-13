//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=37a00b76a98a13c2f3cd47117e12f237fbf042ab8ec479231f23490859cc4497
//META: purpose=A MemberExpr used as an assignment target (lvalue) lowers to `IRFieldAddress` (spelled `get_field_addr` in the dump).
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `p.x = ...` is the lvalue branch of the doc's MemberExpr row. The
// IR address-of producing instruction is `get_field_addr`, whose
// result type is `Ptr(<FieldType>)` and which feeds into a
// subsequent `store`.

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

// CHECK-LABEL: func %main
// CHECK: get_field_addr(%{{.*}}, %{{.*}})
// CHECK: store(%{{.*}}, %{{.*}})
