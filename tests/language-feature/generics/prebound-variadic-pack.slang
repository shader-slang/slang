//TEST:SIMPLE(filecheck=CHECK): -target spirv

struct Set<each T>
{
    Tuple<expand each T> data;
    void f(expand each T v){}
    void h<each U>(U x){}
    void g(expand each T d)
    {
        //CHECK-NOT: error
        f(expand each d); // OK

        //CHECK-NOT: error
        h(54); // OK, specializing free-form parameter U.

        //CHECK: error
        f(); // error, cannot call f without arguments.

        //CHECK: error
        f(5); // error, cannot call f with different type pack.
    }
}

[numthreads(1,1,1)]
void computeMain()
{
    Set<float> v;
}