// conditional-link-time-const.slang
// Test that logical operators (!, &&, ||) work with link-time constants
// in Conditional<> type parameters using extern const with default values.
// This is similar to the unit test but without linking an override module.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHK):-slang -compute -shaderobj
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHK):-vk -compute -shaderobj
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHK):-cpu -compute -shaderobj

// Expected output with default values (HAS_FEATURE=false, ENABLE_EXTRA=false):
// - notValue: !false = true, has value 5.0
// - andValue: false && false = false, empty (0.0)
// - orValue: false || !false = true, has value 20.0
// Total: 5.0 + 0.0 + 20.0 = 25.0 = 0x19
//CHK:19

extern static const bool HAS_FEATURE = false;
extern static const bool ENABLE_EXTRA = false;

// Define a struct with Conditional fields that use link-time constants
struct TestData<bool notFlag, bool andFlag, bool orFlag>
{
    Conditional<float, notFlag> notValue;
    Conditional<float, andFlag> andValue;
    Conditional<float, orFlag> orValue;

    __init(float notVal, float andVal, float orVal)
    {
        notValue = notVal;
        andValue = andVal;
        orValue = orVal;
    }

    float sum()
    {
        float result = 0.0f;

        if (let v = notValue.get())
            result += v;

        if (let v = andValue.get())
            result += v;

        if (let v = orValue.get())
            result += v;

        return result;
    }
}

// Instantiate the struct with link-time constant expressions
// With defaults (HAS_FEATURE=false, ENABLE_EXTRA=false):
// - !HAS_FEATURE = !false = true
// - HAS_FEATURE && ENABLE_EXTRA = false && false = false
// - HAS_FEATURE || !ENABLE_EXTRA = false || true = true
typealias TestInstance = TestData<
    !HAS_FEATURE,
    HAS_FEATURE && ENABLE_EXTRA,
    HAS_FEATURE || !ENABLE_EXTRA
>;

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(int3 dispatchThreadID : SV_DispatchThreadID)
{
    // Create an instance and initialize with test values
    TestInstance testData = TestInstance(5.0f, 10.0f, 20.0f);

    // Sum the values that are present based on link-time constants
    // With defaults (HAS_FEATURE=false, ENABLE_EXTRA=false):
    // - notValue: !false = true, field exists, contributes 5.0
    // - andValue: false && false = false, field removed, contributes 0.0
    // - orValue: false || true = true, field exists, contributes 20.0
    // Expected: 5.0 + 0.0 + 20.0 = 25.0

    outputBuffer[0] = int(testData.sum());
}
