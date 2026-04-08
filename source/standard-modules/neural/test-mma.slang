implementing neural;

#include "common-def.slang"

// Internal test functions for MMA operations.
// Included into neural.slang only when UNIT_TEST is defined.

public DType[OutputSize] testMatVecMul<DType : __BuiltinFloatingPointType, int InputSize, int OutputSize, ShMemSize : ISharedMemorySize, TargetEnum Target, bool Bias>(
    uint tid,
    RWStructuredBuffer<DType>.Handle weightBuffer,
    Optional<RWStructuredBuffer<DType>.Handle> biasBuffer)
    where DType.Differential == DType
{
    typealias Address = BindlessAddress<DType>;
    Address address = Address(weightBuffer);

    typealias MMA = MMAHelper<DType, InputSize, OutputSize, 32, Target, ShMemSize, false, 1>;
    const int InSize = MMA.Uint4AlignedK;
    const int OutSize = MMA.Uint4AlignedM;

    DType inputVector[InSize] = {};
    for (int i = 0; i < InputSize; i++)
        inputVector[i] = DType(tid + 1);

    DType alignedOutput[OutSize];
    if (Bias)
    {
        Address biasAddress = Address(biasBuffer.value);
        alignedOutput = MMA.mma<DType, Address, DType[InSize], DType[OutSize], DType, true>(
            inputVector, 0, 256, 256, address, biasAddress);
    }
    else
    {
        alignedOutput = MMA.mma<DType, Address, DType[InSize], DType[OutSize], DType, false>(
            inputVector, 0, 256, 256, address, none);
    }

    DType output[OutputSize];
    for (int i = 0; i < OutputSize; i++)
        output[i] = alignedOutput[i];
    return output;
}
