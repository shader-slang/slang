//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04-ast-to-ir.md#associated-type-bounds
//META: doc_section_digest=71d487355c4bcb7872fbd2df16f024978d48bf90eba4da3f6bd6ba782dd68d15
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
