// SPDX-FileCopyrightText: The Khronos Group, Inc.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

//TEST:SIMPLE(filecheck=FAST): -target ptx -emit-cuda-via-nvvm -entry computeMain -stage compute -capability cuda_sm_8_0 -O3 -fp-mode fast
//TEST:SIMPLE(filecheck=NORMAL): -target ptx -emit-cuda-via-nvvm -entry computeMain -stage compute -capability cuda_sm_8_0 -O3 -fp-mode precise
//TEST:SIMPLE(filecheck=NORMAL): -target ptx -emit-cuda-via-nvvm -entry computeMain -stage compute -capability cuda_sm_8_0 -O3
//TEST:SIMPLE(filecheck=NORMAL): -target ptx -emit-cuda-via-nvvm -entry computeMain -stage compute -capability cuda_sm_8_0 -O3 -fp-mode fast -denorm-mode-fp32 preserve
//TEST:SIMPLE(filecheck=FAST): -target ptx -emit-cuda-via-nvvm -entry computeMain -stage compute -capability cuda_sm_8_0 -O3 -fp-mode fast -denorm-mode-fp32 ftz

// Runtime inputs prevent constant folding from hiding the selected math implementation.
StructuredBuffer<float> inputs;
RWStructuredBuffer<float> outputs;

[numthreads(1, 1, 1)]
void computeMain()
{
    float x = inputs[0];
    float y = inputs[1];
    outputs[0] = sin(x);
    outputs[1] = cos(x);
    outputs[2] = tan(x);
    outputs[3] = exp(x);
    outputs[4] = log(x);
    outputs[5] = log2(x);
    outputs[6] = log10(x);
    outputs[7] = pow(x, y);
    outputs[8] = sqrt(x);
    outputs[9] = x / y;
}

// FAST-DAG: sin.approx.ftz.f32
// FAST-DAG: cos.approx.ftz.f32
// FAST-DAG: lg2.approx.ftz.f32
// FAST-DAG: ex2.approx.ftz.f32
// FAST-DAG: sqrt.approx.ftz.f32
// FAST-DAG: div.approx.ftz.f32
// NORMAL: result code = 0
// NORMAL-NOT: sin.approx
// NORMAL-NOT: cos.approx
// NORMAL-NOT: sqrt.approx
// NORMAL-NOT: div.approx
