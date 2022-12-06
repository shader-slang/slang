// Hash function for GLSLstd450

#include "../core/slang-common.h"
#include "../core/slang-string.h"
#include "spirv/unified1/GLSL.std.450.h"


namespace Slang
{

static const unsigned tableSalt[81] ={
    2, 1, 5, 1, 1, 0, 1, 0, 3, 0, 3, 0, 0, 4, 6, 3,
    7, 8, 10, 32, 0, 0, 0, 1, 0, 21, 1, 0, 0, 66, 1, 18,
    2, 0, 0, 2, 0, 1, 0, 0, 3, 1, 1, 0, 0, 0, 0, 1,
    4, 2, 2, 0, 2, 0, 0, 2, 0, 2, 3, 0, 0, 1, 5, 4,
    0, 0, 0, 5, 4, 2, 1, 1, 0, 7, 5, 0, 14, 4, 10, 4,
    5
};

struct KV
{
    const char* name;
    GLSLstd450 value;
};

static const KV words[81] =
{
    {"FindSMsb", GLSLstd450FindSMsb},
    {"SClamp", GLSLstd450SClamp},
    {"UnpackHalf2x16", GLSLstd450UnpackHalf2x16},
    {"Normalize", GLSLstd450Normalize},
    {"Pow", GLSLstd450Pow},
    {"Ceil", GLSLstd450Ceil},
    {"InterpolateAtSample", GLSLstd450InterpolateAtSample},
    {"Cosh", GLSLstd450Cosh},
    {"SMax", GLSLstd450SMax},
    {"PackUnorm2x16", GLSLstd450PackUnorm2x16},
    {"ModfStruct", GLSLstd450ModfStruct},
    {"IMix", GLSLstd450IMix},
    {"Ldexp", GLSLstd450Ldexp},
    {"Atan", GLSLstd450Atan},
    {"Round", GLSLstd450Round},
    {"Cos", GLSLstd450Cos},
    {"UMin", GLSLstd450UMin},
    {"FClamp", GLSLstd450FClamp},
    {"PackHalf2x16", GLSLstd450PackHalf2x16},
    {"SAbs", GLSLstd450SAbs},
    {"FindUMsb", GLSLstd450FindUMsb},
    {"PackUnorm4x8", GLSLstd450PackUnorm4x8},
    {"UnpackDouble2x32", GLSLstd450UnpackDouble2x32},
    {"Fma", GLSLstd450Fma},
    {"RoundEven", GLSLstd450RoundEven},
    {"SmoothStep", GLSLstd450SmoothStep},
    {"Refract", GLSLstd450Refract},
    {"UnpackUnorm4x8", GLSLstd450UnpackUnorm4x8},
    {"NClamp", GLSLstd450NClamp},
    {"Trunc", GLSLstd450Trunc},
    {"Sin", GLSLstd450Sin},
    {"Asinh", GLSLstd450Asinh},
    {"Atanh", GLSLstd450Atanh},
    {"Length", GLSLstd450Length},
    {"Fract", GLSLstd450Fract},
    {"Asin", GLSLstd450Asin},
    {"Determinant", GLSLstd450Determinant},
    {"Floor", GLSLstd450Floor},
    {"SMin", GLSLstd450SMin},
    {"MatrixInverse", GLSLstd450MatrixInverse},
    {"Exp2", GLSLstd450Exp2},
    {"PackSnorm2x16", GLSLstd450PackSnorm2x16},
    {"FindILsb", GLSLstd450FindILsb},
    {"FMax", GLSLstd450FMax},
    {"NMin", GLSLstd450NMin},
    {"Frexp", GLSLstd450Frexp},
    {"InverseSqrt", GLSLstd450InverseSqrt},
    {"Atan2", GLSLstd450Atan2},
    {"InterpolateAtCentroid", GLSLstd450InterpolateAtCentroid},
    {"UClamp", GLSLstd450UClamp},
    {"FMix", GLSLstd450FMix},
    {"FaceForward", GLSLstd450FaceForward},
    {"Tan", GLSLstd450Tan},
    {"Modf", GLSLstd450Modf},
    {"Exp", GLSLstd450Exp},
    {"PackSnorm4x8", GLSLstd450PackSnorm4x8},
    {"UnpackUnorm2x16", GLSLstd450UnpackUnorm2x16},
    {"UMax", GLSLstd450UMax},
    {"FSign", GLSLstd450FSign},
    {"Distance", GLSLstd450Distance},
    {"UnpackSnorm2x16", GLSLstd450UnpackSnorm2x16},
    {"Log", GLSLstd450Log},
    {"PackDouble2x32", GLSLstd450PackDouble2x32},
    {"Sinh", GLSLstd450Sinh},
    {"UnpackSnorm4x8", GLSLstd450UnpackSnorm4x8},
    {"Cross", GLSLstd450Cross},
    {"NMax", GLSLstd450NMax},
    {"Acosh", GLSLstd450Acosh},
    {"Reflect", GLSLstd450Reflect},
    {"Degrees", GLSLstd450Degrees},
    {"Acos", GLSLstd450Acos},
    {"Radians", GLSLstd450Radians},
    {"Sqrt", GLSLstd450Sqrt},
    {"Tanh", GLSLstd450Tanh},
    {"InterpolateAtOffset", GLSLstd450InterpolateAtOffset},
    {"Step", GLSLstd450Step},
    {"FAbs", GLSLstd450FAbs},
    {"FrexpStruct", GLSLstd450FrexpStruct},
    {"Log2", GLSLstd450Log2},
    {"SSign", GLSLstd450SSign},
    {"FMin", GLSLstd450FMin},
};

static UInt32 hash(const UnownedStringSlice& str, UInt32 salt)
{
    UInt64 h = salt;
    for(const char c : str)
        h = ((h * 0x00000100000001B3) ^ c);
    return h % (sizeof(tableSalt)/sizeof(tableSalt[0]));
}

bool lookupGLSLstd450(const UnownedStringSlice& str, GLSLstd450& value)
{
    const auto i = hash(str, tableSalt[hash(str, 0)]);
    if(str == words[i].name)
    {
        value = words[i].value;
        return true;
    }
    else
    {
        return false;
    }
}

}
