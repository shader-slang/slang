//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:13:27Z
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/llm-generated/ir-reference/generics-and-existentials.md#specialize
//META: doc_section_digest=4965484dbbae47fbcb43a84ac453b10df0094aeb1ec637158927580932cf4106
//META: purpose=The specialize opcode is hoistable; two textually-identical specialize references collapse to one value in the IR.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `### specialize` says "hoistable, so two references to the same
// specialize with the same arguments collapse to one IR value".
// Two textually-identical `pickFirst<int>(...)` call sites both
// reference the *same* specialized callee value — observable as
// the `call specialize(%pickFirst, Int)(...)` spelling appearing
// twice with the same arguments, never with different specialize
// targets per call site.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform int a;
uniform int b;
uniform RWStructuredBuffer<int> buf;

T pickFirst<T>(T x, T y) { return x; }

[numthreads(1, 1, 1)]
void main()
{
    // Two specialize<int> applications: both should use the same
    // specialize(%pickFirst, Int) callee.
    buf[0] = pickFirst<int>(a, b);
    buf[1] = pickFirst<int>(b, a);
}

// CHECK: call specialize(%{{[A-Za-z0-9_]+}}, Int)
// CHECK: call specialize(%{{[A-Za-z0-9_]+}}, Int)
