//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/structure.md#generic
//META: doc_section_digest=b9f30006218f6bb377aa5edcb42ee78a54d51bcb2a455553e2fcca8e08a738f2
//META: purpose=A generic interface requirement stores a requirement-local generic as its witness-table entry value, so uses read call specialize(lookupWitness(table, key), typeArgs)(...).
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the shape the doc predicts when the requirement itself is generic.
// `IPicker.pick<T>` is a generic method requirement; the CHECK-DAGs assert
// three linked facts: the witness-table row for the requirement holds a
// numbered value (the requirement-local `generic`), the caller looks the
// requirement up with `lookupWitness` producing a value typed `Generic`, and
// the actual invocation is `call specialize(<that lookup>, Int)(...)` rather
// than a flat call on the lookup result.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform int seed;
uniform RWStructuredBuffer<int> buf;

interface IPicker
{
    T pick<T>(T a, T b);
}

struct First : IPicker
{
    T pick<T>(T a, T b) { return a; }
}

int use<P : IPicker>(P p, int x) { return p.pick<int>(x, x + 1); }

[numthreads(1, 1, 1)]
void main()
{
    First f;
    buf[0] = use(f, seed);
}

// CHECK-DAG: witness_table_entry(%IPickerx5Fpick,%{{[0-9]+}})
// CHECK-DAG: let  %{{[0-9]+}}	: Generic	= lookupWitness(%{{[0-9]+}}, %IPickerx5Fpick)
// CHECK-DAG: call specialize(%{{[0-9]+}}, Int)(
