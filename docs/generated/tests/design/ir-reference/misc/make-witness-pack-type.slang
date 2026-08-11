//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/misc.md#makewitnesspack
//META: doc_section_digest=99dbe256ba50c3094f94012201dfec474408118031a5841f5f3f728991acc65e
//META: purpose=`MakeWitnessPack` is typed as a `TypePack` of the individual witness-table types, i.e. a pack of witness tables rather than one witness table over a pack.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the result type of the witness-pack constructor, which is the one
// thing about it that is easy to get backwards. Two distinct conformers of the
// same interface are passed to a variadic generic constrained on that
// interface, so their conformance witnesses must travel together; the CHECK
// requires the inst's type to spell out two `witness_table_t` entries inside a
// `TypePack`, not a single `witness_table_t` over a pack type.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

interface IBar { int getValue(); }
struct BarA : IBar { int v; int getValue() { return v; } }
struct BarB : IBar { int v; int getValue() { return v; } }
struct Wrapper<T : IBar> : IBar { T value; int getValue() { return value.getValue(); } }

int sumBars<each T : IBar>(expand each T values)
{
    int sum = 0;
    expand(sum += (each values).getValue());
    return sum;
}

int wrapAll<each T : IBar>(expand each T vals)
{
    return sumBars<expand Wrapper<each T>>(expand Wrapper<each T>({ each vals }));
}

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void main()
{
    BarA a = { 1 };
    BarB b = { 2 };
    outputBuffer[0] = wrapAll(a, b);
}

// IR: : TypePack(witness_table_t({{.*}}), witness_table_t({{.*}})){{[ 	]*}}= MakeWitnessPack(%{{[A-Za-z0-9_]+}}, %{{[A-Za-z0-9_]+}})
