//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/control-flow.md#terminators-no-continuation
//META: doc_section_digest=78e0bf7576551f8ff9e811edd1abd135d18634ae25bb0afa4cf3437830164ba1
//META: purpose=A switch in which every arm returns leaves the synthesized post-switch merge block terminated by `unreachable`.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// In a helper where every switch arm `return`s, no control can reach
// the synthesized merge block. The optimizer marks that block with
// `unreachable` — the assertion that no path arrives there.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform int a;
uniform RWStructuredBuffer<int> buf;

int helper(int x)
{
    switch (x)
    {
    case 0:
        return 1;
    case 1:
        return 2;
    default:
        return 3;
    }
}

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = helper(a);
}

// CHECK-LABEL: func %helper
// CHECK: switch(%x
// CHECK: unreachable
