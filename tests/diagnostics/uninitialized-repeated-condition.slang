//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv

extern static const int MODE = 1;
static const bool ENABLED = MODE == 1;

struct Data
{
    float value;
};

// Check that the same parameter guarding a store and its load produces no warning.
float repeatedParameter(bool condition)
{
    Data data;
    if (condition)
        data.value = 3.0;
    if (condition)
        return data.value;
    return 0;
}

// Check that an unrelated branch between the guarded store and load produces no warning.
float interveningDiamond(bool condition, bool other)
{
    Data data;
    if (condition)
        data.value = 3.0;
    float result;
    if (other)
        result = 1.0;
    else
        result = 2.0;
    if (condition)
        result += data.value;
    return result;
}

// Check that repeated false branches protect the load just as repeated true branches do.
float repeatedFalseBranch(bool condition)
{
    Data data;
    if (condition)
    {}
    else
        data.value = 3.0;
    if (condition)
        return 0;
    else
        return data.value;
}

// Check that Conditional.set/get under the same specialization constant produces no warning.
[shader("compute")]
void main(uniform float* output)
{
    Conditional<float4, ENABLED> value;
    if (ENABLED)
        value.set(float4(3.0f, 0, 0, 0));
    if (ENABLED)
        output[0] = value.get().value.x;
}
