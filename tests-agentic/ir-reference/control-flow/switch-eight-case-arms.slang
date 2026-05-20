//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T21:00:00+00:00
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/llm-generated/ir-reference/control-flow.md#terminators-switches
//META: doc_section_digest=cc1b75a0316908fad8da474d769a60fbe3fbf9298cbe56506c4cd716ae3d4b7f
//META: purpose=Stress: a switch with eight case-arms emits all eight `(caseVal, caseBlock)` pairs as trailing operands on the single switch terminator.
//META: intent=stress
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Eight-case stress on the variadic operand list of the switch
// terminator. The doc's "zero or more `(caseValue, caseBlock)` pairs"
// claim should hold for the upper end as well; we observe all eight
// literal case values inline on the terminator.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform int sel;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    int r = 0;
    switch (sel)
    {
    case 1: r = 10; break;
    case 2: r = 20; break;
    case 3: r = 30; break;
    case 4: r = 40; break;
    case 5: r = 50; break;
    case 6: r = 60; break;
    case 7: r = 70; break;
    case 8: r = 80; break;
    default: r = 0; break;
    }
    buf[0] = r;
}

// CHECK-LABEL: func %main
// CHECK: switch(%sel, %{{[0-9]+}}, %{{[0-9]+}}, 1 : Int, %{{[0-9]+}}, 2 : Int, %{{[0-9]+}}, 3 : Int, %{{[0-9]+}}, 4 : Int, %{{[0-9]+}}, 5 : Int, %{{[0-9]+}}, 6 : Int, %{{[0-9]+}}, 7 : Int, %{{[0-9]+}}, 8 : Int, %{{[0-9]+}})
