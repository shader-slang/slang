// Regression test for https://github.com/shader-slang/slang/issues/13553
//
// A shift's result has the element type of its left operand, and its shift amount may have a
// different integer type (`int3 >> uint3`, `int s >> uint3`). The C++ and CUDA preludes define
// the mixed-type vector shift operators, and the IR splats a scalar operand to a vector of its own
// type, so that the shift keeps the signedness of its left operand.
// Inputs come from a buffer so that no shift is folded at compile time, and no case shifts a
// negative value left (undefined before C++20).

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cpu -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute -shaderobj -output-using-type
//TEST:SIMPLE(filecheck=CUDA): -target cuda -line-directive-mode none -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=PTX): -target ptx -entry computeMain -stage compute

// The `uint3` shift amount keeps its type, and the `int` scalar is splat to `int3` rather than to
// the amount's type `uint3`.
// CUDA: store3_0(int(0), v_1 >> a_0);
// CUDA: store3_0(int(3), make_int3 ({{[^)]*}}) >> a_0);
// An unsigned scalar is splat to `uint3`, not to the `int3` of its amount.
// CUDA: uint3 u_0 = make_uint3 ({{.*}}) << {{[A-Za-z0-9_]+}};

// NVRTC accepts the emitted CUDA source.
// PTX: .entry computeMain

//TEST_INPUT:ubuffer(data=[-8 1 2 3 5], stride=4):name=inputs
RWStructuredBuffer<int> inputs;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

void store3(int base, int3 v)
{
    outputBuffer[base] = v.x;
    outputBuffer[base + 1] = v.y;
    outputBuffer[base + 2] = v.z;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    int s = inputs[0];                                       // -8
    uint3 a = uint3(uint(inputs[1]), uint(inputs[2]), uint(inputs[3])); // (1, 2, 3)
    int3 v = int3(s, s * 2, s * 4);                          // (-8, -16, -32)
    int five = inputs[4];

    // Signed vector shifted by an unsigned vector: the shift stays arithmetic.
    store3(0, v >> a);
    // CHECK: -4
    // CHECK-NEXT: -4
    // CHECK-NEXT: -4

    // Signed scalar shifted by an unsigned vector: the scalar is splat to `int3`, not to `uint3`,
    // so the shift stays arithmetic.
    store3(3, s >> a);
    // CHECK-NEXT: -4
    // CHECK-NEXT: -2
    // CHECK-NEXT: -1

    // Unsigned vector shifted by a signed vector.
    store3(6, int3(a << int3(inputs[3], inputs[2], inputs[1])));
    // CHECK-NEXT: 8
    // CHECK-NEXT: 8
    // CHECK-NEXT: 6

    // 64-bit vector shifted by a 32-bit vector: the shift is performed in 64 bits.
    int64_t3 wide = int64_t3(int64_t(inputs[1])) << (a + 32);
    store3(9, int3(wide >> 32));
    // CHECK-NEXT: 2
    // CHECK-NEXT: 4
    // CHECK-NEXT: 8

    // A 64-bit amount keeps its type; the result has the 32-bit element type of the left operand.
    store3(12, int3(five, 1, 2) << int64_t3(a));
    // CHECK-NEXT: 10
    // CHECK-NEXT: 4
    // CHECK-NEXT: 16

    // `>>=` is inlined to a plain `>>`, so it takes the same conversion.
    int3 c = v;
    c >>= uint3(a.z, a.y, a.x);
    store3(15, c);
    // CHECK-NEXT: -1
    // CHECK-NEXT: -4
    // CHECK-NEXT: -16

    // The scalar is splat to its own type `uint3`, not to the amount's type `int3`. Both give
    // the same bits here, so the CUDA FileCheck above guards the splat type.
    uint3 u = uint(five) << int3(a);
    store3(18, int3(u));
    // CHECK-NEXT: 10
    // CHECK-NEXT: 20
    // CHECK-NEXT: 40

    // Unsigned vector shifted by a signed scalar: the shift stays logical.
    uint3 high = uint3(uint(s)) >> inputs[1];
    store3(21, int3(high));
    // CHECK-NEXT: 2147483644
    // CHECK-NEXT: 2147483644
    // CHECK-NEXT: 2147483644

    // 8-bit vector shifted by a 32-bit vector.
    int8_t3 narrow = int8_t3(int3(s * 8, s * 8, 100)) >> a;
    store3(24, int3(narrow));
    // CHECK-NEXT: -32
    // CHECK-NEXT: -16
    // CHECK-NEXT: 12

    // Unsigned 16-bit vector shifted by a signed 64-bit vector; the shift stays logical.
    uint16_t3 narrowU = uint16_t3(uint3(uint(s))) >> int64_t3(a);
    store3(27, int3(narrowU));
    // CHECK-NEXT: 32764
    // CHECK-NEXT: 16382
    // CHECK-NEXT: 8191
}
