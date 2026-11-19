// THIS IS A GENERATED FILE. DO NOT EDIT!
// Instead, edit extras/test-generators/generate-generic-type-promotion-tests.py
// and regenerate by running:
//
//     python3 extras/test-generators/generate-generic-type-promotion-tests.py
//
// Equal-cost ties: generic parameter promotion inference.
// Tests 8 type pair combinations in both argument orders.
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
        intptr_t b = intptr_t(7);
        joinArgs(a, b);
/*CHECK: cannot deduce generic argument for 'T': conflicting requirements 'bool' and 'intptr_t'
*/
        joinArgs(b, a);
/*CHECK: cannot deduce generic argument for 'T': conflicting requirements 'intptr_t' and 'bool'
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

    {
        uint8_t a = uint8_t(3);
        intptr_t b = intptr_t(7);
        joinArgs(a, b);
/*CHECK: cannot deduce generic argument for 'T': conflicting requirements 'uint8_t' and 'intptr_t'
*/
        joinArgs(b, a);
/*CHECK: cannot deduce generic argument for 'T': conflicting requirements 'intptr_t' and 'uint8_t'
*/
    }

    {
        uint16_t a = uint16_t(3);
        intptr_t b = intptr_t(7);
        joinArgs(a, b);
/*CHECK: cannot deduce generic argument for 'T': conflicting requirements 'uint16_t' and 'intptr_t'
*/
        joinArgs(b, a);
/*CHECK: cannot deduce generic argument for 'T': conflicting requirements 'intptr_t' and 'uint16_t'
*/
    }

    {
        uint32_t a = 3u;
        intptr_t b = intptr_t(7);
        joinArgs(a, b);
/*CHECK: cannot deduce generic argument for 'T': conflicting requirements 'uint32_t' and 'intptr_t'
*/
        joinArgs(b, a);
/*CHECK: cannot deduce generic argument for 'T': conflicting requirements 'intptr_t' and 'uint32_t'
*/
    }

    {
        int64_t a = int64_t(3);
        uintptr_t b = uintptr_t(7);
        joinArgs(a, b);
/*CHECK: cannot deduce generic argument for 'T': conflicting requirements 'int64_t' and 'uintptr_t'
*/
        joinArgs(b, a);
/*CHECK: cannot deduce generic argument for 'T': conflicting requirements 'uintptr_t' and 'int64_t'
*/
    }
}
