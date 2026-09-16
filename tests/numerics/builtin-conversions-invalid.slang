// Verify that the conversion dispatch marker excludes void and user-defined types.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK, non-exhaustive): -experimental-feature

// CHECK: type argument 'void' does not conform to the required interface '__BuiltinScalarType'
// CHECK: type argument 'CustomScalar' does not conform to the required interface '__BuiltinScalarType'

import slang.numerics;

struct CustomScalar
{}

void requireBuiltinScalar<T : IBuiltinScalarTypeDispatchMarker>()
{}

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    requireBuiltinScalar<void>();
    requireBuiltinScalar<CustomScalar>();
}
