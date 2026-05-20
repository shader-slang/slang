//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/llm-generated/ir-reference/control-flow.md#terminators-unconditional-branches
//META: doc_section_digest=14af4d2a3a4636636d6099f23a462c390fb80ef7faa8966bf7a220163783dc45
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
