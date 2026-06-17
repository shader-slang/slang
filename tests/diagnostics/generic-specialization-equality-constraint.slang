//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

// A failing *equality* constraint (`where T == int`) is solved by the same
// routine as interface-conformance constraints, but its right-hand side is a
// concrete type, not an interface. The focused "does not conform to interface"
// diagnostic must NOT fire here; the failure should fall through to the generic
// specialization fallback instead.

void h<T>(T x) where T == int {}

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    float f = 1.0;
    h(f);
/*CHECK: could not specialize generic for arguments of type
*/
//CHECK-NOT: does not conform to interface
}
