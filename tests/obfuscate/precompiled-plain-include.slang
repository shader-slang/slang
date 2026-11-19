// A module precompiled WITHOUT `-obfuscate` and imported from an `__include`d
// file of an importer compiled with `-obfuscate` (shader-slang/slang#14091).
//
// E45002 points at the `import` in the included file, and it is reported once
// for the module even though two of its force-inline functions are used.

//TEST:COMPILE: tests/obfuscate/precompiled-plain-include-lib.slang -o tests/obfuscate/precompiled-plain-include-lib.slang-module
//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry computeMain -stage compute -obfuscate

module "precompiled-plain-include";

__include "precompiled-plain-include-part.slang";

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = addThree(39);
}

// CHECK: E45002
// CHECK-SAME: cannot link
// CHECK-SAME: 'precompiled-plain-include-lib'
// CHECK: precompiled-plain-include-part.slang:5:1
// CHECK-NOT: E45002
