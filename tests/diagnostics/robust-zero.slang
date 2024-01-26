//TEST:SIMPLE(filecheck=CHECK): -target hlsl -stage compute -entry main
RWStructuredBuffer<float> outZero;
RWStructuredBuffer<float> outScientificFormatPositivePositive;
RWStructuredBuffer<float> outScientificFormatNegativePositive;
RWStructuredBuffer<float> outScientificFormatPositiveNegative;
RWStructuredBuffer<float> outScientificFormatNegativeNegative;
RWStructuredBuffer<float> outFixedFormat1;
RWStructuredBuffer<float> outFixedFormat2;
RWStructuredBuffer<float> outFixedFormat3;
RWStructuredBuffer<float> outFixedFormatNegative;

RWStructuredBuffer<float> outNearZero;
RWStructuredBuffer<float> outNearZeroNegative;
RWStructuredBuffer<float> outRequiresRounding;

void main()
{
    // CHECK: outZero{{.*}}[0U] = 0.0;
    outZero[0] = 0.0;

    // CHECK: outScientificFormatPositivePositive{{.*}}[0U] = 7.0e8;
    outScientificFormatPositivePositive[0] = 700000000;

    // CHECK: outScientificFormatNegativePositive{{.*}}[0U] = -7.0e8;
    outScientificFormatNegativePositive[0] = -700000000;

    // CHECK: outScientificFormatPositiveNegative{{.*}}[0U] = 7.0e-8;
    outScientificFormatPositiveNegative[0] = 0.00000007;

    // CHECK: outScientificFormatNegativeNegative{{.*}}[0U] = -7.0e-8;
    outScientificFormatNegativeNegative[0] = -0.00000007;

    // CHECK: outFixedFormat1{{.*}}[0U] = 32.25;
    outFixedFormat1[0] = 32.25;

    // CHECK: outFixedFormat2{{.*}}[0U] = 32.0;
    outFixedFormat2[0] = 32;

    // CHECK: outFixedFormat3{{.*}}[0U] = 0.001;
    outFixedFormat3[0] = 0.001;

    // CHECK: outFixedFormatNegative{{.*}}[0U] = -4.0;
    outFixedFormatNegative[0] = -4;

    // CHECK: outNearZero{{.*}}[0U] = 1.7e-37;
    outNearZero[0] = 1.7e-37;

    // CHECK: outNearZeroNegative{{.*}}[0U] = -1.7e-37;
    outNearZeroNegative[0] = -1.7e-37;

    // CHECK: outRequiresRounding{{.*}}[0U] = 0.7;
    outRequiresRounding[0] = 0.7;
}
