//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/generics-and-existentials.md#witness-tables-and-witness-facts
//META: doc_section_digest=55d24b9cb1e17ce70457d4ac3ad5c2fd829e24b2244903cc228160630ca43d8f
//META: purpose=When an interface requirement is itself generic the lookup yields a Generic-typed value that the use site wraps in a specialize.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.
// Verifies the composed form a generic requirement forces. `IGen.twice` is a
// generic method, so the witness-table entry holds a generic rather than a
// function; the lookup therefore evaluates to something typed `Generic` and
// cannot be called directly. The CHECKs bind that value and assert the call
// site applies `specialize` to it with the method's own type argument before
// calling.
//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

interface IGen { T twice<T : IArithmetic>(T x); };
struct A : IGen { T twice<T : IArithmetic>(T x) { return x + x; } };

uniform int inVal;
uniform RWStructuredBuffer<int> buf;

int use<U : IGen>(U u, int x) { return u.twice<int>(x); }

[numthreads(1, 1, 1)]
void main()
{
    A a;
    buf[0] = use<A>(a, inVal);
}

// CHECK: let  [[G:%[0-9]+]] : Generic = lookupWitness(%{{[0-9]+}}, %IGenx5Ftwice)
// CHECK: call specialize([[G]], Int,
