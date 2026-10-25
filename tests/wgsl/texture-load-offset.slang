// Texture Load with a texel offset. WGSL's textureLoad takes no offset, so the offset is added to
// the texel coordinate (not to the array layer, the mip level or the sample index).
//TEST:SIMPLE(filecheck=WGSL): -stage compute -entry computeMain -target wgsl
// Validate the emitted WGSL through Tint (ignored where the downstream compiler is absent).
//TEST:SIMPLE(filecheck=SPV): -stage compute -entry computeMain -target wgsl-spirv-asm

RWStructuredBuffer<float4> outputBuffer;
Texture1D<float4> t1D;
Texture2D<float4> t2D;
Texture2DArray<float4> t2DArray;
Texture3D<float4> t3D;
Texture2DMS<float4> t2DMS;

// SPV: OpEntryPoint
// WGSL-NOT: loop
// The offset's components are added to the coordinate's (zero components fold away); the mip
// level, the array layer and the sample index are passed unchanged.
// WGSL: var [[C1:coord_[0-9]+]] : vec2<i32> = vec2<i32>(i_{{[0-9]+}}, i32(0));
// WGSL-NEXT: [[C1]][i32(0)] = [[C1]][i32(0)] + i32(1);
// WGSL: textureLoad((t1D_{{[0-9]+}}), (({{.*}})).x, (({{.*}})).y)
// WGSL: var [[C2:coord_[0-9]+]] : vec3<i32> = vec3<i32>(i_{{[0-9]+}}, i32(1), i32(0));
// WGSL-NEXT: [[C2]][i32(0)] = [[C2]][i32(0)] + i32(1);
// WGSL-NEXT: [[C2]][i32(1)] = [[C2]][i32(1)] + i32(-1);
// WGSL: textureLoad((t2D_{{[0-9]+}}), (({{.*}})).xy, (({{.*}})).z)
// WGSL: var [[C3:coord_[0-9]+]] : vec4<i32> = vec4<i32>(i_{{[0-9]+}}, i32(1), i32(1), i32(0));
// WGSL-NEXT: [[C3]][i32(0)] = [[C3]][i32(0)] + i32(1);
// WGSL-NEXT: var
// WGSL: textureLoad((t2DArray_{{[0-9]+}}), (({{.*}})).xy, i32((({{.*}})).z), (({{.*}})).w)
// WGSL: var [[C4:coord_[0-9]+]] : vec4<i32> = vec4<i32>(i_{{[0-9]+}}, i32(1), i32(2), i32(0));
// WGSL-NEXT: [[C4]][i32(0)] = [[C4]][i32(0)] + i32(1);
// WGSL-NEXT: [[C4]][i32(2)] = [[C4]][i32(2)] + i32(-1);
// WGSL: textureLoad((t3D_{{[0-9]+}}), (({{.*}})).xyz, (({{.*}})).w)
// WGSL: var [[C5:coord_[0-9]+]] : vec2<i32> = vec2<i32>(i_{{[0-9]+}}, i32(2));
// WGSL-NEXT: [[C5]][i32(0)] = [[C5]][i32(0)] + i32(-1);
// WGSL-NEXT: [[C5]][i32(1)] = [[C5]][i32(1)] + i32(1);
// WGSL: textureLoad((t2DMS_{{[0-9]+}}), ([[C5]]), (i32(3)))
[numthreads(4, 1, 1)]
void computeMain(int3 tid: SV_DispatchThreadID)
{
    int i = tid.x;
    float4 r = t1D.Load(int2(i, 0), 1);
    r += t2D.Load(int3(i, 1, 0), int2(1, -1));
    r += t2DArray.Load(int4(i, 1, 1, 0), int2(1, 0));
    r += t3D.Load(int4(i, 1, 2, 0), int3(1, 0, -1));
    r += t2DMS.Load(int2(i, 2), 3, int2(-1, 1));
    outputBuffer[i] = r;
}
