// SPDX-FileCopyrightText: The Khronos Group, Inc.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -cpu -compute -output-using-type -Xslang -O0
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -cpu -compute -output-using-type -Xslang -O3
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -cuda -compute -output-using-type -render-features half -Xslang -O0
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -cuda -compute -output-using-type -render-features half -Xslang -O3

// Load the same Float32 words used by half-constant-rounding.slang to exercise runtime
// Half casts and f32tof16. Both paths must match the independently checked constant results
// on CPU and CUDA. Vulkan permits implementation-dependent rounding for OpFConvert when
// no rounding mode is specified, so it cannot use these nearest-even runtime expectations.
//TEST_INPUT:ubuffer(data=[1065357312 1065365504 3212840960 855638016 855638017 947904512 947904511 1199566848 1199566847 2147483648 3003121665 874512384], stride=4):name=inputBits
StructuredBuffer<uint> inputBits;
//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    for (uint i = 0; i < 12; ++i)
    {
        float value = asfloat(inputBits[i]);
        outputBuffer[i] = uint(bit_cast<uint16_t>(half(value)));
        outputBuffer[12 + i] = f32tof16(value);
    }
}

// CHECK: type: uint32_t
// Runtime Half casts.
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

// Runtime packing through f32tof16.
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
