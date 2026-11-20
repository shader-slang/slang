//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=INVALID,non-exhaustive):-target spirv -entry main -stage compute
//TEST:COMPILE: tests/language-feature/capability/enum-case-require-module/values.slang -o tests/language-feature/capability/enum-case-require-module/cached-values.slang-module
//TEST:SIMPLE(filecheck=CHECK):-r tests/language-feature/capability/enum-case-require-module/cached-values.slang-module -target hlsl -entry main -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=INVALID,non-exhaustive):-r tests/language-feature/capability/enum-case-require-module/cached-values.slang-module -target spirv -entry main -stage compute
// non-exhaustive: capability provenance also emits declaration/use notes.

import "enum-case-require-module/values";

RWStructuredBuffer<int> outputBuffer;

//CHECK: void main
[numthreads(1, 1, 1)]
void main()
//INVALID: entrypoint 'main' uses features that are not available
{
    outputBuffer[0] = consume(TargetValue.hlslOnly);
    outputBuffer[1] = consume(TargetValue(0u));
}
