//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/llm-generated/ir-reference/control-flow.md#block-and-param
//META: doc_section_digest=6464d00693cef2b4a0aff4d2217833f482b5d7a83cc29251b96ae2aaa6d765d4
//META: purpose=A block parameter encodes Slang IR's replacement for an SSA phi node — the receiving block declares the param, predecessors supply the matching argument on their unconditionalBranch.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The post-switch merge block becomes the home of an SSA phi: each
// arm writes a different value to a local, and the merge block
// declares a `param %r : Int` that the predecessors' branches
// supply as their argument.

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
// CHECK: switch(%sel
// CHECK: param %r	: Int
