//TEST:SIMPLE(filecheck=WGSL): -target wgsl -entry computeMain -stage compute
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-cpu -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-wgpu -output-using-type

// WGSL passes structs and arrays by value, so each call copies the whole aggregate.
// Read-only (`in`) struct and array parameters larger than 64 bytes are passed as
// `ptr<function, T>` instead, as the other targets do with `borrow in`. The caller
// passes one function-scope copy of the value, and a callee forwards its own
// pointer parameter without a copy.

//TEST_INPUT:ubuffer(data=[1 2 3 4 5 6 7 8], stride=4):name=inputBuffer
RWStructuredBuffer<int> inputBuffer;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

// 132 bytes: passed by pointer.
struct Big
{
    int4 a[8];
    int s;
};

// 8 bytes: stays a value.
struct Small
{
    int2 v;
};

// WGSL: fn leaf{{.*}}( b{{.*}} : ptr<function, Big{{.*}}>,  i{{.*}} : i32) -> i32
int leaf(Big b, int i)
{
    return b.a[i].x + b.s;
}

// A callee forwards its pointer: no copy of `b` in `mid`.
// WGSL: fn mid{{.*}}( b{{.*}} : ptr<function, Big{{.*}}>,
// WGSL-NOT: : Big{{[_0-9]*}} =
// WGSL: leaf{{.*}}(&((*b
// WGSL: }
int mid(Big b, int i)
{
    return leaf(b, i) + leaf(b, i + 1);
}

// Writing to an `in` parameter changes only the callee's copy.
// WGSL: fn modifies{{.*}}( b{{.*}} : ptr<function, Big{{.*}}>) -> i32
int modifies(Big b)
{
    b.s = 100;
    return b.s + b.a[0].y;
}

// WGSL: fn sumArray{{.*}}( arr{{.*}} : ptr<function, array<vec4<i32>, i32(8)>>) -> i32
int sumArray(int4 arr[8])
{
    int t = 0;
    for (int i = 0; i < 8; i++)
        t += arr[i].x;
    return t;
}

// WGSL: fn smallFunc{{.*}}( s{{.*}} : Small{{.*}}) -> i32
int smallFunc(Small s)
{
    return s.v.x * s.v.y;
}

Big make(int k)
{
    Big b;
    for (int i = 0; i < 8; i++)
        b.a[i] = int4(k + i, 0, 0, 0);
    b.s = k;
    return b;
}

// A value (not a variable) passed to several calls is copied once, where it is
// defined, and every call receives that copy.
// WGSL-LABEL: fn twoCalls
// WGSL: var [[COPY:[_A-Za-z0-9]+]] : Big{{[_0-9]*}} =
// WGSL-NOT: : Big{{[_0-9]*}} =
// WGSL: leaf{{.*}}(&([[COPY:[_A-Za-z0-9]+]])
// WGSL-NOT: : Big{{[_0-9]*}} =
// WGSL: leaf{{.*}}(&([[COPY]])
int twoCalls(int k)
{
    Big m = make(k);
    return leaf(m, 0) + leaf(m, 1);
}

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    Big b;
    for (int i = 0; i < 8; i++)
        b.a[i] = int4(inputBuffer[i], inputBuffer[i] * 2, 0, 0);
    b.s = 1;

    // BUF: 9
    outputBuffer[0] = mid(b, 2);
    b.s = 10;
    // BUF-NEXT: 11
    outputBuffer[1] = leaf(b, 0);
    // BUF-NEXT: 102
    outputBuffer[2] = modifies(b);
    // BUF-NEXT: 10
    outputBuffer[3] = b.s;
    // BUF-NEXT: 36
    outputBuffer[4] = sumArray(b.a);

    Small small;
    small.v = int2(inputBuffer[1], inputBuffer[2]);
    // BUF-NEXT: 6
    outputBuffer[5] = smallFunc(small);

    // Each call sees the value `b` has at the call.
    // BUF-NEXT: 19
    outputBuffer[6] = leaf(b, 3) + leaf(b, 4) - b.s;

    // (3 + 3) + (4 + 3)
    // BUF-NEXT: 13
    outputBuffer[7] = twoCalls(inputBuffer[2]);
}
