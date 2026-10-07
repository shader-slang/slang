// SPDX-FileCopyrightText: The Khronos Group, Inc.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -cpu -compute -output-using-type -Xslang -O0
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -cpu -compute -output-using-type -Xslang -O3
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -cuda -compute -output-using-type -render-features half -Xslang -O0
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -cuda -compute -output-using-type -render-features half -Xslang -O3
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -vk -compute -output-using-type -render-features half -Xslang -O0
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -vk -compute -output-using-type -render-features half -Xslang -O3

// Explicit bit patterns force constant conversion through the compiler's FloatToHalf path.
// Expected encodings are independent of the compiler: ties choose the even Half encoding,
// and the Float32 neighbor above half the minimum subnormal must round to that subnormal.
//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    // Midpoint ties around 1.0 choose the even encoding, for either sign.
    outputBuffer[0] = uint(bit_cast<uint16_t>(half(bit_cast<float>(0x3f801000u))));
    outputBuffer[1] = uint(bit_cast<uint16_t>(half(bit_cast<float>(0x3f803000u))));
    outputBuffer[2] = uint(bit_cast<uint16_t>(half(bit_cast<float>(0xbf801000u))));
    // Half the minimum subnormal ties to zero; its next Float32 neighbor rounds up.
    outputBuffer[3] = uint(bit_cast<uint16_t>(half(bit_cast<float>(0x33000000u))));
    outputBuffer[4] = uint(bit_cast<uint16_t>(half(bit_cast<float>(0x33000001u))));
    // The subnormal-to-normal midpoint rounds up; its lower Float32 neighbor stays subnormal.
    outputBuffer[5] = uint(bit_cast<uint16_t>(half(bit_cast<float>(0x387fe000u))));
    outputBuffer[6] = uint(bit_cast<uint16_t>(half(bit_cast<float>(0x387fdfffu))));
    // The overflow midpoint rounds to infinity; its lower Float32 neighbor stays finite.
    outputBuffer[7] = uint(bit_cast<uint16_t>(half(bit_cast<float>(0x477ff000u))));
    outputBuffer[8] = uint(bit_cast<uint16_t>(half(bit_cast<float>(0x477fefffu))));
    // Preserve the sign of zero and of a negative value just above half the minimum subnormal.
    outputBuffer[9] = uint(bit_cast<uint16_t>(half(bit_cast<float>(0x80000000u))));
    outputBuffer[10] = uint(bit_cast<uint16_t>(half(bit_cast<float>(0xb3000001u))));
    // The midpoint between subnormal encodings 2 and 3 chooses encoding 2.
    outputBuffer[11] = uint(bit_cast<uint16_t>(half(bit_cast<float>(0x34200000u))));
}

// CHECK: type: uint32_t
// CHECK-NEXT: {{^15360$}}
// CHECK-NEXT: {{^15362$}}
// CHECK-NEXT: {{^48128$}}
// CHECK-NEXT: {{^0$}}
// CHECK-NEXT: {{^1$}}
// CHECK-NEXT: {{^1024$}}
// CHECK-NEXT: {{^1023$}}
// CHECK-NEXT: {{^31744$}}
// CHECK-NEXT: {{^31743$}}
// CHECK-NEXT: {{^32768$}}
// CHECK-NEXT: {{^32769$}}
// CHECK-NEXT: {{^2$}}
