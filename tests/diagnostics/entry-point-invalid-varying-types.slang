// Test that invalid types are rejected as entry-point varying parameters.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -entry diffPairParam -stage vertex
//DIAGNOSTIC_TEST:SIMPLE(diag=ATOMIC):-target spirv -entry atomicParam -stage vertex
//DIAGNOSTIC_TEST:SIMPLE(diag=COOPVEC):-target spirv -entry coopVecParam -stage vertex
//DIAGNOSTIC_TEST:SIMPLE(diag=MATRIX):-target spirv -entry matrixParam -stage vertex

// DifferentialPair<T> as a varying parameter
[shader("vertex")]
float4 diffPairParam(DifferentialPair<float> dp : A) : SV_Position
//CHECK: error 38050
{
    return float4(0);
}

// Atomic<T> as a varying parameter
[shader("vertex")]
float4 atomicParam(Atomic<int> a : A) : SV_Position
//ATOMIC: error 38050
{
    return float4(0);
}

// CoopVec<T, N> as a varying parameter
[shader("vertex")]
float4 coopVecParam(CoopVec<float, 4> v : A) : SV_Position
//COOPVEC: error 38050
{
    return float4(0);
}

// matrix with non-standard element type as a SPIR-V varying parameter
[shader("vertex")]
float4 matrixParam(matrix<uint64_t, 2, 2> m : A) : SV_Position
//MATRIX: error 38051
{
    return float4(0);
}
