//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/control-flow.md#loop
//META: doc_section_digest=41069943e707403bf4e509b55db22b8e4cad325fff0b4de1f2732b878f000ad2
//META: purpose=A while-loop lowers to the same loop(body, break, continue, ...) terminator as a for-loop, with the induction variable arriving as a param on the body block.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A while-loop with a single induction variable produces a loop
// terminator with one trailing init operand and a single block
// `param %s : Int` on the body block.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform int n;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    int s = 0;
    while (s < n)
    {
        s = s + 1;
    }
    buf[0] = s;
}

// CHECK-LABEL: func %main
// CHECK: loop(%{{[0-9]+}}, %{{[0-9]+}}, %{{[0-9]+}}, 0 : Int)
// CHECK: param %s	: Int
