// Core-module intrinsics whose `__target_switch` has no case for the target report E41037 when
// they would be generated, instead of compiling to an empty function. Their primary location is in
// the core module, so we check the text rather than the position. PTX is generated from CUDA
// source, so PTX lanes report the target as 'cuda'.

//TEST:SIMPLE(filecheck=SAMPLE):-target cuda -entry sampleLevelWithOffset -stage compute -ignore-capabilities
//TEST:SIMPLE(filecheck=SAMPLE):-target ptx -entry sampleLevelWithOffset -stage compute -ignore-capabilities
//TEST:SIMPLE(filecheck=SAMPLE):-target cuda -entry sampleLevelInLoop -stage compute -ignore-capabilities
//TEST:SIMPLE(filecheck=SAMPLE):-target ptx -entry sampleLevelInLoop -stage compute -ignore-capabilities
//TEST:SIMPLE(filecheck=LOAD):-target cuda -entry typedBufferLoad -stage compute -ignore-capabilities
//TEST:SIMPLE(filecheck=LOAD):-target ptx -entry typedBufferLoad -stage compute -ignore-capabilities
//TEST:SIMPLE(filecheck=LOAD_CPP):-target cpp -entry typedBufferLoad -stage compute -ignore-capabilities

// cpp has no capability diagnostic for `Buffer<T>.Load`; this lane checks E41037 without
// `-ignore-capabilities`.
//TEST:SIMPLE(filecheck=LOAD_CPP):-target cpp -entry typedBufferLoad -stage compute

// `sincos` links `__sincos_metal` from its `metal` case, and `pickTwice` links `metalOnlyTwice`
// from its `metal` case. Both helpers have no case for the other targets, but DCE removes them
// once the caller's switch is resolved, so they must not be diagnosed.
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
//TEST:SIMPLE(filecheck=CLEAN):-target cuda -entry userHelperFromOtherTarget -stage compute -ignore-capabilities
//TEST:SIMPLE(filecheck=CLEAN):-target spirv -entry userHelperFromOtherTarget -stage compute -ignore-capabilities
//TEST:SIMPLE(filecheck=CLEAN):-target hlsl -entry userHelperFromOtherTarget -stage compute -ignore-capabilities

// `SampleLevel` is force-inlined, so the error points at the call in user code.
// SAMPLE: error[E41037]: __target_switch has no case for the current target
// SAMPLE: output[0] = tex.SampleLevel(
// SAMPLE: '_Texture.SampleLevel' contains a __target_switch with no case for target 'cuda'
// SAMPLE-NOT: error[

// LOAD: error[E41037]: __target_switch has no case for the current target
// LOAD: '_Texture.Load' contains a __target_switch with no case for target 'cuda'
// LOAD: note: see call to '_Texture.Load'
// LOAD: output[2] = typedBuf[0];
// LOAD-NOT: error[

// LOAD_CPP: error[E41037]: __target_switch has no case for the current target
// LOAD_CPP: '_Texture.Load' contains a __target_switch with no case for target 'cpp'
// LOAD_CPP: note: see call to '_Texture.Load'
// LOAD_CPP: output[2] = typedBuf[0];
// LOAD_CPP-NOT: error[

// CLEAN: result code = 0
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

float metalOnlyTwice(float x)
{
    __target_switch
    {
    case metal:
        return x * 2;
    }
}

float pickTwice(float x)
{
    __target_switch
    {
    case metal:
        return metalOnlyTwice(x);
    default:
        return x + x;
    }
}

[numthreads(1, 1, 1)]
void userHelperFromOtherTarget()
{
    output[3] = float4(pickTwice(output[0].x));
}

RWStructuredBuffer<int> input;

[numthreads(1, 1, 1)]
void sampleLevelInLoop()
{
    int n = input[0];
    for (int i = 0;; i++)
    {
        if (i >= n)
        {
            output[0] = tex.SampleLevel(samp, float2(0.5, 0.5), 0.0, int2(1, 0));
            break;
        }
    }
}
