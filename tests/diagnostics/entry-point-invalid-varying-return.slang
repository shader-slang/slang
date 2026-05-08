// Test that invalid types are rejected as entry-point return types.
//
// Covers the return-type side of:
//   #9429 DifferentialPair<T>
//   #9443 Atomic<T>
//   #9446 #9448 CoopVec<T, N>
//   #9449 CoopMat<...>
//   #9452 vector<bool> (SPIR-V)

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv -entry atomicReturn -stage vertex
//DIAGNOSTIC_TEST:SIMPLE(diag=DIFFPAIR,non-exhaustive):-target spirv -entry diffPairReturn -stage vertex
//DIAGNOSTIC_TEST:SIMPLE(diag=COOPVEC,non-exhaustive):-target spirv -entry coopVecReturn -stage vertex
//DIAGNOSTIC_TEST:SIMPLE(diag=COOPMAT,non-exhaustive):-target spirv -entry coopMatReturn -stage vertex
//DIAGNOSTIC_TEST:SIMPLE(diag=VECBOOL,non-exhaustive):-target spirv -entry vecBoolReturn -stage vertex

import linalg;

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
//DIFFPAIR: DifferentialPair/DifferentialPtrPair is not a valid varying type
{
    return {};
}

// CoopVec<T, N> as a return type
[shader("vertex")]
CoopVec<float, 4> coopVecReturn()
//COOPVEC: CoopVec is not a valid varying type for this stage
{
    return {};
}

// CoopMat<...> as a return type
[shader("vertex")]
linalg::CoopMat<float, MemoryScope::CrossDevice, 1, 1, linalg::CoopMatMatrixUse::MatrixA> coopMatReturn()
//COOPMAT: CoopMat is not a valid varying type for this stage
{
    return {};
}

// vector<bool> as a SPIR-V return type
[shader("vertex")]
vector<bool, 4> vecBoolReturn()
//VECBOOL: vector<bool> is not a valid SPIR-V varying type for this stage
{
    return {};
}
