// A NaN narrowed to `half` at compile time must become a quiet NaN. `FloatToHalf` used to
// narrow every NaN to `sign | 0x7C00 | 1`, i.e. the signaling NaN 0x7C01 / 0xFC01, while
// the `float` NaN folded from the same expression stayed quiet. The SPIR-V lane checks the
// folded `OpConstant` encodings directly; the vk lane stores the same constants to a buffer,
// so the GPU run cross-checks what the compiler folded; the CPU lane covers the C++ emit
// path. The sign of the folded `0.0 / 0.0` is not specified, so the checks accept either sign.

//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry computeMain -stage compute
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -render-features half -shaderobj
//TEST:COMPARE_COMPUTE(filecheck-buffer=CPU):-cpu -output-using-type

//TEST_INPUT:ubuffer(data=[0xAAAAAAAA 0xAAAAAAAA], stride=4):out,name=o
RWByteAddressBuffer o;

// SPIRV: OpConstant %half {{(-0x1\.8p\+16|0x1\.8p\+16)}}
// SPIRV: OpConstant %half 0x1p+16
// SPIRV: OpConstant %float {{(-0x1\.8p\+128|0x1\.8p\+128)}}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    o.Store<half2>(0, half2(half(0.0 / 0.0), half(1.0 / 0.0)));
    o.Store<float>(4, float(0.0 / 0.0));

    // The low half is the narrowed NaN, quiet with the sign preserved; the high half is +Inf.
    // CHECK: 7C00{{7|F}}E00
    // The `float` NaN stays the default quiet NaN of either sign.
    // CHECK-NEXT: {{7|F}}FC00000
    // CPU: 7C00{{7|F}}E00
}
