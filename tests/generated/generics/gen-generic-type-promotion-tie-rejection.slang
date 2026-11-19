// THIS IS A GENERATED FILE. DO NOT EDIT!
// Instead, edit extras/test-generators/generate-generic-type-promotion-tests.py
// and regenerate by running:
//
//     python3 extras/test-generators/generate-generic-type-promotion-tests.py
//
// Equal-cost ties: generic parameter promotion inference.
// Tests 3 type pair combinations in both argument orders.
//
// Each pair's conversion costs tie in both directions, so neither type is
// better and inference fails instead of picking a side (issue #10164).
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

void joinArgs<T>(T a, T b)
{
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    {
        bool a = true;
        half b = half(7);
        joinArgs(a, b);
/*CHECK: cannot deduce generic argument for 'T': conflicting requirements 'bool' and 'half'
*/
        joinArgs(b, a);
/*CHECK: cannot deduce generic argument for 'T': conflicting requirements 'half' and 'bool'
*/
    }

    {
        bool a = true;
        float b = float(7);
        joinArgs(a, b);
/*CHECK: cannot deduce generic argument for 'T': conflicting requirements 'bool' and 'float'
*/
        joinArgs(b, a);
/*CHECK: cannot deduce generic argument for 'T': conflicting requirements 'float' and 'bool'
*/
    }

    {
        bool a = true;
        double b = double(7);
        joinArgs(a, b);
/*CHECK: cannot deduce generic argument for 'T': conflicting requirements 'bool' and 'double'
*/
        joinArgs(b, a);
/*CHECK: cannot deduce generic argument for 'T': conflicting requirements 'double' and 'bool'
*/
    }
}
