// An unsuffixed integer literal in GLSL is an `int` carrying the raw bit
// pattern, even when the sign bit is set (GLSL 4.60 section 4.1.3):
// 0xf1bbcdcb is -239350325 (not a uint) and 3000000000 is -1294967296
// (not an int64). Only an explicit `u`/`U` suffix selects an unsigned
// literal. This pins the front-end typing, the two constant-folding shapes
// that depend on it (arithmetic shift right of a negative literal, and a
// signed comparison of one), the u-suffix escape hatch, and the INT_MIN
// idiom spelled as a negated literal.

//TEST:SIMPLE_EX(filecheck=CHECK_GLSL): -lang glsl tests/glsl/integer-literal-sign-bit.slang -stage compute -entry main -target glsl
//TEST:SIMPLE_EX(filecheck=CHECK_WGSL): -lang glsl tests/glsl/integer-literal-sign-bit.slang -stage compute -entry main -target wgsl

#version 450
layout(local_size_x = 1) in;
layout(binding = 0) buffer Buf { int i[6]; float f[4]; } buf;

void main()
{
    int n = buf.i[0];
    buf.f[0] = float(0xf1bbcdcb * n);
    buf.f[1] = float(3000000000 * n);
    buf.i[1] = 0xffffffff >> 1;
    buf.i[2] = (0x80000000 < 0) ? 1 : 0;
    buf.i[3] = int(0xf1bbcdcbu);
    buf.i[4] = -2147483648;
}
//CHECK_GLSL: buf_0.f_0[0] = float(-239350325 * buf_0.i_0[0]);
//CHECK_GLSL: buf_0.f_0[1] = float(-1294967296 * buf_0.i_0[0]);
//CHECK_GLSL: buf_0.i_0[1] = -1;
//CHECK_GLSL: buf_0.i_0[2] = 1;
//CHECK_GLSL: buf_0.i_0[3] = -239350325;
//CHECK_GLSL: buf_0.i_0[4] = -2147483648;
//CHECK_WGSL: buf_0[i32(0)].f_0[i32(0)] = f32(i32(-239350325) * buf_0[i32(0)].i_0[i32(0)]);
//CHECK_WGSL: buf_0[i32(0)].f_0[i32(1)] = f32(i32(-1294967296) * n_0);
//CHECK_WGSL: buf_0[i32(0)].i_0[i32(1)] = i32(-1);
//CHECK_WGSL: buf_0[i32(0)].i_0[i32(2)] = i32(1);
//CHECK_WGSL: buf_0[i32(0)].i_0[i32(3)] = i32(-239350325);
//CHECK_WGSL: buf_0[i32(0)].i_0[i32(4)] = i32(-2147483648);
