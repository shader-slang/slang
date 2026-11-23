//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/structure.md#interface-internals
//META: doc_section_digest=631091a3c7dc1e04cb9f969f0a3d91e5c836e06ad2bccf61b2c314a8b98e5d36
//META: purpose=An interface declaration lowers to an interface opcode whose operands are interface_req_entry rows pairing a requirement key with the requirement's type.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that requirements are operands of the `interface` inst, not children
// of it. CHECK first requires the standalone `interface_req_entry` definition
// naming the requirement's key and its `Func(...)` type (with `this_type(...)`
// standing in for the conforming type), then requires the `interface(...)` line
// to reference a numbered entry value.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o - -entry main -stage compute

uniform int inVal;
uniform RWStructuredBuffer<int> buf;

interface IComputable
{
    int compute(int x);
}

struct Doubler : IComputable
{
    int compute(int x) { return x + x; }
}

[numthreads(1, 1, 1)]
void main()
{
    Doubler d;
    buf[0] = d.compute(inVal);
}

// CHECK: interface_req_entry(%IComputablex5Fcompute, Func(Int, this_type(%IComputable), Int))
// CHECK: let  %IComputable	: Type	= interface(%{{[0-9]+}})
