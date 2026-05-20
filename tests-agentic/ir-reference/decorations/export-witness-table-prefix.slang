//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00Z
//META: source_commit=60db9042207670b39b0524118887efacb7a3d42d
//META: doc_ref=docs/llm-generated/ir-reference/decorations.md#linkage-and-lifetime
//META: doc_section_digest=51a07b0aba2f9f7d1b6cdf9777d9f2f83e7fbd3758ac6f0a6e034ab8f5d4454b
//META: purpose=Boundary - a witness-table for an interface-conforming struct carries an export decoration whose mangled name uses the witness-table prefix _SW, distinct from the function (_SR), type (_ST), and variable (_SV) prefixes.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary - the BUNDLE doc gaps already note that the export
// mangled-name encoding distinguishes _SR / _ST / _SV / _SW per
// symbol kind. This test pins the fourth prefix (_SW) by
// producing a witness-table for an interface implementation.

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
