//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#associated-type-bounds
//META: doc_section_digest=71d487355c4bcb7872fbd2df16f024978d48bf90eba4da3f6bd6ba782dd68d15
//META: purpose=An interface-level subtype constraint (`interface IDerived : IBase`) lowers as its own requirement entry whose requirement value is a WitnessTableType — `interface_req_entry(%key, witness_table_t(%IBase))`.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies visitGenericTypeConstraintDecl on a constraint that is a
// direct member of an InterfaceDecl: the inheritance bound becomes a
// requirement in its own right, and its requirement value in the
// interface's entry list is typed witness_table_t of the base
// interface (a conforming type must supply a witness table for IBase).

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

interface IBase
{
    int id();
}

interface IDerived : IBase
{
    int extra();
}

struct Impl : IDerived
{
    int k;
    int id() { return k; }
    int extra() { return k + 1; }
}

uniform int u;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    Impl c;
    c.k = u;
    buf[0] = c.id() + c.extra();
}

// CHECK-DAG: interface_req_entry(%{{[A-Za-z0-9_]+}}, witness_table_t(%IBase))
// CHECK-DAG: let %IDerived : Type = interface
