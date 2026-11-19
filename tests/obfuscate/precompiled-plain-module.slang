// An `[__unsafeForceInlineEarly]` function imported from a module precompiled
// WITHOUT `-obfuscate`, the way the standard modules are built
// (shader-slang/slang#14091).
//
// An importer compiled without `-obfuscate` finds the function under its plain
// name. An importer compiled with `-obfuscate` looks for the hashed name, and
// gets an error rather than a crash.

//TEST:COMPILE: tests/obfuscate/precompiled-plain-module-lib.slang -o tests/obfuscate/precompiled-plain-module-lib.slang-module
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type
//DIAGNOSTIC_TEST:SIMPLE(diag=MISMATCH):-target spirv -entry computeMain -stage compute -obfuscate

import "precompiled-plain-module-lib";
/*MISMATCH:
^^^^^^ cannot link 'addOnePrecompiledPlain' from module 'precompiled-plain-module-lib'
^^^^^^ 'precompiled-plain-module-lib' was compiled with a different `-obfuscate` setting
*/

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = addOnePrecompiledPlain(41);
}

// CHECK: type: int32_t
// CHECK-NEXT: 42
