//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/control-flow.md#terminators-switches
//META: doc_section_digest=4820552ad726706a9ebecdf715453f85eb93c80c7a2decca221f929866c46c78
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
