//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T21:00:00+00:00
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/generated/design/ir-reference/control-flow.md#terminators-returns-and-yields
//META: doc_section_digest=7a22c485cd43a8f37c76060d034a3035010f4828abcf759c51b28677698d3453
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
