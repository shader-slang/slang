// `Access.WriteOnly` is only valid as the access of `RefParam`. We reject it wherever it binds to
// a generic value parameter of any other generic: an explicit argument, a cast of the raw value,
// a default, a generic function argument and a type alias of `Ptr`.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -entry main -stage compute

struct H<let A : Access>
{
    Ptr<int, A> p;
}

struct Defaulted<let A : Access = Access.WriteOnly>
//CHECK:                                 ^^^^^^^^^ E30035
//CHECK:                                 ^^^^^^^^^ 'Defaulted' cannot use 'Access.WriteOnly'; it is only valid as the access of 'RefParam'.
{
    Ptr<int, A> p;
}

int k<let A : Access>()
{
    return 0;
}

typealias WP = Ptr<int, Access.WriteOnly>;
//CHECK:                       ^^^^^^^^^ E30035
//CHECK:                       ^^^^^^^^^ 'Ptr' cannot use 'Access.WriteOnly'; it is only valid as the access of 'RefParam'.

void uses()
{
    H<Access.WriteOnly> a;
//CHECK:     ^^^^^^^^^ E30035
//CHECK:     ^^^^^^^^^ 'H' cannot use 'Access.WriteOnly'; it is only valid as the access of 'RefParam'.
    H<(Access)3> b;
//CHECK:      ^ E30035
//CHECK:      ^ 'H' cannot use 'Access.WriteOnly'; it is only valid as the access of 'RefParam'.
    H<Access.Read> c;
    int v = k<Access.WriteOnly>();
//CHECK:             ^^^^^^^^^ E30035
//CHECK:             ^^^^^^^^^ 'k' cannot use 'Access.WriteOnly'; it is only valid as the access of 'RefParam'.
}

[numthreads(1, 1, 1)]
void main()
{
}
