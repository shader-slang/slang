// An overloaded name passed to `-entry` is ambiguous, so it is reported as such instead of as
// "no function found" (shader-slang/slang#14053). Like E38000, the error is reported at the start
// of the module, and each candidate gets a note. A unique `-entry` name in the same file links
// only that entry point, so the overloaded `[shader]` functions do not collide, and requesting
// the same function twice gives one entry point. A prototype and its definition are one function,
// so `Other` is not ambiguous.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -entry Count -stage compute
//TEST:SIMPLE(filecheck=OTHER):-target spirv-asm -entry Other -stage compute
//TEST:SIMPLE(filecheck=OTHER):-target spirv-asm -entry Other -stage compute -entry Other -stage compute

// OTHER-NOT: E38016
// OTHER: OpEntryPoint GLCompute
// OTHER-NOT: OpEntryPoint

RWStructuredBuffer<uint> outBuf;
/*CHECK:
^^^^^^^^^^^^^^^^^^ entry point name 'Count' is ambiguous
^^^^^^^^^^^^^^^^^^ more than one function matches entry point name 'Count'
*/

[shader("compute")]
[numthreads(1, 1, 1)]
void Count(uniform uint a, uint3 tid: SV_DispatchThreadID)
/*CHECK:
     ^^^^^ candidate: func Count(uint, uint3) -> void
*/
{
    outBuf[0] = a;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void Count(uniform float a, uint3 tid: SV_DispatchThreadID)
/*CHECK:
     ^^^^^ candidate: func Count(float, uint3) -> void
*/
{
    outBuf[0] = uint(a);
}

void Other(uint3 tid: SV_DispatchThreadID);

[shader("compute")]
[numthreads(1, 1, 1)]
void Other(uint3 tid: SV_DispatchThreadID)
{
    outBuf[1] = 1;
}
