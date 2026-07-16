//META: generated=true
//META: model=claude-opus-4-8[1m]
//META: generated_at=2026-06-11T09:43:32Z
//META: source_commit=4a91b58b87e4619ffd76f313e7ae2164d1bd288a
//META: doc_ref=docs/generated/design/ast-reference/modifiers.md#compile-time-hint-attributes-loops-branches-opt-levels
//META: doc_section_digest=bb776d675adb7c4e2a9412ec7d3dac41d5686e57228edbe14f81c6b4581f05ae
//META: purpose=`[unroll]` on a loop appears as an `[unroll]` attribute in emitted HLSL.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Fan-out of the `[unroll]` loop-hint claim onto GLSL. The attribute lowers
// to a `[[unroll]]` control-flow attribute on the emitted loop; CHECK pins
// the emitted `[[unroll]]` token, proving the loop hint survives GLSL codegen.

//TEST:SIMPLE(filecheck=CHECK):-target glsl -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[numthreads(1,1,1)]
void main()
{
    int total = 0;
    [unroll]
    for (int i = 0; i < 4; ++i)
    {
        total += i;
    }
    outBuf[0] = total;
}

//CHECK: {{\[\[}}unroll{{\]\]}}
