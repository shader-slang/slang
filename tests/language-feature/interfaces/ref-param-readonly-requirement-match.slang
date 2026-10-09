// A `__ref_readonly` parameter has the `RefReadOnly` passing mode, the same as the legacy
// `const __ref` and `__ref const` spellings, so any spelling satisfies a requirement that uses
// another.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type
//TEST:SIMPLE(filecheck=IR):-target spirv-asm -entry computeMain -stage compute -dump-ir -o -

// IR-NOT: func %{{[^ ]*}}read{{.*}}RefParam(Array(Int, 4 : Int), 0 : UInt64
// IR: func %{{[^ ]*}}read{{.*}}RefParam(Array(Int, 4 : Int), 1 : UInt64
// IR-NOT: func %{{[^ ]*}}read{{.*}}RefParam(Array(Int, 4 : Int), 0 : UInt64

interface IReadOnlyReader
{
    int read(__ref_readonly int a[4]);
}

interface IConstReader
{
    int read(const __ref int a[4]);
}

struct ReadOnlyForReadOnly : IReadOnlyReader
{
    int read(__ref_readonly int a[4]) { return a[0]; }
}

struct ConstForReadOnly : IReadOnlyReader
{
    int read(const __ref int a[4]) { return a[1]; }
}

struct ReadOnlyForConst : IConstReader
{
    int read(__ref_readonly int a[4]) { return a[2]; }
}

struct RefConstForReadOnly : IReadOnlyReader
{
    int read(__ref const int a[4]) { return a[0] + a[1]; }
}

// The extra defaulted parameter makes `Scaled` satisfy `IReadOnlyReader.read` through a
// synthesized wrapper, whose parameter must keep the requirement's `RefReadOnly` mode.
struct Scaled : IReadOnlyReader
{
    int read(__ref_readonly int a[4], int scale = 2) { return scale * a[3]; }
}

int callReader<T : IReadOnlyReader>(T t, __ref int a[4]) { return t.read(a); }
int callConstReader<T : IConstReader>(T t, __ref int a[4]) { return t.read(a); }

//TEST_INPUT:ubuffer(data=[0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    int a[4] = { 5, 6, 7, 8 };
    // CHECK: 5
    outputBuffer[0] = callReader(ReadOnlyForReadOnly(), a);
    // CHECK: 6
    outputBuffer[1] = callReader(ConstForReadOnly(), a);
    // CHECK: 7
    outputBuffer[2] = callConstReader(ReadOnlyForConst(), a);
    // CHECK: 16
    outputBuffer[3] = callReader(Scaled(), a);
    // CHECK: 11
    outputBuffer[4] = callReader(RefConstForReadOnly(), a);
}
