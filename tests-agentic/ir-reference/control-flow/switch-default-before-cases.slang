//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T21:00:00+00:00
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/llm-generated/ir-reference/control-flow.md#terminators-switches
//META: doc_section_digest=cc1b75a0316908fad8da474d769a60fbe3fbf9298cbe56506c4cd716ae3d4b7f
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
