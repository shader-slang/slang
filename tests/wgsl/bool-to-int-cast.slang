//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry computeMain -stage compute

// A cast from `bool` (scalar or vector) to an integer type is emitted with `select`,
// the emitter's idiom for bool-conditioned values. This is the vector companion to the
// scalar coverage in tests/language-feature/interfaces/enum-bool-switch.slang
// (shader-slang/slang#12260).

RWStructuredBuffer<int2> outSigned;
RWStructuredBuffer<uint2> outUnsigned;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    bool2 b = bool2(tid.x == 0, tid.y == 0);

    // WGSL: select(vec2<i32>(0), vec2<i32>(1),
    outSigned[0] = int2(b);

    // WGSL: select(vec2<u32>(0), vec2<u32>(1),
    outUnsigned[0] = uint2(b);
}
