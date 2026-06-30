implementing neural;


namespace neural_unittest
{

// ---------------------------------------------------------------------------
// testLoadShA: load matrix A from global buffer to shared memory
// ---------------------------------------------------------------------------

void testLoadShAImpl<
        int InputSize,
        int OutputSize,
        int SubgroupSize,
        ShMemSize : ISharedMemorySize,
        TargetEnum Target,
        bool TransposeA,
        int SubgroupCount>
    (uint sharedMemOffset,
     uint tileIndex,
     RWStructuredBuffer<half>.Handle weightBuffer)
{
    typealias Address = BindlessAddress<half>;
    Address address = Address(weightBuffer);
    MMAHelper<half, InputSize, OutputSize, SubgroupSize, Target, ShMemSize, TransposeA, SubgroupCount>.loadShA<half, Address>(sharedMemOffset, tileIndex, address);
    GroupMemoryBarrierWithWaveSync();
}

public void testLoadShA<
        int InputSize,
        int OutputSize,
        int SubgroupSize,
        ShMemSize : ISharedMemorySize,
        bool TransposeA,
        int SubgroupCount>
    (uint sharedMemOffset,
     uint tileIndex,
     RWStructuredBuffer<half>.Handle weightBuffer)
{
    __target_switch
    {
    case cuda:
        testLoadShAImpl<InputSize, OutputSize, SubgroupSize, ShMemSize, TargetEnum.CUDA, TransposeA, SubgroupCount>(sharedMemOffset, tileIndex, weightBuffer);
    case spirv:
        testLoadShAImpl<InputSize, OutputSize, SubgroupSize, ShMemSize, TargetEnum.SPIR_V, TransposeA, SubgroupCount>(sharedMemOffset, tileIndex, weightBuffer);
    }
}

// ---------------------------------------------------------------------------
// testLoadShB: load vector B to shared memory
// ---------------------------------------------------------------------------

void testLoadShBImpl<
        int InputSize,
        int OutputSize,
        int SubgroupSize,
        ShMemSize : ISharedMemorySize,
        TargetEnum Target,
        int SubgroupCount>
    (uint sharedMemOffset,
     uint tileIndex,
     int subgroupIndex,
     half[InputSize] inputVector)
{
    static const int AlignedSize = ((InputSize + 7) / 8) * 8;
    half aligned[AlignedSize] = {};
    for (int i = 0; i < InputSize; i++)
        aligned[i] = inputVector[i];

    MMAHelper<half, InputSize, OutputSize, SubgroupSize, Target, ShMemSize, false, SubgroupCount>.loadVectorToShB<half, half[AlignedSize]>(sharedMemOffset, tileIndex, subgroupIndex, aligned);
    GroupMemoryBarrierWithWaveSync();
}

public void testLoadShB<
        int InputSize,
        int OutputSize,
        int SubgroupSize,
        ShMemSize : ISharedMemorySize,
        int SubgroupCount>
    (uint sharedMemOffset,
     uint tileIndex,
     int subgroupIndex,
     half[InputSize] inputVector)
{
    __target_switch
    {
    case cuda:
        testLoadShBImpl<InputSize, OutputSize, SubgroupSize, ShMemSize, TargetEnum.CUDA, SubgroupCount>(sharedMemOffset, tileIndex, subgroupIndex, inputVector);
    case spirv:
        testLoadShBImpl<InputSize, OutputSize, SubgroupSize, ShMemSize, TargetEnum.SPIR_V, SubgroupCount>(sharedMemOffset, tileIndex, subgroupIndex, inputVector);
    }
}

// ---------------------------------------------------------------------------
// readSharedMemory: read a uint4 from SharedMemoryPool for verification
// ---------------------------------------------------------------------------

public uint4 readSharedMemory<ShMemSize : ISharedMemorySize>(uint index)
{
    return SharedMemoryPool<ShMemSize>.data[index];
}

public void writeSharedMemory<ShMemSize : ISharedMemorySize>(uint index, uint4 value)
{
    SharedMemoryPool<ShMemSize>.data[index] = value;
}

} // namespace neural_unittest
