//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/generated/design/ir-reference/control-flow.md#terminators-switches
//META: doc_section_digest=cc1b75a0316908fad8da474d769a60fbe3fbf9298cbe56506c4cd716ae3d4b7f
//META: purpose=Case-label values in a switch terminator appear as inline literal operands `<N> : Int`, including for case labels named via static const.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Case labels declared with `static const int K = N;` resolve to the
// same inline-literal form `N : Int` on the switch terminator. The
// scrutinee comes from a uniform so that constant folding cannot
// collapse the switch.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

static const int LOW = 7;
static const int HIGH = 42;

uniform int sel;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    int r = 0;
    switch (sel)
    {
    case LOW:
        r = 1;
        break;
    case HIGH:
        r = 2;
        break;
    default:
        r = 0;
        break;
    }
    buf[0] = r;
}

// CHECK-LABEL: func %main
// CHECK: switch(%sel, %{{[0-9]+}}, %{{[0-9]+}}, 7 : Int, %{{[0-9]+}}, 42 : Int, %{{[0-9]+}})
