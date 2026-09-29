// SPDX-FileCopyrightText: The Khronos Group, Inc.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -O3
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3

// The first eight outputs classify runtime-selected NaNs. Literal conversions independently
// exercise constant folding, and the last output is an untouched guard.
//TEST_INPUT: ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 3735928559], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

float valueAt(uint index)
{
    switch (index)
    {
    case 0: return asfloat(0x7f800001u);
    case 1: return asfloat(0x7fa12345u);
    case 2: return asfloat(0x7fc12345u);
    case 3: return asfloat(0x7fffffffu);
    case 4: return asfloat(0xff800001u);
    case 5: return asfloat(0xffa12345u);
    case 6: return asfloat(0xffc12345u);
    case 7: return asfloat(0xffffffffu);
    default: return 0.0;
    }
}

[numthreads(8, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    half converted = half(valueAt(tid.x));
    uint bits = uint(bit_cast<uint16_t>(converted));
    outputBuffer[tid.x] = ((bits & 0x7c00u) == 0x7c00u && (bits & 0x3ffu) != 0) ? 1u : 0u;
    if (tid.x == 0)
    {
        // Even and odd ties, subnormal sticky bits, negative ties, and exponent carries.
        outputBuffer[8] = uint(bit_cast<uint16_t>(half(1.00048828125f)));
        outputBuffer[9] = uint(bit_cast<uint16_t>(half(1.00146484375f)));
        outputBuffer[10] = uint(bit_cast<uint16_t>(half(2.98023223876953125e-8f)));
        outputBuffer[11] = uint(bit_cast<uint16_t>(half(2.980232594040899e-8f)));
        outputBuffer[12] = uint(bit_cast<uint16_t>(half(-1.00048828125f)));
        outputBuffer[13] = uint(bit_cast<uint16_t>(half(65520.0f)));
        outputBuffer[14] = uint(bit_cast<uint16_t>(half(0.0000610053539276123046875f)));
    }
}

// CHECK: type: uint
// CHECK-NEXT: 1{{$}}
// CHECK-NEXT: 1{{$}}
// CHECK-NEXT: 1{{$}}
// CHECK-NEXT: 1{{$}}
// CHECK-NEXT: 1{{$}}
// CHECK-NEXT: 1{{$}}
// CHECK-NEXT: 1{{$}}
// CHECK-NEXT: 1{{$}}
// CHECK-NEXT: 15360{{$}}
// CHECK-NEXT: 15362{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1{{$}}
// CHECK-NEXT: 48128{{$}}
// CHECK-NEXT: 31744{{$}}
// CHECK-NEXT: 1024{{$}}
// CHECK-NEXT: 3735928559{{$}}
