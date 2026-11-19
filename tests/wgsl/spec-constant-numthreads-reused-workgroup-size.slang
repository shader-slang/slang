//TEST:SIMPLE(filecheck=CHECK): -target wgsl -stage compute -entry computeMain
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-wgpu -output-using-type

// A `WorkgroupSize()` value that is used more than once is bound to a local. When an axis is a
// specialization constant, that value reads an `override`, so it is an override-expression and
// must be bound with `var` (or `let`); a WGSL `const` initializer requires a const-expression.
// The `-wgpu` row runs the shader, which pipeline creation rejects if the local is a `const`.
// See shader-slang/slang#13767.

// CHECK-NOT: const {{.*}}kX_0
// CHECK: var {{[A-Za-z0-9_]+}} : vec3<i32> = vec3<i32>(i32(kX_0), i32(1), i32(1));
// CHECK-NOT: const {{.*}}kX_0

[vk::constant_id(0)]
const uint kX = 4;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[noinline]
void put(int slot, int3 v)
{
    outputBuffer[slot] = v.x;
    outputBuffer[slot + 1] = v.y;
    outputBuffer[slot + 2] = v.z;
}

[numthreads(kX, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    int3 w = WorkgroupSize();
    if (tid.x != 0)
        return;
    // BUF: 4
    // BUF-NEXT: 1
    // BUF-NEXT: 1
    // BUF-NEXT: 8
    // BUF-NEXT: 2
    // BUF-NEXT: 2
    put(0, w);
    put(3, w * 2);
}
