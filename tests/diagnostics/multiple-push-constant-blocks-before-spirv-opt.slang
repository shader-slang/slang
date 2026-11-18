// The entry point below reads the global push-constant block only inside
// `extra`, on a path that `extra(false)` never takes. The downstream optimizer
// can prove that read unreachable, but Slang's dead-code elimination keeps it,
// so Slang's own SPIR-V, before spirv-opt runs, lists both push-constant blocks in the
// entry point's interface, so it fails
// VUID-StandaloneSpirv-OpEntryPoint-06674 at every optimization level (and
// spirv-opt does not remove the read at -O0). The GLSL output declares both
// `layout(push_constant)` blocks, which glslang rejects. We therefore report
// the error even though spirv-opt can sometimes hide it (shader-slang/slang#13713).
//
// The entry point also takes an explicitly bound resource parameter, which
// makes parameter binding lay out each entry-point parameter separately.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -entry main -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -entry main -stage compute -O0
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target glsl -entry main -stage compute

struct PC
{
    float4 a;
}

[[vk::push_constant]]
uniform PC push;
//CHECK:   ^^^^ push-constant block `push` is used by this entry point

float extra(bool useGlobal)
{
    if (useGlobal)
        return push.a.x;
    return 0;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main([[vk::binding(3, 2)]] RWStructuredBuffer<float> outBuf, uniform float k)
/*CHECK:
     ^^^^ error E39033
     ^^^^ entry point `main` uses more than one push-constant block
     ^^^^ the push-constant block that holds the `uniform` parameters `k` of `main` is used by this entry point
     ^^^^ combine the data into one
*/
{
    outBuf[0] = k + extra(false);
}

