//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:13:48Z
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/llm-generated/ir-reference/decorations.md#other
//META: doc_section_digest=99f72cda245d99360168c24acf24a82fd577e139a911dfa073addcf5493b365a
//META: purpose=The synthesized struct initializer carries a constructor decoration; the dump prints constructor(true) with a Bool literal operand.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc records `constructor` with a variadic min=1 operand list
// and AST origin "__init declaration". An aggregate literal
// `S{7}` invokes the synthesized __init, which carries the
// [constructor(true)] decoration in the dump.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

struct S { int x; };

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    S s = { int(tid.x) + 7 };
    buf[0] = s.x;
}

// CHECK: [constructor(true)]
