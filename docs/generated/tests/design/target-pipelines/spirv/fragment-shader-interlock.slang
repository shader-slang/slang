//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools
//META: doc_section_digest=9bf971e79309b04b98ac1b07e618148a41a5c833af2e6a7f18f5e4b57eddceb9
//META: purpose=A fragment entry point that uses a RasterizerOrderedTexture2D inside beginInvocationInterlock/endInvocationInterlock triggers insertFragmentShaderInterlock — the SPIR-V emit declares the FragmentShaderPixelInterlockEXT capability and adds OpExecutionMode %main PixelInterlockOrderedEXT.
//META: intent=expansion
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: insertFragmentShaderInterlock (Phase D, step 9) only
// acts on raster-ordered resources in fragment entry points. The
// pass injects the OpBeginInvocationInterlockEXT /
// OpEndInvocationInterlockEXT terminator pair and the PixelInterlock
// execution mode so the runtime serializes overlapping fragments.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage fragment

RasterizerOrderedTexture2D<float4> tex;

[shader("fragment")]
float4 main(float4 coords : COORDS) : SV_Target
{
    float4 result;
    beginInvocationInterlock();
    {
        result = tex[uint2(coords.xy)];
        tex[uint2(coords.xy)] = result + coords;
    }
    endInvocationInterlock();
    return result;
}

// CHECK-DAG: OpCapability FragmentShaderPixelInterlockEXT
// CHECK: OpExecutionMode %main PixelInterlockOrderedEXT
