// An `[__unsafeForceInlineEarly]` function imported from a module precompiled
// WITH `-obfuscate` (shader-slang/slang#14091).
//
// An importer compiled with `-obfuscate` finds the function under the same
// hashed name the module exported it under. An importer compiled without it
// looks for the plain name, and gets an error rather than an assertion failure.

//TEST:COMPILE: tests/obfuscate/precompiled-obfuscated-module-lib.slang -o tests/obfuscate/precompiled-obfuscated-module-lib.slang-module -obfuscate
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type -Xslang -obfuscate
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -Xslang -obfuscate
//DIAGNOSTIC_TEST:SIMPLE(diag=MISMATCH):-target spirv -entry computeMain -stage compute

import "precompiled-obfuscated-module-lib";
/*MISMATCH:
^^^^^^ cannot link 'addOnePrecompiledObf' from module 'precompiled-obfuscated-module-lib'
^^^^^^ 'precompiled-obfuscated-module-lib' was compiled with a different `-obfuscate` setting
*/

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = addOnePrecompiledObf(41);
}

// CHECK: type: int32_t
// CHECK-NEXT: 42
