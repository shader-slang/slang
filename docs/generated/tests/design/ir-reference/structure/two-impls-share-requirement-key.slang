//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T19:00:00+00:00
//META: source_commit=ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
//META: doc_ref=docs/generated/design/ir-reference/structure.md#witnesstableentry-vs-interfacereqentry
//META: doc_section_digest=e8a37c42024011b8e8da83a48b8a19297651c7ef6969d0dc113a8d44a20563b1
//META: purpose=Two distinct struct conformances to the same interface produce two distinct witness_tables both keyed by the same requirementKey.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Two implementing types (`Pos` and `Neg`) of the same interface `IS`
// each produce a `witness_table` parented at their own struct, but
// every `witness_table_entry` references the same `%ISx5Frun` key —
// the interface side defines the key once.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

interface IS
{
    int run(int x);
};

struct Pos : IS { int run(int x) { return x + 1; } };
struct Neg : IS { int run(int x) { return -x; } };

uniform int v;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    Pos p;
    Neg n;
    buf[0] = p.run(v) + n.run(v);
}

// CHECK-DAG: witness_table %{{[0-9]+}}	: witness_table_t(%IS)(%Pos)
// CHECK-DAG: witness_table %{{[0-9]+}}	: witness_table_t(%IS)(%Neg)
// CHECK-DAG: witness_table_entry(%ISx5Frun,%Posx5Frun)
// CHECK-DAG: witness_table_entry(%ISx5Frun,%Negx5Frun)
