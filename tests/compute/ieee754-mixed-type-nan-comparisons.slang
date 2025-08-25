//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -shaderobj -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -shaderobj -output-using-type -cuda

// Test IEEE 754 NaN comparison behavior with mixed int/float types
// Tests the type promotion logic across integer and floating-point categories
// Also tests both operand orders since implementation bugs could affect operand handling

static const float fNAN = 0.0f / 0.0f;
static const float fONE = 1.0f;
static const int iONE = 1;
static const int iZERO = 0;
static const uint uONE = 1u;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint testIndex = 0;

    // Test int compared with float NaN - all should follow IEEE 754
    // CHECK: 0
    // CHECK: 1
    // CHECK: 0
    // CHECK: 0
    // CHECK: 0
    // CHECK: 0
    outputBuffer[testIndex++] = (iONE == fNAN) ? 1u : 0u;   // int 1 == float NaN -> false
    outputBuffer[testIndex++] = (iONE != fNAN) ? 1u : 0u;   // int 1 != float NaN -> true
    outputBuffer[testIndex++] = (iONE < fNAN) ? 1u : 0u;    // int 1 < float NaN -> false
    outputBuffer[testIndex++] = (iONE > fNAN) ? 1u : 0u;    // int 1 > float NaN -> false
    outputBuffer[testIndex++] = (iONE <= fNAN) ? 1u : 0u;   // int 1 <= float NaN -> false
    outputBuffer[testIndex++] = (iONE >= fNAN) ? 1u : 0u;   // int 1 >= float NaN -> false

    // Test float NaN compared with int - same results but different operand order
    // CHECK: 0
    // CHECK: 1
    // CHECK: 0
    // CHECK: 0
    // CHECK: 0
    // CHECK: 0
    outputBuffer[testIndex++] = (fNAN == iONE) ? 1u : 0u;   // float NaN == int 1 -> false
    outputBuffer[testIndex++] = (fNAN != iONE) ? 1u : 0u;   // float NaN != int 1 -> true
    outputBuffer[testIndex++] = (fNAN < iONE) ? 1u : 0u;    // float NaN < int 1 -> false
    outputBuffer[testIndex++] = (fNAN > iONE) ? 1u : 0u;    // float NaN > int 1 -> false
    outputBuffer[testIndex++] = (fNAN <= iONE) ? 1u : 0u;   // float NaN <= int 1 -> false
    outputBuffer[testIndex++] = (fNAN >= iONE) ? 1u : 0u;   // float NaN >= int 1 -> false

    // Test with different int values to ensure consistent behavior
    // CHECK: 0
    // CHECK: 1
    // CHECK: 0
    // CHECK: 1
    outputBuffer[testIndex++] = (iZERO == fNAN) ? 1u : 0u;  // int 0 == float NaN -> false
    outputBuffer[testIndex++] = (iZERO != fNAN) ? 1u : 0u;  // int 0 != float NaN -> true
    outputBuffer[testIndex++] = (fNAN == iZERO) ? 1u : 0u;  // float NaN == int 0 -> false
    outputBuffer[testIndex++] = (fNAN != iZERO) ? 1u : 0u;  // float NaN != int 0 -> true

    // Test unsigned int with float NaN
    // CHECK: 0
    // CHECK: 1
    // CHECK: 0
    // CHECK: 1
    outputBuffer[testIndex++] = (uONE == fNAN) ? 1u : 0u;   // uint 1 == float NaN -> false
    outputBuffer[testIndex++] = (uONE != fNAN) ? 1u : 0u;   // uint 1 != float NaN -> true
    outputBuffer[testIndex++] = (fNAN == uONE) ? 1u : 0u;   // float NaN == uint 1 -> false
    outputBuffer[testIndex++] = (fNAN != uONE) ? 1u : 0u;   // float NaN != uint 1 -> true

    // Test normal int vs float comparisons (no NaN) to ensure type promotion works
    // CHECK: 1
    // CHECK: 1
    // CHECK: 1
    // CHECK: 1
    outputBuffer[testIndex++] = (iONE == fONE) ? 1u : 0u;   // int 1 == float 1.0 -> true
    outputBuffer[testIndex++] = (fONE == iONE) ? 1u : 0u;   // float 1.0 == int 1 -> true
    outputBuffer[testIndex++] = (iZERO < fONE) ? 1u : 0u;   // int 0 < float 1.0 -> true
    outputBuffer[testIndex++] = (fONE > iZERO) ? 1u : 0u;   // float 1.0 > int 0 -> true
}

