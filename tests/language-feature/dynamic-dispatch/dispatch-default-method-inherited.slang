// Default method on a base interface, dispatched through an existential
// typed as a derived interface and constructed via createDynamicObject.
// Regression for shader-slang/slang#11487: the dynamic-dispatch
// specialization pass used to crash because the inherited default
// method's body issues `this`-method lookups, and the witness-table
// lookup on the derived interface returned null when the entry
// actually lives in a nested base-interface witness table.
//
// The issue body's exact shape (createDynamicObject<IBSDF> + conformance
// args) is mirrored here. The `-cpu` COMPARE_COMPUTE run validates
// arithmetic for both conformances and would catch silent
// mis-resolution (e.g. dispatching the inherited default method to the
// wrong witness entry). The SIMPLE directives that follow only assert
// "the shader codegens"; they guard against a compile-time crash
// reintroduction on each of the three target backends called out in
// the original report, not against silent mis-resolution.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type
//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -stage compute -entry computeMain -conformance "Lambert:IBSDF" -conformance "Ggx:IBSDF"
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry computeMain -conformance "Lambert:IBSDF" -conformance "Ggx:IBSDF"
//TEST:SIMPLE(filecheck=GLSL):-target glsl -stage compute -entry computeMain -conformance "Lambert:IBSDF" -conformance "Ggx:IBSDF"

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<float> outputBuffer;

interface IShadingFunction
{
    float eval(float wi, float wo);
    float sample(float wi);
    float pdf(float wi, float wo);

    // Default method on the BASE interface, dispatching to sibling `this`
    // methods.
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

// SPV: OpEntryPoint
// HLSL: void {{.*}}computeMain
// GLSL: void {{.*}}main

//TEST_INPUT: type_conformance Lambert:IBSDF = 0
//TEST_INPUT: type_conformance Ggx:IBSDF = 1

//TEST_INPUT: cbuffer(data=[0]):name=type_id
uniform int type_id;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    // Both existentials go through createDynamicObject so the typeflow
    // specializer sees a runtime-tagged dispatch for each. With type_id=0,
    // bsdfA is Lambert and bsdfB is Ggx, so the four outputs exercise both
    // conformances against both the inherited default method (sample_eval)
    // and the derived-interface direct method (albedo).
    IBSDF bsdfA = createDynamicObject<IBSDF>(type_id, 0);
    IBSDF bsdfB = createDynamicObject<IBSDF>(1 - type_id, 0);

    // Lambert: sample(1)=1, eval(1,1)=1, pdf(1,1)=1 -> sample_eval = 1.0.
    outputBuffer[0] = bsdfA.sample_eval(1.0);  // CHECK: 1.0
    // Lambert: albedo(1)=0.5.
    outputBuffer[1] = bsdfA.albedo(1.0);       // CHECK: 0.5
    // Ggx: sample(1)=-1, eval(1,-1)=1, pdf(1,-1)=2 -> sample_eval = 0.5.
    outputBuffer[2] = bsdfB.sample_eval(1.0);  // CHECK: 0.5
    // Ggx: albedo(1)=0.8.
    outputBuffer[3] = bsdfB.albedo(1.0);       // CHECK: 0.8
}
