//TEST:SIMPLE(filecheck=CHECK): -target spirv

// CHECK: %[[PTR:[0-9a-zA-Z_]+]] = OpVectorShuffle %v2uint %{{.*}} %{{.*}} 0 1
// CHECK: %{{.*}} = OpUConvert %v2ushort %[[PTR]]

RWTexture2D<uint> tex;

void writeFlags(int2 position, RWTexture2D<uint> flagsTexture, uint flags)
{
    flagsTexture[position] = flags;
}

[shader("compute")]
[numthreads(1,1,1)]
void main(uint3 threadId : SV_DispatchThreadID)
{
    uint16_t2 position = uint16_t2(threadId.xy);
    uint flags = 1;
    writeFlags(position, tex, flags);
}
