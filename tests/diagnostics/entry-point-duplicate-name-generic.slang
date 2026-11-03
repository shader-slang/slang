// Two specializations of one generic entry point are written under the generic's name, so
// requesting both in one output collides (shader-slang/slang#14053). The error and the note both
// point at the generic declaration.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -entry G<half> -stage compute -entry G<float> -stage compute
//TEST:SIMPLE(filecheck=ONE):-target spirv-asm -entry G<float> -stage compute

// ONE-NOT: E38016
// ONE: OpEntryPoint GLCompute %{{.*}} "main"

RWStructuredBuffer<float> outBuf;

[numthreads(1, 1, 1)]
void G<T : __BuiltinFloatingPointType>(uint3 tid: SV_DispatchThreadID)
/*CHECK:
     ^ more than one entry point would be emitted as 'G' for target
     ^ this entry point would also be emitted as 'G'
     ^ the other entry point emitted as 'G' is here
*/
{
    outBuf[tid.x] = 1.0;
}
