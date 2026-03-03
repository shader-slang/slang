//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):


T doThing<T>(T in)
{
    return in;
}

__target_intrinsic(hlsl, "doSomethingIntrinsically($0)")
T doThing<T>(T in);
/*CHECK:
  ^^^^^^^ see previous definition of 'doThing'
*/

__target_intrinsic(hlsl, "letsRedefineIt($0)")
T doThing<T>(T in);
/*CHECK:
  ^^^^^^^ function 'doThing' already has a body
  ^^^^^^^ redeclared here
*/

void test()
{
    int a = 5;
    int b = doThing(a);
}
