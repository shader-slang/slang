//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv -entry mainParam -stage vertex
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK2,non-exhaustive):-target spirv -entry mainReturn -stage vertex

// Test that non-float matrix element types are diagnosed on SPIR-V targets
// when used as entry-point varying parameters or return types (issue #9451).

[shader("vertex")]
void mainParam(matrix<uint64_t> a)
//CHECK:                        ^ type 'matrix<uint64_t,4,4>' cannot be used as input parameter 'a' of entry point 'mainParam' when targeting spirv because matrix element type must be a floating-point type (half, float, or double) for SPIR-V entry-point varyings
{
    unused(a);
}

[shader("vertex")]
matrix<uint64_t> mainReturn()
//CHECK2:        ^^^^^^^^^^ type 'matrix<uint64_t,4,4>' cannot be used as output return type of entry point 'mainReturn' when targeting spirv because matrix element type must be a floating-point type (half, float, or double) for SPIR-V entry-point varyings
{
    return {};
}
