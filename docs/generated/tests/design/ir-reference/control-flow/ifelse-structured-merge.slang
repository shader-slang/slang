//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/control-flow.md#ifelse
//META: doc_section_digest=58ff39ee4b5eb63d9fd7dd839e0f9f67e7ef5727cd278b7eee0411b98e5d64fe
//META: purpose=An if/else statement lowers to ifElse(cond, trueBlock, falseBlock, mergeBlock) with the explicit fourth-operand merge block recording the structured join.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A simple `if (a > b) ... else ...` against uniforms produces the
// structured ifElse terminator. The four operands are condition,
// true-arm block, false-arm block, and the explicit merge block.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform int a;
uniform int b;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    if (a > b)
    {
        buf[0] = a;
    }
    else
    {
        buf[0] = b;
    }
}

// CHECK-LABEL: func %main
// CHECK: cmpGT(%a, %b)
// CHECK: ifElse(%{{[0-9]+}}, %{{[0-9]+}}, %{{[0-9]+}}, %{{[0-9]+}})
