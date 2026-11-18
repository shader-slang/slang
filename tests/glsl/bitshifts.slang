// Regression test for shader-slang/slang#14590:
// glslang rejects `scalar >> ivec2` in the GLSL emitted for -emit-spirv-via-glsl, so the
// GLSL legalization splats the scalar operand to the other operand's vector shape.

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

    // Vector shifted by a scalar: already valid GLSL; the shared helper splats the
    // shift amount too, matching what Metal and WGSL emit.
    int2 w2 = int2(s, s) >> amount;
    outputBuffer[4] = w2.x;

    int2 u2 = int2(s, s) << amount;
    outputBuffer[5] = u2.y;

    // Mixed-signedness shifts are valid GLSL and must stay accepted.
    int2 m2 = int2(s, s) >> uint2(1u);
    outputBuffer[6] = m2.x;
    outputBuffer[7] = m2.y;

    // The splat keeps the scalar's own element type even when the vector's differs:
    // `s` is an int, so it splats to ivec2, not uvec2.
    int2 m3 = s >> uint2(1u, 1u);
    outputBuffer[8] = m3.y;

    // A runtime (loaded) scalar shift amount is splatted too, not just constants.
    int2 r2 = int2(s, s) >> s;
    outputBuffer[9] = r2.x;
}

// The scalar operand of a scalar-vector shift is splatted to the vector shape, so both sides
// of every emitted shift have matching shapes. The splat keeps the scalar's own element
// type (`int`): the shift overloads type the result independently of the other operand's
// element type, so `vector<L,N> operator>>(L left, vector<R,N> right)` yields vector<L,N>.
//GLSL: ivec2 _S1 = ivec2(inputBuffer_0._data[uint(0)]);
//GLSL: v2_0 = _S1 >> (ivec2(1));
//GLSL: v4_0 = (ivec4(s_0)) << (ivec4(1));
// A scalar shift amount is splatted to the vector too, keeping its own element type.
//GLSL: ivec2 _S3 = ivec2(1);
//GLSL: (_S2 >> _S3)
//GLSL: (_S2 << _S3)
// Mixed-signedness operands stay untouched: GLSL accepts `ivec2 >> uvec2`.
//GLSL: m2_0 = _S2 >> (uvec2(1U));
// The splat of an int scalar by a uint vector keeps the scalar's element type: the shifted
// value stays ivec2 (`_S1` above) next to the untouched uvec2 amount.
//GLSL: (_S1 >> uvec2(1U, 1U))
// A runtime scalar amount is splatted the same way.
//GLSL: (_S2 >> _S1)

// Compiling the emitted GLSL through glslang must succeed and produce the shift ops.
//GLSLSPV: OpShiftRightArithmetic
//GLSLSPV: OpShiftLeftLogical

