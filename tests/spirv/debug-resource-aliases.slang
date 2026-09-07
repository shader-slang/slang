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

// A helper taking opaque resources by value, kept as a real function so its parameters exercise
// the parameter gate in insertDebugValueStore (which runs before general inlining).
[noinline]
float4 sampleThroughParams(Texture2D<float4> texParam, SamplerState sampParam, float2 uv)
{
    return texParam.SampleLevel(sampParam, uv, 0.0);
}

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
                          textureLet.SampleLevel(samplerLet, uv, 0.0) +
                          sampleThroughParams(inputTexture, inputSampler, uv);
    results[threadId.x] =
        sampledValue + float4(float(scalarMarker) * 0.001, 0.0, 0.0, 0.0);
}

// Group 1 — the OpString names (emitted early, in the debug-string section).
// CHECK-DAG: %[[TEXNAME:[0-9]+]] = OpString "textureAlias"
// CHECK-DAG: %[[SMPNAME:[0-9]+]] = OpString "samplerAlias"
// CHECK-DAG: %[[TEXLETNAME:[0-9]+]] = OpString "textureLet"
// CHECK-DAG: %[[SMPLETNAME:[0-9]+]] = OpString "samplerLet"
// CHECK-DAG: %[[TEXPNAME:[0-9]+]] = OpString "texParam"
// CHECK-DAG: %[[SMPPNAME:[0-9]+]] = OpString "sampParam"

// No opaque-handle alias may get a backing Function OpVariable — that would be illegal SPIR-V.
// A backing var carries an `OpName %id "_dbgvar_<name>"` (scalars get one, e.g. _dbgvar_scalarMarker),
// so its absence for the handles locks the core invariant. These CHECK-NOTs sit between the OpString
// group above and the DebugLocalVariable/DebugValue group below, so their region spans the OpName
// section where any such backing-var name would appear. (The -g2 source blob escapes quotes as \",
// so it cannot match the bare-quote OpName literal here.)
// CHECK-NOT: OpName {{%[A-Za-z0-9_]+}} "_dbgvar_textureAlias"
// CHECK-NOT: OpName {{%[A-Za-z0-9_]+}} "_dbgvar_samplerAlias"
// CHECK-NOT: OpName {{%[A-Za-z0-9_]+}} "_dbgvar_textureLet"
// CHECK-NOT: OpName {{%[A-Za-z0-9_]+}} "_dbgvar_samplerLet"
// CHECK-NOT: OpName {{%[A-Za-z0-9_]+}} "_dbgvar_texParam"
// CHECK-NOT: OpName {{%[A-Za-z0-9_]+}} "_dbgvar_sampParam"

// Group 2 — each alias gets a DebugLocalVariable bound to its OpString name, plus a DebugValue.
// CHECK-DAG: %[[TEXVAR:[A-Za-z0-9_]+]] = OpExtInst %void %{{[0-9]+}} DebugLocalVariable %[[TEXNAME]]
// CHECK-DAG: OpExtInst %void %{{[0-9]+}} DebugValue %[[TEXVAR]]
// CHECK-DAG: %[[SMPVAR:[A-Za-z0-9_]+]] = OpExtInst %void %{{[0-9]+}} DebugLocalVariable %[[SMPNAME]]
// CHECK-DAG: OpExtInst %void %{{[0-9]+}} DebugValue %[[SMPVAR]]
// CHECK-DAG: %[[TEXLETVAR:[A-Za-z0-9_]+]] = OpExtInst %void %{{[0-9]+}} DebugLocalVariable %[[TEXLETNAME]]
// CHECK-DAG: OpExtInst %void %{{[0-9]+}} DebugValue %[[TEXLETVAR]]
// CHECK-DAG: %[[SMPLETVAR:[A-Za-z0-9_]+]] = OpExtInst %void %{{[0-9]+}} DebugLocalVariable %[[SMPLETNAME]]
// CHECK-DAG: OpExtInst %void %{{[0-9]+}} DebugValue %[[SMPLETVAR]]
// CHECK-DAG: %[[TEXPVAR:[A-Za-z0-9_]+]] = OpExtInst %void %{{[0-9]+}} DebugLocalVariable %[[TEXPNAME]]
// CHECK-DAG: OpExtInst %void %{{[0-9]+}} DebugValue %[[TEXPVAR]]
// CHECK-DAG: %[[SMPPVAR:[A-Za-z0-9_]+]] = OpExtInst %void %{{[0-9]+}} DebugLocalVariable %[[SMPPNAME]]
// CHECK-DAG: OpExtInst %void %{{[0-9]+}} DebugValue %[[SMPPVAR]]
