// precompiled-spirv-address-space-specialization.slang

// Address-space specialization of functions exported from, and imported from, a module with
// embedded precompiled SPIR-V (#14004).

//TEST:COMPILE: tests/library/module-library-address-space-specialization.slang -o tests/library/module-library-address-space-specialization.slang-module -target spirv -embed-downstream-ir -incomplete-library -fvk-use-entrypoint-name
//TEST:COMPILE: tests/library/module-library-address-space-specialization-reuse.slang -o tests/library/module-library-address-space-specialization-reuse.slang-module -target spirv -embed-downstream-ir -incomplete-library

// The library's own test checks that its precompiled SPIR-V exports each name once.

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry useMain -stage compute -fvk-use-entrypoint-name
//TEST:SIMPLE(filecheck=SHARED): -target spirv-asm -entry useSharedReceiver -stage compute -fvk-use-entrypoint-name
//TEST:SIMPLE(filecheck=BOTH): -target spirv-asm -entry useBothReceivers -stage compute -fvk-use-entrypoint-name
//TEST:SIMPLE(filecheck=REUSE): -target spirv-asm -entry useLocal -stage compute -fvk-use-entrypoint-name -O0

// This file specializes the imported functions again. The precompiled definitions have the
// unspecialized parameter types, where a pointer without an address space is a `Function`
// pointer. A specialization with that signature, and the linkage, is reduced to an import of the
// precompiled definition; any other specialization keeps its own body. When linking with the
// precompiled SPIR-V fails, slangc writes no SPIR-V, so the entry-point checks fail.
//
// - `useMain` calls `bump` with a local and a `groupshared` element and `Acc.add` on a local.
// - `useSharedReceiver` gives `Acc.add` a single specialization whose `this` is a `Workgroup`
//   pointer, which must not become an import.
// - `useBothReceivers` gives `Acc.add` a `Function` and a `Workgroup` specialization, which both
//   lose the linkage, so neither can become an import.
// - `useLocal` gives the imported `scale` a single `Function` specialization, which reuses the
//   precompiled definition, so the unoptimized output has one function named `scale`.

// CHECK: OpEntryPoint GLCompute %useMain "useMain"

// SHARED: OpEntryPoint GLCompute %useSharedReceiver "useSharedReceiver"

// BOTH: OpEntryPoint GLCompute %useBothReceivers "useBothReceivers"

// REUSE-COUNT-1: OpName %{{[A-Za-z_0-9]+}} "scale"
// REUSE-NOT: OpName %{{[A-Za-z_0-9]+}} "scale"

import "module-library-address-space-specialization";
import "module-library-address-space-specialization-reuse";

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

[shader("compute")]
[numthreads(4, 1, 1)]
void useBothReceivers(uint3 id: SV_DispatchThreadID)
{
    Acc acc = {};
    acc.add(1);
    gAcc.add(2);
    GroupMemoryBarrierWithGroupSync();
    outputBuffer[id.x] = acc.v + gAcc.v;
}

[shader("compute")]
[numthreads(4, 1, 1)]
void useLocal(uint3 id: SV_DispatchThreadID)
{
    float local = id.x;
    scale(local, 5);
    outputBuffer[id.x] = local;
}
