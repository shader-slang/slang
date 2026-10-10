// Core-module intrinsics whose `__target_switch` has no case for the target report E41037 when
// they are reached, instead of compiling to an empty function. Their primary location is in the
// core module, so we check the text rather than the position.

//TEST:SIMPLE(filecheck=SAMPLE):-target cuda -entry sampleLevelWithOffset -stage compute -ignore-capabilities
//TEST:SIMPLE(filecheck=SAMPLE):-target ptx -entry sampleLevelWithOffset -stage compute -ignore-capabilities
//TEST:SIMPLE(filecheck=LOAD):-target cuda -entry typedBufferLoad -stage compute -ignore-capabilities
//TEST:SIMPLE(filecheck=LOAD):-target ptx -entry typedBufferLoad -stage compute -ignore-capabilities
//TEST:SIMPLE(filecheck=LOAD):-target cpp -entry typedBufferLoad -stage compute -ignore-capabilities

// The cpp target has no capability error for `Buffer<T>.Load`, so E41037 is also reported
// without `-ignore-capabilities`.
//TEST:SIMPLE(filecheck=LOAD):-target cpp -entry typedBufferLoad -stage compute

// Core-module functions with target-specific switches that are linked into every target, such
// as `__sincos_metal`, must not be diagnosed when they are not called on the current target.
//TEST:SIMPLE(filecheck=CLEAN):-target cuda -entry sinCos -stage compute -ignore-capabilities
//TEST:SIMPLE(filecheck=CLEAN):-target ptx -entry sinCos -stage compute -ignore-capabilities
//TEST:SIMPLE(filecheck=CLEAN):-target cpp -entry sinCos -stage compute -ignore-capabilities
//TEST:SIMPLE(filecheck=CLEAN):-target spirv -entry sinCos -stage compute -ignore-capabilities
//TEST:SIMPLE(filecheck=CLEAN):-target glsl -entry sinCos -stage compute -ignore-capabilities
//TEST:SIMPLE(filecheck=CLEAN):-target hlsl -entry sinCos -stage compute -ignore-capabilities
//TEST:SIMPLE(filecheck=CLEAN):-target metal -entry sinCos -stage compute -ignore-capabilities
//TEST:SIMPLE(filecheck=CLEAN):-target wgsl -entry sinCos -stage compute -ignore-capabilities
//TEST:SIMPLE(filecheck=CLEAN):-target cuda -entry sinCos -stage compute
//TEST:SIMPLE(filecheck=CLEAN):-target spirv -entry sinCos -stage compute

// A function that has no case for the target but is never called is removed before the check.
//TEST:SIMPLE(filecheck=CLEAN):-target cuda -entry unusedMetalOnly -stage compute -ignore-capabilities
//TEST:SIMPLE(filecheck=CLEAN):-target spirv -entry unusedMetalOnly -stage compute -ignore-capabilities

// `SampleLevel` is force-inlined, so the error points at the call in user code.
// SAMPLE: error[E41037]: no implementation of '_Texture.SampleLevel' for the current target
// SAMPLE: output[0] = tex.SampleLevel(
// SAMPLE: has no implementation for target '{{cuda|ptx}}'

// LOAD: error[E41037]: no implementation of '_Texture.Load' for the current target
// LOAD: has no implementation for target '{{cuda|ptx|cpp}}'
// LOAD: note: see call to '_Texture.Load'
// LOAD: output[2] = typedBuf[0];

// CLEAN-NOT: error[

Texture2D<float4> tex;
SamplerState samp;
Buffer<float4> typedBuf;
RWStructuredBuffer<float4> output;

[numthreads(1, 1, 1)]
void sampleLevelWithOffset()
{
    output[0] = tex.SampleLevel(samp, float2(0.5, 0.5), 0.0, int2(1, 0));
}

[numthreads(1, 1, 1)]
void typedBufferLoad()
{
    output[2] = typedBuf[0];
}

[numthreads(1, 1, 1)]
void sinCos()
{
    float s, c;
    sincos(output[0].x, s, c);
    output[1] = float4(s + c);
}

float metalOnly(float x)
{
    __target_switch
    {
    case metal:
        return x * 2;
    }
}

[numthreads(1, 1, 1)]
void unusedMetalOnly()
{
    output[3] = float4(1);
}
