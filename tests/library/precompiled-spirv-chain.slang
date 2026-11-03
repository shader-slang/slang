// A module that imports another precompiled module precompiles to SPIR-V at `-O1`. Its embedded
// SPIR-V imports `chainBase`, a declaration that SPIRV-Tools dead-branch elimination cannot
// process, so it is not optimized until the final link. The `COMPILE` lines name `-O1` because
// slang-test otherwise passes `-O0`, which skips the optimizer. The final program links both
// precompiled modules and is optimized as a whole.

//TEST:COMPILE: tests/library/precompiled-spirv-chain-base.slang -o tests/library/precompiled-spirv-chain-base.slang-module -target spirv -embed-downstream-ir -profile lib_6_6 -incomplete-library -O1
//TEST:COMPILE: tests/library/precompiled-spirv-chain-mid.slang -o tests/library/precompiled-spirv-chain-mid.slang-module -target spirv -embed-downstream-ir -profile lib_6_6 -incomplete-library -O1
//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry main -stage compute -O1

// CHECK-NOT: Linkage
// CHECK: OpEntryPoint
// CHECK-NOT: Linkage
// CHECK-NOT: OpFunctionCall

import "precompiled-spirv-chain-mid";

[[vk::binding(0, 0)]] RWStructuredBuffer<uint> data;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 id: SV_DispatchThreadID)
{
    data[id.x] = chainMid(id.x);
}
