//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -stage compute -entry main -O0 -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -stage compute -entry main -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=ATOMIC): -target spirv-asm -stage compute -entry main -O0 -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=ATOMIC): -target spirv-asm -stage compute -entry main -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=GLSL): -target glsl -stage compute -entry main -validate-ir -validate-ir-detailed

// We evaluate the atomic coordinate calculation once and write only the selected buffer.
// Legalization may duplicate the image store into conditional arms, but speculative stores
// would change the contents of the unselected buffer.
// The separate `ATOMIC` checks cover the complete module so cloned coordinate calculations
// inside either store arm cannot escape the exactly-once requirement.
// ATOMIC: OpAtomicIAdd
// ATOMIC-NOT: OpAtomicIAdd
// SPIRV: OpAtomicIAdd
// SPIRV-NOT: OpAtomicIAdd
// SPIRV: OpBranchConditional
// SPIRV: OpImageWrite
// SPIRV: OpImageWrite
// SPIRV-NOT: OpImageWrite
// GLSL: atomicAdd
// GLSL: if(
// GLSL: imageStore
// GLSL: imageStore

RWBuffer<uint> bufferA;
RWBuffer<uint> bufferB;
RWStructuredBuffer<uint> counter;
cbuffer Parameters { uint chooseA; };

[numthreads(1, 1, 1)]
void main()
{
    RWBuffer<uint> selected = select(chooseA != 0, bufferA, bufferB);
    // We compute the coordinate after the resource selection so that dispatch must remain
    // after the atomic operation rather than move the store to the selection's definition.
    uint coordinate;
    InterlockedAdd(counter[0], 1, coordinate);
    selected[coordinate] = 42;
}
