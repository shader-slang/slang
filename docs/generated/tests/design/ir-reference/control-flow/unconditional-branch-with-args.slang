//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/control-flow.md#terminators-unconditional-branches
//META: doc_section_digest=a0f708a11d81970e17fbe525500eba453951d03287c41957fe027cbfb06d85f6
//META: purpose=unconditionalBranch carries argument operands beyond the target block; these become the values bound to the target's block params, the Slang-IR SSA phi handoff.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// At the end of each switch arm, the unconditionalBranch into the
// post-switch merge block carries the per-arm value as an argument
// operand; the merge block receives it as a block-level param.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform int sel;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    int r = 0;
    switch (sel)
    {
    case 1:
        r = 10;
        break;
    case 2:
        r = 20;
        break;
    default:
        r = 99;
        break;
    }
    buf[0] = r;
}

// CHECK-LABEL: func %main
// CHECK: unconditionalBranch(%{{[0-9]+}}, 10 : Int)
// CHECK: unconditionalBranch(%{{[0-9]+}}, 20 : Int)
// CHECK: unconditionalBranch(%{{[0-9]+}}, 99 : Int)
// CHECK: param %r	: Int
