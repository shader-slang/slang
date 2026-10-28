// Texture Load with a texel offset. WGSL's textureLoad and Metal's read() take no offset, so the
// offset is added to the texel coordinate (not to the array layer, the mip level or the sample index).
//TEST:SIMPLE(filecheck=WGSL): -stage compute -entry computeMain -target wgsl
// Validate the emitted WGSL through Tint (ignored where the downstream compiler is absent).
//TEST:SIMPLE(filecheck=SPV): -stage compute -entry computeMain -target wgsl-spirv-asm
// The lowering is straight-line code: no loop anywhere in the module.
//TEST:SIMPLE(filecheck=NOLOOP): -stage compute -entry computeMain -target wgsl
// Metal's read() takes no offset either, so it gets the same coordinate arithmetic.
//TEST:SIMPLE(filecheck=METAL): -stage compute -entry computeMain -target metal

RWStructuredBuffer<float4> outputBuffer;
Texture1D<float4> t1D;
Texture2D<float4> t2D;
Texture2DArray<float4> t2DArray;
Texture3D<float4> t3D;
Texture2DMS<float4> t2DMS;

// SPV: OpEntryPoint
// NOLOOP-NOT: loop
// The offset's components are added to the coordinate's (zero components fold away); the mip
// level, the array layer and the sample index are passed unchanged. Each load reads the adjusted
// coordinate.
// WGSL: var [[C1:coord_[0-9]+]] : vec2<i32> = vec2<i32>(i_{{[0-9]+}}, i32(0));
// WGSL-NEXT: [[C1]][i32(0)] = [[C1]][i32(0)] + i32(1);
// WGSL-NEXT: var [[L1:_S[0-9]+]] : vec2<i32> = [[C1]];
// WGSL-NEXT: textureLoad((t1D_{{[0-9]+}}), (([[L1]])).x, (([[L1]])).y)
// The mip level (2) is the coordinate's last component, untouched by the offset.
// WGSL: var [[C2:coord_[0-9]+]] : vec3<i32> = vec3<i32>(i_{{[0-9]+}}, i32(1), i32(2));
// WGSL-NEXT: [[C2]][i32(0)] = [[C2]][i32(0)] + i32(1);
// WGSL-NEXT: [[C2]][i32(1)] = [[C2]][i32(1)] + i32(-1);
// WGSL-NEXT: var [[L2:_S[0-9]+]] : vec3<i32> = [[C2]];
// WGSL-NEXT: textureLoad((t2D_{{[0-9]+}}), (([[L2]])).xy, (([[L2]])).z)
// WGSL: var [[C3:coord_[0-9]+]] : vec4<i32> = vec4<i32>(i_{{[0-9]+}}, i32(1), i32(1), i32(0));
// WGSL-NEXT: [[C3]][i32(0)] = [[C3]][i32(0)] + i32(1);
// WGSL-NEXT: var [[L3:_S[0-9]+]] : vec4<i32> = [[C3]];
// WGSL-NEXT: textureLoad((t2DArray_{{[0-9]+}}), (([[L3]])).xy, i32((([[L3]])).z), (([[L3]])).w)
// WGSL: var [[C4:coord_[0-9]+]] : vec4<i32> = vec4<i32>(i_{{[0-9]+}}, i32(1), i32(2), i32(1));
// WGSL-NEXT: [[C4]][i32(0)] = [[C4]][i32(0)] + i32(1);
// WGSL-NEXT: [[C4]][i32(2)] = [[C4]][i32(2)] + i32(-1);
// WGSL-NEXT: var [[L4:_S[0-9]+]] : vec4<i32> = [[C4]];
// WGSL-NEXT: textureLoad((t3D_{{[0-9]+}}), (([[L4]])).xyz, (([[L4]])).w)
// WGSL: var [[C5:coord_[0-9]+]] : vec2<i32> = vec2<i32>(i_{{[0-9]+}}, i32(2));
// WGSL-NEXT: [[C5]][i32(0)] = [[C5]][i32(0)] + i32(-1);
// WGSL-NEXT: [[C5]][i32(1)] = [[C5]][i32(1)] + i32(1);
// WGSL-NEXT: textureLoad((t2DMS_{{[0-9]+}}), ([[C5]]), (i32(3)))
// METAL: [[M1:coord_[0-9]+]] = int2(i_{{[0-9]+}}, int(0));
// METAL: [[M1]].x = [[M1]].x + int(1);
// METAL: (t1D_{{[0-9]+}}).read(
// METAL: [[M2:coord_[0-9]+]] = int3(i_{{[0-9]+}}, int(1), int(2));
// METAL: [[M2]].x = [[M2]].x + int(1);
// METAL: [[M2]].y = [[M2]].y + int(-1);
// METAL: (t2D_{{[0-9]+}}).read(
// METAL: [[M3:coord_[0-9]+]] = int4(i_{{[0-9]+}}, int(1), int(1), int(0));
// METAL: [[M3]].x = [[M3]].x + int(1);
// METAL: (t2DArray_{{[0-9]+}}).read(
// METAL: [[M4:coord_[0-9]+]] = int4(i_{{[0-9]+}}, int(1), int(2), int(1));
// METAL: [[M4]].x = [[M4]].x + int(1);
// METAL: [[M4]].z = [[M4]].z + int(-1);
// METAL: (t3D_{{[0-9]+}}).read(
// METAL: [[M5:coord_[0-9]+]] = int2(i_{{[0-9]+}}, int(2));
// METAL: [[M5]].x = [[M5]].x + int(-1);
// METAL: [[M5]].y = [[M5]].y + int(1);
// METAL: (t2DMS_{{[0-9]+}}).read(([[M5]]), uint((int(3))))
[numthreads(4, 1, 1)]
void computeMain(int3 tid: SV_DispatchThreadID)
{
    int i = tid.x;
    float4 r = t1D.Load(int2(i, 0), 1);
    r += t2D.Load(int3(i, 1, 2), int2(1, -1));
    r += t2DArray.Load(int4(i, 1, 1, 0), int2(1, 0));
    r += t3D.Load(int4(i, 1, 2, 1), int3(1, 0, -1));
    r += t2DMS.Load(int2(i, 2), 3, int2(-1, 1));
    outputBuffer[i] = r;
}
