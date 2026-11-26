// A scalar ordering comparison on `bool` must reach the GLSL emitter as the
// logical operators GLSL defines on Booleans (`!`, `&&`, `||`), not as a
// relational operator, which GLSL does not allow on Boolean operands. The
// operands are loaded from a buffer so constant folding cannot replace the
// comparison before emission.

//TEST:CROSS_COMPILE(filecheck=CHECK): -entry main -stage compute -target glsl

StructuredBuffer<int> inBuf;
RWStructuredBuffer<int> outBuf;

[numthreads(1, 1, 1)]
void main()
{
    bool a = inBuf[0] != 0;
    bool b = inBuf[1] != 0;
    outBuf[0] = int(a < b);
    outBuf[1] = int(a <= b);
    outBuf[2] = int(a > b);
    outBuf[3] = int(a >= b);
}

// The operands are Booleans, and every comparison lowers to `!` plus `&&`/`||`.
// CHECK: bool a_0 = (inBuf_0._data[uint(0)]) != 0;
// CHECK: bool b_0 = (inBuf_0._data[uint(1)]) != 0;
// CHECK-NOT: <
// CHECK: !a_0
// CHECK-NOT: <
// CHECK: &&b_0
// CHECK-NOT: <
// CHECK: ||b_0
// CHECK-NOT: <
// CHECK: !b_0
// CHECK-NOT: <
// CHECK: &&a_0
// CHECK-NOT: <
// CHECK: ||a_0
// CHECK-NOT: <
