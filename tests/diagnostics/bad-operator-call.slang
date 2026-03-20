// bad-operator-call.slang

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Test that bad calls to operators produce reasonable diagnostic messages.

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
	a = ~b;
/*CHECK:
     ^ no overload for '~' applicable to arguments of type (S)
*/

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
