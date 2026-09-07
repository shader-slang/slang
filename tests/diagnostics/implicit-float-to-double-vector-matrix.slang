//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// The "implicit float-to-double conversion" warning (E30082) fires for element-wise float->double
// widening passed as a call argument, in scalar, vector, AND matrix shapes. The check is exhaustive,
// so an unannotated line asserts it emits no diagnostic at all: the float literal and the
// scalar->vector splat below are unannotated (proving silence), while the double->float narrowing is
// annotated for its own, different, E30081 warning.

void takeDoubleScalar(double v) {}
void takeDoubleVector(double4 v) {}
void takeDoubleMatrix(double4x4 v) {}
void takeFloatScalar(float v) {}

void test(float fs, float4 fv, float4x4 fm, double ds)
{
    // Positives: element-wise float->double widening as call arguments.
    takeDoubleScalar(fs);
//CHECK:             ^^ implicit float-to-double conversion
//CHECK:             ^^ implicit float-to-double conversion may cause unexpected performance issues, use explicit cast if intended.
    takeDoubleVector(fv);
//CHECK:             ^^ implicit float-to-double conversion
//CHECK:             ^^ implicit float-to-double conversion may cause unexpected performance issues, use explicit cast if intended.
    takeDoubleMatrix(fm);
//CHECK:             ^^ implicit float-to-double conversion
//CHECK:             ^^ implicit float-to-double conversion may cause unexpected performance issues, use explicit cast if intended.

    // A vector built from float literals is a float-valued widening (its argument is a `float4`
    // constructor, not a bare literal), so unlike the exempt scalar literal below it still warns.
    takeDoubleVector(float4(1.0f, 2.0f, 3.0f, 4.0f));
//CHECK:                   ^ implicit float-to-double conversion
//CHECK:                   ^ implicit float-to-double conversion may cause unexpected performance issues, use explicit cast if intended.

    // Negatives: a double->float narrowing emits a different warning, a bare float literal is exempt,
    // and a scalar->vector splat is a different shape -- none produce the float-to-double warning.
    takeFloatScalar(ds);
//CHECK:            ^^ implicit conversion not recommended
//CHECK:            ^^ implicit conversion from 'double' to 'float' is not recommended
    takeDoubleScalar(1.0f);
    takeDoubleVector(fs);
}
