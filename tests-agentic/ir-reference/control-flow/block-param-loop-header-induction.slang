//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T21:00:00+00:00
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/llm-generated/ir-reference/control-flow.md#block-and-param
//META: doc_section_digest=6464d00693cef2b4a0aff4d2217833f482b5d7a83cc29251b96ae2aaa6d765d4
//META: purpose=Boundary: a loop header carrying two independent induction variables receives two `param` declarations — one per induction value — and both initial values appear as trailing operands on the loop terminator.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A while-loop with a separately-declared induction variable plus an
// accumulator both arrive on the body header as block params; the
// loop terminator's trailing operands carry both initial values.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform int n;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    int s = 0;
    int i = 0;
    while (i < n)
    {
        s = s + i;
        i = i + 1;
    }
    buf[0] = s;
}

// CHECK-LABEL: func %main
// CHECK: loop(%{{[0-9]+}}, %{{[0-9]+}}, %{{[0-9]+}}, 0 : Int, 0 : Int)
// CHECK: param %i	: Int
// CHECK: param %s	: Int
