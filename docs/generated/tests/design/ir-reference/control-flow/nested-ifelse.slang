//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/control-flow.md#ifelse
//META: doc_section_digest=8e8ff9867878750c6e494dc89c9ad99cc9c750ba3dbe5e5c6e752d1886d3e399
//META: purpose=Nested if-statements produce nested structured ifElse terminators — each level carries its own four-operand cond/true/false/merge tuple.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Two nested conditionals on uniforms produce two distinct ifElse
// terminators in the same function — the outer arm contains an
// inner ifElse of its own.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o - -entry main -stage compute

uniform int a;
uniform int b;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    if (a > 0)
    {
        if (b > 0)
        {
            buf[0] = a + b;
        }
        else
        {
            buf[0] = a;
        }
    }
    else
    {
        buf[0] = -1;
    }
}

// CHECK-LABEL: func %main
// CHECK: ifElse(%{{[0-9]+}}, %{{[0-9]+}}, %{{[0-9]+}}, %{{[0-9]+}})
// CHECK: ifElse(%{{[0-9]+}}, %{{[0-9]+}}, %{{[0-9]+}}, %{{[0-9]+}})
