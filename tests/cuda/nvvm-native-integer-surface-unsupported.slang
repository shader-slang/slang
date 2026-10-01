// Direct NVVM deliberately supports only scalar, two-lane, and four-lane native integer surface
// payloads. A three-lane payload must not be padded into a different physical operation.

//DIAGNOSTIC_TEST:SIMPLE(diag=INT3): -target ptx -emit-cuda-via-nvvm -entry int3Main -stage compute -capability cuda_sm_7_0 -o nvvm-native-integer-surface-int3-unsupported.ptx

RWTexture2D<int3> int3Texture;
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void int3Main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    outputBuffer[0] = int3Texture[int2(dispatchThreadID.xy)].x;
}

//INT3: direct NVVM lowering does not support Slang IR instruction or shape 'imageLoad'

// Native32 1D arrays are supported. Half arrays still have no selected physical surface row.

//DIAGNOSTIC_TEST:SIMPLE(diag=ARRAY): -target ptx -emit-cuda-via-nvvm -entry arrayMain -stage compute -capability cuda_sm_7_0 -o nvvm-native-integer-surface-array-unsupported.ptx

RWTexture1DArray<half4> arrayTexture;

[numthreads(1, 1, 1)]
void arrayMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    outputBuffer[0] = int(arrayTexture[int2(dispatchThreadID.xy)].x);
}

//ARRAY: direct NVVM lowering does not support Slang IR instruction or shape 'imageLoad'
