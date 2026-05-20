//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:30:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir
//META: doc_section_digest=7d43267959b0c547516266beea43e6e96622d37270bfab810d9819a48f90812b
//META: purpose=A local VarDecl whose address is taken (struct field access) lowers to an `IRVar` allocated inside the enclosing block.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's mapping table says local `VarDecl` → `IRVar`. Trivial
// integer locals are eliminated by initial lowering, so we use a
// struct local accessed by field address — that pattern keeps the
// `var` opcode alive in the IR dump. The variable type is
// `Ptr(<StructType>)`.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

struct Pair { int x; int y; }

uniform int a;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    Pair p;
    p.x = a;
    p.y = a + 1;
    buf[0] = p.x + p.y;
}

// CHECK-LABEL: func %main
// CHECK: var
