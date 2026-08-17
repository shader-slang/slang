//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/types.md#resource-and-texture-types
//META: doc_section_digest=9bc7cd21838a70254771d6cf744cc574fc1ff6d4c25215b75c1badb8cf87e771
//META: purpose=An OutputPatch<T,N> domain-shader parameter surfaces the OutputPatch opcode parameterised by element type and elementCount in the IR dump.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// An OutputPatch<T,N> domain-shader parameter surfaces the OutputPatch
// opcode parameterised by element type and elementCount in the IR
// dump. A domain-shader parameter `OutputPatch<HS_OUT, 4>` surfaces
// the `OutputPatch` opcode in the IR dump.

//TEST:SIMPLE(filecheck=IR):-target hlsl -stage domain -entry domainMain -dump-ir -o /dev/null

struct HS_OUT
{
    float3 position : POSITION;
};

struct HSC_OUT
{
    float EdgeTessFactor[4] : SV_TessFactor;
    float InsideTessFactor[2] : SV_InsideTessFactor;
};

struct DS_OUT
{
    float4 position : SV_Position;
};

[domain("quad")]
DS_OUT domainMain(HSC_OUT hsc, float2 uv : SV_DomainLocation, OutputPatch<HS_OUT, 4> patch)
{
    DS_OUT o;
    o.position = float4(patch[0].position, 1);
    return o;
}

// IR-LABEL: func %domainMain
// IR: OutputPatch
