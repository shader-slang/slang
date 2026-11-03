// SPDX-FileCopyrightText: The Khronos Group, Inc.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -output-using-type -render-features fp8 -Xslang -O0
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -output-using-type -render-features fp8 -Xslang -O3

// Check that constant folding preserves finite FP8 values and ties-to-even rounding.
// The expected output contains independently computed Float32 bits, so a matching
// narrow/widen error cannot make the test pass through a broken round trip.
//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = asuint(float(FloatE4M3(0x1p-9f)));
    outputBuffer[1] = asuint(float(FloatE4M3(0x1p-10f)));
    outputBuffer[2] = asuint(float(FloatE4M3(0x1.ep-7f)));
    outputBuffer[3] = asuint(float(FloatE4M3(0x1.4p-8f)));
    outputBuffer[4] = asuint(float(FloatE4M3(256.0f)));
    outputBuffer[5] = asuint(float(FloatE4M3(288.0f)));
    outputBuffer[6] = asuint(float(FloatE4M3(416.0f)));
    outputBuffer[7] = asuint(float(FloatE4M3(448.0f)));
    outputBuffer[8] = asuint(float(FloatE5M2(0x1p-16f)));
    outputBuffer[9] = asuint(float(FloatE5M2(0x1p-17f)));
    outputBuffer[10] = asuint(float(FloatE5M2(0x1.cp-15f)));
    outputBuffer[11] = asuint(float(FloatE5M2(0x1.4p-15f)));
}

// CHECK: type: uint32_t
// CHECK-NEXT: {{^}}989855744{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}1015021568{{$}}
// CHECK-NEXT: {{^}}998244352{{$}}
// CHECK-NEXT: {{^}}1132462080{{$}}
// CHECK-NEXT: {{^}}1133510656{{$}}
// CHECK-NEXT: {{^}}1137704960{{$}}
// CHECK-NEXT: {{^}}1138753536{{$}}
// CHECK-NEXT: {{^}}931135488{{$}}
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}947912704{{$}}
// CHECK-NEXT: {{^}}939524096{{$}}
