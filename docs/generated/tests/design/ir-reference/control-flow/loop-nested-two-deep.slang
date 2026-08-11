//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/control-flow.md#loop
//META: doc_section_digest=41069943e707403bf4e509b55db22b8e4cad325fff0b4de1f2732b878f000ad2
//META: purpose=Stress: two nested for-loops produce two independent loop terminators in the same function, each with its own (body, break, continue) structured-join triple.
//META: intent=stress
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Two nested loops over independent uniform bounds. The LOWER-TO-IR
// pass emits two `loop` terminators back-to-back in the same function;
// the inner loop's break/continue labels are distinct from the outer's.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform int n;
uniform int m;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    int s = 0;
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < m; j++)
        {
            s = s + i * j;
        }
    }
    buf[0] = s;
}

// CHECK-LABEL: func %main
// CHECK: loop(%{{[0-9]+}}, %{{[0-9]+}}, %{{[0-9]+}}, 0 : Int, 0 : Int)
// CHECK: loop(%{{[0-9]+}}, %{{[0-9]+}}, %{{[0-9]+}}, %{{[a-zA-Z0-9_]+}}, 0 : Int)
