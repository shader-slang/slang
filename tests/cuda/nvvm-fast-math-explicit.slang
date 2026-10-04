// SPDX-FileCopyrightText: The Khronos Group, Inc.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

//TEST:SIMPLE(filecheck=CHECK): -target ptx -emit-cuda-via-nvvm -entry computeMain -stage compute -capability cuda_sm_8_0 -O3 -fp-mode fast

// Explicitly named intrinsics retain their semantics even when ordinary library calls
// permit approximation. A backend-wide name substitution would violate this contract.
[require(nvvm)]
float preciseSin(float x)
{
    __target_switch
    {
    case nvvm: __intrinsic_asm "__nv_sinf", x;
    }
}

[require(nvvm)]
float preciseSqrt(float x)
{
    __target_switch
    {
    case nvvm: __intrinsic_asm "llvm.sqrt", x;
    }
}

StructuredBuffer<float> inputs;
RWStructuredBuffer<float> outputs;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputs[0] = preciseSin(inputs[0]);
    outputs[1] = preciseSqrt(inputs[0]);
}

// CHECK: result code = 0
// CHECK-NOT: sin.approx
// CHECK-NOT: sqrt.approx
// CHECK: sqrt.rn
// CHECK-NOT: sin.approx
// CHECK-NOT: sqrt.approx
