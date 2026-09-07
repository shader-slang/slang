//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// The "implicit float-to-double conversion" warning (E30082) fires for element-wise float->double
// widening passed as a call argument, in scalar, vector, AND matrix shapes. The check is
// exhaustive: the negative cases below carry no annotations, which asserts they emit no such warning.

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

    // Negatives: a double->float narrowing emits a different warning, a float literal is exempt,
    // and a scalar->vector splat is a different shape -- none produce the float-to-double warning.
    takeFloatScalar(ds);
//CHECK:            ^^ implicit conversion not recommended
//CHECK:            ^^ implicit conversion from 'double' to 'float' is not recommended
    takeDoubleScalar(1.0f);
    takeDoubleVector(fs);
}
