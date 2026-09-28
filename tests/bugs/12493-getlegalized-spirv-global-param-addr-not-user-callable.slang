// shader-slang/slang#12493: `__getLegalizedSPIRVGlobalParamAddr` must not be
// callable from user code. E30015 is a front-end name-lookup error, identical
// on any target; both targets are checked because if the name were reachable
// the emit-time abort differs by target — E99997 on SPIR-V, E99999 on HLSL —
// so each target guards its own crash form. `*p` consumes the result so that a
// reachable intrinsic reaches emit rather than being dead-code-eliminated.
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl

RWStructuredBuffer<float> gOut;

uniform float gVal;

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    let p = __getLegalizedSPIRVGlobalParamAddr(gVal);
//CHECK:    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error E30015
//CHECK:    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ undefined identifier '__getLegalizedSPIRVGlobalParamAddr'.
    gOut[0] = *p;
}
