//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Verifies slangc rejects an HLSL `register()` class letter that does not match the declared resource type (E31222) across texture, sampler, buffer and constant-buffer types.
//META: intent=characterization
//META: covers=source/slang/slang-check-decl.cpp
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterization: `checkHLSLRegisterSemantic` cross-checks the register class
// letter against the variable's resource type. Each declaration below pairs a
// resource with the wrong class letter and pins the observed E31222 text,
// including how the type is spelled back in the message. The last two
// declarations use the letter the checker accepts for their type, so the
// exhaustive diagnostic test also pins that a correct pairing is silent.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl

Texture2D readOnlyInUav : register(u0);
//CHECK:                           ^^ E31222
//CHECK:                           ^^ invalid HLSL register name 'u0' for type 'Texture2D<vector<float,4>>'.
RWTexture2D<float4> writableInSrv : register(t1);
//CHECK:                                     ^^ E31222
//CHECK:                                     ^^ invalid HLSL register name 't1' for type 'RWTexture2D<vector<float,4>>'.
SamplerState samplerInSrv : register(t2);
//CHECK:                             ^^ E31222
//CHECK:                             ^^ invalid HLSL register name 't2' for type 'SamplerState'.
ByteAddressBuffer babInUav : register(u3);
//CHECK:                              ^^ E31222
//CHECK:                              ^^ invalid HLSL register name 'u3' for type 'ByteAddressBuffer'.
RWStructuredBuffer<int> rwsbInSrv : register(t4);
//CHECK:                                     ^^ E31222
//CHECK:                                     ^^ invalid HLSL register name 't4' for type 'RWStructuredBuffer<int, DefaultDataLayout>'.
ConstantBuffer<float4> cbInSrv : register(t5);
//CHECK:                                  ^^ E31222
//CHECK:                                  ^^ invalid HLSL register name 't5' for type 'ConstantBuffer<vector<float,4>, DefaultDataLayout>'.

Texture2D readOnlyInSrv : register(t6);
SamplerState samplerInSampler : register(s7);

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
}
