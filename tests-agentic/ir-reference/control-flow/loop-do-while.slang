//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/llm-generated/ir-reference/control-flow.md#loop
//META: doc_section_digest=04d0daf0564c731241b1f5e0a49a1a7dc27737ec3435d045559640aef919740e
//META: purpose=A do-while loop lowers to a loop terminator; the trailing test is implemented by negating the condition so the ifElse branches to break on true.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A do-while loop produces the same loop opcode. The test placement
// at the end inverts the condition: the lowering computes
// not(cond) and branches to break when true.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform int n;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    int s = 0;
    int i = 0;
    do
    {
        s = s + i;
        i = i + 1;
    } while (i < n);
    buf[0] = s;
}

// CHECK-LABEL: func %main
// CHECK: loop(%{{[0-9]+}}, %{{[0-9]+}}, %{{[0-9]+}}, 0 : Int, 0 : Int)
// CHECK: cmpLT(%{{[a-zA-Z0-9_]+}}, %n)
// CHECK: not(%{{[0-9]+}})
