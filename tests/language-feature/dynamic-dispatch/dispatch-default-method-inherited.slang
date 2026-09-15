// Regression test for shader-slang/slang#11487 (the dynamic-dispatch variant of
// shader-slang/slang#12814).
//
// A default method declared on a *base* interface (`IShadingFunction.sample_eval`)
// dispatches to sibling `this` methods, and is called through an existential typed
// as a *derived* interface (`IBSDF : IShadingFunction`). The inherited default-impl
// witness entry on the nested base-interface witness table must be synthesized
// against that base table; otherwise the interior `this`-method lookups resolve to
// null and specialization asserts (slang-ir.cpp(9011)). Same producer root cause as
// #12814, reached through dynamic dispatch instead of static specialization.
//
// The `-cpu` compute run validates the arithmetic for both conformances (catching
// silent mis-resolution); the SPIR-V / HLSL / GLSL compile-only runs guard against
// a crash reintroduction on each backend called out in the original report.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type
//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=GLSL):-target glsl -stage compute -entry computeMain

interface IShadingFunction
{
    float eval(float wi, float wo);
    float sample(float wi);
    float pdf(float wi, float wo);

    // Default method on the BASE interface, dispatching to sibling `this` methods.
    float sample_eval(float wi)
    {
        float wo = sample(wi);
        return eval(wi, wo) / pdf(wi, wo);
    }
}

interface IBSDF : IShadingFunction
{
    float albedo(float wi);
}

struct Lambert : IBSDF
{
    float eval(float wi, float wo) { return wo; }
    float sample(float wi) { return wi; }
    float pdf(float wi, float wo) { return 1.0; }
    float albedo(float wi) { return 0.5; }
}

struct Ggx : IBSDF
{
    float eval(float wi, float wo) { return -wo; }
    float sample(float wi) { return -wi; }
    float pdf(float wi, float wo) { return 2.0; }
    float albedo(float wi) { return 0.8; }
}

IBSDF makeBSDF(int id)
{
    if (id == 0)
        return Lambert();
    return Ggx();
}

// SPV: OpEntryPoint
// HLSL: void {{.*}}computeMain
// GLSL: void {{.*}}main

//TEST_INPUT: set resultBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<float> resultBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    IBSDF lambert = makeBSDF(0);
    IBSDF ggx = makeBSDF(1);

    // Lambert: sample(1)=1, eval(1,1)=1, pdf(1,1)=1 -> sample_eval = 1.0.
    resultBuffer[0] = lambert.sample_eval(1.0);  // CHECK: 1.0
    // Lambert: albedo(1)=0.5.
    resultBuffer[1] = lambert.albedo(1.0);       // CHECK: 0.5
    // Ggx: sample(1)=-1, eval(1,-1)=1, pdf(1,-1)=2 -> sample_eval = 0.5.
    resultBuffer[2] = ggx.sample_eval(1.0);      // CHECK: 0.5
    // Ggx: albedo(1)=0.8.
    resultBuffer[3] = ggx.albedo(1.0);           // CHECK: 0.8
}
