//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:30:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=7d43267959b0c547516266beea43e6e96622d37270bfab810d9819a48f90812b
//META: purpose=A BlockStmt lowers to a sequence of basic blocks; the function body has multiple `block %{{.*}}` regions when control flow branches.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc row "BlockStmt → A sequence of basic blocks; locals turn
// into IRVar." A function with an `if` introduces multiple `block`
// regions in the IR (then-block, else-block, merge-block). We CHECK
// for at least three `block` lines inside `main`.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    if (a > 0)
        buf[0] = a;
    else
        buf[0] = -a;
}

// CHECK-LABEL: func %main
// CHECK: block %{{.*}}
// CHECK: block %{{.*}}
// CHECK: block %{{.*}}
// CHECK: block %{{.*}}
