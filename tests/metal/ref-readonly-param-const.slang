// A `__ref_readonly` parameter that keeps its call boundary is emitted as a pointer to `const`,
// so the Metal compiler enforces the read-only access. `__ref` and `__ref_writeonly` parameters
// stay pointers to mutable storage.
//
// `[noRefInline]` is a compiler-internal attribute that user code should not use: a reference
// parameter that keeps its call boundary is not faithful on every target. The test needs it
// because a callee with a reference parameter is otherwise inlined, leaving no parameter to check.

//TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -entry main -experimental-feature

RWStructuredBuffer<int> output;

// CHECK-DAG: int readIt{{.*}}(const array<int, int(4)> thread* {{.*}}, uint
[noRefInline]
int readIt(__ref_readonly int a[4], uint i)
{
    return a[i];
}

// CHECK-DAG: void writeIt{{.*}}(array<int, int(4)> thread* {{.*}}, uint
[noRefInline]
void writeIt(__ref_writeonly int a[4], uint i)
{
    a[i] = 1;
}

// CHECK-DAG: void bumpIt{{.*}}(array<int, int(4)> thread* {{.*}}, uint
[noRefInline]
void bumpIt(__ref int a[4], uint i)
{
    a[i] += 1;
}

[numthreads(4, 1, 1)]
void main(uint i: SV_GroupIndex)
{
    int x[4];
    writeIt(x, i);
    bumpIt(x, i);
    output[i] = readIt(x, i);
}
