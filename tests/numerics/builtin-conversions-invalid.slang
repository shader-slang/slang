// Verify that the conversion dispatch marker excludes types without compiler-defined scalar casts.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK, non-exhaustive): -experimental-feature

// CHECK: type argument 'void' does not conform to the required interface '__BuiltinScalarType'
// CHECK: type argument 'CustomScalar' does not conform to the required interface '__BuiltinScalarType'
// CHECK: type argument 'CustomEnum' does not conform to the required interface '__BuiltinScalarType'
// CHECK: type argument 'BFloat16' does not conform to the required interface '__BuiltinScalarType'
// CHECK: type argument 'FloatE4M3' does not conform to the required interface '__BuiltinScalarType'
// CHECK: type argument 'FloatE5M2' does not conform to the required interface '__BuiltinScalarType'

import slang.numerics;

struct CustomScalar
{}

enum CustomEnum
{
    value,
}

void requireBuiltinScalar<T : IBuiltinScalarTypeDispatchMarker>()
{}

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    requireBuiltinScalar<void>();
    requireBuiltinScalar<CustomScalar>();
    requireBuiltinScalar<CustomEnum>();
    requireBuiltinScalar<BFloat16>();
    requireBuiltinScalar<FloatE4M3>();
    requireBuiltinScalar<FloatE5M2>();
}
