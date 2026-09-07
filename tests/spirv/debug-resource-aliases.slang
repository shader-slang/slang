// Regression test for https://github.com/shader-slang/slang/issues/12918
// Function-local aliases of opaque resource types (Texture2D, SamplerState) must get
// source-level debug records: an OpString name, a DebugLocalVariable, and a DebugValue
// bound to the loaded opaque SSA handle. No backing Function OpVariable / DebugDeclare is
// expected (illegal in SPIR-V for opaque handles), matching how scalars differ from handles.
//
// Note: -g2 embeds the entire source as one OpString, so the CHECK lines below anchor on the
// real `= OpString "<name>"` (closing quote right after the name, which the source-blob
// OpString cannot satisfy) and on DebugLocalVariable / DebugValue lines, never on bare words.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry debugResourceAliases -stage compute -g2 -O0 -emit-spirv-directly

[[vk::binding(0)]]
Texture2D<float4> inputTexture;

[[vk::binding(1)]]
SamplerState inputSampler;

[[vk::binding(2)]]
RWStructuredBuffer<float4> results;

[shader("compute")]
[numthreads(1, 1, 1)]
void debugResourceAliases(uint3 threadId : SV_DispatchThreadID)
{
    int scalarMarker = 5;
    // Mutable-var aliases (lower to an IRVar).
    Texture2D<float4> textureAlias = inputTexture;
    SamplerState samplerAlias = inputSampler;
    // Immutable let aliases (lower directly to the initializer's SSA handle, no IRVar).
    let textureLet = inputTexture;
    let samplerLet = inputSampler;

    float2 uv = (float2(threadId.xy) + 0.5) / 64.0;
    float4 sampledValue = textureAlias.SampleLevel(samplerAlias, uv, 0.0) +
                          textureLet.SampleLevel(samplerLet, uv, 0.0);
    results[threadId.x] =
        sampledValue + float4(float(scalarMarker) * 0.001, 0.0, 0.0, 0.0);
}

// The mutable-var texture alias gets a named DebugLocalVariable and a DebugValue bound to it.
// CHECK-DAG: %[[TEXNAME:[0-9]+]] = OpString "textureAlias"
// CHECK-DAG: %[[TEXVAR:[A-Za-z0-9_]+]] = OpExtInst %void %{{[0-9]+}} DebugLocalVariable %[[TEXNAME]]
// CHECK-DAG: OpExtInst %void %{{[0-9]+}} DebugValue %[[TEXVAR]]

// The mutable-var sampler alias gets a named DebugLocalVariable and a DebugValue bound to it.
// CHECK-DAG: %[[SMPNAME:[0-9]+]] = OpString "samplerAlias"
// CHECK-DAG: %[[SMPVAR:[A-Za-z0-9_]+]] = OpExtInst %void %{{[0-9]+}} DebugLocalVariable %[[SMPNAME]]
// CHECK-DAG: OpExtInst %void %{{[0-9]+}} DebugValue %[[SMPVAR]]

// The immutable let texture alias likewise gets a DebugLocalVariable + DebugValue.
// CHECK-DAG: %[[TEXLETNAME:[0-9]+]] = OpString "textureLet"
// CHECK-DAG: %[[TEXLETVAR:[A-Za-z0-9_]+]] = OpExtInst %void %{{[0-9]+}} DebugLocalVariable %[[TEXLETNAME]]
// CHECK-DAG: OpExtInst %void %{{[0-9]+}} DebugValue %[[TEXLETVAR]]

// The immutable let sampler alias likewise gets a DebugLocalVariable + DebugValue.
// CHECK-DAG: %[[SMPLETNAME:[0-9]+]] = OpString "samplerLet"
// CHECK-DAG: %[[SMPLETVAR:[A-Za-z0-9_]+]] = OpExtInst %void %{{[0-9]+}} DebugLocalVariable %[[SMPLETNAME]]
// CHECK-DAG: OpExtInst %void %{{[0-9]+}} DebugValue %[[SMPLETVAR]]
