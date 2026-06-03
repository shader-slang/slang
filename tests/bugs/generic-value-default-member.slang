//TEST:INTERPRET(filecheck=CHECK):

// Regression test for lowering a default generic value argument that depends
// on a static member of a constrained type parameter.

interface I
{
    static const int f;
}

struct S<T, let f : int>
{
    static int get()
    {
        return f;
    }
}

typealias X<T : I, let f : int = T.f> = S<T, f>;

struct A : I
{
    static const int f = 1;
}

X<A> x;

void main()
{
    printf("%d\n", X<A>::get());
    // CHECK: 1
}
