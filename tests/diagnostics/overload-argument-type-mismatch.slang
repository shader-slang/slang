// Regression test for https://github.com/shader-slang/slang/issues/7857
//
// When no overload of a function applies to the given arguments, each candidate
// note should also report which argument failed to match and the expected vs.
// actual type, so the user can see precisely why the call did not resolve.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

struct Test {}

float overloaded(float2 x, float y) { return 0; }
/*CHECK:
      ^^^^^^^^^^ candidate: func overloaded(float2, float) -> float
      ^^^^^^^^^^ argument 0 does not match: expected 'vector<float,2>', got 'Test'
*/

float overloaded(float3 x, float y) { return 0; }
/*CHECK:
      ^^^^^^^^^^ candidate: func overloaded(float3, float) -> float
      ^^^^^^^^^^ argument 0 does not match: expected 'vector<float,3>', got 'Test'
*/

float test()
{
    return overloaded(Test(), 0);
/*CHECK:
                     ^ no overload for 'overloaded' applicable to arguments of type (Test, int)
*/
}
