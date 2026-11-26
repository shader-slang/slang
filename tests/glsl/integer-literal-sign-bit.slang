// An unsuffixed integer literal in GLSL is an `int` carrying the raw bit
// pattern, even when the sign bit is set (GLSL 4.60 section 4.1.3):
// 0xf1bbcdcb is -239350325 (not a uint) and 3000000000 is -1294967296
// (not an int64). Only an explicit `u`/`U` suffix selects an unsigned
// literal, and literals that do not fit in 32 bits, or that carry a width
// suffix GLSL does not define, keep the Slang typing rules instead. This
// pins the front-end typing, the constant-folding shapes that depend on
// it (an arithmetic shift right of a negative literal, and a signed
// comparison of one), and the INT_MIN spellings: decimal 2147483648 is
// int INT_MIN on its own (no unary minus needed), and the negated
// -2147483648 idiom must still fold to INT_MIN.
//
// The `float(...)` casts of the suffixed, >32-bit, and width-suffixed
// literals are the discriminating checks for those escape hatches: each
// emits a different constant when the literal is mistyped as a signed
// int than when its suffix/width is honored.

//TEST:SIMPLE_EX(filecheck=CHECK_GLSL): -lang glsl tests/glsl/integer-literal-sign-bit.slang -stage compute -entry main -target glsl
//TEST:SIMPLE_EX(filecheck=CHECK_WGSL): -lang glsl tests/glsl/integer-literal-sign-bit.slang -stage compute -entry main -target wgsl

#version 450
layout(local_size_x = 1) in;
layout(binding = 0) buffer Buf { int i[10]; float f[4]; } buf;

void main()
{
    int n = buf.i[0];
    buf.i[1] = 0xffffffff >> 1;
    buf.i[2] = (0x80000000 < 0) ? 1 : 0;
    buf.i[3] = int(0xf1bbcdcbu);
    buf.i[4] = -2147483648;
    buf.i[5] = 2147483648;
    buf.i[6] = (2147483648 < 0) ? 1 : 0;
    buf.i[7] = int(0xffffffffL >> 1);
    buf.i[8] = 3000000000 >> 1;
    buf.i[9] = int(0xf1bbcdcbU);
    buf.f[0] = float(0xf1bbcdcb * n);
    buf.f[1] = float(3000000000 * n);
    buf.f[2] = float(0xf1bbcdcbu);
    buf.f[3] = float(0x100000000);
}
//CHECK_GLSL: buf_0.i_0[1] = -1;
//CHECK_GLSL: buf_0.i_0[2] = 1;
//CHECK_GLSL: buf_0.i_0[3] = -239350325;
//CHECK_GLSL: buf_0.i_0[4] = -2147483648;
//CHECK_GLSL: buf_0.i_0[5] = -2147483648;
//CHECK_GLSL: buf_0.i_0[6] = 1;
//CHECK_GLSL: buf_0.i_0[7] = 2147483647;
//CHECK_GLSL: buf_0.i_0[8] = -647483648;
//CHECK_GLSL: buf_0.i_0[9] = -239350325;
//CHECK_GLSL: buf_0.f_0[0] = float(-239350325 * buf_0.i_0[0]);
//CHECK_GLSL: buf_0.f_0[1] = float(-1294967296 * buf_0.i_0[0]);
//CHECK_GLSL: buf_0.f_0[2] = 4.055617024e+09;
//CHECK_GLSL: buf_0.f_0[3] = 4.294967296e+09;
//CHECK_WGSL: buf_0[i32(0)].i_0[i32(1)] = i32(-1);
//CHECK_WGSL: buf_0[i32(0)].i_0[i32(2)] = i32(1);
//CHECK_WGSL: buf_0[i32(0)].i_0[i32(3)] = i32(-239350325);
//CHECK_WGSL: buf_0[i32(0)].i_0[i32(4)] = i32(-2147483648);
//CHECK_WGSL: buf_0[i32(0)].i_0[i32(5)] = i32(-2147483648);
//CHECK_WGSL: buf_0[i32(0)].i_0[i32(6)] = i32(1);
//CHECK_WGSL: buf_0[i32(0)].i_0[i32(7)] = i32(2147483647);
//CHECK_WGSL: buf_0[i32(0)].i_0[i32(8)] = i32(-647483648);
//CHECK_WGSL: buf_0[i32(0)].i_0[i32(9)] = i32(-239350325);
//CHECK_WGSL: buf_0[i32(0)].f_0[i32(0)] = f32(i32(-239350325) * n_0);
//CHECK_WGSL: buf_0[i32(0)].f_0[i32(1)] = f32(i32(-1294967296) * n_0);
//CHECK_WGSL: buf_0[i32(0)].f_0[i32(2)] = 4.055617024e+09f;
//CHECK_WGSL: buf_0[i32(0)].f_0[i32(3)] = 4.294967296e+09f;