// `-entry` with an overloaded name reports E38017, with a note for each candidate, at the start of
// the module like E38000 (shader-slang/slang#14053). The other lanes compile. In the first one, a
// unique name links only that entry point, so the overloaded `[shader]` functions in this file do
// not collide, and the prototype and the definition of `Other` are one function. In the second,
// slangc merges a repeated adjacent `-entry` into one request. In the third, a request between the
// two keeps both, so `linkIR` lists `Other` twice, and one function listed twice is not a collision.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -entry Count -stage compute
//TEST:SIMPLE(filecheck=OTHER):-target spirv-asm -entry Other -stage compute
//TEST:SIMPLE(filecheck=OTHER):-target spirv-asm -entry Other -stage compute -entry Other -stage compute
//TEST:SIMPLE(filecheck=TWICE):-target spirv-asm -entry Other -stage compute -entry Third -stage compute -entry Other -stage compute

// OTHER-NOT: E38016
// OTHER: OpEntryPoint GLCompute
// OTHER-NOT: OpEntryPoint

// TWICE-NOT: E38016
// TWICE-DAG: OpEntryPoint GLCompute %{{.*}} "Other"
// TWICE-DAG: OpEntryPoint GLCompute %{{.*}} "Third"

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

[shader("compute")]
[numthreads(1, 1, 1)]
void Third(uint3 tid: SV_DispatchThreadID)
{
    outBuf[2] = 1;
}
