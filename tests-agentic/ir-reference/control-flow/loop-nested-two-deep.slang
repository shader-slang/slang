//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T21:00:00+00:00
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/llm-generated/ir-reference/control-flow.md#loop
//META: doc_section_digest=04d0daf0564c731241b1f5e0a49a1a7dc27737ec3435d045559640aef919740e
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
