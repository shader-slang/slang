//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=37a00b76a98a13c2f3cd47117e12f237fbf042ab8ec479231f23490859cc4497
//META: purpose=A BlockStmt lowers to a sequence of basic blocks; the function body has multiple `block %{{.*}}` regions when control flow branches.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc row "BlockStmt → A sequence of basic blocks; locals turn
// into IRVar." A function with an `if` introduces multiple `block`
// regions in the IR (then-block, else-block, merge-block). We CHECK
// for at least three `block` lines inside `main`.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    if (a > 0)
        buf[0] = a;
    else
        buf[0] = -a;
}

// CHECK-LABEL: func %main
// CHECK: block %{{.*}}
// CHECK: block %{{.*}}
// CHECK: block %{{.*}}
// CHECK: block %{{.*}}
