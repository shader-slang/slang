//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:35:00Z
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/ir-reference/control-flow.md#ifelse
//META: doc_section_digest=69efda819e39cba201908703879949bee162c97716e69f8114247bf4114d9ec0
//META: purpose=With -minimum-slang-optimization the mandatory simplifyCFG is skipped, so an if with an empty then-arm keeps its own then block: trueBlock and afterBlock stay distinct in the first dump.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Companion to ifelse-empty-then-arm.slang. The doc says the fold of an
// empty arm onto the merge block happens only when minimum optimizations are
// off. With -minimum-slang-optimization the empty then-arm survives as its own
// block (which just branches to the merge block), so the ifElse operands are
// four distinct blocks and the then block contains only the branch.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o - -entry main -stage compute -minimum-slang-optimization

uniform int a;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    if (a > 0)
    {
        // empty then-arm
    }
    else
    {
        buf[1] = a;
    }
    buf[0] = a;
}

// CHECK-LABEL: func %main
// CHECK: ifElse(%{{[0-9]+}}, %[[THEN:[0-9]+]], %[[ELSE:[0-9]+]], %[[AFTER:[0-9]+]])
// CHECK: block %[[THEN]]:
// CHECK-NEXT: unconditionalBranch(%[[AFTER]])
// CHECK: block %[[ELSE]]:
