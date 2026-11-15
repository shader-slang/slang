//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/structure.md#witnesstableentry-vs-interfacereqentry
//META: doc_section_digest=efe3a8c4431ab5a6b399324347401abcc5166c36a6b9143375ce7938cda0b03d
//META: purpose=Two distinct conformances to one interface produce two distinct witness_tables whose entries both name the single interface-side requirement key.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the requirement key is owned by the interface, not minted per
// conformance. `Pos` and `Neg` each conform to `IS`; the CHECK-DAGs require two
// separate tables, one per concrete type, and require each table's entry to
// name the same `%ISx5Frun` key while pointing at its own implementing
// function.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o - -entry main -stage compute

interface IS
{
    int run(int x);
}

struct Pos : IS { int run(int x) { return x + 1; } }
struct Neg : IS { int run(int x) { return -x; } }

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
