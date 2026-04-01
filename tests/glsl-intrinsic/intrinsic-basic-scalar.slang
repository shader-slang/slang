//TEST:SIMPLE(filecheck=CHECK_GLSL): -allow-glsl -stage compute -entry computeMain -target glsl
//TEST:SIMPLE(filecheck=CHECK_GLSL_SPIRV): -allow-glsl -stage compute -entry computeMain -target spirv -emit-spirv-via-glsl
//TEST:SIMPLE(filecheck=CHECK_SPIR): -allow-glsl -stage compute -entry computeMain -target spirv -emit-spirv-directly
//TEST:SIMPLE(filecheck=CHECK_HLSL): -allow-glsl -stage compute -entry computeMain -target hlsl

//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -compute -entry computeMain -allow-glsl -output-using-type -emit-spirv-via-glsl
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -compute -entry computeMain -allow-glsl -output-using-type -emit-spirv-directly
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=BUF):-llvm -compute -entry computeMain -allow-glsl -output-using-type -Xslang -DTARGET_LLVM

//TEST_INPUT:ubuffer(data=[0 1 -1], stride=4):name=inputBuffer
buffer MyBlockName1
{
    int data[10];
} inputBuffer;

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
buffer MyBlockName2
{
    int result;
} outputBuffer;


__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
bool equals(matrix<T, N, M> lhs, matrix<T, N, M> rhs)
{
    for (int i = 0; i < N; ++i)
    {
        for (int j = 0; j < M; ++j)
        {
            if (lhs[i][j] != rhs[i][j])
            {
                return false;
            }
        }
    }
    return true;
}

bool Test_ScalarType()
{
    // CHECK_GLSL-LABEL: Test_ScalarType

    typealias genFType = float;
    typealias genDType = double;
    typealias genIType = int;
    typealias genUType = uint;
    typealias genBType = bool;

    // Temporary variables
    genFType outGenFType;
    genDType outGenDType;
    genIType outGenIType, outGenIType2;
    genUType outGenUType, outGenUType2;

    constexpr const float epsilon = 0.000001;
    const mat2 identity2x2 = mat2(vec2(1,0),vec2(0,1));
    const mat3 identity3x3 = mat3(vec3(1,0,0),vec3(0,1,0),vec3(0,0,1));
    const mat4 identity4x4 = mat4(vec4(1,0,0,0),vec4(0,1,0,0),vec4(0,0,1,0),vec4(0,0,0,1));

    // CHECK_SPIR-LABEL: Op{{.*}}AccessChain {{.*}} %inputBuffer
    const int zero = inputBuffer.data[0];
    const int one = inputBuffer.data[1];
    const int negaOne = inputBuffer.data[2];

    bool voidResults = true;

    // 8.8. Integer Functions

    // CHECK_GLSL: umulExtended(
    // CHECK_SPIR: OpUMulExtended{{ }}
    umulExtended(genUType(zero), genUType(zero), outGenUType, outGenUType2);
    voidResults = voidResults && genUType(0) == outGenUType;
    voidResults = voidResults && genUType(0) == outGenUType2;

    // CHECK_GLSL: imulExtended(
    // CHECK_SPIR: OpSMulExtended{{ }}
    imulExtended(genIType(zero), genIType(zero), outGenIType, outGenIType2);
    voidResults = voidResults && genIType(0) == outGenIType;
    voidResults = voidResults && genIType(0) == outGenIType2;

    return voidResults
        // 8.1. Angle and Trigonometry Functions

        // CHECK_GLSL: radians(
        // CHECK_SPIR: Radians{{ }}
        && genFType(0) == radians(genFType(zero))

        // CHECK_GLSL: degrees(
        // CHECK_SPIR: Degrees{{ }}
        && genFType(0) == degrees(genFType(zero))

        // CHECK_GLSL: sin(
        // CHECK_SPIR: Sin{{ }}
        && genFType(0) == sin(genFType(zero))

        // CHECK_GLSL: cos(
        // CHECK_SPIR: Cos{{ }}
        && genFType(1) == cos(genFType(zero))

        // CHECK_GLSL: tan(
        // CHECK_SPIR: Tan{{ }}
        && genFType(0) == tan(genFType(zero))

        // CHECK_GLSL: asin(
        // CHECK_SPIR: Asin{{ }}
        && genFType(0) == asin(genFType(zero))

        // CHECK_GLSL: acos(
        // CHECK_SPIR: Acos{{ }}
        && genFType(0) == acos(genFType(one))

        // CHECK_GLSL-COUNT-2: atan(
        // CHECK_SPIR: Atan2{{ }}
        // CHECK_SPIR: Atan{{ }}
        && genFType(0) == atan(genFType(zero), genFType(zero))
        && genFType(0) == atan(genFType(zero))

        // CHECK_GLSL: sinh(
        // CHECK_SPIR: Sinh{{ }}
        && genFType(0) == sinh(genFType(zero))

        // CHECK_GLSL: cosh(
        // CHECK_SPIR: Cosh{{ }}
        && genFType(1) == cosh(genFType(zero))

        // CHECK_GLSL: tanh(
        // CHECK_SPIR: Tanh{{ }}
        && genFType(0) == tanh(genFType(zero))

        // CHECK_GLSL: asinh(
        // CHECK_SPIR: Asinh{{ }}
        && genFType(0) == asinh(genFType(zero))

        // CHECK_GLSL: acosh(
        // CHECK_SPIR: Acosh{{ }}
        && genFType(0) == acosh(genFType(one))

        // CHECK_GLSL: atanh(
        // CHECK_SPIR: Atanh{{ }}
        && genFType(0) == atanh(genFType(zero))

        // 8.2. Exponential Functions

        // CHECK_GLSL: pow(
        // CHECK_SPIR: Pow{{ }}
        && genFType(1) == pow(genFType(one), genFType(zero))

        // CHECK_GLSL: exp(
        // CHECK_SPIR: Exp{{ }}
        && genFType(1) == exp(genFType(zero))

        // CHECK_GLSL: log(
        // CHECK_SPIR: Log{{ }}
        && genFType(0) == log(genFType(one))

        // CHECK_GLSL: exp2(
        // CHECK_SPIR: Exp2{{ }}
        && genFType(1) == exp2(genFType(zero))

        // CHECK_GLSL: log2(
        // CHECK_SPIR: Log2{{ }}
        && genFType(0) == log2(genFType(one))

        // CHECK_GLSL-COUNT-2: sqrt(
        // CHECK_SPIR-COUNT-2: Sqrt{{ }}
        && genFType(0) == sqrt(genFType(zero))
        && genDType(0) == sqrt(genDType(zero))

        // CHECK_GLSL-COUNT-2: inversesqrt(
        // CHECK_SPIR-COUNT-2: InverseSqrt{{ }}
        && genFType(1) == inversesqrt(genFType(one))
        && genDType(1) == inversesqrt(genDType(one))

        // 8.3. Common Functions

        // CHECK_GLSL-COUNT-3: abs(
        // CHECK_SPIR: FAbs{{ }}
        // CHECK_SPIR: SAbs{{ }}
        // CHECK_SPIR: FAbs{{ }}
        && genFType(0) == abs(genFType(zero))
        && genIType(0) == abs(genIType(zero))
        && genDType(0) == abs(genDType(zero))

        // CHECK_GLSL-COUNT-3: sign(
        // CHECK_SPIR: FSign{{ }}
        // CHECK_SPIR: SSign{{ }}
        // CHECK_SPIR: FSign{{ }}
        && genFType(0) == sign(genFType(zero))
#if !defined(TARGET_CUDA)
        && genIType(0) == sign(genIType(zero))
#endif // #if !defined(TARGET_CUDA)
        && genDType(0) == sign(genDType(zero))

        // CHECK_GLSL-COUNT-2: floor(
        // CHECK_SPIR-COUNT-2: Floor{{ }}
        && genFType(0) == floor(genFType(zero))
        && genDType(0) == floor(genDType(zero))

        // CHECK_GLSL-COUNT-2: trunc(
        // CHECK_SPIR-COUNT-2: Trunc{{ }}
        && genFType(0) == trunc(genFType(zero))
        && genDType(0) == trunc(genDType(zero))

        // CHECK_GLSL-COUNT-2: round(
        // CHECK_SPIR-COUNT-2: Round{{ }}
        && genFType(0) == round(genFType(zero))
        && genDType(0) == round(genDType(zero))

        // CHECK_GLSL-COUNT-2: roundEven(
        // CHECK_SPIR-COUNT-2: RoundEven{{ }}
        && genFType(0) == roundEven(genFType(zero))
        && genDType(0) == roundEven(genDType(zero))

        // CHECK_GLSL-COUNT-2: ceil(
        // CHECK_SPIR-COUNT-2: Ceil{{ }}
        && genFType(0) == ceil(genFType(zero))
        && genDType(0) == ceil(genDType(zero))

        // CHECK_GLSL-COUNT-2: fract(
        // CHECK_SPIR-COUNT-2: Fract{{ }}
        && genFType(0) == fract(genFType(zero))
        && genDType(0) == fract(genDType(zero))

        // CHECK_GLSL-COUNT-2: mod(
        // CHECK_SPIR-COUNT-2: Floor{{ }}
        && genFType(0) == mod(genFType(zero), genFType(one))
        && genDType(0) == mod(genDType(zero), genDType(one))

        // CHECK_GLSL-COUNT-2: modf(
        // CHECK_SPIR-COUNT-2: Modf{{ }}
        && genFType(0) == modf(genFType(zero), outGenFType) && 0 == outGenFType
        && genDType(0) == modf(genDType(zero), outGenDType) && 0 == outGenDType

        // CHECK_GLSL-COUNT-4: min(
        // CHECK_SPIR-COUNT-2: FMin{{ }}
        // CHECK_SPIR: SMin{{ }}
        // CHECK_SPIR: UMin{{ }}
        && genFType(0) == min(genFType(zero), genFType(zero))
        && genDType(0) == min(genDType(zero), genDType(zero))
        && genIType(0) == min(genIType(zero), genIType(zero))
        && genUType(0) == min(genUType(zero), genUType(zero))

        // CHECK_GLSL-COUNT-4: max(
        // CHECK_SPIR-COUNT-2: FMax{{ }}
        // CHECK_SPIR: SMax{{ }}
        // CHECK_SPIR: UMax{{ }}
        && genFType(0) == max(genFType(zero), genFType(zero))
        && genDType(0) == max(genDType(zero), genDType(zero))
        && genIType(0) == max(genIType(zero), genIType(zero))
        && genUType(0) == max(genUType(zero), genUType(zero))

        // CHECK_GLSL-COUNT-4: clamp(
        // CHECK_SPIR-COUNT-2: FClamp{{ }}
        // CHECK_SPIR: SClamp{{ }}
        // CHECK_SPIR: UClamp{{ }}
        && genFType(0) == clamp(genFType(zero), genFType(zero), genFType(zero))
        && genDType(0) == clamp(genDType(zero), genDType(zero), genDType(zero))
        && genIType(0) == clamp(genIType(zero), genIType(zero), genIType(zero))
        && genUType(0) == clamp(genUType(zero), genUType(zero), genUType(zero))

        // CHECK_GLSL-COUNT-2: mix(
        // CHECK_SPIR-COUNT-2: FMix{{ }}
        && genFType(0) == mix(genFType(zero), genFType(zero), genFType(zero))
        && genDType(0) == mix(genDType(zero), genDType(zero), genDType(zero))

        // CHECK_GLSL-COUNT-5: mix(
        // C-HECK_SPIR-COUNT-5: OpSelect{{ }}
        && genFType(0) == mix(genFType(zero), genFType(zero), genBType(zero))
        && genDType(0) == mix(genDType(zero), genDType(zero), genBType(zero))
        && genIType(0) == mix(genIType(zero), genIType(zero), genBType(zero))
        && genUType(0) == mix(genUType(zero), genUType(zero), genBType(zero))
        && genBType(0) == mix(genBType(zero), genBType(zero), genBType(zero))

        // CHECK_GLSL-COUNT-4: step(
        // CHECK_SPIR-COUNT-4: Step{{ }}
        && genFType(0) == step(genFType(one), genFType(zero))
        && genFType(0) == step(float(one), genFType(zero))
        && genDType(0) == step(genDType(one), genDType(zero))
        && genDType(0) == step(double(one), genDType(zero))

        // CHECK_GLSL-COUNT-4: smoothstep(
        // CHECK_SPIR-COUNT-4: SmoothStep{{ }}
        && genFType(0) == smoothstep(genFType(zero), genFType(one), genFType(zero))
        && genFType(0) == smoothstep(float(zero), float(one), genFType(zero))
        && genDType(0) == smoothstep(genDType(zero), genDType(one), genDType(zero))
        && genDType(0) == smoothstep(double(zero), double(one), genDType(zero))

        // CHECK_GLSL-COUNT-2: isnan(
        // CHECK_SPIR-COUNT-2: OpIsNan{{ }}
        && genBType(0) == isnan(genFType(zero))
        && genBType(0) == isnan(genDType(zero))

        // CHECK_GLSL-COUNT-2: isinf(
        // CHECK_SPIR-COUNT-2: OpIsInf{{ }}
        && genBType(0) == isinf(genFType(zero))
        && genBType(0) == isinf(genDType(zero))

        // CHECK_GLSL: floatBitsToInt(
        // CHECK_SPIR: OpBitcast{{ }}
        && genIType(0) == floatBitsToInt(genFType(zero))

        // CHECK_GLSL: floatBitsToUint(
        // CHECK_SPIR: OpBitcast{{ }}
        && genUType(0) == floatBitsToUint(genFType(zero))

        // CHECK_GLSL: intBitsToFloat(
        // CHECK_SPIR: OpBitcast{{ }}
        && genFType(0) == intBitsToFloat(genIType(zero))

        // CHECK_GLSL: uintBitsToFloat(
        // CHECK_SPIR: OpBitcast{{ }}
        && genFType(0) == uintBitsToFloat(genUType(zero))

        // CHECK_GLSL-COUNT-2: fma(
        // CHECK_SPIR-COUNT-2: Fma{{ }}
        && genFType(0) == fma(genFType(zero), genFType(zero), genFType(zero))
        && genDType(0) == fma(genDType(zero), genDType(zero), genDType(zero))

        // CHECK_GLSL-COUNT-2: frexp(
        // CHECK_SPIR-COUNT-2: Frexp{{ }}
        && genFType(0) == frexp(genFType(zero), outGenIType) && genIType(0) == outGenIType
        && genDType(0) == frexp(genDType(zero), outGenIType) && genIType(0) == outGenIType

        // CHECK_GLSL-COUNT-2: ldexp(
        // CHECK_SPIR-COUNT-2: Ldexp{{ }}
        && genFType(0) == ldexp(genFType(zero), genIType(zero))
        && genDType(0) == ldexp(genDType(zero), genIType(zero))

        // 8.4. Floating-Point Pack and Unpack Functions

        // CHECK_GLSL: packUnorm2x16(
        // CHECK_SPIR: PackUnorm2x16{{ }}
        && uint(0)  == packUnorm2x16(vec2(zero))

        // CHECK_GLSL: packSnorm2x16(
        // CHECK_SPIR: PackSnorm2x16{{ }}
        && uint(0)  == packSnorm2x16(vec2(zero))

        // CHECK_GLSL: packUnorm4x8(
        // CHECK_SPIR: PackUnorm4x8{{ }}
        && uint(0)  == packUnorm4x8(vec4(zero))

        // CHECK_GLSL: packSnorm4x8(
        // CHECK_SPIR: PackSnorm4x8{{ }}
        && uint(0)  == packSnorm4x8(vec4(zero))

        // CHECK_GLSL: unpackUnorm2x16(
        // CHECK_SPIR: UnpackUnorm2x16{{ }}
        && vec2(0)  == unpackUnorm2x16(uint(zero))

        // CHECK_GLSL: unpackSnorm2x16(
        // CHECK_SPIR: UnpackSnorm2x16{{ }}
        && vec2(0)  == unpackSnorm2x16(uint(zero))

        // CHECK_GLSL: unpackUnorm4x8(
        // CHECK_SPIR: UnpackUnorm4x8{{ }}
        && vec4(0)  == unpackUnorm4x8(uint(zero))

        // CHECK_GLSL: unpackSnorm4x8(
        // CHECK_SPIR: UnpackSnorm4x8{{ }}
        && vec4(0)  == unpackSnorm4x8(uint(zero))

        // CHECK_GLSL: packHalf2x16(
        // CHECK_SPIR: PackHalf2x16{{ }}
        && uint(0)  == packHalf2x16(vec2(zero))

        // CHECK_GLSL: unpackHalf2x16(
        // CHECK_SPIR: UnpackHalf2x16{{ }}
        && vec2(0)  == unpackHalf2x16(uint(zero))

        // CHECK_GLSL: packDouble2x32(
        // CHECK_SPIR: PackDouble2x32{{ }}
        && double(0) == packDouble2x32(uvec2(zero))

        // CHECK_GLSL: unpackDouble2x32(
        // CHECK_SPIR: UnpackDouble2x32{{ }}
        && uvec2(0) == unpackDouble2x32(packDouble2x32(uvec2(zero)))

        // 8.5. Geometric Functions

        // CHECK_GLSL-COUNT-2: length(
        // CHECK_SPIR-COUNT-2: Length{{ }}
        && float(0)  == length(genFType(zero))
        && double(0) == length(genDType(zero))

        // CHECK_GLSL-COUNT-2: distance(
        // CHECK_SPIR-COUNT-2: Distance{{ }}
        && float(0)  == distance(genFType(zero), genFType(zero))
        && double(0) == distance(genDType(zero), genDType(zero))

        // CHECK_GLSL-COUNT-2: dot(
        // SPIR-V doesn't have OpDot for scalar types
        && float(0)  == dot(genFType(zero), genFType(zero))
        && double(0) == dot(genDType(zero), genDType(zero))

        // CHECK_GLSL-COUNT-2: cross(
        // CHECK_SPIR-COUNT-2: Cross{{ }}
        && vec3(0)  == cross(vec3(0), vec3(zero))
        && dvec3(0) == cross(dvec3(0), dvec3(zero))

        // CHECK_GLSL-COUNT-2: normalize(
        // CHECK_SPIR-COUNT-2: Normalize{{ }}
        && (abs(float(1)  - length(normalize(genFType(one)))) < epsilon)
        && (abs(double(1) - length(normalize(genDType(one)))) < double(epsilon))

        // CHECK_GLSL-COUNT-2: faceforward(
        // CHECK_SPIR-COUNT-2: FaceForward{{ }}
        && genFType(1) == faceforward(genFType(one), genFType(one), genFType(negaOne))
        && genDType(1) == faceforward(genDType(one), genDType(one), genDType(negaOne))

        // CHECK_GLSL-COUNT-2: reflect(
        // CHECK_SPIR-COUNT-2: Reflect{{ }}
        && genFType(0) == reflect(genFType(zero), genFType(zero))
        && genDType(0) == reflect(genDType(zero), genDType(zero))

        // CHECK_GLSL-COUNT-2: refract(
        // CHECK_SPIR-COUNT-2: Refract{{ }}
        && genFType(0) == refract(genFType(zero), genFType(zero), float(zero))
        && genDType(0) == refract(genDType(zero), genDType(zero), double(zero))

        // 8.6. Matrix Functions

        // CHECK_GLSL-COUNT-9: matrixCompMult(
        && equals(mat2x2(0), matrixCompMult(mat2x2(0), mat2x2(zero)))
        && equals(mat2x3(0), matrixCompMult(mat2x3(0), mat2x3(zero)))
        && equals(mat2x4(0), matrixCompMult(mat2x4(0), mat2x4(zero)))
        && equals(mat3x2(0), matrixCompMult(mat3x2(0), mat3x2(zero)))
        && equals(mat3x3(0), matrixCompMult(mat3x3(0), mat3x3(zero)))
        && equals(mat3x4(0), matrixCompMult(mat3x4(0), mat3x4(zero)))
        && equals(mat4x2(0), matrixCompMult(mat4x2(0), mat4x2(zero)))
        && equals(mat4x3(0), matrixCompMult(mat4x3(0), mat4x3(zero)))
        && equals(mat4x4(0), matrixCompMult(mat4x4(0), mat4x4(zero)))

        // CHECK_GLSL-COUNT-4: outerProduct(
        // CHECK_SPIR-COUNT-4: OpOuterProduct{{ }}
        && equals(mat2(0), outerProduct(vec2(0), vec2(zero)))
        && equals(mat3(0), outerProduct(vec3(0), vec3(zero)))
        && equals(mat4(0), outerProduct(vec4(0), vec4(zero)))
        && equals(mat2x3(0), outerProduct(vec3(0), vec2(zero)))
        && equals(mat3x2(0), outerProduct(vec2(0), vec3(zero)))
        && equals(mat2x4(0), outerProduct(vec4(0), vec2(zero)))
        && equals(mat4x2(0), outerProduct(vec2(0), vec4(zero)))
        && equals(mat3x4(0), outerProduct(vec4(0), vec3(zero)))
        && equals(mat4x3(0), outerProduct(vec3(0), vec4(zero)))

        // CHECK_GLSL-COUNT-9: transpose(
        // CHECK_SPIR-COUNT-9: OpTranspose{{ }}
        && equals(mat2(0), transpose(mat2(zero)))
        && equals(mat3(0), transpose(mat3(zero)))
        && equals(mat4(0), transpose(mat4(zero)))
        && equals(mat2x3(0), transpose(mat3x2(zero)))
        && equals(mat3x2(0), transpose(mat2x3(zero)))
        && equals(mat2x4(0), transpose(mat4x2(zero)))
        && equals(mat4x2(0), transpose(mat2x4(zero)))
        && equals(mat3x4(0), transpose(mat4x3(zero)))
        && equals(mat4x3(0), transpose(mat3x4(zero)))

        // CHECK_GLSL-COUNT-3: determinant(
        // CHECK_SPIR-COUNT-3: Determinant{{ }}
        && float(0) == determinant(mat2(zero))
        && float(0) == determinant(mat3(zero))
        && float(0) == determinant(mat4(zero))

        // CHECK_GLSL-COUNT-3: inverse(
        // CHECK_SPIR-COUNT-3: MatrixInverse{{ }}
#if !defined(TARGET_LLVM)
        && equals(identity2x2, inverse(identity2x2))
        && equals(identity3x3, inverse(identity3x3))
        && equals(identity4x4, inverse(identity4x4))
#endif

        // 8.8. Integer Functions

        // CHECK_GLSL: uaddCarry(
        // CHECK_SPIR: OpIAddCarry{{ }}
        && genUType(0) == uaddCarry(genUType(zero), genUType(zero), outGenUType) && genUType(0) == outGenUType

        // CHECK_GLSL: usubBorrow(
        // CHECK_SPIR: OpISubBorrow{{ }}
        && genUType(0) == usubBorrow(genUType(zero), genUType(zero), outGenUType) && genUType(0) == outGenUType

        // CHECK_GLSL-COUNT-2: bitfieldExtract(
        // CHECK_SPIR: OpBitFieldSExtract{{ }}
        // CHECK_SPIR: OpBitFieldUExtract{{ }}
        && genIType(0) == bitfieldExtract(genIType(zero), uint(zero), uint(zero))
        && genUType(0) == bitfieldExtract(genUType(zero), uint(zero), uint(zero))

        // CHECK_GLSL-COUNT-2: bitfieldInsert(
        // CHECK_SPIR-COUNT-2: OpBitFieldInsert{{ }}
        && genIType(0) == bitfieldInsert(genIType(zero), genIType(zero), uint(zero), uint(zero))
        && genUType(0) == bitfieldInsert(genUType(zero), genUType(zero), uint(zero), uint(zero))

        // CHECK_GLSL-COUNT-2: bitfieldReverse(
        // CHECK_SPIR-COUNT-2: OpBitReverse{{ }}
        && genIType(0) == bitfieldReverse(genIType(zero))
        && genUType(0) == bitfieldReverse(genUType(zero))

        // CHECK_GLSL-COUNT-2: bitCount(
        // CHECK_SPIR-COUNT-2: OpBitCount{{ }}
        && genIType(0) == bitCount(genIType(zero))
        && genIType(0) == bitCount(genUType(zero))

        // CHECK_GLSL-COUNT-2: findLSB(
        // CHECK_SPIR-COUNT-2: FindILsb{{ }}
        && genIType(-1) == findLSB(genIType(zero))
        && genIType(-1) == findLSB(genUType(zero))

        // CHECK_GLSL-COUNT-2: findMSB(
        // CHECK_SPIR: FindSMsb{{ }}
        // CHECK_SPIR: FindUMsb{{ }}
        && genIType(-1) == findMSB(genIType(zero))
        && genIType(-1) == findMSB(genUType(zero))
        ;
}

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // CHECK_GLSL: void main(
    // CHECK_GLSL_SPIRV: OpEntryPoint
    // CHECK_HLSL: void computeMain(
    // CHECK_CUDA: void computeMain(
    // CHECK_CPP: void _computeMain(
    // BUF: 1

    bool r = true
        && Test_ScalarType()
        ;

    outputBuffer.result = int(r);
}
