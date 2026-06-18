//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

// A failing *equality* constraint (`where T == int`) is solved by the same
// routine as interface-conformance constraints, but its right-hand side is a
// concrete type, not an interface. It must report the general "generic
// constraint not satisfied" diagnostic, rendering the constraint adaptively as
// an equality (`T == int`, not a hard-coded form), with a see-declaration note,
// and NOT the interface-conformance message.

void h<T>(T x) where T == int {}

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    float f = 1.0;
    h(f);
/*CHECK: could not satisfy the generic constraint 'T == int'
*/
//CHECK: see generic constraint declaration
//CHECK-NOT: does not conform to interface
}
