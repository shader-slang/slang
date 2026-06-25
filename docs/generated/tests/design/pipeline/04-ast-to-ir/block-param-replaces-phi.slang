//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:30:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=7d43267959b0c547516266beea43e6e96622d37270bfab810d9819a48f90812b
//META: purpose=Phi-style joining is encoded as block parameters (IRParam at the start of a block); branches to a block carry the parameter values as arguments — not via an explicit `phi` opcode.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says "Phi-style joining is encoded as block parameters
// (IRParam at the start of a block) rather than explicit phi
// instructions; branches to a block carry the parameter values as
// arguments." A `for` loop is the cleanest probe: the loop header
// block takes its induction value as a `param`, and the back edge
// `unconditionalBranch(%header, %i+1, %sum+i)` supplies the next
// iteration's values as branch arguments. We CHECK for the absence
// of any `phi` opcode and the presence of `param` inside the loop.

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
// CHECK: unconditionalBranch(%{{.*}}, %{{.*}}
// CHECK-NOT: = phi(
