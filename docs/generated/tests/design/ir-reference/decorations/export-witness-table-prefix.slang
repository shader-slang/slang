//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/decorations.md#linkage-and-lifetime
//META: doc_section_digest=71129d05eff64ad258a0ba3326a79769f2e4a43a55b9975b0f92d155b6e49e80
//META: purpose=Boundary - a witness table for an interface-conforming struct carries an export decoration whose mangled name uses the witness-table prefix _SW.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the fourth symbol-kind namespace of the export mangled name.
// Assigning a `Mul2` value to an `IDoubler` existential forces the
// conformance witness table to be lowered and exported; the CHECK pins the
// `_SW` prefix, which is what distinguishes a witness-table export from the
// function (`_SR`), type (`_ST`) and variable (`_SV`) forms the sibling
// tests pin.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

interface IDoubler { int doubled(int x); }

struct Mul2 : IDoubler
{
    int doubled(int x) { return x * 2; }
}

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    Mul2 m;
    IDoubler d = m;
    buf[0] = d.doubled(int(tid.x));
}

// CHECK: [export("_SW{{.*}}")]
