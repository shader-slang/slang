//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj
//DISABLED_TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj
// Disabled due to DXC seemingly mishandling the NaN corner case of min() & max()
//DISABLED_TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -dx12 -profile cs_6_2 -Xslang... -Xdxc -Gis -X. -shaderobj
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -emit-spirv-directly -shaderobj
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj
// Disabled due to missing support for 16-bit integers:
// https://github.com/gpuweb/gpuweb/issues/273
// They are needed for `asuint16` and functions that would be implemented in
// terms of `asuint16` on WGSL, like `isinf` and `isnan`.
//DISABLED_TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-wgpu -compute -shaderobj

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // We test the "interesting" functions in more detail - the ones that have
    // a special implementation on the CPU target. The ones that simply defer
    // to f32 math get a collective test that just checks that nothing changes.

    // CHECK: 1F
    outputBuffer[0] =
        (abs(-2.0h) == 2.0h ? 1<<0 : 0) |
        (abs(2.0h) == 2.0h ? 1<<1 : 0) |
        (abs(-0.0h) == 0.0h ? 1<<2 : 0) |
        (abs(-1.0h/0.0h) == 1.0h/0.0h ? 1<<3 : 0) |
        (abs(1.0h/0.0h) == 1.0h/0.0h ? 1<<4 : 0);

    // CHECK: 7
    outputBuffer[1] =
        (!isnan(0.0h) ? 1<<0 : 0) |
        (!isnan(-1.0h/0.0h) ? 1<<1 : 0) |
        (isnan(0.0h/0.0h) ? 1<<2 : 0);

    // CHECK: 7
    outputBuffer[2] =
        (isfinite(-12.0h) ? 1<<0 : 0) |
        (!isfinite(0.0h/0.0h) ? 1<<1 : 0) |
        (!isfinite(1.0h/0.0h) ? 1<<2 : 0);

    // CHECK: F
    outputBuffer[3] =
        (!isinf(-0.0h) ? 1<<0 : 0) |
        (!isinf(0.0h/0.0h) ? 1<<1 : 0) |
        (isinf(1.0h/0.0h) ? 1<<2 : 0) |
        (isinf(-1.0h/0.0h) ? 1<<3 : 0);

    // CHECK: FF
    outputBuffer[4] =
        (min(1.0h, 1.0h) == 1.0h ? 1<<0 : 0) |
        (min(-1.0h, -0.3h) == -1.0h ? 1<<1 : 0) |
        (min(0.25h, 100.0h) == 0.25h ? 1<<2 : 0) |
        (min(-0.25h, 100.0h) == -0.25h ? 1<<3 : 0) |
        (min(0.0h, 1.0h/0.0h) == 0.0h ? 1<<4 : 0) |
        (min(0.0h, -1.0h/0.0h) == -1.0h/0.0h ? 1<<5 : 0) |
        (min(0.0h, 0.0h/0.0h) == 0.0h ? 1<<6 : 0) |
        (min(0.0h/0.0h, 0.0h) == 0.0h ? 1<<7 : 0);

    // CHECK: FF
    outputBuffer[5] =
        (max(1.0h, 1.0h) == 1.0h ? 1<<0 : 0) |
        (max(-1.0h, -0.3h) == -0.3h ? 1<<1 : 0) |
        (max(0.25h, 100.0h) == 100.0h ? 1<<2 : 0) |
        (max(-0.25h, 100.0h) == 100.0h ? 1<<3 : 0) |
        (max(0.0h, 1.0h/0.0h) == 1.0h/0.0h ? 1<<4 : 0) |
        (max(0.0h, -1.0h/0.0h) == 0.0h ? 1<<5 : 0) |
        (max(0.0h, 0.0h/0.0h) == 0.0h ? 1<<6 : 0) |
        (max(0.0h/0.0h, 0.0h) == 0.0h ? 1<<7 : 0);

    // CHECK: 3F
    outputBuffer[6] =
        (sign(0.0h) == 0 ? 1<<0 : 0) |
        (sign(-0.0h) == 0 ? 1<<1 : 0) |
        (sign(-17.0h) == -1 ? 1<<2 : 0) |
        (sign(23.0h) == 1 ? 1<<3 : 0) |
        (sign(1.0h/0.0h) == 1 ? 1<<4 : 0) |
        (sign(-1.0h/0.0h) == -1 ? 1<<5 : 0);
    
    // The rest of the functions.
    half ft = 42.3h;

    ft = ceil(ft) + floor(ft) + round(ft);
    ft = sin(ft);
    ft = cos(ft);
    ft = tan(ft);
    ft = asin(frac(ft));
    ft = acos(frac(ft));
    ft = atan(ft);
    ft = sinh(ft) + cosh(ft) + tanh(ft);

    //ft = asinh(ft);
    //ft = acosh(abs(ft) + 1.0h);
    //ft = atanh(fmod(ft, 1.0h));

    ft = log2(ft);
    ft = exp2(ft);
    ft = log(ft);
    ft = exp(ft);
    ft = log10(ft);

    ft += trunc(ft);
    ft = sqrt(abs(ft));
    ft = pow(ft, 2.5h);

    ft = atan2(ft, 2.0h);

    ft = fma(ft, 2.0h, 1.0h);

    ft = rsqrt(ft);

    int e;
    ft = frexp(ft, e);
    ft += half(e);

    half ip;
    modf(ft, ip);
    ft += ip;

    // CHECK: 5AF0
    outputBuffer[7] = asuint16(round(ft * 256.0h));
}
