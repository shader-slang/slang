//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#variadic-pack-count-witnesses
//META: doc_section_digest=27de2c3f4422e88c12831854004fb0c605b318f0f9e5ce8e1fd3e5ab1ae208f4
//META: purpose=A `where countof(T) == N` constraint on a variadic generic lowers as a proof-only witness: the generic's parameter list ends with a hidden `param %w : witness_table_t(Void)`, the module holds one `witness_table %t : witness_table_t(Void)(Void);`, and the call site reads `call specialize(%sum, 2 : Int, TypePack(Int, Int), %t)(...)`.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a pack-count constraint carries no runtime data: because the
// front end has already proved `countof(T) == N`, lowering only needs some
// concrete `IRInst` to fill the generic-parameter and call-argument slots, and
// it uses a witness table over `Void`. The test observes all three halves of
// that representation in one dump — the hidden trailing generic parameter, the
// single module-level `witness_table_t(Void)(Void)`, and the call site handing
// that table across as the last specialization argument alongside the value
// `2 : Int` and the deduced `TypePack(Int, Int)`.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

RWStructuredBuffer<int> outputBuffer;

int sum<let N : int, each T>(T values) where countof(T) == N
{
    return N;
}

[numthreads(1, 1, 1)]
void main()
{
    outputBuffer[0] = sum<2>(1, 2);
}

// The constraint becomes a hidden parameter at the end of the generic's list.
// CHECK: param %{{[A-Za-z0-9_]+}}{{.*}}witness_table_t(Void))
// One proof-only table is created at module scope and reused.
// CHECK: witness_table %[[T:[A-Za-z0-9_]+]]{{.*}}witness_table_t(Void)(Void);
// CHECK-LABEL: func %main
// The call site passes the count, the deduced pack, and that same table.
// CHECK: call specialize(%{{[A-Za-z0-9_]+}}, 2 : Int, TypePack(Int, Int), %[[T]])
