// Regression for shader-slang/slang#13446: integer matrices stay matrices until buffer element
// lowering has stored them with their layout, and only then become arrays of row vectors. The
// passes that now see them as matrices must still handle the shapes they used to see as arrays.

// A matrix with one row or column has no matrix type on these targets, but as an array of
// vectors it is legal; storing it with its layout must not produce 1-component vectors.
//TEST:SIMPLE(filecheck=SPV):-target spirv -entry computeMain -stage compute -matrix-layout-column-major
//TEST:SIMPLE(filecheck=SPV):-target spirv -entry computeMain -stage compute -matrix-layout-row-major
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry computeMain -stage compute -matrix-layout-column-major
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry computeMain -stage compute -matrix-layout-column-major
//TEST:SIMPLE(filecheck=METAL):-target metal -entry computeMain -stage compute -matrix-layout-column-major
//TEST:SIMPLE(filecheck=WGSLSPIRV):-target wgsl-spirv-asm -entry computeMain -stage compute -matrix-layout-column-major

//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -compute -output-using-type -shaderobj -xslang -matrix-layout-column-major
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -compute -output-using-type -shaderobj -emit-spirv-via-glsl -xslang -matrix-layout-column-major

struct Narrow
{
    column_major int3x1 a;
    column_major int1x3 b;
    row_major int3x1 c;
    int k;
}

// Reflection gives `a` (one `int3` column) 16 bytes, then `b` (three `int` columns) at 16, `c`
// (three `int` rows) at 28 and `k` at 40; word `i` holds `i + 1`.
//TEST_INPUT:ubuffer(data=[1 2 3 4 5 6 7 8 9 10 11 12], stride=48):name input
RWStructuredBuffer<Narrow> input;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0], stride=4):out,name output
RWStructuredBuffer<int> output;

// A function that is not inlined and returns an integer matrix returns an array once the matrix is
// lowered, which GLSL and WGSL must pass back through an `out` parameter.
[noinline]
int2x3 shifted(int2x3 m, uint amount)
{
    // Metal and WGSL give a matrix shift a shift amount of the matrix's shape.
    return (m << amount) >> 1;
}

// Integer matrix division and shifts by a matrix are row-wise operations on these targets; only
// a float matrix is divided as a matrix, which Metal and WGSL reject.
[noinline]
int2x3 divideAndShift(int2x3 m, int2x3 n, int s)
{
    return (m / n) + (s / n) + (m / s) + (m << n) + (m >> n);
}

[numthreads(1, 1, 1)]
void computeMain()
{
    output[0] = input[0].a[2][0];
    output[1] = input[0].b[0][1];
    output[2] = input[0].c[1][0];
    output[3] = input[0].k;

    int2x3 m = int2x3(input[0].k, 1, 2, 3, 4, 5);
    output[4] = shifted(m, 2)[0][0];

    int2x3 n = int2x3(2, 1, 1, 1, 1, 2);
    output[5] = divideAndShift(m, n, 4)[1][2];
}

// SPV-NOT: OpTypeVector %int 1
// SPV: OpEntryPoint

// GLSL: void shifted_0(
// GLSL-SAME: out ivec3 {{.*}}[2])

// WGSL: fn shifted_0(
// WGSL-SAME: ptr<function, array<vec3<i32>, i32(2)>>
// WGSL: vec3<u32> = vec3<u32>(amount

// METAL: uint3 {{.*}} = uint3(amount
// METAL-NOT: division by matrix

// WGSLSPIRV: OpEntryPoint

// BUF: type: int32_t
// BUF-NEXT: 3
// BUF-NEXT: 6
// BUF-NEXT: 9
// BUF-NEXT: 11
// BUF-NEXT: 22
// BUF-NEXT: 26
