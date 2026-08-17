// PROTOTYPE. `-trace-coverage-bindless <space> <index>` synthesizes `__slang_coverage`
// as an unbounded descriptor array of structured buffers instead of a single
// buffer, so that many SEPARATELY COMPILED shaders sharing one pipeline can
// occupy one descriptor binding instead of one binding each.
//
// Shaders compiled together as one program already get disjoint counter slots
// in a single buffer, which is what every other test here exercises — this
// mode exists only for hosts whose shader database compiles each shader on its
// own, so each one's slots restart at 0 and would otherwise collide.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -stage compute -entry computeMain -profile spirv_1_5 -trace-coverage -trace-coverage-bindless 31 7

// A SECOND run with its own prefix, carrying only a -NOT directive so it scans
// the whole disassembly. A `CHECK-NOT` appended to the run above would only
// scan from the last CHECK match to the end of input — and decorations are
// emitted BEFORE the code, so a stray `NonUniform` would sit in the region
// that check never looks at, and the test would pass while asserting nothing.
//TEST:SIMPLE(filecheck=NOUNIFORM):-target spirv-asm -stage compute -entry computeMain -profile spirv_1_5 -trace-coverage -trace-coverage-bindless 31 7

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

// An unbounded array of buffers needs descriptor indexing.
//CHECK-DAG: OpCapability RuntimeDescriptorArray

// One binding for the whole array, at the slot the host pinned. This is the
// point of the mode: not one binding per shader.
//CHECK-DAG: OpDecorate %__slang_coverage Binding 0
//CHECK-DAG: OpDecorate %__slang_coverage DescriptorSet 31

// The global is the ARRAY, not one buffer.
//CHECK: %__slang_coverage = OpVariable %{{.*}}runtimearr{{.*}} StorageBuffer

// Two-level access: select this shader's buffer by the compile-time index,
// then index the counter slot within it.
//CHECK: OpAccessChain %{{.*}} %__slang_coverage %int_7

// The array index is one constant for the whole module, so it is uniform by
// construction. Marking it non-uniform would push every counter increment onto
// the non-uniform descriptor-indexing path for nothing.
//NOUNIFORM-NOT: NonUniform
