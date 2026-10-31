// An imported `[__unsafeForceInlineEarly]` function, with the library compiled
// from source by the same `-obfuscate` request (shader-slang/slang#14091).
//
// The library's linkage names are hashed, so the importer must look its
// definition up -- and declare it -- by the hashed name too.

//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type -Xslang -obfuscate
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -Xslang -obfuscate
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -stage compute -obfuscate

import "imported-force-inline-obfuscated-lib";

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = addOneForceInlineObf(41);
    outputBuffer[1] = tripleObf(5);
}

// CHECK: type: int32_t
// CHECK-NEXT: 42
// CHECK-NEXT: 15

// HLSL-NOT: tripleObf
// HLSL: void computeMain()
// HLSL-NOT: tripleObf
