//TEST:SIMPLE(filecheck=METAL): -stage compute -entry computeMain -target metal

// Regression test for https://github.com/shader-slang/slang/issues/11837
//
// Floating-point literals emitted for the Metal target must carry an MSL type
// suffix so that the literal's type is preserved. Metal Shading Language reads
// a bare decimal such as `61440.0` as a 64-bit `double`; without an `h`/`f`
// suffix a half/float literal silently becomes a double. That breaks
// size-sensitive contexts: e.g. `as_type<ushort>(half_literal)` compiled to a
// bare double becomes a 64-bit -> 16-bit bit-cast, which MSL rejects.

RWStructuredBuffer<uint> outputBuffer;
RWStructuredBuffer<float> floatBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // A half literal must be emitted with the `h` suffix. Before the fix this
    // was a bare `61440.0`, so `as_type<ushort>(61440.0)` was a double->ushort
    // bit-cast that does not compile.
    half h = 61440.hf;
    // METAL: as_type<ushort>(61440.0h)
    outputBuffer[0] = uint(bit_cast<uint16_t>(h));

    // A float literal must be emitted with the `f` suffix.
    // METAL: = 1.5f
    floatBuffer[0] = 1.5f;
}
