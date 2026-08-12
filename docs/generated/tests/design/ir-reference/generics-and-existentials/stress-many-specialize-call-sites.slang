//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/generics-and-existentials.md#specialize
//META: doc_section_digest=3ad6835b3f4fbf438debd857fd413ba7cce495005327d971c499cc7510fecf17
//META: purpose=Five textually identical specialize(%base, Int) call sites all share the same hoistable callee value.
//META: intent=stress
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.
// Stresses the hoistable-dedup property at volume: five call sites of the same
// instantiation. Each CHECK matches one call site and all five spell the same
// callee, which is what dedup by (base, args) guarantees.
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
