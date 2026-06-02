//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:13:48Z
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/generated/design/ir-reference/decorations.md#layout-and-binding
//META: doc_section_digest=53c33072d342ae9430710df507cdde301b5918b46f7119412b00900374c8e222
//META: purpose=A global parameter declared with a register(uN) binding lowers to a HasExplicitHLSLBinding decoration on the parameter.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Per the doc table row for HasExplicitHLSLBinding: "Marks a
// parameter as having an explicit HLSL register binding"; AST
// origin is `register(...)` modifier. The decoration takes no
// operands.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

[[vk::binding(3, 0)]]
uniform RWStructuredBuffer<int> buf : register(u3);

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = int(tid.x);
}

// CHECK: [HasExplicitHLSLBinding]
