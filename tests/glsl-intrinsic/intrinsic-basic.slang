//TEST:SIMPLE(filecheck=CHECK_GLSL): -allow-glsl -stage fragment -entry computeMain -target glsl
//TEST:SIMPLE(filecheck=CHECK_SPV):  -allow-glsl -stage fragment -entry computeMain -target spirv
//TEST:SIMPLE(filecheck=CHECK_HLSL): -allow-glsl -stage fragment -entry computeMain -target hlsl
//TEST:SIMPLE(filecheck=CHECK_CUDA): -allow-glsl -stage fragment -entry computeMain -target cuda -DTARGET_CUDA
//TEST:SIMPLE(filecheck=CHECK_CPP):  -allow-glsl -stage fragment -entry computeMain -target cpp

//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -compute -entry computeMain -allow-glsl -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -compute -entry computeMain -allow-glsl -output-using-type -emit-spirv-directly

// "inverse()" function is not implemented yet.
//#defined TEST_when_inverse_works

// "ftransform()" function is not implemented yet.
//#defined TEST_when_fransform_works

// "exp2" for double type is causing an issue with SPIRV
//#define TEST_when_exp2_double_type_works

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
buffer MyBlockName2
{
    int4 result;
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

bool dim1TypeFuncs()
{
    typealias genFType = float;
    typealias genDType = double;
    typealias genIType = int;
    typealias genUType = uint;
    typealias genBType = bool;

    genFType outGenFType;
    genDType outGenDType;
    genIType outGenIType, outGenIType2;
    genUType outGenUType, outGenUType2;

    constexpr float epsilon = 0.000001;

    bool voidResults = true;

    // 8.8. Integer Functions
    umulExtended(genUType(0), genUType(0), outGenUType, outGenUType2);
    voidResults = voidResults && genUType(0) == outGenUType;
    voidResults = voidResults && genUType(0) == outGenUType2;

    imulExtended(genIType(0), genIType(0), outGenIType, outGenIType2);
    voidResults = voidResults && genIType(0) == outGenIType;
    voidResults = voidResults && genIType(0) == outGenIType2;

    return voidResults
        // 8.1. Angle and Trigonometry Functions
        && genFType(0) == radians(genFType(0))
        && genFType(0) == degrees(genFType(0))
        && genFType(0) == sin(genFType(0))
        && genFType(1) == cos(genFType(0))
        && genFType(0) == tan(genFType(0))
        && genFType(0) == asin(genFType(0))
        && genFType(0) == acos(genFType(1))
        && genFType(0) == atan(genFType(0), genFType(0))
        && genFType(0) == atan(genFType(0))
        && genFType(0) == sinh(genFType(0))
        && genFType(1) == cosh(genFType(0))
        && genFType(0) == tanh(genFType(0))
        && genFType(0) == asinh(genFType(0))
        && genFType(0) == acosh(genFType(1))
        && genFType(0) == atanh(genFType(0))

        // 8.2. Exponential Functions
        && genFType(1) == pow(genFType(1), genFType(0))
        && genFType(1) == exp(genFType(0))
        && genFType(0) == log(genFType(1))
        && genFType(1) == exp2(genFType(0))
#if defined(TEST_when_exp2_double_type_works)
        && genDType(1) == exp2(genDType(0))
#endif // #if defined(TEST_when_exp2_double_type_works)
        && genFType(0) == log2(genFType(1))
        && genFType(0) == sqrt(genFType(0))
        && genDType(0) == sqrt(genDType(0))
        && genFType(1) == inversesqrt(genFType(1))
        && genDType(1) == inversesqrt(genDType(1))

        // 8.3. Common Functions
        && genFType(0) == abs(genFType(0))
        && genIType(0) == abs(genIType(0))
        && genDType(0) == abs(genDType(0))
        && genFType(0) == sign(genFType(0))
#if !defined(TARGET_CUDA)
        && genIType(0) == sign(genIType(0))
#endif // #if !defined(TARGET_CUDA)
        && genDType(0) == sign(genDType(0))
        && genFType(0) == floor(genFType(0))
        && genDType(0) == floor(genDType(0))
        && genFType(0) == trunc(genFType(0))
        && genDType(0) == trunc(genDType(0))
        && genFType(0) == round(genFType(0))
        && genDType(0) == round(genDType(0))
        && genFType(0) == roundEven(genFType(0))
        && genDType(0) == roundEven(genDType(0))
        && genFType(0) == ceil(genFType(0))
        && genDType(0) == ceil(genDType(0))
        && genFType(0) == fract(genFType(0))
        && genDType(0) == fract(genDType(0))
        && genFType(0) == mod(genFType(0), float(1))
        && genFType(0) == mod(genFType(0), genFType(1))
        && genDType(0) == mod(genDType(0), double(1))
        && genDType(0) == mod(genDType(0), genDType(0))
        && genFType(0) == modf(genFType(0), outGenFType) && 0 == outGenFType
        && genDType(0) == modf(genDType(0), outGenDType) && 0 == outGenDType
        && genFType(0) == min(genFType(0), genFType(0))
        && genFType(0) == min(genFType(0), float(0))
        && genDType(0) == min(genDType(0), genDType(0))
        && genDType(0) == min(genDType(0), double(0))
        && genIType(0) == min(genIType(0), genIType(0))
        && genIType(0) == min(genIType(0), int(0))
        && genUType(0) == min(genUType(0), genUType(0))
        && genUType(0) == min(genUType(0), uint(0))
        && genFType(0) == max(genFType(0), genFType(0))
        && genFType(0) == max(genFType(0), float(0))
        && genDType(0) == max(genDType(0), genDType(0))
        && genDType(0) == max(genDType(0), double(0))
        && genIType(0) == max(genIType(0), genIType(0))
        && genIType(0) == max(genIType(0), int(0))
        && genUType(0) == max(genUType(0), genUType(0))
        && genUType(0) == max(genUType(0), uint(0))
        && genFType(0) == clamp(genFType(0), genFType(0), genFType(0))
        && genFType(0) == clamp(genFType(0), float(0), float(0))
        && genDType(0) == clamp(genDType(0), genDType(0), genDType(0))
        && genDType(0) == clamp(genDType(0), double(0), double(0))
        && genIType(0) == clamp(genIType(0), genIType(0), genIType(0))
        && genIType(0) == clamp(genIType(0), int(0), int(0))
        && genUType(0) == clamp(genUType(0), genUType(0), genUType(0))
        && genUType(0) == clamp(genUType(0), uint(0), uint(0))
        && genFType(0) == mix(genFType(0), genFType(0), genFType(0))
        && genFType(0) == mix(genFType(0), genFType(0), float(0))
        && genDType(0) == mix(genDType(0), genDType(0), genDType(0))
        && genDType(0) == mix(genDType(0), genDType(0), double(0))
        && genFType(0) == mix(genFType(0), genFType(0), genBType(0))
        && genDType(0) == mix(genDType(0), genDType(0), genBType(0))
        && genIType(0) == mix(genIType(0), genIType(0), genBType(0))
        && genUType(0) == mix(genUType(0), genUType(0), genBType(0))
        && genBType(0) == mix(genBType(0), genBType(0), genBType(0))
        && genFType(0) == step(genFType(1), genFType(0))
        && genFType(0) == step(float(1), genFType(0))
        && genDType(0) == step(genDType(1), genDType(0))
        && genDType(0) == step(double(1), genDType(0))
        && genFType(0) == smoothstep(genFType(0), genFType(1), genFType(0))
        && genFType(0) == smoothstep(float(0), float(1), genFType(0))
        && genDType(0) == smoothstep(genDType(0), genDType(1), genDType(0))
        && genDType(0) == smoothstep(double(0), double(1), genDType(0))
        && genBType(0) == isnan(genFType(0))
        && genBType(0) == isnan(genDType(0))
        && genBType(0) == isinf(genFType(0))
        && genBType(0) == isinf(genDType(0))
        && genIType(0) == floatBitsToInt(genFType(0))
        && genUType(0) == floatBitsToUint(genFType(0))
        && genFType(0) == intBitsToFloat(genIType(0))
        && genFType(0) == uintBitsToFloat(genUType(0))
        && genFType(0) == fma(genFType(0), genFType(0), genFType(0))
        && genDType(0) == fma(genDType(0), genDType(0), genDType(0))
        && genFType(0) == frexp(genFType(0), outGenIType) && genIType(0) == outGenIType
        && genDType(0) == frexp(genDType(0), outGenIType) && genIType(0) == outGenIType
        && genFType(0) == ldexp(genFType(0), genIType(0))
#if defined(TEST_when_exp2_double_type_works)
        && genDType(0) == ldexp(genDType(0), genIType(0))
#endif // #if defined(TEST_when_exp2_double_type_works)

        // 8.5. Geometric Functions
        && float(0) == length(genFType(0))
        && double(0) == length(genDType(0))
        && float(0) == distance(genFType(0), genFType(0))
        && double(0) == distance(genDType(0), genDType(0))
        && float(0) == dot(genFType(0), genFType(0))
        && double(0) == dot(genDType(0), genDType(0))
        && (abs(float(1) - length(normalize(genFType(1)))) < epsilon)
        && (abs(double(1) - length(normalize(genDType(1)))) < double(epsilon))
        && genFType(1) == faceforward(genFType(1), genFType(1), genFType(-1))
        && genDType(1) == faceforward(genDType(1), genDType(1), genDType(-1))
        && genFType(0) == reflect(genFType(0), genFType(0))
        && genDType(0) == reflect(genDType(0), genDType(0))
        && genFType(0) == refract(genFType(0), genFType(0), float(0))
        && genDType(0) == refract(genDType(0), genDType(0), double(0))

        // 8.6. Matrix Functions
        && equals(mat2x2(0), matrixCompMult(mat2x2(0), mat2x2(0)))
        && equals(mat2x3(0), matrixCompMult(mat2x3(0), mat2x3(0)))
        && equals(mat2x4(0), matrixCompMult(mat2x4(0), mat2x4(0)))
        && equals(mat3x2(0), matrixCompMult(mat3x2(0), mat3x2(0)))
        && equals(mat3x3(0), matrixCompMult(mat3x3(0), mat3x3(0)))
        && equals(mat3x4(0), matrixCompMult(mat3x4(0), mat3x4(0)))
        && equals(mat4x2(0), matrixCompMult(mat4x2(0), mat4x2(0)))
        && equals(mat4x3(0), matrixCompMult(mat4x3(0), mat4x3(0)))
        && equals(mat4x4(0), matrixCompMult(mat4x4(0), mat4x4(0)))
        && equals(mat2(0), outerProduct(vec2(0), vec2(0)))
        && equals(mat3(0), outerProduct(vec3(0), vec3(0)))
        && equals(mat4(0), outerProduct(vec4(0), vec4(0)))
        && equals(mat2x3(0), outerProduct(vec3(0), vec2(0)))
        && equals(mat3x2(0), outerProduct(vec2(0), vec3(0)))
        && equals(mat2x4(0), outerProduct(vec4(0), vec2(0)))
        && equals(mat4x2(0), outerProduct(vec2(0), vec4(0)))
        && equals(mat3x4(0), outerProduct(vec4(0), vec3(0)))
        && equals(mat4x3(0), outerProduct(vec3(0), vec4(0)))
        && equals(mat2(0), transpose(mat2(0)))
        && equals(mat3(0), transpose(mat3(0)))
        && equals(mat4(0), transpose(mat4(0)))
        && equals(mat2x3(0), transpose(mat3x2(0)))
        && equals(mat3x2(0), transpose(mat2x3(0)))
        && equals(mat2x4(0), transpose(mat4x2(0)))
        && equals(mat4x2(0), transpose(mat2x4(0)))
        && equals(mat3x4(0), transpose(mat4x3(0)))
        && equals(mat4x3(0), transpose(mat3x4(0)))
        && float(0) == determinant(mat2(0))
        && float(0) == determinant(mat3(0))
        && float(0) == determinant(mat4(0))
#if defined(TEST_when_inverse_works)
        && equals(mat2(0), inverse(mat2(0)))
        && equals(mat3(0), inverse(mat3(0)))
        && equals(mat4(0), inverse(mat4(0)))
#endif // #if defined(TEST_when_inverse_works)

        // 8.8. Integer Functions
        && genUType(0) == uaddCarry(genUType(0), genUType(0), outGenUType) && genUType(0) == outGenUType
        && genUType(0) == usubBorrow(genUType(0), genUType(0), outGenUType) && genUType(0) == outGenUType
        && genIType(0) == bitfieldExtract(genIType(0), int(0), int(0))
        && genUType(0) == bitfieldExtract(genUType(0), int(0), int(0))
        && genIType(0) == bitfieldInsert(genIType(0), genIType(0), int(0), int(0))
        && genUType(0) == bitfieldInsert(genUType(0), genUType(0), int(0), int(0))
        && genIType(0) == bitfieldReverse(genIType(0))
        && genUType(0) == bitfieldReverse(genUType(0))
        && genIType(0) == bitCount(genIType(0))
        && genIType(0) == bitCount(genUType(0))
        && genIType(-1) == findLSB(genIType(0))
        && genIType(-1) == findLSB(genUType(0))
        && genIType(-1) == findMSB(genIType(0))
        && genIType(-1) == findMSB(genUType(0))
        ;
}

__generic<let N : int>
bool dimNTypeFuncs()
{
    typealias genFType = vector<float, N>;
    typealias genDType = vector<double, N>;
    typealias genIType = vector<int, N>;
    typealias genUType = vector<uint, N>;
    typealias genBType = vector<bool, N>;
    typealias vec      = vector<float, N>;
    typealias ivec     = vector<int, N>;
    typealias uvec     = vector<uint, N>;
    typealias bvec     = vector<bool, N>;

    genFType outGenFType;
    genDType outGenDType;
    genIType outGenIType, outGenIType2;
    genUType outGenUType, outGenUType2;

    constexpr float epsilon = 0.000001;

    bool voidResults = true;

    // 8.8. Integer Functions
    umulExtended(genUType(0), genUType(0), outGenUType, outGenUType2);
    voidResults = voidResults && genUType(0) == outGenUType;
    voidResults = voidResults && genUType(0) == outGenUType2;

    imulExtended(genIType(0), genIType(0), outGenIType, outGenIType2);
    voidResults = voidResults && genIType(0) == outGenIType;
    voidResults = voidResults && genIType(0) == outGenIType2;

    return voidResults
        // 8.1. Angle and Trigonometry Functions
        && genFType(0) == radians(genFType(0))
        && genFType(0) == degrees(genFType(0))
        && genFType(0) == sin(genFType(0))
        && genFType(1) == cos(genFType(0))
        && genFType(0) == tan(genFType(0))
        && genFType(0) == asin(genFType(0))
        && genFType(0) == acos(genFType(1))
        && genFType(0) == atan(genFType(0), genFType(0))
        && genFType(0) == atan(genFType(0))
        && genFType(0) == sinh(genFType(0))
        && genFType(1) == cosh(genFType(0))
        && genFType(0) == tanh(genFType(0))
#if !defined(TARGET_CUDA)
        && genFType(0) == asinh(genFType(0))
        && genFType(0) == acosh(genFType(1))
        && genFType(0) == atanh(genFType(0))
#endif // #if !defined(TARGET_CUDA)

        // 8.2. Exponential Functions
        && genFType(1) == pow(genFType(1), genFType(0))
        && genFType(1) == exp(genFType(0))
        && genFType(0) == log(genFType(1))
        && genFType(1) == exp2(genFType(0))
#if defined(TEST_when_exp2_double_type_works)
        && genDType(1) == exp2(genDType(0))
#endif // #if defined(TEST_when_exp2_double_type_works)
        && genFType(0) == log2(genFType(1))
        && genFType(0) == sqrt(genFType(0))
        && genDType(0) == sqrt(genDType(0))
        && genFType(1) == inversesqrt(genFType(1))
        && genDType(1) == inversesqrt(genDType(1))

        // 8.3. Common Functions
        && genFType(0) == abs(genFType(0))
        && genIType(0) == abs(genIType(0))
        && genDType(0) == abs(genDType(0))
        && genFType(0) == sign(genFType(0))
#if !defined(TARGET_CUDA)
        && genIType(0) == sign(genIType(0))
#endif // #if !defined(TARGET_CUDA)
        && genDType(0) == sign(genDType(0))
        && genFType(0) == floor(genFType(0))
        && genDType(0) == floor(genDType(0))
        && genFType(0) == trunc(genFType(0))
        && genDType(0) == trunc(genDType(0))
        && genFType(0) == round(genFType(0))
        && genDType(0) == round(genDType(0))
        && genFType(0) == roundEven(genFType(0))
        && genDType(0) == roundEven(genDType(0))
        && genFType(0) == ceil(genFType(0))
        && genDType(0) == ceil(genDType(0))
        && genFType(0) == fract(genFType(0))
        && genDType(0) == fract(genDType(0))
        && genFType(0) == mod(genFType(0), float(1))
        && genFType(0) == mod(genFType(0), genFType(1))
        && genDType(0) == mod(genDType(0), double(1))
        && genDType(0) == mod(genDType(0), genDType(0))
        && genFType(0) == modf(genFType(0), outGenFType) && genFType(0) == outGenFType
        && genDType(0) == modf(genDType(0), outGenDType) && genDType(0) == outGenDType
        && genFType(0) == min(genFType(0), genFType(0))
        && genFType(0) == min(genFType(0), float(0))
        && genDType(0) == min(genDType(0), genDType(0))
        && genDType(0) == min(genDType(0), double(0))
        && genIType(0) == min(genIType(0), genIType(0))
        && genIType(0) == min(genIType(0), int(0))
        && genUType(0) == min(genUType(0), genUType(0))
        && genUType(0) == min(genUType(0), uint(0))
        && genFType(0) == max(genFType(0), genFType(0))
        && genFType(0) == max(genFType(0), float(0))
        && genDType(0) == max(genDType(0), genDType(0))
        && genDType(0) == max(genDType(0), double(0))
        && genIType(0) == max(genIType(0), genIType(0))
        && genIType(0) == max(genIType(0), int(0))
        && genUType(0) == max(genUType(0), genUType(0))
        && genUType(0) == max(genUType(0), uint(0))
        && genFType(0) == clamp(genFType(0), genFType(0), genFType(0))
        && genFType(0) == clamp(genFType(0), float(0), float(0))
        && genDType(0) == clamp(genDType(0), genDType(0), genDType(0))
        && genDType(0) == clamp(genDType(0), double(0), double(0))
        && genIType(0) == clamp(genIType(0), genIType(0), genIType(0))
        && genIType(0) == clamp(genIType(0), int(0), int(0))
        && genUType(0) == clamp(genUType(0), genUType(0), genUType(0))
        && genUType(0) == clamp(genUType(0), uint(0), uint(0))
        && genFType(0) == mix(genFType(0), genFType(0), genFType(0))
        && genFType(0) == mix(genFType(0), genFType(0), float(0))
        && genDType(0) == mix(genDType(0), genDType(0), genDType(0))
        && genDType(0) == mix(genDType(0), genDType(0), double(0))
#if !defined(TARGET_CUDA)
        && genFType(0) == mix(genFType(0), genFType(0), genBType(0))
        && genDType(0) == mix(genDType(0), genDType(0), genBType(0))
        && genIType(0) == mix(genIType(0), genIType(0), genBType(0))
        && genUType(0) == mix(genUType(0), genUType(0), genBType(0))
        && genBType(0) == mix(genBType(0), genBType(0), genBType(0))
#endif // #if !defined(TARGET_CUDA)
        && genFType(0) == step(genFType(1), genFType(0))
        && genFType(0) == step(float(1), genFType(0))
        && genDType(0) == step(genDType(1), genDType(0))
        && genDType(0) == step(double(1), genDType(0))
        && genFType(0) == smoothstep(genFType(0), genFType(1), genFType(0))
        && genFType(0) == smoothstep(float(0), float(1), genFType(0))
        && genDType(0) == smoothstep(genDType(0), genDType(1), genDType(0))
        && genDType(0) == smoothstep(double(0), double(1), genDType(0))
#if !defined(TARGET_CUDA)
        && genBType(0) == isnan(genFType(0))
        && genBType(0) == isnan(genDType(0))
        && genBType(0) == isinf(genFType(0))
        && genBType(0) == isinf(genDType(0))
#endif // #if !defined(TARGET_CUDA)
        && genIType(0) == floatBitsToInt(genFType(0))
        && genUType(0) == floatBitsToUint(genFType(0))
        && genFType(0) == intBitsToFloat(genIType(0))
        && genFType(0) == uintBitsToFloat(genUType(0))
        && genFType(0) == fma(genFType(0), genFType(0), genFType(0))
        && genDType(0) == fma(genDType(0), genDType(0), genDType(0))
        && genFType(0) == frexp(genFType(0), outGenIType) && genIType(0) == outGenIType
        && genDType(0) == frexp(genDType(0), outGenIType) && genIType(0) == outGenIType
        && genFType(0) == ldexp(genFType(0), genIType(0))
#if defined(TEST_when_exp2_double_type_works)
        && genDType(0) == ldexp(genDType(0), genIType(0))
#endif // #if defined(TEST_when_exp2_double_type_works)

        // 8.4. Floating-Point Pack and Unpack Functions
        && uint(0) == packUnorm2x16(vec2(0))
        && uint(0) == packSnorm2x16(vec2(-1))
        && uint(0) == packUnorm4x8(vec4(0))
        && uint(0) == packSnorm4x8(vec4(-1))
        && vec2(0) == unpackUnorm2x16(uint(0))
        && vec2(-1) == unpackSnorm2x16(uint(0))
        && vec4(0) == unpackUnorm4x8(uint(0))
        && vec4(-1) == unpackSnorm4x8(uint(0))
        && uint(0) == packHalf2x16(vec2(0))
        && vec2(0) == unpackHalf2x16(uint(0))
        && double(0) == packDouble2x32(uvec2(0))
        && uvec2(0) == unpackDouble2x32(double(0))

        // 8.5. Geometric Functions
        && float(0) == length(genFType(0))
        && double(0) == length(genDType(0))
        && float(0) == distance(genFType(0), genFType(0))
        && double(0) == distance(genDType(0), genDType(0))
        && float(0) == dot(genFType(0), genFType(0))
        && double(0) == dot(genDType(0), genDType(0))
        && vec3(0) == cross(vec3(0), vec3(0))
        && dvec3(0) == cross(dvec3(0), dvec3(0))
        && (abs(float(1) - length(normalize(genFType(1)))) < epsilon)
        && (abs(double(1) - length(normalize(genDType(1)))) < double(epsilon))
#if defined(TEST_when_fransform_works)
        && vec4(0) == ftransform()
#endif // #if defined(TEST_when_fransform_works)
        && genFType(1) == faceforward(genFType(1), genFType(1), genFType(-1))
        && genDType(1) == faceforward(genDType(1), genDType(1), genDType(-1))
        && genFType(0) == reflect(genFType(0), genFType(0))
        && genDType(0) == reflect(genDType(0), genDType(0))
        && genFType(0) == refract(genFType(0), genFType(0), float(0))
        && genDType(0) == refract(genDType(0), genDType(0), double(0))

        // 8.7. Vector Relational Functions
#if !defined(TARGET_CUDA)
        && bvec(1) == lessThan(vec(0), vec(1))
        && bvec(1) == lessThan(ivec(0), ivec(1))
        && bvec(1) == lessThan(uvec(0), uvec(1))
        && bvec(1) == lessThanEqual(vec(0), vec(1))
        && bvec(1) == lessThanEqual(ivec(0), ivec(1))
        && bvec(1) == lessThanEqual(uvec(0), uvec(1))
        && bvec(0) == greaterThan(vec(0), vec(1))
        && bvec(0) == greaterThan(ivec(0), ivec(1))
        && bvec(0) == greaterThan(uvec(0), uvec(1))
        && bvec(0) == greaterThanEqual(vec(0), vec(1))
        && bvec(0) == greaterThanEqual(ivec(0), ivec(1))
        && bvec(0) == greaterThanEqual(uvec(0), uvec(1))
        && bvec(1) == equal(vec(0), vec(0))
        && bvec(1) == equal(ivec(0), ivec(0))
        && bvec(1) == equal(uvec(0), uvec(0))
        && bvec(1) == equal(bvec(0), bvec(0))
        && bvec(0) == notEqual(vec(0), vec(0))
        && bvec(0) == notEqual(ivec(0), ivec(0))
        && bvec(0) == notEqual(uvec(0), uvec(0))
        && bvec(0) == notEqual(bvec(0), bvec(0))
        && bool(0) == any(bvec(0))
        && bool(0) == all(bvec(0))
        && bvec(1) == not(bvec(0))
#endif // #if !defined(TARGET_CUDA)

        // 8.8. Integer Functions
        && genUType(0) == uaddCarry(genUType(0), genUType(0), outGenUType) && genUType(0) == outGenUType
        && genUType(0) == usubBorrow(genUType(0), genUType(0), outGenUType) && genUType(0) == outGenUType
        && genIType(0) == bitfieldExtract(genIType(0), int(0), int(0))
        && genUType(0) == bitfieldExtract(genUType(0), int(0), int(0))
        && genIType(0) == bitfieldInsert(genIType(0), genIType(0), int(0), int(0))
        && genUType(0) == bitfieldInsert(genUType(0), genUType(0), int(0), int(0))
        && genIType(0) == bitfieldReverse(genIType(0))
        && genUType(0) == bitfieldReverse(genUType(0))
        && genIType(0) == bitCount(genIType(0))
        && genIType(0) == bitCount(genUType(0))
#if !defined(TARGET_CUDA)
        && genIType(-1) == findLSB(genIType(0))
        && genIType(-1) == findLSB(genUType(0))
        && genIType(-1) == findMSB(genIType(0))
        && genIType(-1) == findMSB(genUType(0))
#endif // #if !defined(TARGET_CUDA)
        ;
}

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    // CHECK_GLSL: void main(
    // CHECK_SPV: OpEntryPoint
    // CHECK_HLSL: void computeMain(
    // CHECK_CUDA: void computeMain(
    // CHECK_CPP: void _computeMain(
    // BUF: 1

    bool r = true
        && dim1TypeFuncs()
        && dimNTypeFuncs<2>()
        && dimNTypeFuncs<3>()
        && dimNTypeFuncs<4>();

    outputBuffer.result[0] = int(r);
}
