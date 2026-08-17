//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=1195a752790819415908e083ce9656a353d4764f2fab38cc5aa27d92f184c486
//META: purpose=A MemberExpr in rvalue position lowers to `IRFieldExtract` (spelled `get_field` in the dump).
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `p.x` on a value-typed struct parameter reads through `get_field`
// — the doc's "MemberExpr → IRFieldAddress / IRFieldExtract (lvalue
// vs rvalue)" claim, rvalue branch.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

struct Pair { int x; int y; }

int makeSum(Pair p) {
    return p.x + p.y;
}

uniform RWStructuredBuffer<Pair> ibuf;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = makeSum(ibuf[0]);
}

// CHECK-LABEL: func %makeSum
// CHECK: get_field(%{{.*}}, %{{.*}})
// CHECK: get_field(%{{.*}}, %{{.*}})
