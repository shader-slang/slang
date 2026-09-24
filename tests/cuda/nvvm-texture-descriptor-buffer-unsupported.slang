// A CUDA buffer descriptor carries a pointer and count, unlike a scalar texture-object handle.
// Integer texture descriptor support must not admit these buffer conversions.

//DIAGNOSTIC_TEST:SIMPLE(diag=FROM): -target ptx -emit-cuda-via-nvvm -entry fromBits -stage compute -capability cuda_sm_8_0 -O0 -o -
//DIAGNOSTIC_TEST:SIMPLE(diag=TO): -target ptx -emit-cuda-via-nvvm -entry toBits -stage compute -capability cuda_sm_8_0 -O0 -o -
//DIAGNOSTIC_TEST:SIMPLE(diag=FROM): -target ptx -emit-cuda-via-nvvm -entry fromBits -stage compute -capability cuda_sm_8_0 -O3 -o -
//DIAGNOSTIC_TEST:SIMPLE(diag=TO): -target ptx -emit-cuda-via-nvvm -entry toBits -stage compute -capability cuda_sm_8_0 -O3 -o -

uint64_t bits;
DescriptorHandle<ByteAddressBuffer> bufferHandle;
RWStructuredBuffer<uint64_t> outputBuffer;

[numthreads(1, 1, 1)]
void fromBits()
{
    ByteAddressBuffer buffer = DescriptorHandle<ByteAddressBuffer>(bits);
    outputBuffer[0] = buffer.Load(0);
}

[numthreads(1, 1, 1)]
void toBits()
{
    outputBuffer[0] = uint64_t(bufferHandle);
}

//FROM: direct NVVM lowering does not support Slang IR instruction or shape 'CastUInt64ToDescriptorHandle'
//TO: direct NVVM lowering does not support Slang IR instruction or shape 'CastDescriptorHandleToUInt64'
