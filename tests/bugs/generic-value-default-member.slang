//TEST:SIMPLE: -target hlsl

// Regression test for lowering a default generic value argument that depends
// on a static member of a constrained type parameter.

interface I
{
    static const int f;
}

struct S<T, let f : int>
{
}

typealias X<T : I, let f : int = T.f> = S<T, f>;

struct A : I
{
    static const int f = 1;
}

X<A> x;
