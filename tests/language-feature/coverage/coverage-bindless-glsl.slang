// GLSL companion to coverage-bindless-spirv.slang. `isKhronosTarget` admits
// GLSL as well as SPIR-V, so the bindless form is an enabled path there and
// needs its own coverage — the SPIR-V test alone would let a GLSL-side
// regression through while the guard still claimed the target was supported.
//
// GLSL spells the same thing differently: an unsized array of shader storage
// blocks, where the block's own trailing `uint64_t _data[]` is the structured
// buffer's contents and the OUTER `[]` is the descriptor array.

//TEST:SIMPLE(filecheck=CHECK):-target glsl -stage compute -entry computeMain -profile glsl_460 -trace-coverage -trace-coverage-binding 0 31 -trace-coverage-bindless-index 7
//TEST:SIMPLE(filecheck=NONZERO):-target glsl -stage compute -entry computeMain -profile glsl_460 -trace-coverage -trace-coverage-binding 5 31 -trace-coverage-bindless-index 7
//TEST:SIMPLE(filecheck=NOUNIFORM):-target glsl -stage compute -entry computeMain -profile glsl_460 -trace-coverage -trace-coverage-bindless-index 7

RWStructuredBuffer<uint> outBuf;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    if (tid.x == 0)
    {
        outBuf[0] = 1;
    }
    else
    {
        outBuf[0] = 2;
    }
}

// One binding for the whole array, at the (binding, space) the host pinned.
// The binding is 0 here only because this run asks for 0 -- an unsized array
// does NOT force binding 0, and a non-zero pinned binding is honored; the
// SPIR-V companion pins that with an explicit `Binding 5` run.
//CHECK: layout(std430, binding = 0, set = 31) buffer
// The trailing `[]` after the block instance name is the descriptor array;
// without it this would be a single buffer again.
//CHECK: }{{.*}}[];

// GLSL emission is an independent path from SPIR-V, so it needs the same two
// guards the SPIR-V companion carries.
//
// `binding = 0` above proves nothing on its own: auto-allocation also lands
// the array at binding 0, so only the set carries signal there. Pinning a
// non-zero binding is what shows `-trace-coverage-binding` composes with the
// bindless form on this path too.
//NONZERO: layout(std430, binding = 5, set = 31) buffer

// The array index is a compile-time constant and therefore uniform, so the
// access must NOT be wrapped in `nonuniformEXT` -- doing so would push every
// counter update onto the non-uniform descriptor-indexing path for no reason.
// This needs its own prefix so the -NOT scans the whole output.
//NOUNIFORM-NOT: nonuniformEXT
