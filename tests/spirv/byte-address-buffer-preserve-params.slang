//TEST:SIMPLE(filecheck=READONLY):-target spirv-asm -entry computeMain -stage compute -preserve-params -DREADONLY
//TEST:SIMPLE(filecheck=WRITABLE):-target spirv-asm -entry computeMain -stage compute -preserve-params

// A bug 9262 reported that Slang was crashing when `-preserve-params` is used.
// It turned out that `KeepAlive` and `Export` decorations were not properly moved over
// during the legalization process from ByteAddressBuffer to StructuredBuffer.

RWStructuredBuffer<uint> buffer;

#if defined(READONLY)
ByteAddressBuffer byte_buffer;
#else
RWByteAddressBuffer byte_buffer;
#endif

//READONLY: OpName %[[BAB:[A-Za-z_][A-Za-z_0-9]*]] "byte_buffer"
//READONLY: OpDecorate %[[BAB]] Binding 1
//READONLY: OpDecorate %[[BAB]] NonWritable

//WRITABLE: OpName %[[BAB:[A-Za-z_][A-Za-z_0-9]*]] "byte_buffer"
//WRITABLE: OpDecorate %[[BAB]] Binding 1
//WRITABLE-NOT: OpDecorate %[[BAB]] NonWritable

[numthreads(1, 1, 1)]
void computeMain(uint3 i: SV_DispatchThreadID)
{
    buffer[0] = byte_buffer.Load(0);
}
