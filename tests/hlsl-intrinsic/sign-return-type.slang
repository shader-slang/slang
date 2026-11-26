// Regression test: Slang/HLSL sign() returns int (or vector<int, N>) for all
// signed arithmetic input types:
//   sign<T : __BuiltinSignedArithmeticType>(T x) -> int
//   sign<T : __BuiltinSignedArithmeticType, let N : int>(vector<T, N> x) -> vector<int, N>
//
// Each target lowers this to its native sign() and converts to int when the
// native sign() returns float (GLSL, Metal, WGSL, SPIR-V FSign).

//TEST:SIMPLE(filecheck=SPIR):  -target spirv-asm -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=HLSL):  -target hlsl      -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=GLSL):  -target glsl      -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=METAL): -target metal     -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=WGSL):  -target wgsl      -stage compute -entry computeMain

RWStructuredBuffer<float> outFloat;
RWStructuredBuffer<int>   outInt;

//TEST_INPUT:ubuffer(data=[-3.0 3.0 0.0], stride=4):name=outFloat
//TEST_INPUT:ubuffer(data=[-3 3 0 0 0 0 0 0], stride=4):name=outInt

[numthreads(1, 1, 1)]
void computeMain()
{
    float f = outFloat[0]; // -3.0
    int   i = outInt[0];   // -3

    // -----------------------------------------------------------------------
    // 1. sign(float) -> int
    //    SPIR-V uses FSign then OpConvertFToS to %int
    //    GLSL   uses int(sign(...))
    //    Metal  uses int(sign(...))
    //    WGSL   uses i32(sign(...))
    //    HLSL   uses sign(...)
    // -----------------------------------------------------------------------
    // SPIR-DAG: FSign
    // SPIR-DAG: OpConvertFToS %int
    // HLSL: sign(
    // GLSL: int({{.*}}sign(
    // METAL: int({{.*}}sign(
    // WGSL: i32({{.*}}sign(
    int sf = sign(f);
    outInt[1] = sf;

    // -----------------------------------------------------------------------
    // 2. sign(int) -> int
    //    SPIR-V uses SSign %int
    // -----------------------------------------------------------------------
    // SPIR-DAG: SSign
    // HLSL: sign(
    // GLSL: sign(
    // METAL: sign(
    // WGSL: sign(
    int si = sign(i);
    outInt[2] = si;

    // -----------------------------------------------------------------------
    // 3. sign(float3) -> int3
    //    SPIR-V uses FSign %v3float then OpConvertFToS %v3int
    //    GLSL   uses ivec3(sign(...))
    //    Metal  uses vec<int,3>(sign(...))
    //    WGSL   uses vec3<i32>(sign(...))
    //    HLSL   uses sign(...)
    // -----------------------------------------------------------------------
    // SPIR-DAG: FSign
    // SPIR-DAG: OpConvertFToS %v3int
    // HLSL: sign(
    // GLSL: ivec3({{.*}}sign(
    // METAL: vec<int,3>({{.*}}sign(
    // WGSL: vec3<i32>({{.*}}sign(
    float3 vf  = float3(f, f, f);
    int3   svf = sign(vf);
    outInt[3] = svf.x;

    // -----------------------------------------------------------------------
    // 4. sign(int3) -> int3
    //    SPIR-V uses SSign %v3int
    // -----------------------------------------------------------------------
    // SPIR-DAG: SSign
    // HLSL: sign(
    // GLSL: ivec3({{.*}}sign(
    // METAL: vec<int,3>({{.*}}sign(
    // WGSL: vec3<i32>({{.*}}sign(
    int3 vi  = int3(i, i, i);
    int3 svi = sign(vi);
    outInt[4] = svi.x;
}
