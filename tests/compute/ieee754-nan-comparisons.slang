//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK): -shaderobj -output-using-type -compute

// Test IEEE 754 NaN comparison behavior
// According to IEEE 754 standard:
// - Any comparison with NaN (except !=) should return false
// - The != comparison with NaN should return true

static const float fNAN = 0.0f / 0.0f;
static const float fPOSITIVE_INFINITY = 1.0f / 0.0f;
static const float fNEGATIVE_INFINITY = -1.0f / 0.0f;
static const float fZERO = 0.0f;
static const float fONE = 1.0f;
static const float fNEG_ONE = -1.0f;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint testIndex = 0;

    // Test 1: NaN == NaN should be false
    // CHECK: 0
    outputBuffer[testIndex++] = (fNAN == fNAN) ? 1u : 0u;

    // Test 2: NaN != NaN should be true
    // CHECK: 1
    outputBuffer[testIndex++] = (fNAN != fNAN) ? 1u : 0u;

    // Test 3: NaN > NaN should be false
    // CHECK: 0
    outputBuffer[testIndex++] = (fNAN > fNAN) ? 1u : 0u;

    // Test 4: NaN < NaN should be false
    // CHECK: 0
    outputBuffer[testIndex++] = (fNAN < fNAN) ? 1u : 0u;

    // Test 5: NaN >= NaN should be false
    // CHECK: 0
    outputBuffer[testIndex++] = (fNAN >= fNAN) ? 1u : 0u;

    // Test 6: NaN <= NaN should be false
    // CHECK: 0
    outputBuffer[testIndex++] = (fNAN <= fNAN) ? 1u : 0u;

    // Test 7: NaN == 0.0 should be false
    // CHECK: 0
    outputBuffer[testIndex++] = (fNAN == fZERO) ? 1u : 0u;

    // Test 8: NaN != 0.0 should be true
    // CHECK: 1
    outputBuffer[testIndex++] = (fNAN != fZERO) ? 1u : 0u;

    // Test 9: NaN > 0.0 should be false
    // CHECK: 0
    outputBuffer[testIndex++] = (fNAN > fZERO) ? 1u : 0u;

    // Test 10: NaN < 0.0 should be false
    // CHECK: 0
    outputBuffer[testIndex++] = (fNAN < fZERO) ? 1u : 0u;

    // Test 11: NaN >= 0.0 should be false
    // CHECK: 0
    outputBuffer[testIndex++] = (fNAN >= fZERO) ? 1u : 0u;

    // Test 12: NaN <= 0.0 should be false
    // CHECK: 0
    outputBuffer[testIndex++] = (fNAN <= fZERO) ? 1u : 0u;

    // Test 13: 0.0 == NaN should be false
    // CHECK: 0
    outputBuffer[testIndex++] = (fZERO == fNAN) ? 1u : 0u;

    // Test 14: 0.0 != NaN should be true
    // CHECK: 1
    outputBuffer[testIndex++] = (fZERO != fNAN) ? 1u : 0u;

    // Test 15: 0.0 > NaN should be false
    // CHECK: 0
    outputBuffer[testIndex++] = (fZERO > fNAN) ? 1u : 0u;

    // Test 16: 0.0 < NaN should be false
    // CHECK: 0
    outputBuffer[testIndex++] = (fZERO < fNAN) ? 1u : 0u;

    // Test 17: 0.0 >= NaN should be false
    // CHECK: 0
    outputBuffer[testIndex++] = (fZERO >= fNAN) ? 1u : 0u;

    // Test 18: 0.0 <= NaN should be false
    // CHECK: 0
    outputBuffer[testIndex++] = (fZERO <= fNAN) ? 1u : 0u;

    // Test 19: NaN == +infinity should be false
    // CHECK: 0
    outputBuffer[testIndex++] = (fNAN == fPOSITIVE_INFINITY) ? 1u : 0u;

    // Test 20: NaN != +infinity should be true
    // CHECK: 1
    outputBuffer[testIndex++] = (fNAN != fPOSITIVE_INFINITY) ? 1u : 0u;

    // Test 21: NaN > +infinity should be false
    // CHECK: 0
    outputBuffer[testIndex++] = (fNAN > fPOSITIVE_INFINITY) ? 1u : 0u;

    // Test 22: NaN < +infinity should be false
    // CHECK: 0
    outputBuffer[testIndex++] = (fNAN < fPOSITIVE_INFINITY) ? 1u : 0u;

    // Test 23: NaN >= +infinity should be false
    // CHECK: 0
    outputBuffer[testIndex++] = (fNAN >= fPOSITIVE_INFINITY) ? 1u : 0u;

    // Test 24: NaN <= +infinity should be false
    // CHECK: 0
    outputBuffer[testIndex++] = (fNAN <= fPOSITIVE_INFINITY) ? 1u : 0u;

    // Test 25: NaN == -infinity should be false
    // CHECK: 0
    outputBuffer[testIndex++] = (fNAN == fNEGATIVE_INFINITY) ? 1u : 0u;

    // Test 26: NaN != -infinity should be true
    // CHECK: 1
    outputBuffer[testIndex++] = (fNAN != fNEGATIVE_INFINITY) ? 1u : 0u;

    // Test 27: NaN > -infinity should be false
    // CHECK: 0
    outputBuffer[testIndex++] = (fNAN > fNEGATIVE_INFINITY) ? 1u : 0u;

    // Test 28: NaN < -infinity should be false
    // CHECK: 0
    outputBuffer[testIndex++] = (fNAN < fNEGATIVE_INFINITY) ? 1u : 0u;

    // Test 29: NaN >= -infinity should be false
    // CHECK: 0
    outputBuffer[testIndex++] = (fNAN >= fNEGATIVE_INFINITY) ? 1u : 0u;

    // Test 30: NaN <= -infinity should be false
    // CHECK: 0
    outputBuffer[testIndex++] = (fNAN <= fNEGATIVE_INFINITY) ? 1u : 0u;

    // Test 31: NaN == 1.0 should be false
    // CHECK: 0
    outputBuffer[testIndex++] = (fNAN == fONE) ? 1u : 0u;

    // Test 32: NaN != 1.0 should be true
    // CHECK: 1
    outputBuffer[testIndex++] = (fNAN != fONE) ? 1u : 0u;
}
