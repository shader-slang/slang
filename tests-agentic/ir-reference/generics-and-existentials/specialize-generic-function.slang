//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:13:27Z
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/llm-generated/ir-reference/generics-and-existentials.md#specialize
//META: doc_section_digest=4965484dbbae47fbcb43a84ac453b10df0094aeb1ec637158927580932cf4106
//META: purpose=A call to a generic function with a concrete type argument lowers to a call whose callee is a specialize(generic, T) opcode.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `pickFirst<int>(a, b)` for a generic `T pickFirst<T>(T x, T y)`
// should produce a call site whose callee is
// `specialize(%pickFirst, Int)` — the doc's "Applies generic
// arguments to a generic value" claim under `### specialize`.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform int a;
uniform RWStructuredBuffer<int> buf;

T pickFirst<T>(T x, T y) { return x; }

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = pickFirst<int>(a, a + 1);
}

// CHECK: call specialize(%{{[A-Za-z0-9_]+}}, Int)
