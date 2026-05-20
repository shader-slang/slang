//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:13:27Z
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/llm-generated/ir-reference/generics-and-existentials.md#lookupwitness-lookupwitnessmethod
//META: doc_section_digest=c4815aeb9b6c67b5092b523e0e44f337c6c8104ecdaafadc93a47d08bdef573b
//META: purpose=Two distinct interface methods called on the same generic-constrained value produce two distinct lookupWitness sites keyed by different requirement keys.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary on existential-dispatch axis: multi-method dispatch.
// `### lookupWitness / LookupWitnessMethod` prose says the
// second operand is "the `key` that identifies the requirement
// being called". Calling two different methods on the same
// generic-constrained value therefore produces two distinct
// `lookupWitness` instructions, each keyed by its own
// requirement key.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

interface IMulti
{
    int methA(int x);
    int methB(int x);
};

struct S : IMulti
{
    int methA(int x) { return x; }
    int methB(int x) { return x + 1; }
};

uniform int inVal;
uniform RWStructuredBuffer<int> buf;

int helper<T : IMulti>(T t, int x) { return t.methA(x) + t.methB(x); }

[numthreads(1, 1, 1)]
void main()
{
    S s;
    buf[0] = helper<S>(s, inVal);
}

// CHECK: lookupWitness(%{{[0-9]+}}, %IMultix5FmethA)
// CHECK: lookupWitness(%{{[0-9]+}}, %IMultix5FmethB)
