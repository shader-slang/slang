// byte-address-buffer-grouped-values-11593.slang

// Feature test for https://github.com/shader-slang/slang/issues/11593. It checks the values that
// grouped byte-address accesses read and write, component by component, so a wrong group offset or
// component order changes the output. Every input word is distinct. The cases cover a group that
// follows a single component (the second `int3` of an array at offset 12, and an `int4` struct
// field at offset 4), plain `Load<T>` at constant locations, `half4` groups, and grouped stores with
// and without an alignment.

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

//TEST_INPUT:ubuffer(data=[10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 0x40003c00 0x44004200], stride=4):name=buffer
RWByteAddressBuffer buffer;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

struct FieldAtOffset4
{
    int a;
    int4 v;
};

static uint outputIndex = 0;

void output(int value)
{
    outputBuffer[outputIndex++] = value;
}

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    // Two `int2` groups.
    int4 a = buffer.LoadAligned<int4>(16, 8);
    output(a.x); output(a.y); output(a.z); output(a.w);

    // An `int2` group, then a single `int`.
    int3 b = buffer.LoadAligned<int3>(16, 8);
    output(b.x); output(b.y); output(b.z);

    // `arr[1]` starts at offset 12: a single `int`, then an `int2` group at 16.
    int3 arr[2] = buffer.LoadAligned<int3[2]>(0, 8);
    output(arr[0].x); output(arr[0].y); output(arr[0].z);
    output(arr[1].x); output(arr[1].y); output(arr[1].z);

    // `s.v` starts at offset 36: a single `int`, an `int2` group at 40, then a single `int`.
    FieldAtOffset4 s = buffer.LoadAligned<FieldAtOffset4>(32, 8);
    output(s.a); output(s.v.x); output(s.v.y); output(s.v.z); output(s.v.w);

    // Plain `Load<T>` at constant locations: two `int2` groups at 8, and a single `int`, an
    // `int2` group and a single `int` at 4.
    int4 c = buffer.Load<int4>(8);
    output(c.x); output(c.y); output(c.z); output(c.w);
    int4 d = buffer.Load<int4>(4);
    output(d.x); output(d.y); output(d.z); output(d.w);

    // Two `half2` groups holding 1.0, 2.0, 3.0 and 4.0.
    half4 h = buffer.LoadAligned<half4>(96, 4);
    output(int(h.x)); output(int(h.y)); output(int(h.z)); output(int(h.w));

    // Grouped stores, read back one word at a time; word 19 (offset 76) is left unchanged.
    buffer.Store<int3>(64, int3(101, 102, 103), 8);
    buffer.Store<int4>(80, int4(104, 105, 106, 107), 8);
    for (uint i = 0; i < 8; ++i)
        output(buffer.Load<int>(64 + 4 * i));

    // A plain `Store<T>` at a constant address: a single `int` at 4, an `int2` group at 8 and a
    // single `int` at 16. Words 0 and 5 are left unchanged.
    buffer.Store<int4>(4, int4(201, 202, 203, 204));
    for (uint i = 0; i < 6; ++i)
        output(buffer.Load<int>(4 * i));
}

// CHECK: 14
// CHECK-NEXT: 15
// CHECK-NEXT: 16
// CHECK-NEXT: 17
// CHECK-NEXT: 14
// CHECK-NEXT: 15
// CHECK-NEXT: 16
// CHECK-NEXT: 10
// CHECK-NEXT: 11
// CHECK-NEXT: 12
// CHECK-NEXT: 13
// CHECK-NEXT: 14
// CHECK-NEXT: 15
// CHECK-NEXT: 18
// CHECK-NEXT: 19
// CHECK-NEXT: 20
// CHECK-NEXT: 21
// CHECK-NEXT: 22
// CHECK-NEXT: 12
// CHECK-NEXT: 13
// CHECK-NEXT: 14
// CHECK-NEXT: 15
// CHECK-NEXT: 11
// CHECK-NEXT: 12
// CHECK-NEXT: 13
// CHECK-NEXT: 14
// CHECK-NEXT: 1
// CHECK-NEXT: 2
// CHECK-NEXT: 3
// CHECK-NEXT: 4
// CHECK-NEXT: 101
// CHECK-NEXT: 102
// CHECK-NEXT: 103
// CHECK-NEXT: 29
// CHECK-NEXT: 104
// CHECK-NEXT: 105
// CHECK-NEXT: 106
// CHECK-NEXT: 107
// CHECK-NEXT: 10
// CHECK-NEXT: 201
// CHECK-NEXT: 202
// CHECK-NEXT: 203
// CHECK-NEXT: 204
// CHECK-NEXT: 15
