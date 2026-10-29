// precompiled-spirv-address-space-specialization.slang

// Address-space specialization of functions exported from, and imported from, a module with
// embedded precompiled SPIR-V (#14004).

//TEST:COMPILE: tests/library/module-library-address-space-specialization.slang -o tests/library/module-library-address-space-specialization.slang-module -target spirv -embed-downstream-ir -incomplete-library -fvk-use-entrypoint-name

// The library's own test checks that its precompiled SPIR-V exports each name once.

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry useMain -stage compute -fvk-use-entrypoint-name
//TEST:SIMPLE(filecheck=SHARED): -target spirv-asm -entry useSharedReceiver -stage compute -fvk-use-entrypoint-name

// This file specializes the imported functions again. A specialization never becomes an import
// of the precompiled definition, whose parameter types are the unspecialized ones: `useMain`
// gives `bump` two specializations, which lose their linkage, and `useSharedReceiver` gives
// `Acc.add` a single specialization whose `this` is a `Workgroup` pointer. When linking with the
// precompiled SPIR-V fails, slangc writes no SPIR-V, so these checks fail.

// CHECK: OpEntryPoint GLCompute %useMain "useMain"

// SHARED: OpEntryPoint GLCompute %useSharedReceiver "useSharedReceiver"

import "module-library-address-space-specialization";

RWStructuredBuffer<float> outputBuffer;
groupshared float gValues[4];

[shader("compute")]
[numthreads(4, 1, 1)]
void useMain(uint3 id: SV_DispatchThreadID)
{
    float local = id.x;
    bump(local, 5);
    bump(gValues[id.x], 1);

    Acc acc = {};
    acc.add(2);

    GroupMemoryBarrierWithGroupSync();
    outputBuffer[id.x] = local + gValues[id.x] + acc.v;
}

groupshared Acc gAcc;

[shader("compute")]
[numthreads(4, 1, 1)]
void useSharedReceiver(uint3 id: SV_DispatchThreadID)
{
    gAcc.add(2);
    GroupMemoryBarrierWithGroupSync();
    outputBuffer[id.x] = gAcc.v;
}
