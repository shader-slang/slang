//DIAGNOSTIC_TEST:SIMPLE(diag=diag):


// Generally speaking:
// Passing an argument for an `out` parameter such
// that implicit conversion would be required in
// both directions.
//
// But we do have special case code to handle uint/int of the same type and scalar/vector/matrix forms.
// So for this to fail we need to use floats or something else

void a(out float x)
{
	x = 0;
}

void b(double y)
{
	a(y);
/*diag:
   ^ implicit conversion from 'double' to 'float' is not recommended
   ^ argument must be l-value
   ^ argument passed to parameter '0' must be l-value.
   ^ argument was implicitly cast from 'double' to 'float', and Slang does not support using an implicit cast as an l-value with this type
*/
}
