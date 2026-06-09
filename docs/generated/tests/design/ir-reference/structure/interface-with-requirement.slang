//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T19:00:00+00:00
//META: source_commit=ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
//META: doc_ref=docs/generated/design/ir-reference/structure.md#interface-internals
//META: doc_section_digest=881ba3684ff6d20a2e1a262e82de634f0b857cc0f62fb483fbb5984cc6845bdf
//META: purpose=An interface declaration lowers to an interface opcode whose operands are interface_req_entry rows pairing a requirement key with the requirement's type.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `interface IComputable { int compute(int x); }` should emit an
// `interface_req_entry(%IComputablex5Fcompute, Func(Int, this_type(%IComputable), Int))`
// followed by `let %IComputable : Type = interface(%1)`. The
// requirement-entry pair (key, requirement-type) is the structural
// row inside an interface's parent.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform int inVal;
uniform RWStructuredBuffer<int> buf;

interface IComputable
{
    int compute(int x);
};

struct Doubler : IComputable
{
    int compute(int x) { return x + x; }
};

[numthreads(1, 1, 1)]
void main()
{
    Doubler d;
    buf[0] = d.compute(inVal);
}

// CHECK: interface_req_entry(%IComputablex5Fcompute, Func(Int, this_type(%IComputable), Int))
// CHECK: let  %IComputable	: Type	= interface(%{{[0-9]+}})
