//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv

// shader-slang/slang#12493: importing `glsl` must not expose the module's
// internal builtin declarations to user code. `__getLegalizedSPIRVGlobalParamAddr`
// lives in the GLSL module; an explicit `import glsl` brings the name into
// lookup, but the declaration is internal to its module, so the call is
// rejected as inaccessible (E30600). The result is consumed (`*p`) so that a
// reachable intrinsic reaches emit rather than being dead-code-eliminated.

import glsl;

RWStructuredBuffer<float> gOut;

uniform float gVal;

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    let p = __getLegalizedSPIRVGlobalParamAddr(gVal);
//CHECK:    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error E30600
//CHECK:    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ '__getLegalizedSPIRVGlobalParamAddr' is not accessible from the current context.
    gOut[0] = *p;
}
