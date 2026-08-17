//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/control-flow.md#terminators-switches
//META: doc_section_digest=93e3e0077c420a8fd04038823472fc8368e454763e1fd6cc905f959fe8e07ee8
//META: purpose=Boundary: a switch with no `default:` arm still emits the switch terminator; the defaultLabel operand is supplied by the lowering as the post-switch merge block.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Omitting the explicit `default:` arm does not change the operand
// shape — the lowering still provides a defaultLabel as the third
// operand, satisfying the doc's `value, breakLabel, defaultLabel, ...`
// layout.

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
    }
    buf[0] = r;
}

// CHECK-LABEL: func %main
// CHECK: switch(%sel, %{{[0-9]+}}, %{{[0-9]+}}, 1 : Int, %{{[0-9]+}}, 2 : Int, %{{[0-9]+}})
