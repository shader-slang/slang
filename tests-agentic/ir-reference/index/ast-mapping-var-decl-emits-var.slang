//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:58:43Z
//META: source_commit=30ae111120515b7406aa6f427a4eaaa28a0903d8
//META: doc_ref=docs/llm-generated/ir-reference/index.md#how-ast-nodes-lower-to-ir
//META: doc_section_digest=e4b96c5106c1f3a08e7bc994b1774dbb0f39e0c83b35d7a5f72e4ed9186e3b3e
//META: purpose=The index doc names visitVarDecl as the AST-to-IR mapping that emits a Var opcode; a struct-typed local should produce a var instruction in the IR.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The index doc says: "visitVarDecl emits Var". `_common.md`
// notes that trivial scalar locals are eliminated during initial
// lowering — so we use a struct-typed local whose `var` survives
// because the local is accessed by field address.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -dump-ir -o /dev/null -entry main -stage compute

struct Pair
{
    int lo;
    int hi;
};

uniform int a;
uniform int b;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    Pair p;
    p.lo = a;
    p.hi = b;
    buf[0] = p.lo + p.hi;
}

// CHECK: var
