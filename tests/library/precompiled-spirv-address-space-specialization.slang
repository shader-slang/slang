// precompiled-spirv-address-space-specialization.slang

// Address-space specialization of functions exported from, and imported from, a module with
// embedded precompiled SPIR-V (#14004). The library's entry point clones `bump` and `Acc.add`
// while the module is precompiled; the exported originals must stay, so that the precompiled
// SPIR-V exports each of them once. This file clones the imported `bump` again; those clones
// lose their linkage, so they must keep their own bodies instead of becoming imports.

//TEST:COMPILE: tests/library/module-library-address-space-specialization.slang -o tests/library/module-library-address-space-specialization.slang-module -target spirv -embed-downstream-ir -incomplete-library -fvk-use-entrypoint-name
//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry useMain -stage compute -fvk-use-entrypoint-name

// CHECK: OpEntryPoint GLCompute %useMain "useMain"

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
