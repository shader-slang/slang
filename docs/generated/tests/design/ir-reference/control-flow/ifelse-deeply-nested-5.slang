//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/control-flow.md#ifelse
//META: doc_section_digest=9107af33a205c736447b4c02d66c4032cb7ffae2ebc1f3e72dc1132780edf3e5
//META: purpose=Stress: five nested if-statements produce five distinct structured ifElse terminators in the same function, each with the four-operand cond/true/false/merge tuple.
//META: intent=stress
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Five-deep nested ifs against five independent uniforms; the LOWER-TO-IR
// pass emits five ifElse terminators back-to-back without collapsing
// any of them (uniforms defeat constant folding).

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform int a;
uniform int b;
uniform int c;
uniform int d;
uniform int e;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    int r = 0;
    if (a > 0)
        if (b > 0)
            if (c > 0)
                if (d > 0)
                    if (e > 0)
                        r = 1;
    buf[0] = r;
}

// CHECK-LABEL: func %main
// CHECK: ifElse(%{{[0-9]+}}, %{{[0-9]+}}, %{{[0-9]+}}, %{{[0-9]+}})
// CHECK: ifElse(%{{[0-9]+}}, %{{[0-9]+}}, %{{[0-9]+}}, %{{[0-9]+}})
// CHECK: ifElse(%{{[0-9]+}}, %{{[0-9]+}}, %{{[0-9]+}}, %{{[0-9]+}})
// CHECK: ifElse(%{{[0-9]+}}, %{{[0-9]+}}, %{{[0-9]+}}, %{{[0-9]+}})
// CHECK: ifElse(%{{[0-9]+}}, %{{[0-9]+}}, %{{[0-9]+}}, %{{[0-9]+}})
