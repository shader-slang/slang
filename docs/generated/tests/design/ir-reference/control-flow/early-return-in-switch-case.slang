//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/control-flow.md#terminators-returns-and-yields
//META: doc_section_digest=010f71e10305e8452b6909c94e2da52806525700f6c78aa6daffe80e8b4ca211
//META: purpose=Boundary: a `return` from inside a switch case terminates that arm's block with `return_val`, independent of the surrounding switch's structured layout; the synthesized post-switch merge is reached by no path and is terminated by `unreachable`.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Each switch arm returns directly, so each arm's block ends in
// `return_val(<literal>)` and the synthesized post-switch merge block
// ends in `unreachable`.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform int sel;
uniform RWStructuredBuffer<int> buf;

int helper(int x)
{
    switch (x)
    {
    case 1:
        return 10;
    case 2:
        return 20;
    default:
        return 0;
    }
}

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = helper(sel);
}

// CHECK-LABEL: func %helper
// CHECK: switch(%x
// CHECK: return_val(10 : Int)
// CHECK: return_val(20 : Int)
// CHECK: return_val(0 : Int)
// CHECK: unreachable
