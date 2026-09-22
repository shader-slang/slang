// A `static_assert` at declaration scope with any enclosing generic is diagnosed (E41405) rather
// than lowered, because its condition can depend on the generic parameters. `non-exhaustive` because
// each failure emits both an error and a span note; the ordered checks below verify the direct
// generic-aggregate case and the nested-non-generic-in-generic case, plus the remediation text.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target spirv -entry computeMain -stage compute

struct Holder<T>
{
    T value;
    static_assert(sizeof(T) > 0, "T must be non-empty");
//CHECK: static assertion enclosed by a generic is not supported
//CHECK: no enclosing generic
}

// The guard walks the whole enclosing-declaration chain, so a `static_assert` in a non-generic
// aggregate nested inside a generic one is rejected too (it still depends on the outer `T`).
struct Outer<T>
{
    struct Inner
    {
        static_assert(sizeof(T) > 0, "nested T must be non-empty");
//CHECK: static assertion enclosed by a generic is not supported
    }
}

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = 0;
}
