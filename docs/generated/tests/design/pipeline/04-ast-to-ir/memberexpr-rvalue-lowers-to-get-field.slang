//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:30:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=7d43267959b0c547516266beea43e6e96622d37270bfab810d9819a48f90812b
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
