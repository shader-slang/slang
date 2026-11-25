//TEST:SIMPLE(filecheck=UNKNOWN-ASM): -target spirv-asm -stage fragment -entry unknownAssembly -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=CALLEE): -target spirv-asm -stage fragment -entry derivativeCallee -O0 -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=MIXED): -target spirv-asm -stage fragment -entry derivativeAndUnknown -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=MIXED-ASM): -target spirv-asm -stage fragment -entry assemblyDerivativeAndUnknown -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=UNKNOWN-CALL): -target spirv -stage fragment -entry unresolvedCallee -incomplete-library -validate-ir -validate-ir-detailed

// Each entry point chooses a texture, then performs one operation that the compiler must copy
// into conditional branches. We require exactly one diagnostic for that original operation.
// The negative checks cover both diagnostics about conditional execution before, between, and
// after the positive checks, so a duplicate warning or the wrong classification cannot pass.
// UNKNOWN-ASM-NOT: warning[E5602{{[01]}}]
// UNKNOWN-ASM: warning[E56021]: resource selection introduces conditional execution
// UNKNOWN-ASM-NOT: warning[E5602{{[01]}}]
// UNKNOWN-ASM: OpFAdd
// UNKNOWN-ASM-NOT: warning[E5602{{[01]}}]
// CALLEE-NOT: warning[E5602{{[01]}}]
// CALLEE: warning[E56020]: resource selection introduces conditional execution
// CALLEE-NOT: warning[E5602{{[01]}}]
// CALLEE: OpFunctionCall
// CALLEE-NOT: warning[E5602{{[01]}}]
// CALLEE: OpImageSampleImplicitLod
// CALLEE-NOT: warning[E5602{{[01]}}]
// MIXED-NOT: warning[E5602{{[01]}}]
// MIXED: warning[E56020]: resource selection introduces conditional execution
// MIXED-NOT: warning[E5602{{[01]}}]
// MIXED: OpFunctionCall
// MIXED-NOT: warning[E5602{{[01]}}]
// MIXED: OpFAdd
// MIXED-NOT: warning[E5602{{[01]}}]
// MIXED: OpImageSampleImplicitLod
// MIXED-NOT: warning[E5602{{[01]}}]
// MIXED-ASM-NOT: warning[E5602{{[01]}}]
// MIXED-ASM: warning[E56020]: resource selection introduces conditional execution
// MIXED-ASM-NOT: warning[E5602{{[01]}}]
// MIXED-ASM: OpFAdd
// MIXED-ASM-NOT: warning[E5602{{[01]}}]
// MIXED-ASM: OpDPdx
// MIXED-ASM-NOT: warning[E5602{{[01]}}]
// UNKNOWN-CALL-NOT: warning[E5602{{[01]}}]
// UNKNOWN-CALL: warning[E56021]: resource selection introduces conditional execution
// UNKNOWN-CALL-NOT: warning[E5602{{[01]}}]

Texture2D<float4> textureA;
Texture2D<float4> textureB;
SamplerState sampler;

// `OpFAdd` is a valid SPIR-V instruction but is outside the diagnostic classifier's explicit
// assembly allow-list. The fetch is recognized as independent; the unrecognized addition must
// still cause the warning about unknown invocation-participation requirements.
float4 unknownAssembly(float2 uv : TEXCOORD) : SV_Target
{
    Texture2D<float4> texture = select(uv.x > 0.5, textureA, textureB);
    int2 coordinate = int2(0);
    int lod = 0;
    return spirv_asm
    {
        %color : $$float4 = OpImageFetch $texture $coordinate Lod $lod;
        result : $$float4 = OpFAdd %color %color;
    };
}

// `[noinline]` keeps a real callee for the classifier to inspect. The warning concerns the
// conditional call, even though the derivative-dependent operation is inside its callee.
[noinline]
float4 sampleInCallee(Texture2D<float4> texture, float2 uv)
{
    return texture.Sample(sampler, uv);
}

float4 derivativeCallee(float2 uv : TEXCOORD) : SV_Target
{
    Texture2D<float4> texture = select(uv.x > 0.5, textureA, textureB);
    return sampleInCallee(texture, uv);
}

// The unknown assembly precedes the sample in this callee. The diagnostic classifier must report
// the recognized derivative requirement even after visiting the unrecognized assembly operation.
[noinline]
float4 sampleAfterUnknownInCallee(Texture2D<float4> texture, float2 uv)
{
    float2 adjusted = spirv_asm
    {
        result : $$float2 = OpFAdd $uv $uv;
    };
    return texture.Sample(sampler, adjusted);
}

float4 derivativeAndUnknown(float2 uv : TEXCOORD) : SV_Target
{
    Texture2D<float4> texture = select(uv.x > 0.5, textureA, textureB);
    return sampleAfterUnknownInCallee(texture, uv);
}

// The same precedence rule also applies within one assembly block. The classifier encounters
// the unrecognized addition before `OpDPdx`, but must still choose the derivative warning.
float4 assemblyDerivativeAndUnknown(float2 uv : TEXCOORD) : SV_Target
{
    Texture2D<float4> texture = select(uv.x > 0.5, textureA, textureB);
    int2 coordinate = int2(0);
    int lod = 0;
    return spirv_asm
    {
        %color : $$float4 = OpImageFetch $texture $coordinate Lod $lod;
        %sum : $$float4 = OpFAdd %color %color;
        result : $$float4 = OpDPdx %sum;
    };
}

// Incomplete-library compilation permits this unresolved declaration to survive linking. There
// is no body to inspect, so the conditional call must receive the unknown-requirement warning.
extern float4 unavailableCallee(Texture2D<float4> texture, float2 uv);

float4 unresolvedCallee(float2 uv : TEXCOORD) : SV_Target
{
    Texture2D<float4> texture = select(uv.x > 0.5, textureA, textureB);
    return unavailableCallee(texture, uv);
}
