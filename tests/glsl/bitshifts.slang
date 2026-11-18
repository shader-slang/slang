// Regression test for shader-slang/slang#14590:
// glslang rejects `scalar >> ivec2` in the GLSL emitted for -emit-spirv-via-glsl.
// GLSL requires both operands of a shift to have the same shape, so the GLSL
// legalization splats the scalar operand to the other operand's vector shape.

//TEST:SIMPLE(filecheck=GLSL): -target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=GLSLSPV): -target spirv-asm -entry computeMain -stage compute -emit-spirv-via-glsl

RWStructuredBuffer<int> inputBuffer;
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    int amount = 1;
    int s = inputBuffer[0];

    // Scalar shifted by a vector: the scalar value must be splatted to a vector.
    int2 v2 = s >> int2(amount);
    outputBuffer[0] = v2.x;
    outputBuffer[1] = v2.y;

    int4 v4 = s << int4(amount);
    outputBuffer[2] = v4.x;
    outputBuffer[3] = v4.w;

    // Vector shifted by a scalar: GLSL also rejects `ivec2 >> int`, so the
    // shift amount must be splatted to a vector.
    int2 w2 = int2(s, s) >> amount;
    outputBuffer[4] = w2.x;

    int2 u2 = int2(s, s) << amount;
    outputBuffer[5] = u2.y;

    // Mixed-signedness shifts are valid GLSL and must stay accepted.
    int2 m2 = int2(s, s) >> uint2(1u);
    outputBuffer[6] = m2.x;
    outputBuffer[7] = m2.y;
}

// The scalar operand of a scalar-vector shift is splatted to the vector shape, so both sides
// of every emitted shift have matching shapes. The splat of the shifted value keeps the
// scalar's own element type (`int`), preserving the shift-amount semantics of the overload
// `vector<L,N> operator>>(L left, vector<R,N> right)`.
//GLSL: v2_0 = ivec2(inputBuffer_0._data[uint(0)]) >> ivec2(1)
//GLSL: v4_0 = ivec4(s_0) << ivec4(1)
//GLSL: w2 >> ivec2(1)
//GLSL: u2 << ivec2(1)
// Mixed-signedness operands stay untouched: GLSL accepts `ivec2 >> uvec2`.
//GLSL: m2_0 = _S1 >> uvec2(1U)

// Compiling the emitted GLSL through glslang must succeed and produce the shift ops.
//GLSLSPV: OpShiftRight
//GLSLSPV: OpShiftLeft
