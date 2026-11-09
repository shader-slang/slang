// The entry point never reads its `uniform` parameter, but with `-g` Slang
// still loads it so that a debugger can show its value, and lists its
// push-constant block in the entry point's interface next to the global
// block. That SPIR-V fails VUID-StandaloneSpirv-OpEntryPoint-06674 before
// spirv-opt runs and at -O0, so we report it (shader-slang/slang#13713).
// Without `-g` the parameter's block is unused and no error is reported; see
// tests/spirv/push-constant-single-block-per-entry-point.slang.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -entry main -stage compute -g
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -entry main -stage compute -g -O0

struct PC
{
    float4 a;
}

[[vk::push_constant]]
uniform PC push;
//CHECK:   ^^^^ push-constant block `push` is used by this entry point

RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uniform float unused)
/*CHECK:
     ^^^^ error E39033
     ^^^^ entry point `main` uses more than one push-constant block
     ^^^^ the push-constant block that holds the `uniform` parameters `unused` of `main` is used by this entry point
     ^^^^ combine the data into one
*/
{
    outBuf[0] = push.a.y;
}
