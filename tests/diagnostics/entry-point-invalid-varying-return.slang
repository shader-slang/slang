// Test that invalid types are rejected as entry-point return types.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv -entry atomicReturn -stage vertex
//DIAGNOSTIC_TEST:SIMPLE(diag=DIFFPAIR,non-exhaustive):-target spirv -entry diffPairReturn -stage vertex

// Atomic<T> as a return type
[shader("vertex")]
Atomic<int> atomicReturn()
//CHECK: Atomic is not a valid varying type
{
    return {};
}

// DifferentialPair<T> as a return type
[shader("vertex")]
DifferentialPair<float> diffPairReturn()
//DIFFPAIR: DifferentialPair is not a valid varying type
{
    return {};
}
