// Regression test for https://github.com/shader-slang/slang/issues/12796.
//
// Both entry points must be compiled together with NO -entry: the crash only reproduces while the
// generic is specialized with its captured pack still an unflattened concatenation, which requires
// keeping the whole module alive. An explicit -entry specializes the generic eagerly and compiles
// cleanly.
//
//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -experimental-feature

import slang.neural;

// Empty `RestDim`: the capture is `makeValuePack(OutputSize, <empty pack>)`, whose flattened arity 1
// disagrees with its raw operand count 2 — the crash case that aborted tuple lowering before the fix.
typealias Conv = NetworkParameterLayoutConverter<float, 1, 4, 2>; // OutputSize=2, RestDim empty

// Depth-2 `RestDim` ({2, 6}): the capture is `makeValuePack(OutputSize, makeValuePack(2, 6))`, so the
// count helper must recurse to a flattened arity of 3. This exercises the recursion path (rather than
// the depth-1 crash above); note it compiles cleanly on both the buggy and fixed compilers — the
// pre-fix under-unroll was a silent dropped iteration with no crash and no stable SPIR-V token — so it
// is coverage for the recursion/no-over-defer path, not a standalone before/after control.
typealias MultiConv = NetworkParameterLayoutConverter<float, 1, 4, 5, 2, 6>;

uniform RWStructuredBuffer<float>.Handle optimal;
uniform RWStructuredBuffer<float>.Handle portable;

[shader("compute")]
[numthreads(1, 1, 1)]
void toOptimal(uint tid : SV_GroupIndex)
{
    Conv.toOptimalLayout(BindlessAddress<float>(portable), BindlessAddress<float>(optimal), tid, 32);
    MultiConv.toOptimalLayout(BindlessAddress<float>(portable), BindlessAddress<float>(optimal), tid, 32);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void toPortable(uint tid : SV_GroupIndex)
{
    Conv.toPortableLayout(BindlessAddress<float>(optimal), BindlessAddress<float>(portable), tid, 32);
    MultiConv.toPortableLayout(BindlessAddress<float>(optimal), BindlessAddress<float>(portable), tid, 32);
}

// CHECK-DAG: OpEntryPoint GLCompute {{.*}} "toOptimal"
// CHECK-DAG: OpEntryPoint GLCompute {{.*}} "toPortable"
