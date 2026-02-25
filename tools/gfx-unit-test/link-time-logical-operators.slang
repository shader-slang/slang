// Test that logical operators (!, &&, ||) work with link-time constants
// in Conditional<> type parameters, and that extern const values can be overridden.

extern static const bool HAS_FEATURE = false;
extern static const bool ENABLE_EXTRA = false;

// Define a struct with Conditional fields that use link-time constants
// Generic params are bool because Conditional expects bool, and !, &&, || return bool
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
typealias TestInstance = TestData<
    !HAS_FEATURE,                    // notValue: !false = true (default), !true = false (override)
    HAS_FEATURE && ENABLE_EXTRA,     // andValue: false && false = false (default), true && true = true (override)
    HAS_FEATURE || !ENABLE_EXTRA     // orValue: false || true = true (default), true || false = true (override)
>;

RWStructuredBuffer<float> Output;

[shader("compute")]
[NumThreads(1, 1, 1)]
void computeMain()
{
    // Create an instance and initialize with test values
    TestInstance testData = TestInstance(5.0f, 10.0f, 20.0f);

    // Sum the values that are present based on link-time constants
    // When HAS_FEATURE = true, ENABLE_EXTRA = true (overridden at link time):
    // - notValue: !true = false, field is removed, contributes 0.0
    // - andValue: true && true = true, field exists, contributes 10.0
    // - orValue: true || false = true, field exists, contributes 20.0
    // Expected: 0.0 + 10.0 + 20.0 = 30.0

    Output[0] = testData.sum();
}
