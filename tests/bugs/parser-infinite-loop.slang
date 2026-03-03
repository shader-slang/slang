//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):
// Using non-exhaustive because this produces many cascading parse errors

struct test
{
    float3 field;
}
void f()
{
    test x; x
    vector<int,2> v;
//CHECK:       ^ unexpected token
}
