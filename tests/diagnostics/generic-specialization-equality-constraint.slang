//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

// A failing *equality* constraint (`where T == int`) is solved by the same
// routine as interface-conformance constraints, but its right-hand side is a
// concrete type, not an interface. It must report the general "does not satisfy
// generic constraint" diagnostic (with a see-declaration note), NOT the
// interface-conformance message.

void h<T>(T x) where T == int {}

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    float f = 1.0;
    h(f);
/*CHECK: generic argument 'float' does not satisfy the generic constraint 'T == int'
*/
//CHECK: see generic constraint declaration
//CHECK-NOT: does not conform to interface
}
