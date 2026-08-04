//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T16:30:00Z
//META: source_commit=110ca68915f1b6690e19a647e3437e1ad1f78a03
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#generics-and-existentials
//META: doc_section_digest=d6ee07799b141a6850f1700c9e4ae870d18815ea897d847d8fe60117b962be60
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
