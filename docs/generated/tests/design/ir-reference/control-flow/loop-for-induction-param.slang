//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/control-flow.md#loop
//META: doc_section_digest=41069943e707403bf4e509b55db22b8e4cad325fff0b4de1f2732b878f000ad2
//META: purpose=A for-loop lowers to loop(body, break, continue, ...) with the three structured-join operands plus initial values for body-block induction params.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A canonical for-loop accumulator surfaces the loop terminator. The
// loop body block then receives the induction variables as `param`
// declarations — Slang IR's encoding of SSA phi.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform int n;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    int s = 0;
    for (int i = 0; i < n; i++)
    {
        s = s + i;
    }
    buf[0] = s;
}

// CHECK-LABEL: func %main
// CHECK: loop(%{{[0-9]+}}, %{{[0-9]+}}, %{{[0-9]+}}, 0 : Int, 0 : Int)
// CHECK: param %s	: Int
// CHECK: param %i	: Int
