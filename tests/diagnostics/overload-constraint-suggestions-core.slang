//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target spirv

// Check that real core overloads yield the documented floating-point constraint.
// Leave other core candidate diagnostics unconstrained as the overload set evolves.
void coreScalar<T : IFloat>(T value) { abs(value); }
/*CHECK:
                                          ^ no overload for 'abs' applicable to arguments of type (T)
                ^ consider constraining 'T' as 'T : __BuiltinFloatingPointType' to satisfy an overload's requirement '__BuiltinFloatingPointType'
*/
