// A `__constref` parameter that keeps its call boundary is emitted as a pointer to `const`, so
// the Metal compiler enforces the read-only access. `inout` parameters stay pointers to mutable
// storage. The aliasing reference modes (`__ref` and its readonly/writeonly forms) are always
// inlined into their callers on ordinary functions, so the modes that keep an emitted parameter
// shape are the non-aliasing ones: `__constref` and `inout`.

//TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -entry main -experimental-feature

RWStructuredBuffer<int> output;

// CHECK-DAG: int readIt{{.*}}(const array<int, int(4)> thread* {{.*}}, uint
[noinline]
int readIt(__constref int a[4], uint i)
{
    return a[i];
}

// CHECK-DAG: void writeIt{{.*}}(array<int, int(4)> thread* {{.*}}, uint
[noinline]
void writeIt(inout int a[4], uint i)
{
    a[i] = 1;
}

// CHECK-DAG: void bumpIt{{.*}}(array<int, int(4)> thread* {{.*}}, uint
[noinline]
void bumpIt(inout int a[4], uint i)
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
