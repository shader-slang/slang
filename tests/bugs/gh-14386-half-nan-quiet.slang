// A NaN narrowed to `half` at compile time must become a quiet NaN. `FloatToHalf` used to
// narrow every NaN to `sign | 0x7C00 | 1`, i.e. the signaling NaN 0x7C01 / 0xFC01, while
// the `float` NaN folded from the same expression stayed quiet. The SPIR-V lane checks the
// folded `OpConstant` encodings directly; the vk lane stores the same constants to a buffer,
// so the GPU run cross-checks what the compiler folded; the CPU lane covers the C++ emit
// path. The byte-code VM path is covered by the companion gh-14386-half-nan-quiet-vm.slang.
// The sign of the folded `0.0 / 0.0` is not specified, so the checks accept either sign.

//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry computeMain -stage compute
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -render-features half -shaderobj
//TEST:COMPARE_COMPUTE(filecheck-buffer=CPU):-cpu -output-using-type

// spirv-dis prints a non-finite OpConstant as a hex float that re-reads the bit pattern as
// an ordinary value of the constant's own type, so the three patterns below are the
// encodings in disguise: `0x1.8p+16` is the half quiet NaN 0x7E00 / 0xFE00 (fraction 0x200
// read as 0.5 at exponent 16), `0x1p+16` is the half +Inf 0x7C00, and `0x1.8p+128` is the
// float quiet NaN 0x7FC00000 / 0xFFC00000. The pre-fix signaling encodings 0x7C01 / 0xFC01
// print as `0x1.004p+16` / `-0x1.004p+16`, so they do not match.
// SPIRV: OpConstant %half {{(-0x1\.8p\+16|0x1\.8p\+16)}}
// SPIRV: OpConstant %half 0x1p+16
// SPIRV: OpConstant %float {{(-0x1\.8p\+128|0x1\.8p\+128)}}

//TEST_INPUT:ubuffer(data=[0xAAAAAAAA 0xAAAAAAAA], stride=4):out,name=o
RWByteAddressBuffer o;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    o.Store<half2>(0, half2(half(0.0 / 0.0), half(1.0 / 0.0)));
    o.Store<float>(4, float(0.0 / 0.0));

    // The stored word reads the high half first: +Inf 0x7C00, then the narrowed NaN, quiet
    // with the sign preserved.
    // CHECK: 7C00{{7|F}}E00
    // The `float` NaN stays the canonical quiet NaN of either sign.
    // CHECK-NEXT: {{7|F}}FC00000
    // CPU: 7C00{{7|F}}E00
}
