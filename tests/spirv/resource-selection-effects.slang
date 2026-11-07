//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -stage compute -entry main -O0 -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -stage compute -entry main -validate-ir -validate-ir-detailed -O1
//TEST:SIMPLE(filecheck=ATOMIC): -target spirv-asm -stage compute -entry main -O0 -validate-ir -validate-ir-detailed
//TEST:SIMPLE(filecheck=ATOMIC): -target spirv-asm -stage compute -entry main -validate-ir -validate-ir-detailed -O1
//TEST:SIMPLE(filecheck=GLSL): -target glsl -stage compute -entry main -validate-ir -validate-ir-detailed

// We evaluate the atomic coordinate calculation once and write only the selected buffer.
// Legalization may duplicate the image store into conditional arms, but speculative stores
// would change the contents of the unselected buffer.
// The separate `ATOMIC` checks cover the complete module so cloned coordinate calculations
// inside either store arm cannot escape the exactly-once requirement.
// ATOMIC-NOT: OpImageWrite
// ATOMIC: OpAtomicIAdd
// ATOMIC-NOT: Op{{(AtomicIAdd|ImageWrite)}}
// ATOMIC: OpImageWrite
// ATOMIC-NOT: Op{{(AtomicIAdd|ImageWrite)}}
// ATOMIC: OpImageWrite
// ATOMIC-NOT: Op{{(AtomicIAdd|ImageWrite)}}
// SPIRV: %[[CHOICE_POINTER:[^ ]+]] = OpAccessChain {{[^ ]+}} %Parameters %int_0{{( |$)}}
// SPIRV: %[[CHOICE:[^ ]+]] = OpLoad %uint %[[CHOICE_POINTER]]{{( |$)}}
// SPIRV: %[[CONDITION:[^ ]+]] = OpINotEqual %bool %[[CHOICE]] %uint_0{{( |$)}}
// SPIRV: %[[COUNTER:[^ ]+]] = OpAccessChain {{[^ ]+}} %counter %int_0 %int_0{{( |$)}}
// SPIRV: %[[COORDINATE:[^ ]+]] = OpAtomicIAdd %uint %[[COUNTER]] %uint_1 %uint_0 %uint_1{{( |$)}}
// SPIRV-NOT: OpAtomicIAdd
// SPIRV: OpSelectionMerge %[[MERGE:[^ ]+]] None
// SPIRV: OpBranchConditional %[[CONDITION]] %[[THEN:[^ ]+]] %[[ELSE:[^ ]+]]{{( |$)}}
// SPIRV: %[[THEN]] = OpLabel
// SPIRV: %[[BUFFER_A:[^ ]+]] = OpLoad {{[^ ]+}} %bufferA{{( |$)}}
// SPIRV: OpImageWrite %[[BUFFER_A]] %[[COORDINATE]]{{( |$)}}
// SPIRV: OpBranch %[[MERGE]]{{( |$)}}
// SPIRV: %[[ELSE]] = OpLabel
// SPIRV: %[[BUFFER_B:[^ ]+]] = OpLoad {{[^ ]+}} %bufferB{{( |$)}}
// SPIRV: OpImageWrite %[[BUFFER_B]] %[[COORDINATE]]{{( |$)}}
// SPIRV: OpBranch %[[MERGE]]{{( |$)}}
// SPIRV: %[[MERGE]] = OpLabel
// SPIRV-NOT: OpImageWrite
// GLSL-NOT: {{(atomicAdd|imageStore)}}
// GLSL-DAG: uniform uimageBuffer [[BUFFER_A:bufferA[^; ]*]];
// GLSL-DAG: uniform uimageBuffer [[BUFFER_B:bufferB[^; ]*]];
// GLSL-NOT: {{(atomicAdd|imageStore)}}
// GLSL: bool [[CONDITION:[^ ]+]] = ({{Parameters[^.]*}}.{{chooseA[^)]*}}) != 0U;
// GLSL-NOT: {{(atomicAdd|imageStore)}}
// GLSL: uint [[COORDINATE:[^ ]+]] = atomicAdd
// GLSL-NOT: {{(atomicAdd|imageStore)}}
// GLSL: if([[CONDITION]])
// GLSL-NOT: {{(atomicAdd|imageStore)}}
// GLSL: imageStore(([[BUFFER_A]]), int(([[COORDINATE]])),
// GLSL-NOT: {{(atomicAdd|imageStore)}}
// GLSL: else
// GLSL-NOT: {{(atomicAdd|imageStore)}}
// GLSL: imageStore(([[BUFFER_B]]), int(([[COORDINATE]])),
// GLSL-NOT: {{(atomicAdd|imageStore)}}

RWBuffer<uint> bufferA;
RWBuffer<uint> bufferB;
RWStructuredBuffer<uint> counter;
cbuffer Parameters { uint chooseA; };

[numthreads(1, 1, 1)]
void main()
{
    RWBuffer<uint> selected = select(chooseA != 0, bufferA, bufferB);
    // We compute the coordinate after choosing the buffer. When `specializeResourceSelections`
    // branches around the store, both store instructions must follow the atomic operation so
    // they use its result without repeating its side effect.
    uint coordinate;
    InterlockedAdd(counter[0], 1, coordinate);
    selected[coordinate] = 42;
}
