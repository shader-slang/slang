//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#generic-interface-requirements
//META: doc_section_digest=2baff5b5f1d715938cf9566d6f570582a342e5719c0e3486b541366359660b9d
//META: purpose=Calling a generic interface method through a constrained type parameter lowers to `specialize(lookupWitness(table, key), methodArgs...)` rather than a flat witness lookup that drops the method's generic arguments.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the use-site half of the requirement-local generic encoding. Inside
// the generic function `use`, the call `u.apply<int>(x)` has two independent
// sets of generic arguments: `U` (bound by `use`'s own generic) and `int` (the
// method's). The CHECKs capture the value produced by `lookupWitness` — typed
// `Generic`, which is itself the proof that the entry is not a flat function —
// and require the call to specialize that value with the method's `Int`
// argument before invoking it.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

interface IOps
{
    T apply<T : IArithmetic>(T v);
}

struct S : IOps
{
    T apply<T : IArithmetic>(T v) { return v + v; }
}

uniform int a;
uniform RWStructuredBuffer<int> buf;

int use<U : IOps>(U u, int x)
{
    return u.apply<int>(x);
}

[numthreads(1, 1, 1)]
void main()
{
    S s;
    buf[0] = use(s, a);
}

// CHECK: let [[W:%[A-Za-z0-9_]+]] : Generic = lookupWitness(
// CHECK: func %use
// CHECK: call specialize([[W]], Int,
