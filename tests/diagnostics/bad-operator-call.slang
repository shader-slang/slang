// bad-operator-call.slang

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

// Test that bad calls to operators produce reasonable diagnostic messages.
//
// This test is `non-exhaustive` because a failed call to a builtin operator
// now also emits a per-candidate "argument N does not match" note (E40018) for
// each of the operator's many overloads (issue #7857); enumerating all of them
// here would be unwieldy and brittle.

struct S {}

void test()
{
	int a;
    S b;
    a += b;
/*CHECK:
         ^ type mismatch in expression
         ^ expected an expression of type 'vector<int,1>', got 'S'
    ^ argument must be l-value
    ^ argument passed to parameter '0' must be l-value.
    ^ argument was implicitly cast from 'int' to 'vector<int,1>', and Slang does not support using an implicit cast as an l-value with this type
*/
    a = a + b;
/*CHECK:
          ^ no overload for '+' applicable to arguments of type (int, S)
*/
    // The binary-operator failure path (NoApplicableWithArgs, no funcName) must
    // still emit the per-candidate argument-mismatch note (E40018) from issue
    // #7857. Matched loosely (operators have many overloads, so the exact
    // expected type varies); this guards against the note being suppressed on the
    // operator path.
//CHECK: does not match: expected
	a = ~b;
/*CHECK:
     ^ no overload for '~' applicable to arguments of type (S)
*/
    // ...and the unary-operator path must emit it too. This ordered `CHECK`
    // follows the '~' error above, so the argument-0 mismatch note it matches is
    // the one for the unary '~' rejection — guarding against E40018 being
    // suppressed specifically on the unary path while kept on the binary path.
//CHECK: argument 0 does not match: expected

	vector<int, 4> c;
    vector<float, 3> d;
	a += c;
/*CHECK:
 ^ argument must be l-value
 ^ argument passed to parameter '0' must be l-value.
 ^ argument was implicitly cast from 'int' to 'vector<int,4>', and Slang does not support using an implicit cast as an l-value with this type
*/

    c = a + c;
    d += c;
/*CHECK:
      ^^ no overload for '+=' applicable to arguments of type (vector<float,3>, vector<int,4>)
*/
	d = c + d;
/*CHECK:
       ^ no overload for '+' applicable to arguments of type (vector<int,4>, vector<float,3>)
*/
}
