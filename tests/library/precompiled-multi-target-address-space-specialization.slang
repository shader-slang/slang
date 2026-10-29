// precompiled-multi-target-address-space-specialization.slang

// Address-space specialization of a function imported from a module with precompiled SPIR-V and
// DXIL (#14004). The imported `Acc.add` carries one `AvailableInDownstreamIR` decoration per
// target. `useSharedReceiver` gives it a single specialization whose `this` is a `Workgroup`
// pointer, which must keep its own body instead of becoming an import of the precompiled SPIR-V
// definition, whose `this` is the unspecialized pointer. When linking with the precompiled SPIR-V
// fails, slangc writes no SPIR-V, so the check fails. Precompiling DXIL needs DXC, so the test
// runs on Windows, as the other precompiled-DXIL tests do.

//TEST(windows):COMPILE: tests/library/module-library-address-space-specialization-multi-target.slang -o tests/library/module-library-address-space-specialization-multi-target.slang-module -target spirv -embed-downstream-ir -target dxil -embed-downstream-ir -profile lib_6_6 -incomplete-library
//TEST(windows):SIMPLE(filecheck=CHECK): -target spirv-asm -entry useSharedReceiver -stage compute

// CHECK: OpEntryPoint GLCompute %useSharedReceiver "main"

import "module-library-address-space-specialization-multi-target";

RWStructuredBuffer<float> outputBuffer;
groupshared Acc gAcc;

[shader("compute")]
[numthreads(4, 1, 1)]
void useSharedReceiver(uint3 id: SV_DispatchThreadID)
{
    gAcc.add(2);
    GroupMemoryBarrierWithGroupSync();
    outputBuffer[id.x] = gAcc.v;
}
