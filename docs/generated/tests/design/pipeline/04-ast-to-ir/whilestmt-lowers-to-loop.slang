//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:30:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=7d43267959b0c547516266beea43e6e96622d37270bfab810d9819a48f90812b
//META: purpose=A WhileStmt lowers to a `loop` structured-branch terminator (sibling of for; doc lists `WhileStmt` in the same row as `ForStmt`).
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc lists `WhileStmt` alongside `ForStmt` as lowering to a
// structured branch. The `while` loop here produces a `loop(...)`
// terminator in IR — the same opcode as `for` — confirming the
// row's claim.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int n;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    int i = 0;
    while (i < n)
    {
        buf[i] = i;
        i++;
    }
}

// CHECK-LABEL: func %main
// CHECK: loop(%{{.*}}
