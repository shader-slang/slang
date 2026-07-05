//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T10:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=7d43267959b0c547516266beea43e6e96622d37270bfab810d9819a48f90812b
//META: purpose=A void-returning FuncDecl with an empty body lowers to an IRFunc whose sole block is a single `return_val(void_constant)` terminator.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary: function-body lower edge. The empty-body source `void
// noop() {}` still produces an IRFunc with one IRBlock and a return
// terminator — confirming that the "FuncDecl → IRFunc containing one
// or more IRBlocks" mapping holds at the smallest possible body.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform RWStructuredBuffer<int> buf;

void noop() { }

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    noop();
    buf[0] = 0;
}

// CHECK-LABEL: func %noop
// CHECK: block %{{.*}}
// CHECK: return_val(void_constant)
