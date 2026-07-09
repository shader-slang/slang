//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T16:30:00Z
//META: source_commit=110ca68915f1b6690e19a647e3437e1ad1f78a03
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#generics-and-existentials
//META: doc_section_digest=d6ee07799b141a6850f1700c9e4ae870d18815ea897d847d8fe60117b962be60
//META: purpose=A subtype bound on an associated type (`associatedtype Elem : IBase`) is relocated to interface level and lowers to a requirement entry whose requirement value is `witness_table_t(%IBase)`.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the second shape the doc names for constraint lowering: a
// non-equality subtype constraint that originated nested inside an
// associated type is lowered at interface level, and its requirement
// value in the owning interface's entry list is the witness-table type
// of the bound interface.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

interface IBase
{
    int id();
}

interface IHolder
{
    associatedtype Elem : IBase;
    int consume(Elem e);
}

struct Base7 : IBase
{
    int id() { return 7; }
}

struct Holder : IHolder
{
    typealias Elem = Base7;
    int consume(Base7 e) { return e.id(); }
}

uniform int u;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    Holder h;
    Base7 e;
    buf[0] = h.consume(e) + u;
}

// CHECK-DAG: interface_req_entry(%{{[A-Za-z0-9_]+}}, witness_table_t(%IBase))
// CHECK-DAG: let %IHolder : Type = interface
