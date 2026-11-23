// THIS IS A GENERATED FILE. DO NOT EDIT!
// Instead, edit extras/test-generators/generate-generic-type-promotion-tests.py
// and regenerate by running:
//
//     python3 extras/test-generators/generate-generic-type-promotion-tests.py
//
// Equal-cost ties: generic parameter promotion inference.
// Tests 4 type pair combinations in both argument orders.
//
// Each pair's conversion costs tie in both directions and neither side is
// `bool` (which loses its ties by the compatibility rule), so neither type
// is better and inference fails instead of picking a side (issue #10164).
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

void joinArgs<T>(T a, T b)
{
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
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
