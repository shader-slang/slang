// Test that invalid types inside struct fields / array elements are detected
// recursively, including when the struct is a generic instantiation. Exercises
// multiple rule predicates through the recursive paths so a bug that affects
// one predicate shape but not another is caught.

//DIAGNOSTIC_TEST:SIMPLE(diag=DIFFSTRUCT,non-exhaustive):-target spirv -entry structParam -stage vertex
//DIAGNOSTIC_TEST:SIMPLE(diag=ATOMICSTRUCT,non-exhaustive):-target spirv -entry atomicStructParam -stage vertex
//DIAGNOSTIC_TEST:SIMPLE(diag=GENERIC,non-exhaustive):-target spirv -entry genericStructParam -stage vertex
//DIAGNOSTIC_TEST:SIMPLE(diag=DIFFARRAY,non-exhaustive):-target spirv -entry arrayParam -stage vertex
//DIAGNOSTIC_TEST:SIMPLE(diag=MATARRAY,non-exhaustive):-target spirv -entry matrixArrayParam -stage vertex

// --- Struct field recursion: DifferentialPair ---
struct BadFields
{
    float a;
    DifferentialPair<float> bad;
}

[shader("vertex")]
float4 structParam(BadFields b : A) : SV_Position
//DIFFSTRUCT: DifferentialPair/DifferentialPtrPair is not a valid varying type
{
    return float4(0);
}

// --- Struct field recursion: Atomic (different predicate shape) ---
struct AtomicField
{
    float a;
    Atomic<int> count;
}

[shader("vertex")]
float4 atomicStructParam(AtomicField b : A) : SV_Position
//ATOMICSTRUCT: Atomic is not a valid varying type
{
    return float4(0);
}

// --- Generic struct with substitution ---
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

// --- Array element recursion: DifferentialPair ---
[shader("vertex")]
float4 arrayParam(DifferentialPair<float> arr[4] : A) : SV_Position
//DIFFARRAY: DifferentialPair/DifferentialPtrPair is not a valid varying type
{
    return float4(0);
}

// --- Array element recursion: out-of-range matrix (different predicate) ---
[shader("vertex")]
float4 matrixArrayParam(matrix<float, 5, 5> arr[2] : A) : SV_Position
//MATARRAY: matrix row and column counts must be between 1 and 4
{
    return float4(0);
}
