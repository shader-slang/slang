//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Test that passing the same variable to multiple out/inout/ref parameters
// produces a warning about potential aliasing (issue #10699).

void addConsumeTwoValues(
    inout int i1, inout int i2, inout int o)
{
    o += i1;
    i1 = -1;
    o += i2;
    i2 = -1;
}

void twoOut(out int a, out int b)
{
    a = 1;
    b = 2;
}

void inAndInout(int a, inout int b)
{
    b += a;
}

void twoRef(__ref int a, __ref int b)
{
    int t = a; a = b; b = t;
}

struct S { int x; int y; }

void twoInoutInt(inout int a, inout int b)
{
    a += b; b -= a;
}

void twoInoutUint(inout uint a, inout uint b)
{
    a += b; b -= a;
}

void readBoth(int a, int b) {}

[numthreads(1,1,1)]
void computeMain()
{
    int a = 0;
    int b = 1;

    // Positive: same variable to two inout params.
    addConsumeTwoValues(a, b, a);
//CHECK:                ^ potentially aliased argument to 'out'/'inout'/'ref' parameter
//CHECK:                ^ argument for 'inout' parameter 'i1' may alias argument for 'inout' parameter 'o'; passing the same value to both may produce unexpected results

    // Positive: same variable to two out params.
    twoOut(a, a);
//CHECK:   ^ potentially aliased argument to 'out'/'inout'/'ref' parameter
//CHECK:   ^ argument for 'out' parameter 'a' may alias argument for 'out' parameter 'b'; passing the same value to both may produce unexpected results

    // Positive: same variable to in and inout params.
    inAndInout(a, a);
//CHECK:          ^ potentially aliased argument to 'out'/'inout'/'ref' parameter
//CHECK:          ^ argument for 'inout' parameter 'b' may alias argument for 'in' parameter 'a'; passing the same value to both may produce unexpected results

    // Positive: same variable to two ref params.
    twoRef(a, a);
//CHECK:   ^ potentially aliased argument to 'out'/'inout'/'ref' parameter
//CHECK:   ^ argument for 'ref' parameter 'a' may alias argument for 'ref' parameter 'b'; passing the same value to both may produce unexpected results

    // Positive: identical struct field to two inout params.
    S s;
    twoInoutInt(s.x, s.x);
//CHECK:          ^ potentially aliased argument to 'out'/'inout'/'ref' parameter
//CHECK:          ^ passing the same value to both may produce unexpected results

    // Positive: identical array element to two inout params.
    int arr[4];
    twoInoutInt(arr[0], arr[0]);
//CHECK:           ^ potentially aliased argument to 'out'/'inout'/'ref' parameter
//CHECK:           ^ passing the same value to both may produce unexpected results

    // Positive: coerced inout argument (exercises TypeCastExpr peeling).
    twoInoutUint(a, a);
//CHECK:         ^ potentially aliased argument to 'out'/'inout'/'ref' parameter
//CHECK:         ^ passing the same value to both may produce unexpected results

    // Negative: distinct variables — no warning expected.
    twoOut(a, b);
//CHECK-NOT: potentially aliased

    // Negative: two in params — no warning expected.
    readBoth(a, a);
//CHECK-NOT: potentially aliased

    // Negative: distinct struct fields — no warning expected.
    twoInoutInt(s.x, s.y);
//CHECK-NOT: potentially aliased

    // Negative: distinct array elements — no warning expected.
    twoInoutInt(arr[0], arr[1]);
//CHECK-NOT: potentially aliased

    // Negative: compound assignment operator (a += a is well-defined).
    a += a;
//CHECK-NOT: potentially aliased
}
