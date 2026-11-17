// Writes and atomics on the texels of a texture-array element, made in a callee. The alias query
// treats a texel write as unable to change a location that holds texture handles; on SPIR-V
// each callee still loads its element and writes through it, with no extra copy of the array.
// (shader-slang/slang#13771)

//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry computeMain -stage compute -emit-spirv-directly

RWTexture2D<float4> colors[];
RWTexture2D<uint> counters[];
RWTexture2D<float4> sized[8];
RWStructuredBuffer<float4> outputBuffer;

// CHECK: %paint = OpFunction
// CHECK: [[COLOR_PTR:%[0-9]+]] = OpAccessChain {{.*}} %colors %i
// CHECK-NEXT: [[COLOR:%[0-9]+]] = OpLoad {{.*}} [[COLOR_PTR]]
// CHECK-NEXT: OpImageWrite [[COLOR]] %p
// CHECK-NEXT: OpReturn
[noinline]
void paint(uint i, uint2 p)
{
    colors[i][p] = float4(1.0);
}

// CHECK: %bump = OpFunction
// CHECK: [[COUNTER_PTR:%[0-9]+]] = OpAccessChain {{.*}} %counters %i_0
// CHECK-NEXT: [[TEXEL:%[0-9]+]] = OpImageTexelPointer {{.*}} [[COUNTER_PTR]] %p_0
// CHECK-NEXT: OpAtomicIAdd {{.*}} [[TEXEL]]
// CHECK-NEXT: OpReturn
[noinline]
void bump(uint i, uint2 p)
{
    uint previous;
    InterlockedAdd(counters[i][p], 1, previous);
}

// CHECK: %addOne = OpFunction
// CHECK: [[SIZED_PTR:%[0-9]+]] = OpAccessChain {{.*}} %sized %i_1
// CHECK: [[SIZED:%[0-9]+]] = OpLoad {{.*}} [[SIZED_PTR]]
// CHECK: OpImageRead {{.*}} [[SIZED]]
// CHECK: OpImageWrite [[SIZED]] %p_1
// CHECK-NEXT: OpReturn
[noinline]
void addOne(uint i, uint2 p)
{
    sized[i][p] = sized[i][p] + 1.0;
}

[shader("compute")]
[numthreads(8, 8, 1)]
void computeMain(uint3 id: SV_DispatchThreadID)
{
    paint(id.z, id.xy);
    bump(id.z, id.xy);
    addOne(id.z, id.xy);
    outputBuffer[0] = colors[id.z][id.xy] + float(counters[id.z][id.xy]) + sized[id.z][id.xy];
}
