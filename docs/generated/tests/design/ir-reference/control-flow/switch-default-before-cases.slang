//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/control-flow.md#terminators-switches
//META: doc_section_digest=93e3e0077c420a8fd04038823472fc8368e454763e1fd6cc905f959fe8e07ee8
//META: purpose=Boundary: the source-text position of `default:` (before or after the case-arms) does not affect the switch terminator's operand layout — the default label is still the third operand.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Writing `default:` ahead of the case-arms is legal Slang. The
// lowering still emits the switch with `value, breakLabel, defaultLabel`
// as the three leading operands and the `(caseVal, caseBlock)` pairs
// after them.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform int sel;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    int r = 0;
    switch (sel)
    {
    default:
        r = 0;
        break;
    case 1:
        r = 10;
        break;
    case 2:
        r = 20;
        break;
    }
    buf[0] = r;
}

// CHECK-LABEL: func %main
// CHECK: switch(%sel, %{{[0-9]+}}, %{{[0-9]+}}, 1 : Int, %{{[0-9]+}}, 2 : Int, %{{[0-9]+}})
