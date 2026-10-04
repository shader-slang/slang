// SPDX-FileCopyrightText: The Khronos Group, Inc.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3 -Xslang -fp-mode -Xslang fast
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0 -Xslang -fp-mode -Xslang fast
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3 -Xslang -fp-mode -Xslang precise
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3 -Xslang -fp-mode -Xslang fast -Xslang -denorm-mode-fp32 -Xslang preserve

// Runtime bit patterns: 0.5, 2.0, minimum positive subnormal, -0, +Inf, NaN.
//TEST_INPUT: ubuffer(data=[1056964608 1073741824 1 2147483648 2139095040 2143289344], stride=4):name=inputs
StructuredBuffer<uint> inputs;
//TEST_INPUT: ubuffer(data=[99 99 99 99], stride=4):out,name=outputs
RWStructuredBuffer<uint> outputs;

bool close(float actual, float expected)
{
    // These small-angle/positive-domain inputs are well within the approximate functions'
    // documented accuracy. NaN comparisons fail rather than silently passing the tolerance.
    return abs(actual - expected) <= 0.000002;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    float x = asfloat(inputs[0]);
    float y = asfloat(inputs[1]);
    uint errors = 0;
    errors |= !close(sin(x), 0.4794255386) ? 1u : 0u;
    errors |= !close(cos(x), 0.8775825619) ? 2u : 0u;
    errors |= !close(tan(x), 0.5463024898) ? 4u : 0u;
    errors |= !close(exp(x), 1.6487212707) ? 8u : 0u;
    errors |= !close(log(x), -0.6931471806) ? 16u : 0u;
    errors |= !close(log2(x), -1.0) ? 32u : 0u;
    errors |= !close(log10(x), -0.3010299957) ? 64u : 0u;
    errors |= !close(pow(x, y), 0.25) ? 128u : 0u;
    errors |= !close(sqrt(x), 0.7071067812) ? 256u : 0u;
    errors |= !close(x / y, 0.25) ? 512u : 0u;
    outputs[0] = errors;

    float2 s, c;
    sincos(float2(x, -x), s, c);
    errors = uint(!close(s.x, 0.4794255386) || !close(s.y, -0.4794255386));
    errors |= !all(abs(c - 0.8775825619) < 0.000002) ? 2u : 0u;
    errors |= !all(abs(float3(x, y, -x) / y - float3(0.25, 1.0, -0.25)) < 0.000002) ? 4u : 0u;
    errors |= !all(abs(y / float3(x, y, -x) - float3(4.0, 1.0, -4.0)) < 0.000002) ? 8u : 0u;
    errors |= abs(float(sin(half(x))) - 0.4794255386) > 0.0005 ? 16u : 0u;
    errors |= abs(sin(double(x)) - 0.479425538604203l) > 0.000000000000002l ? 32u : 0u;
    outputs[1] = errors;

    float tiny = asfloat(inputs[2]);
    bool fast = __isFloat32FastIntrinsicAllowed();
    uint expected = fast ? 0 : inputs[2];
    errors = uint(asuint(sin(tiny)) != expected);
    errors |= asuint(tiny + tiny) != (fast ? 0 : inputs[2] * 2) ? 2u : 0u;
    // Classify transported bits so the oracle does not itself flush the result.
    errors |= asuint(tiny / x) != (fast ? 0 : inputs[2] * 2) ? 4u : 0u;
    outputs[2] = errors;

    float negativeZero = asfloat(inputs[3]);
    float infinity = asfloat(inputs[4]);
    float nanValue = asfloat(inputs[5]);
    errors = uint(asuint(sin(negativeZero)) != inputs[3]);
    errors |= !isnan(sin(infinity)) ? 2u : 0u;
    errors |= !isnan(sqrt(-y)) ? 4u : 0u;
    errors |= !isnan(log(-y)) ? 8u : 0u;
    errors |= !isnan(cos(nanValue)) ? 16u : 0u;
    errors |= !isinf(log(-negativeZero)) ? 32u : 0u;
    outputs[3] = errors;
}

// CHECK: type: uint
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
