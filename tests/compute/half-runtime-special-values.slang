// SPDX-FileCopyrightText: The Khronos Group, Inc.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type -Xslang -O0
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type -Xslang -O3

// NaN payload encodings are backend-specific. Prevent the harness from synthesizing
// CUDA runs with the CPU prelude's expected payloads.
//DISABLE_TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type

// Exercise the CPU prelude with runtime inputs. Its existing NaN policy retains the
// high payload bits, including the quiet/signaling bit, and keeps a nonzero payload
// when all input payload bits would be truncated. The compiler constant helper has
// a different NaN policy, so use explicit expected encodings for this runtime path.
// Also cover signed zero, Float32 subnormals, overflow, and infinities in both signs.
//TEST_INPUT:ubuffer(data=[0 1 8388607 8388608 1199570944 2139095039 2139095040 2139095041 2141266757 2143363909 2147483647 2147483648 2147483649 2155872255 2155872256 3347054592 4286578687 4286578688 4286578689 4288750405 4290847557 4294967295], stride=4):name=inputBits
StructuredBuffer<uint> inputBits;
//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(32, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    if (tid.x < 22)
        outputBuffer[tid.x] = f32tof16(asfloat(inputBits[tid.x]));
}

// CHECK: type: uint32_t
// CHECK-NEXT: {{^0$}}
// CHECK-NEXT: {{^0$}}
// CHECK-NEXT: {{^0$}}
// CHECK-NEXT: {{^0$}}
// CHECK-NEXT: {{^31744$}}
// CHECK-NEXT: {{^31744$}}
// CHECK-NEXT: {{^31744$}}
// CHECK-NEXT: {{^31745$}}
// CHECK-NEXT: {{^32009$}}
// CHECK-NEXT: {{^32265$}}
// CHECK-NEXT: {{^32767$}}
// CHECK-NEXT: {{^32768$}}
// CHECK-NEXT: {{^32768$}}
// CHECK-NEXT: {{^32768$}}
// CHECK-NEXT: {{^32768$}}
// CHECK-NEXT: {{^64512$}}
// CHECK-NEXT: {{^64512$}}
// CHECK-NEXT: {{^64512$}}
// CHECK-NEXT: {{^64513$}}
// CHECK-NEXT: {{^64777$}}
// CHECK-NEXT: {{^65033$}}
// CHECK-NEXT: {{^65535$}}
