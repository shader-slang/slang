// Test that invalid types inside struct fields are detected recursively,
// including when the struct is a generic instantiation.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv -entry structParam -stage vertex
//DIAGNOSTIC_TEST:SIMPLE(diag=GENERIC,non-exhaustive):-target spirv -entry genericStructParam -stage vertex
//DIAGNOSTIC_TEST:SIMPLE(diag=ARRAY,non-exhaustive):-target spirv -entry arrayParam -stage vertex

struct BadFields
{
    float a;
    DifferentialPair<float> bad;
}

// Invalid type as a field of a varying struct parameter
[shader("vertex")]
float4 structParam(BadFields b : A) : SV_Position
//CHECK: DifferentialPair/DifferentialPtrPair is not a valid varying type
{
    return float4(0);
}

struct Wrapper<T>
{
    T x;
}

// Invalid type hidden behind a generic struct parameter substitution.
// This previously escaped validation because field types were taken from the
// declaration without applying substitutions.
[shader("vertex")]
float4 genericStructParam(Wrapper<DifferentialPair<float>> w : A) : SV_Position
//GENERIC: DifferentialPair/DifferentialPtrPair is not a valid varying type
{
    return float4(0);
}

// Invalid type as the element of an array parameter.
[shader("vertex")]
float4 arrayParam(DifferentialPair<float> arr[4] : A) : SV_Position
//ARRAY: DifferentialPair/DifferentialPtrPair is not a valid varying type
{
    return float4(0);
}
