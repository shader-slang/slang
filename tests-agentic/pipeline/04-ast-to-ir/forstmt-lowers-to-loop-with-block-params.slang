//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:30:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=7d43267959b0c547516266beea43e6e96622d37270bfab810d9819a48f90812b
//META: purpose=A ForStmt lowers to a `loop` terminator; the induction variable is carried as a block `param` (phi-style joining encoded as block parameters).
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc claims `ForStmt` → structured branch and that "phi-style
// joining is encoded as block parameters (IRParam at the start of a
// block) rather than explicit phi instructions". The for-loop
// induction variable shows up as a block `param`, and the loop
// header is reached by `loop(...)`.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int n;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int sum = 0;
    for (int i = 0; i < n; i++)
        sum += i;
    buf[0] = sum;
}

// CHECK-LABEL: func %main
// CHECK: loop(%{{.*}}
// CHECK: param %i
// CHECK: param %sum
