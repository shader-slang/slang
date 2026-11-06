//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry computeMain -stage compute -preserve-params -O0
//TEST:SIMPLE(filecheck=NEG):-target spirv-asm -entry computeMain -stage compute -preserve-params -O0
//TEST:SIMPLE(filecheck=OPT):-target spirv-asm -entry computeMain -stage compute -preserve-params

// At -O0 with -preserve-params, every byte-address buffer that the shader declares but never
// accesses is still declared in the SPIR-V, as the structured buffer that byte-address buffer
// legalization translates it to. The shapes covered are a plain buffer, a read-write one, a
// `globallycoherent` one, sized and unsized arrays, a field of a `ParameterBlock`, a field of a
// global struct, and an entry-point `uniform` parameter. An indexed array is declared once, as
// its translation, with no leftover byte-address buffer array (#14039).

RWStructuredBuffer<uint> outputBuffer;

ByteAddressBuffer unusedBuffer;
RWByteAddressBuffer unusedRWBuffer;
ByteAddressBuffer unusedArray[2];
ByteAddressBuffer unusedUnsizedArray[];
ByteAddressBuffer usedArray[2];
globallycoherent RWByteAddressBuffer unusedCoherentBuffer;

struct BufferPair
{
    ByteAddressBuffer used;
    ByteAddressBuffer unused;
};
ParameterBlock<BufferPair> block;

struct BufferAndValue
{
    ByteAddressBuffer buffer;
    float value;
};
uniform BufferAndValue bufferAndValue;

// At -O0 every unused buffer is still declared with the binding it was given as a
// byte-address buffer, only the read-only ones are `NonWritable`, and `globallycoherent` is kept.

// CHECK-DAG: OpDecorate %unusedBuffer Binding 2
// CHECK-DAG: OpDecorate %unusedBuffer DescriptorSet 0
// CHECK-DAG: OpDecorate %unusedBuffer NonWritable
// CHECK-DAG: OpDecorate %unusedRWBuffer Binding 3
// CHECK-DAG: OpDecorate %unusedArray Binding 4
// CHECK-DAG: OpDecorate %unusedArray NonWritable
// CHECK-DAG: OpDecorate %unusedUnsizedArray Binding 5
// CHECK-DAG: OpDecorate %unusedUnsizedArray NonWritable
// CHECK-DAG: OpDecorate %usedArray Binding 6
// CHECK-DAG: OpDecorate %unusedCoherentBuffer Binding 7
// CHECK-DAG: OpDecorate %unusedCoherentBuffer Coherent
// CHECK-DAG: OpDecorate %block_unused Binding 1
// CHECK-DAG: OpDecorate %block_unused DescriptorSet 1
// CHECK-DAG: OpDecorate %block_unused NonWritable
// CHECK-DAG: OpDecorate %globalParams_bufferAndValue_buffer Binding 8
// CHECK-DAG: OpDecorate %globalParams_bufferAndValue_buffer NonWritable
// CHECK-DAG: OpDecorate %entryPointParams_unusedEntryPointBuffer Binding 9
// CHECK-DAG: OpDecorate %entryPointParams_unusedEntryPointBuffer NonWritable

// CHECK-DAG: %unusedBuffer = OpVariable
// CHECK-DAG: %unusedRWBuffer = OpVariable
// CHECK-DAG: %unusedArray = OpVariable
// CHECK-DAG: %unusedUnsizedArray = OpVariable
// CHECK-DAG: %unusedCoherentBuffer = OpVariable
// CHECK-DAG: %block_unused = OpVariable
// CHECK-DAG: %globalParams_bufferAndValue_buffer = OpVariable
// CHECK-DAG: %entryPointParams_unusedEntryPointBuffer = OpVariable

// The indexed array is declared once, and the read-write buffer is writable.

// NEG: OpName %usedArray "usedArray"
// NEG-NOT: "usedArray"
// NEG-NOT: OpDecorate %unusedRWBuffer NonWritable

// With optimizations, the shader still compiles and declares the indexed array.

// OPT: OpEntryPoint GLCompute %computeMain "main"
// OPT: %usedArray = OpVariable

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID, uniform ByteAddressBuffer unusedEntryPointBuffer)
{
    outputBuffer[0] = usedArray[1].Load(0) + usedArray[tid.x].Load(4) + block.used.Load(0);
}
