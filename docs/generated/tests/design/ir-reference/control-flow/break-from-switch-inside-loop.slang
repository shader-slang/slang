//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T21:00:00+00:00
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/generated/design/ir-reference/control-flow.md#terminators-unconditional-branches
//META: doc_section_digest=14af4d2a3a4636636d6099f23a462c390fb80ef7faa8966bf7a220163783dc45
//META: purpose=Boundary: a `break;` inside a switch nested in a loop targets the switch's breakLabel — it leaves the switch, not the enclosing loop. Both a switch terminator and a loop terminator coexist with distinct break labels.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A loop body containing a switch with break in each arm: each
// arm's break exits the switch (returns to the switch's break label),
// not the enclosing loop. The IR shows both a `loop` terminator and a
// `switch` terminator with distinct second operands.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform int n;
uniform int sel;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    int s = 0;
    for (int i = 0; i < n; i++)
    {
        switch (sel)
        {
        case 1:
            s = s + i;
            break;
        case 2:
            s = s + 2*i;
            break;
        default:
            s = s - 1;
            break;
        }
    }
    buf[0] = s;
}

// CHECK-LABEL: func %main
// CHECK: loop(%{{[0-9]+}}, %{{[0-9]+}}, %{{[0-9]+}}, 0 : Int, 0 : Int)
// CHECK: switch(%sel, %{{[0-9]+}}, %{{[0-9]+}}, 1 : Int, %{{[0-9]+}}, 2 : Int, %{{[0-9]+}})
