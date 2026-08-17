//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=1195a752790819415908e083ce9656a353d4764f2fab38cc5aa27d92f184c486
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
