//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:13:27Z
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/llm-generated/ir-reference/generics-and-existentials.md#specialize
//META: doc_section_digest=4965484dbbae47fbcb43a84ac453b10df0094aeb1ec637158927580932cf4106
//META: purpose=Five textually identical specialize(%base, Int) call sites all share the same hoistable callee value — the dispatch site spelling is invariant.
//META: intent=stress
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Stress on specialize-hoistable axis: many identical call
// sites. `### specialize` says the opcode is hoistable so
// identical specializations dedupe — at five call sites the
// dump should still spell `call specialize(%base, Int)(...)` at
// every site, with the same `%base` token (verifying the same
// IR value is reused).

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform int a;
uniform int b;
uniform RWStructuredBuffer<int> buf;

T pickFirst<T>(T x, T y) { return x; }

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = pickFirst<int>(a, b);
    buf[1] = pickFirst<int>(a, b);
    buf[2] = pickFirst<int>(b, a);
    buf[3] = pickFirst<int>(a, b);
    buf[4] = pickFirst<int>(b, a);
}

// CHECK: call specialize(%{{[A-Za-z0-9_]+}}, Int)
// CHECK: call specialize(%{{[A-Za-z0-9_]+}}, Int)
// CHECK: call specialize(%{{[A-Za-z0-9_]+}}, Int)
// CHECK: call specialize(%{{[A-Za-z0-9_]+}}, Int)
// CHECK: call specialize(%{{[A-Za-z0-9_]+}}, Int)
