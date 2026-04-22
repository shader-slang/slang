//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Test: diagnostic when IDifferentiable conformance is provided via extension
// but no explicit dzero/dadd are given, and the struct has differentiable fields.

struct MyVec<T : __BuiltinArithmeticType>
{
    T x;
/*CHECK:
      ^ synthesized IDifferentiable method will not include differentiable field
      ^ field 'x' of type 'T' is differentiable but will not be included in the synthesized 'dzero'/'dadd' methods because the 'IDifferentiable' conformance is provided via extension. Provide explicit 'dzero()' and 'dadd()' implementations, or mark the field 'no_diff'.
      ^ synthesized IDifferentiable method will not include differentiable field
      ^ field 'x' of type 'T' is differentiable but will not be included in the synthesized 'dzero'/'dadd' methods because the 'IDifferentiable' conformance is provided via extension. Provide explicit 'dzero()' and 'dadd()' implementations, or mark the field 'no_diff'.
*/
    T y;
/*CHECK:
      ^ synthesized IDifferentiable method will not include differentiable field
      ^ field 'y' of type 'T' is differentiable but will not be included in the synthesized 'dzero'/'dadd' methods because the 'IDifferentiable' conformance is provided via extension. Provide explicit 'dzero()' and 'dadd()' implementations, or mark the field 'no_diff'.
      ^ synthesized IDifferentiable method will not include differentiable field
      ^ field 'y' of type 'T' is differentiable but will not be included in the synthesized 'dzero'/'dadd' methods because the 'IDifferentiable' conformance is provided via extension. Provide explicit 'dzero()' and 'dadd()' implementations, or mark the field 'no_diff'.
*/
    int count; // non-differentiable, should NOT trigger diagnostic
};

extension<T : __BuiltinFloatingPointType> MyVec<T> : IDifferentiable
    where T.Differential == T
{
    typedef MyVec<T> Differential;
    // No explicit dzero/dadd -- should produce diagnostics for x and y.
}

// Test: fields are differentiable at both definition and extension site.
// Diagnostic should NOT fire because dzero/dadd synthesis can reach the fields.
struct MyVec2<T : __BuiltinFloatingPointType>
    where T.Differential == T
{
    T x;
    T y;
};

extension<T : __BuiltinFloatingPointType> MyVec2<T> : IDifferentiable
    where T.Differential == T
{
    typedef MyVec2<T> Differential;
    // No explicit dzero/dadd, but fields are differentiable at the definition site.
}
