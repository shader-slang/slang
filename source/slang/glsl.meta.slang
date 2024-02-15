// TODO: These keywords are not recognized but they should be.
#define highp
#define mediump
#define lowp

#define VECTOR_MAP_UNARY(TYPE, COUNT, FUNC, VALUE) \
    vector<TYPE,COUNT> result; for(int i = 0; i < COUNT; ++i) { result[i] = FUNC(VALUE[i]); } return result

#define VECTOR_MAP_TRINARY(TYPE, COUNT, FUNC, A, B, C) \
    vector<TYPE,COUNT> result; for(int i = 0; i < COUNT; ++i) { result[i] = FUNC(A[i], B[i], C[i]); } return result

#define REQUIRE_KHRONOS [require(glsl)] [require(spirv)]

//
// OpenGL 4.60 spec
//

//
// Section 4.1. 'asic Types'
//

public typealias vec2 = vector<float, 2>;
public typealias vec3 = vector<float, 3>;
public typealias vec4 = vector<float, 4>;

public typealias dvec2 = vector<double, 2>;
public typealias dvec3 = vector<double, 3>;
public typealias dvec4 = vector<double, 4>;

public typealias bvec2 = vector<bool, 2>;
public typealias bvec3 = vector<bool, 3>;
public typealias bvec4 = vector<bool, 4>;

public typealias ivec2 = vector<int, 2>;
public typealias ivec3 = vector<int, 3>;
public typealias ivec4 = vector<int, 4>;

public typealias uvec2 = vector<uint, 2>;
public typealias uvec3 = vector<uint, 3>;
public typealias uvec4 = vector<uint, 4>;

public typealias i8vec2 = vector<int8_t, 2>;
public typealias i8vec3 = vector<int8_t, 3>;
public typealias i8vec4 = vector<int8_t, 4>;

public typealias u8vec2 = vector<uint8_t, 2>;
public typealias u8vec3 = vector<uint8_t, 3>;
public typealias u8vec4 = vector<uint8_t, 4>;

public typealias i16vec2 = vector<int16_t, 2>;
public typealias i16vec3 = vector<int16_t, 3>;
public typealias i16vec4 = vector<int16_t, 4>;

public typealias u16vec2 = vector<uint16_t, 2>;
public typealias u16vec3 = vector<uint16_t, 3>;
public typealias u16vec4 = vector<uint16_t, 4>;

public typealias i64vec2 = vector<int64_t, 2>;
public typealias i64vec3 = vector<int64_t, 3>;
public typealias i64vec4 = vector<int64_t, 4>;

public typealias u64vec2 = vector<uint64_t, 2>;
public typealias u64vec3 = vector<uint64_t, 3>;
public typealias u64vec4 = vector<uint64_t, 4>;

public typealias mat2 = matrix<float, 2, 2>;
public typealias mat3 = matrix<float, 3, 3>;
public typealias mat4 = matrix<float, 4, 4>;

public typealias mat2x2 = matrix<float, 2, 2>;
public typealias mat2x3 = matrix<float, 3, 2>;
public typealias mat2x4 = matrix<float, 4, 2>;

public typealias mat3x2 = matrix<float, 2, 3>;
public typealias mat3x3 = matrix<float, 3, 3>;
public typealias mat3x4 = matrix<float, 4, 3>;

public typealias mat4x2 = matrix<float, 2, 4>;
public typealias mat4x3 = matrix<float, 3, 4>;
public typealias mat4x4 = matrix<float, 4, 4>;

public typealias dmat2 = matrix<double, 2, 2>;
public typealias dmat3 = matrix<double, 3, 3>;
public typealias dmat4 = matrix<double, 4, 4>;

public typealias dmat2x2 = matrix<double, 2, 2>;
public typealias dmat2x3 = matrix<double, 3, 2>;
public typealias dmat2x4 = matrix<double, 4, 2>;

public typealias dmat3x2 = matrix<double, 2, 3>;
public typealias dmat3x3 = matrix<double, 3, 3>;
public typealias dmat3x4 = matrix<double, 4, 3>;

public typealias dmat4x2 = matrix<double, 2, 4>;
public typealias dmat4x3 = matrix<double, 3, 4>;
public typealias dmat4x4 = matrix<double, 4, 4>;


public out float4 gl_Position : SV_Position;
public out float gl_PointSize : SV_PointSize;
public in vec4 gl_FragCoord : SV_Position;
public out float gl_FragDepth : SV_Depth;
public out int gl_FragStencilRef : SV_StencilRef;

public in uvec3 gl_GlobalInvocationID : SV_DispatchThreadID;
public in uvec3 gl_WorkGroupID : SV_GroupID;
public in uvec3 gl_LocalInvocationIndex : SV_GroupIndex;
public in uvec3 gl_LocalInvocationID : SV_GroupThreadID;

// TODO: define overload for tessellation control stage.
public in int gl_InvocationID : SV_GSInstanceID;

public in int gl_InstanceIndex : SV_InstanceID;
public in bool gl_FrontFacing : SV_IsFrontFace;

// TODO: define overload for geometry stage.
public in int gl_Layer : SV_RenderTargetArrayIndex;

public in int gl_SampleID : SV_SampleIndex;
public in int gl_VertexIndex : SV_VertexID;
public in int gl_ViewIndex : SV_ViewID;
public in int gl_ViewportIndex : SV_ViewportArrayIndex;


// Override operator* behavior to compute algebric product of matrices and vectors.

[OverloadRank(15)]
[ForceInline]
public matrix<float, N, N> operator*<let N:int>(matrix<float, N, N> m1, matrix<float, N, N> m2)
{
    return mul(m2, m1);
}

[OverloadRank(15)]
[ForceInline]
public matrix<half, N, N> operator*<let N:int>(matrix<half, N, N> m1, matrix<half, N, N> m2)
{
    return mul(m2, m1);
}

[OverloadRank(15)]
[ForceInline]
public matrix<double, N, N> operator*<let N:int>(matrix<double, N, N> m1, matrix<double, N, N> m2)
{
    return mul(m2, m1);
}

[ForceInline]
[OverloadRank(15)]
public matrix<T, R, L> operator*<T:__BuiltinFloatingPointType, let L : int, let C : int, let R : int>(matrix<T, C, L> m1, matrix<T, R, C> m2)
{
    return mul(m2, m1);
}

[ForceInline]
[OverloadRank(15)]
public vector<T, R> operator*<T:__BuiltinFloatingPointType, let C : int, let R : int>(vector<T, C> v, matrix<T, R, C> m)
{
    return mul(m, v);
}

[ForceInline]
[OverloadRank(15)]
public vector<T, C> operator*<T:__BuiltinFloatingPointType, let C : int, let R : int>(matrix<T, R, C> m, vector<T, R> v)
{
    return mul(v, m);
}

__intrinsic_op(mul)
public matrix<T, N, M> matrixCompMult<T:__BuiltinFloatingPointType, let N:int, let M : int>(matrix<T,N,M> left, matrix<T,N,M> right);

__intrinsic_op(cmpLE)
public vector<bool, N> lessThanEqual<T, let N:int>(vector<T, N> x, vector<T, N> y);
__intrinsic_op(cmpLT)
public vector<bool, N> lessThan<T, let N:int>(vector<T, N> x, vector<T, N> y);
__intrinsic_op(cmpGT)
public vector<bool, N> greaterThan<T, let N:int>(vector<T, N> x, vector<T, N> y);
__intrinsic_op(cmpGE)
public vector<bool, N> greaterThanEqual<T, let N:int>(vector<T, N> x, vector<T, N> y);
__intrinsic_op(cmpEQ)
public vector<bool, N> equal<T, let N:int>(vector<T, N> x, vector<T, N> y);
__intrinsic_op(cmpNE)
public vector<bool, N> notEqual<T, let N:int>(vector<T, N> x, vector<T, N> y);

__generic<T>
public extension vector<T, 2>
{
    [ForceInline] public __init(vector<T, 3> bigger) { this = bigger.xy; }
    [ForceInline] public __init(vector<T, 4> bigger) { this = bigger.xy; }
}

__generic<T>
public extension vector<T, 3>
{
    [ForceInline] public __init(vector<T, 4> bigger) { this = bigger.xyz; }
}

[ForceInline]
[OverloadRank(15)]
public bool operator==<T:__BuiltinArithmeticType, let N:int>(vector<T, N> left, vector<T, N> right)
{
    return all(equal(left, right));
}

[ForceInline]
[OverloadRank(15)]
public bool operator!=<T:__BuiltinArithmeticType, let N:int>(vector<T, N> left, vector<T, N> right)
{
    return any(notEqual(left, right));
}

[ForceInline]
[OverloadRank(14)]
public bool operator==<T:__BuiltinFloatingPointType, let N:int>(vector<T, N> left, vector<T, N> right)
{
    return all(equal(left, right));
}

[ForceInline]
[OverloadRank(14)]
public bool operator!=<T:__BuiltinFloatingPointType, let N:int>(vector<T, N> left, vector<T, N> right)
{
    return any(notEqual(left, right));
}

[ForceInline]
[OverloadRank(14)]
public bool operator==<T:__BuiltinLogicalType, let N:int>(vector<T, N> left, vector<T, N> right)
{
    return all(equal(left, right));
}

[ForceInline]
[OverloadRank(14)]
public bool operator!=<T:__BuiltinLogicalType, let N:int>(vector<T, N> left, vector<T, N> right)
{
    return any(notEqual(left, right));
}

${{{{
for (auto type : kBaseTypes) {
    char const* typeName = type.name;
    if (!type.flags) continue;
}}}}
[ForceInline]
[OverloadRank(15)]
public bool operator==<let N:int>(vector<$(typeName), N> left, vector<$(typeName), N> right)
{
    return all(equal(left, right));
}

[ForceInline]
[OverloadRank(15)]
public bool operator!=<let N:int>(vector<$(typeName), N> left, vector<$(typeName), N> right)
{
    return any(notEqual(left, right));
}
${{{{
}
}}}}

//
// Section 8.1. Angle and Trigonometry Functions
//

__generic<T : __BuiltinFloatingPointType>
[__readNone]
[ForceInline]
public T atan(T y, T x)
{
    return atan2(y, x);
}

__generic<T : __BuiltinFloatingPointType, let N:int>
[__readNone]
[ForceInline]
public vector<T,N> atan(vector<T,N> y, vector<T,N> x)
{
    return atan2(y, x);
}

__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(cuda, "$P_asinh($0)")
__target_intrinsic(cpp, "$P_asinh($0)")
[__readNone]
[ForceInline]
public T asinh(T x)
{
    return log(x + sqrt(x * x + T(1)));
}

__generic<T : __BuiltinFloatingPointType, let N:int>
[__readNone]
[ForceInline]
public vector<T,N> asinh(vector<T,N> x)
{
    VECTOR_MAP_UNARY(T, N, asinh, x);
}

__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(cuda, "$P_acosh($0)")
__target_intrinsic(cpp, "$P_acosh($0)")
[__readNone]
[ForceInline]
public T acosh(T x)
{
    return log(x + sqrt( x * x - T(1)));
}

__generic<T : __BuiltinFloatingPointType, let N:int>
[__readNone]
[ForceInline]
public vector<T,N> acosh(vector<T,N> x)
{
    VECTOR_MAP_UNARY(T, N, acosh, x);
}

__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(cuda, "$P_atanh($0)")
__target_intrinsic(cpp, "$P_atanh($0)")
[__readNone]
[ForceInline]
public T atanh(T x)
{
    return T(0.5) * log((T(1) + x) / (T(1) - x));
}

__generic<T : __BuiltinFloatingPointType, let N:int>
[__readNone]
[ForceInline]
public vector<T,N> atanh(vector<T,N> x)
{
    VECTOR_MAP_UNARY(T, N, atanh, x);
}

//
// Section 8.2. Exponential Functions
//

__generic<T : __BuiltinFloatingPointType>
[__readNone]
[ForceInline]
public T inversesqrt(T x)
{
    return rsqrt(x);
}

__generic<T : __BuiltinFloatingPointType, let N:int>
[__readNone]
[ForceInline]
public vector<T, N> inversesqrt(vector<T, N> x)
{
    return rsqrt(x);
}

//
// Section 8.3. Common Functions
//

__generic<T : __BuiltinFloatingPointType>
[__readNone]
[ForceInline]
public T roundEven(T x)
{
    T i;
    if (T(0.5) <= fmod(x, i))
    {
        bool evenInteger = (fmod(i, T(2)) == T(0));
        if (!evenInteger)
        {
            x += T(0.1);
        }
    }
    return round(x);
}

__generic<T : __BuiltinFloatingPointType, let N:int>
[__readNone]
[ForceInline]
public vector<T,N> roundEven(vector<T,N> x)
{
    VECTOR_MAP_UNARY(T, N, roundEven, x);
}

__generic<T : __BuiltinFloatingPointType>
[__readNone]
[ForceInline]
public T fract(T x)
{
    return frac(x);
}

__generic<T : __BuiltinFloatingPointType, let N:int>
[__readNone]
[ForceInline]
public vector<T, N> fract(vector<T, N> x)
{
    return frac(x);
}

__generic<T : __BuiltinFloatingPointType>
[__readNone]
[ForceInline]
public T mod(T x, T y)
{
    return fmod(x, y);
}

__generic<T : __BuiltinFloatingPointType, let N:int>
[__readNone]
[ForceInline]
public vector<T, N> mod(vector<T, N> x, T y)
{
    return fmod(x, vector<T, N>(y));
}

__generic<T : __BuiltinFloatingPointType, let N:int>
[__readNone]
[ForceInline]
public vector<T, N> mod(vector<T, N> x, vector<T, N> y)
{
    return fmod(x, y);
}

__generic<T : __BuiltinFloatingPointType>
[__readNone]
[ForceInline]
public T mix(T x, T y, T a)
{
    return lerp(x, y, a);
}

__generic<T : __BuiltinFloatingPointType, let N:int>
[__readNone]
[ForceInline]
public vector<T, N> mix(vector<T, N> x, vector<T, N> y, T a)
{
    return lerp(x, y, vector<T, N>(a));
}

__generic<T : __BuiltinFloatingPointType, let N:int>
[__readNone]
[ForceInline]
public vector<T, N> mix(vector<T, N> x, vector<T, N> y, vector<T, N> a)
{
    return lerp(x, y, a);
}

__generic<T>
[__readNone]
[ForceInline]
public T mix(T x, T y, bool a)
{
    return (a ? y : x);
}

__generic<T, let N:int>
[__readNone]
[ForceInline]
public vector<T, N> mix(vector<T, N> x, vector<T, N> y, vector<bool, N> a)
{
    vector<T, N> result;
    for (int i = 0; i < N; i++)
    {
        result[i] = (a[i] ? y[i] : x[i]);
    }
    return result;
}

[__readNone]
[ForceInline]
public int floatBitsToInt(highp float x)
{
    return asint(x);
}

__generic<let N:int>
[__readNone]
[ForceInline]
public vector<int, N> floatBitsToInt(highp vector<float, N> x)
{
    return asint(x);
}

[__readNone]
[ForceInline]
public uint floatBitsToUint(highp float x)
{
    return asuint(x);
}

__generic<let N:int>
[__readNone]
[ForceInline]
public vector<uint, N> floatBitsToUint(highp vector<float, N> x)
{
    return asuint(x);
}

[__readNone]
[ForceInline]
public float intBitsToFloat(highp int x)
{
    return asfloat(x);
}

__generic<let N:int>
[__readNone]
[ForceInline]
public vector<float, N> intBitsToFloat(highp vector<int, N> x)
{
    return asfloat(x);
}

[__readNone]
[ForceInline]
public float uintBitsToFloat(highp uint x)
{
    return asfloat(x);
}

__generic<let N:int>
[__readNone]
[ForceInline]
public vector<float, N> uintBitsToFloat(highp vector<uint, N> x)
{
    return asfloat(x);
}

//
// Section 8.4. Floating-Point Pack and Unpack Functions
//

[__readNone]
[ForceInline]
uint packUnorm1x16(float c)
{
    return uint(clamp(c, 0.0, 1.0) * 65535.0 + 0.5);
}

[__readNone]
[ForceInline]
uint packSnorm1x16(float v)
{
    return uint(clamp(v ,-1.0, 1.0) * 32767.0 + 32767.5);
}

[__readNone]
[ForceInline]
uint packUnorm1x8(float c)
{
    return uint(clamp(c, 0.0, 1.0) * 255.0 + 0.5);
}

[__readNone]
[ForceInline]
uint packSnorm1x8(float c)
{
    return uint(clamp(c, -1.0, 1.0) * 127.0 + 127.5);
}

[__readNone]
[ForceInline]
float unpackUnorm1x16(uint p)
{
    return float(p) / 65535.0;
}

[__readNone]
[ForceInline]
float unpackSnorm1x16(uint p)
{
    return clamp((float(p) - 32767.0) / 32767.0, -1.0, 1.0);
}

[__readNone]
[ForceInline]
float unpackUnorm1x8(uint p)
{
    return float(p) / 255.0;
}

[__readNone]
[ForceInline]
float unpackSnorm1x8(uint p)
{
    return clamp((float(p) - 127.0) / 127.0, -1.0, 1.0);
}

[__readNone]
[ForceInline]
uint float2half(float f)
{
    uint u = floatBitsToUint(f);
    uint s = ((u >> uint(16)) & uint(0x8000));
    uint e = 0;
    uint m = ((u >> uint(13)) & uint(0x03ff));
    if (m != 0)
    {
        e = ((((u & uint(0x7f800000)) - uint(0x38000000)) >> uint(13)) & uint(0x7c00));
    }
    return (s | e | m);
}

__target_intrinsic(glsl)
[__readNone]
[ForceInline]
public uint packUnorm2x16(vec2 v)
{
    return packUnorm1x16(v.x) | (packUnorm1x16(v.y) << uint(16));
}

__target_intrinsic(glsl)
[__readNone]
[ForceInline]
public uint packSnorm2x16(vec2 v)
{
    return packSnorm1x16(v.x) | (packSnorm1x16(v.y) << uint(16));
}

__target_intrinsic(glsl)
[__readNone]
[ForceInline]
public uint packUnorm4x8(vec4 v)
{
    return packUnorm1x8(v.x) | (packUnorm1x8(v.y) << uint(8)) | (packUnorm1x8(v.z) << uint(16)) | (packUnorm1x8(v.w) << uint(24));
}

__target_intrinsic(glsl)
[__readNone]
[ForceInline]
public uint packSnorm4x8(vec4 v)
{
    return packSnorm1x8(v.x) | (packSnorm1x8(v.y) << uint(8)) | (packSnorm1x8(v.z) << uint(16)) | (packSnorm1x8(v.w) << uint(24));
}

__target_intrinsic(glsl)
[__readNone]
[ForceInline]
public vec2 unpackUnorm2x16(uint p)
{
    return vec2(unpackUnorm1x16(p & uint(0xffff)), unpackUnorm1x16(p >> uint(16)));
}

__target_intrinsic(glsl)
[__readNone]
[ForceInline]
public vec2 unpackSnorm2x16(uint p)
{
    return vec2(unpackSnorm1x16(p & uint(0xffff)), unpackSnorm1x16(p >> uint(16)));
}

__target_intrinsic(glsl)
[__readNone]
[ForceInline]
public vec4 unpackUnorm4x8(highp uint p)
{
    return vec4(unpackUnorm1x8(p & uint(0xffff)), unpackUnorm1x8(p >> uint(8)), unpackUnorm1x8(p >> uint(16)), unpackUnorm1x8(p >> uint(24)));
}

__target_intrinsic(glsl)
[__readNone]
[ForceInline]
public vec4 unpackSnorm4x8(highp uint p)
{
    return vec4(unpackSnorm1x8(p & uint(0xffff)), unpackSnorm1x8(p >> uint(8)), unpackSnorm1x8(p >> uint(16)), unpackSnorm1x8(p >> uint(24)));
}

__target_intrinsic(glsl)
[__readNone]
[ForceInline]
public uint packHalf2x16(vec2 v)
{
    return float2half(v.x) | (float2half(v.y) << uint(16));
}

__target_intrinsic(glsl)
[__readNone]
[ForceInline]
public float half2float(uint h)
{
    uint s = ((h & uint(0x8000)) << uint(16));
    uint e = 0;
    uint m = ((h & uint(0x03ff)) << uint(13));
    if (m != 0)
    {
        e = (((h & uint(0x7c00)) + uint(0x1c000)) << uint(13));
    }
    return uintBitsToFloat(s | e | m); 
}

__target_intrinsic(glsl)
[__readNone]
[ForceInline]
public vec2 unpackHalf2x16(uint p)
{
    return vec2(half2float(p & uint(0xffff)), half2float(p >> uint(16)));
}

__target_intrinsic(glsl)
[__readNone]
[ForceInline]
public double packDouble2x32(uvec2 v)
{
    // TODO: there is no "asdouble()"
    //return asdouble(uint64_t(v.x) | (uint64_t(v.y) << 32));
    return 0.0;
}

__target_intrinsic(glsl)
[__readNone]
[ForceInline]
public uvec2 unpackDouble2x32(double v)
{
    // TODO: there is no "asuint64()"
    uint64_t u = 0; // asuint64(v);
    return uvec2(uint(u & 0xFFFFFFFF), uint(u >> 32));
}

//
// Section 8.5. Geometric Functions
//

__generic<T : __BuiltinFloatingPointType>
[__readNone]
[ForceInline]
public T faceforward(T n, T i, T ng)
{
    return dot(ng, i) < T(0.0f) ? n : -n;
}

//
// Section 8.6. Matrix Functions
//

__generic<T : __BuiltinFloatingPointType, let C : int, let R : int>
__target_intrinsic(glsl)
[__readNone]
[ForceInline]
[OverloadRank(15)]
public matrix<T, C, R> outerProduct(vector<T, C> c, vector<T, R> r)
{
    // Column major matrix in GLSL
    matrix<T, C, R> result;
    for (int i = 0; i < C; ++i)
    {
        for (int j = 0; j < R; ++j)
        {
            result[i][j] = c[i] * r[j];
        }
    }
    return result;
}

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
matrix<T,N,N> inverse(matrix<T,N,N> m);

//
// Section 8.8. Integer Functions
//

[__readNone]
[ForceInline]
public uint uaddCarry(highp uint x, highp uint y, out lowp uint carry)
{
    let result = x * y;
    carry = ((result < x || result < y) ? 1 : 0);
    return result;
}

__generic<let N:int>
[__readNone]
[ForceInline]
public vector<uint,N> uaddCarry(highp vector<uint,N> x, highp vector<uint,N> y, out lowp vector<uint,N> carry)
{
    VECTOR_MAP_TRINARY(uint, N, uaddCarry, x, y, carry);
}

[__readNone]
[ForceInline]
public uint usubBorrow(highp uint x, highp uint y, out lowp uint borrow)
{
    borrow = (y > x) ? 1 : 0;
    return x - y;
}

__generic<let N:int>
[__readNone]
[ForceInline]
public vector<uint,N> usubBorrow(highp vector<uint,N> x, highp vector<uint,N> y, out lowp vector<uint,N> borrow)
{
    VECTOR_MAP_TRINARY(uint, N, usubBorrow, x, y, borrow);
}

[__readNone]
[ForceInline]
public void umulExtended(highp uint x, highp uint y, out highp uint msb, out highp uint lsb)
{
    uint64_t result = x * y;
    msb = uint(result >> 32);
    lsb = uint(result);
}

__generic<let N:int>
[__readNone]
[ForceInline]
public void umulExtended(highp vector<uint,N> x, highp vector<uint,N> y, out highp vector<uint,N> msb, out highp vector<uint,N> lsb)
{
    for(int i = 0; i < N; ++i)
    {
       umulExtended(x[i], y[i], msb[i], lsb[i]);
    }
}

[__readNone]
[ForceInline]
public void imulExtended(highp int x, highp int y, out highp int msb, out highp int lsb)
{
    int64_t result = x * y;
    msb = int(result >> 32);
    lsb = int(result);
}

__generic<let N:int>
[__readNone]
[ForceInline]
public void imulExtended(highp vector<int,N> x, highp vector<int,N> y, out highp vector<int,N> msb, out highp vector<int,N> lsb)
{
    for(int i = 0; i < N; ++i)
    {
       imulExtended(x[i], y[i], msb[i], lsb[i]);
    }
}

[__readNone]
[ForceInline]
public int bitfieldExtract(int value, int offset, int bits)
{
    return int(uint(value >> offset) & ((1u << bits) - 1));
}

__generic<let N:int>
[__readNone]
[ForceInline]
public vector<int,N> bitfieldExtract(vector<int,N> value, int offset, int bits)
{
    vector<int,N> result;
    for (int i = 0; i < N; ++i)
    {
        result[i] = bitfieldExtract(value[i], offset, bits);
    }
    return result;
}

[__readNone]
[ForceInline]
public uint bitfieldExtract(uint value, int offset, int bits)
{
    return (value >> offset) & ((1u << bits) - 1);
}

__generic<let N:int>
[__readNone]
[ForceInline]
public vector<uint,N> bitfieldExtract(vector<uint,N> value, int offset, int bits)
{
    vector<uint,N> result;
    for (int i = 0; i < N; ++i)
    {
        result[i] = bitfieldExtract(value[i], offset, bits);
    }
    return result;
}

[__readNone]
[ForceInline]
public uint bitfieldInsert(uint base, uint insert, int offset, int bits)
{
    uint clearMask = ~(((1u << bits) - 1u) << offset);
    uint clearedBase = base & clearMask;
    uint maskedInsert = (insert & ((1u << bits) - 1u)) << offset;
    return clearedBase | maskedInsert;
}

__generic<let N:int>
[__readNone]
[ForceInline]
public vector<uint,N> bitfieldInsert(vector<uint,N> base, vector<uint,N> insert, int offset, int bits)
{
    vector<uint,N> result;
    for (int i = 0; i < N; ++i)
    {
        result[i] = bitfieldInsert(base[i], insert[i], offset, bits);
    }
    return result;
}

[__readNone]
[ForceInline]
public int bitfieldInsert(int base, int insert, int offset, int bits)
{
    uint clearMask = ~(((1u << bits) - 1u) << offset);
    uint clearedBase = base & clearMask;
    uint maskedInsert = (insert & ((1u << bits) - 1u)) << offset;
    return clearedBase | maskedInsert;
}

__generic<let N:int>
[__readNone]
[ForceInline]
public vector<int,N> bitfieldInsert(vector<int,N> base, vector<int,N> insert, int offset, int bits)
{
    vector<int,N> result;
    for (int i = 0; i < N; ++i)
    {
        result[i] = bitfieldInsert(base[i], insert[i], offset, bits);
    }
    return result;
}

[__readNone]
[ForceInline]
public int bitfieldReverse(highp int value)
{
    value = ((value & 0xAAAAAAAA) >> 1) | ((value & 0x55555555) << 1);
    value = ((value & 0xCCCCCCCC) >> 2) | ((value & 0x33333333) << 2);
    value = ((value & 0xF0F0F0F0) >> 4) | ((value & 0x0F0F0F0F) << 4);
    value = ((value & 0xFF00FF00) >> 8) | ((value & 0x00FF00FF) << 8);
    value = ((value & 0xFFFF0000) >> 16) | ((value & 0x0000FFFF) << 16);
    return value;
}

__generic<let N:int>
[__readNone]
[ForceInline]
public vector<int,N> bitfieldReverse(highp vector<int,N> value)
{
    VECTOR_MAP_UNARY(int, N, bitfieldReverse, value);
}

[__readNone]
[ForceInline]
public uint bitfieldReverse(highp uint value)
{
    value = ((value & 0xAAAAAAAA) >> 1) | ((value & 0x55555555) << 1);
    value = ((value & 0xCCCCCCCC) >> 2) | ((value & 0x33333333) << 2);
    value = ((value & 0xF0F0F0F0) >> 4) | ((value & 0x0F0F0F0F) << 4);
    value = ((value & 0xFF00FF00) >> 8) | ((value & 0x00FF00FF) << 8);
    value = ((value & 0xFFFF0000) >> 16) | ((value & 0x0000FFFF) << 16);
    return value;
}

__generic<let N:int>
[__readNone]
[ForceInline]
public vector<uint,N> bitfieldReverse(highp vector<uint,N> value)
{
    VECTOR_MAP_UNARY(int, N, bitfieldReverse, value);
}

[__readNone] [ForceInline] REQUIRE_KHRONOS
public uint bitCount(uint value)
{
    return countbits(value);
}

__generic<let N:int>
[__readNone] [ForceInline] REQUIRE_KHRONOS
public vector<uint,N> bitCount(vector<uint,N> value)
{
    VECTOR_MAP_UNARY(uint, N, countbits, value);
}

[__readNone] [ForceInline] REQUIRE_KHRONOS
public int bitCount(int value)
{
    return countbits(uint(value));
}
    
__generic<let N:int>
[__readNone] [ForceInline] REQUIRE_KHRONOS
public vector<int,N> bitCount(vector<int,N> value)
{
    VECTOR_MAP_UNARY(int, N, countbits, value);
}

[__readNone]
[ForceInline]
public int findLSB(int v)
{
    return firstbitlow(v);
}

__generic<let N:int>
[__readNone]
[ForceInline]
public vector<int,N> findLSB(vector<int,N> value)
{
    return firstbitlow(value);
}

[__readNone]
[ForceInline]
public uint findLSB(uint v)
{
    return firstbitlow(v);
}

__generic<let N:int>
[__readNone]
[ForceInline]
public vector<uint,N> findLSB(vector<uint,N> value)
{
    return firstbitlow(value);
}

[__readNone]
[ForceInline]
public int findMSB(int value)
{
    return firstbithigh(value);
}

__generic<let N:int>
[__readNone]
[ForceInline]
public vector<int,N> findMSB(vector<int,N> value)
{
    return firstbithigh(value);
}

[__readNone]
[ForceInline]
public uint findMSB(uint value)
{
    return firstbithigh(value);
}

__generic<let N:int>
[__readNone]
[ForceInline]
public vector<uint,N> findMSB(vector<uint,N> value)
{
    return firstbithigh(value);
}

__generic<let N:int>
[__readNone]
[ForceInline]
public vector<bool,N> not(vector<bool,N> x)
{
    return !x;
}

//
// Section 8.9.1. Texture Query Functions
//

public typealias usampler1D = Sampler1D<uint4>;
public typealias isampler1D = Sampler1D<int4>;
public typealias sampler1D = Sampler1D<float4>;

public typealias usampler2D = Sampler2D<uint4>;
public typealias isampler2D = Sampler2D<int4>;
public typealias sampler2D = Sampler2D<float4>;

public typealias usampler3D = Sampler3D<uint4>;
public typealias isampler3D = Sampler3D<int4>;
public typealias sampler3D = Sampler3D<float4>;

public typealias usamplerCube = SamplerCube<uint4>;
public typealias isamplerCube = SamplerCube<int4>;
public typealias samplerCube = SamplerCube<float4>;

__generic<let sampleCount:int=0, let format:int=0>
public typealias sampler1DShadow = __TextureImpl<
    float,
    __Shape1D,
    0, // isArray
    0, // isMS
    sampleCount,
    0, // access
    1, // isShadow
    1, // isCombined
    format
>;

__generic<let sampleCount:int=0, let format:int=0>
public typealias sampler2DShadow = __TextureImpl<
    float,
    __Shape2D,
    0, // isArray
    0, // isMS
    sampleCount,
    0, // access
    1, // isShadow
    1, // isCombined
    format
>;

__generic<let sampleCount:int=0, let format:int=0>
public typealias samplerCubeShadow = __TextureImpl<
    float,
    __ShapeCube,
    0, // isArray
    0, // isMS
    sampleCount,
    0, // access
    1, // isShadow
    1, // isCombined
    format
>;

public typealias usampler1DArray = Sampler1DArray<uint4>;
public typealias isampler1DArray = Sampler1DArray<int4>;
public typealias sampler1DArray = Sampler1DArray<float4>;

public typealias usampler2DArray = Sampler2DArray<uint4>;
public typealias isampler2DArray = Sampler2DArray<int4>;
public typealias sampler2DArray = Sampler2DArray<float4>;

public typealias usamplerCubeArray = SamplerCubeArray<uint4>;
public typealias isamplerCubeArray = SamplerCubeArray<int4>;
public typealias samplerCubeArray = SamplerCubeArray<float4>;

__generic<let sampleCount:int=0, let format:int=0>
public typealias sampler1DArrayShadow = __TextureImpl<
    float,
    __Shape1D,
    1, // isArray
    0, // isMS
    sampleCount,
    0, // access
    1, // isShadow
    1, // isCombined
    format
>;

__generic<let sampleCount:int=0, let format:int=0>
public typealias sampler2DArrayShadow = __TextureImpl<
    float,
    __Shape2D,
    1, // isArray
    0, // isMS
    sampleCount,
    0, // access
    1, // isShadow
    1, // isCombined
    format
>;

__generic<let sampleCount:int=0, let format:int=0>
public typealias samplerCubeArrayShadow = __TextureImpl<
    float,
    __ShapeCube,
    1, // isArray
    0, // isMS
    sampleCount,
    0, // access
    1, // isShadow
    1, // isCombined
    format
>;

public typealias sampler2DMS = Sampler2DMS<float4>;
public typealias isampler2DMS = Sampler2DMS<int4>;
public typealias usampler2DMS = Sampler2DMS<uint4>;

__generic<T=float4, let sampleCount:int=0, let format:int=0>
public typealias Sampler2DMSArray = Sampler2DArrayMS<T, sampleCount, format>;
public typealias sampler2DMSArray = Sampler2DMSArray<float4>;
public typealias isampler2DMSArray = Sampler2DMSArray<int4>;
public typealias usampler2DMSArray = Sampler2DMSArray<uint4>;

__generic<T=float4, let sampleCount:int=0, let format:int=0>
public typealias Sampler2DRect = __TextureImpl<T, __Shape2D, 0, 0, sampleCount, 0, 0, 1, format>;
public typealias sampler2DRect = Sampler2DRect<float4>;
public typealias isampler2DRect = Sampler2DRect<int4>;
public typealias usampler2DRect = Sampler2DRect<uint4>;

__generic<let sampleCount:int=0, let format:int=0>
public typealias sampler2DRectShadow = __TextureImpl<
    float,
    __Shape2D,
    0, // isArray
    0, // isMS
    sampleCount,
    0, // access
    1, // isShadow
    1, // isCombined
    format
>;

__generic<T, let format:int=0>
public typealias SamplerBuffer = __TextureImpl<
    T,
    __ShapeBuffer,
    0, // isArray
    0, // isMS
    0, // sampleCount
    1, // RW
    0, // isShadow
    0, // isCombined
    format
>;
public typealias samplerBuffer = SamplerBuffer<vec4>;
public typealias isamplerBuffer = SamplerBuffer<int4>;
public typealias usamplerBuffer = SamplerBuffer<uint4>;


// -------------------
// textureSize
// -------------------

__generic<T:__BuiltinArithmeticType, let N:int>
[ForceInline]
public int textureSize(Sampler1D<vector<T,N>> sampler, int lod)
{
    int result;
    int numberOfLevels;
    sampler.GetDimensions(lod, result, numberOfLevels);
    return result;
}

__generic<T:__BuiltinArithmeticType, let N:int>
[ForceInline]
public ivec2 textureSize(Sampler2D<vector<T,N>> sampler, int lod)
{
    vector<int,2> result;
    int numberOfLevels;
    sampler.GetDimensions(lod, result.x, result.y, numberOfLevels);
    return result;
}

__generic<T:__BuiltinArithmeticType, let N:int>
[ForceInline]
public ivec3 textureSize(Sampler3D<vector<T,N>> sampler, int lod)
{
    vector<int,3> result;
    int numberOfLevels;
    sampler.GetDimensions(lod, result.x, result.y, result.z, numberOfLevels);
    return result;
}

__generic<T:__BuiltinArithmeticType, let N:int>
[ForceInline]
public ivec2 textureSize(SamplerCube<vector<T,N>> sampler, int lod)
{
    vector<int,2> result;
    int numberOfLevels;
    sampler.GetDimensions(lod, result.x, result.y, numberOfLevels);
    return result;
}

[ForceInline]
public int textureSize(sampler1DShadow sampler, int lod)
{
    int result;
    int numberOfLevels;
    sampler.GetDimensions(lod, result, numberOfLevels);
    return result;
}

[ForceInline]
public ivec2 textureSize(sampler2DShadow sampler, int lod)
{
    vector<int,2> result;
    int numberOfLevels;
    sampler.GetDimensions(lod, result.x, result.y, numberOfLevels);
    return result;
}

[ForceInline]
public ivec2 textureSize(samplerCubeShadow sampler, int lod)
{
    vector<int,2> result;
    int numberOfLevels;
    sampler.GetDimensions(lod, result.x, result.y, numberOfLevels);
    return result;
}

__generic<T:__BuiltinArithmeticType, let N:int>
[ForceInline]
public ivec3 textureSize(SamplerCubeArray<vector<T,N>> sampler, int lod)
{
    vector<int,3> result;
    int numberOfLevels;
    sampler.GetDimensions(lod, result.x, result.y, result.z, numberOfLevels);
    return result;
}

[ForceInline]
public ivec3 textureSize(samplerCubeArrayShadow sampler, int lod)
{
    vector<int,3> result;
    int numberOfLevels;
    sampler.GetDimensions(lod, result.x, result.y, result.z, numberOfLevels);
    return result;
}

__generic<T:__BuiltinArithmeticType, let N:int>
[ForceInline]
public ivec2 textureSize(Sampler2DRect<vector<T,N>> sampler)
{
    vector<int,2> result;
    int numberOfLevels;
    sampler.GetDimensions(0, result.x, result.y, numberOfLevels);
    return result;
}

[ForceInline]
public ivec2 textureSize(sampler2DRectShadow sampler)
{
    vector<int,2> result;
    int numberOfLevels;
    sampler.GetDimensions(result.x, result.y);
    return result;
}

__generic<T:__BuiltinArithmeticType, let N:int>
[ForceInline]
public ivec2 textureSize(Sampler1DArray<vector<T,N>> sampler, int lod)
{
    vector<int,2> result;
    int numberOfLevels;
    sampler.GetDimensions(lod, result.x, result.y, numberOfLevels);
    return result;
}

[ForceInline]
public ivec2 textureSize(sampler1DArrayShadow sampler, int lod)
{
    vector<int,2> result;
    int numberOfLevels;
    sampler.GetDimensions(lod, result.x, result.y, numberOfLevels);
    return result;
}

__generic<T:__BuiltinArithmeticType, let N:int>
[ForceInline]
public ivec3 textureSize(Sampler2DArray<vector<T,N>> sampler, int lod)
{
    vector<int,3> result;
    int numberOfLevels;
    sampler.GetDimensions(lod, result.x, result.y, result.z, numberOfLevels);
    return result;
}

[ForceInline]
public ivec3 textureSize(sampler2DArrayShadow sampler, int lod)
{
    vector<int,3> result;
    int numberOfLevels;
    sampler.GetDimensions(lod, result.x, result.y, result.z, numberOfLevels);
    return result;
}

__generic<T:__BuiltinArithmeticType, let N:int, let format:int>
[ForceInline]
public int textureSize(SamplerBuffer<vector<T,N>,format> sampler)
{
    uint result;
    sampler.GetDimensions(result);
    return int(result);
}

__generic<T:__BuiltinArithmeticType, let N:int, let sampleCount:int>
[ForceInline]
public ivec2 textureSize(Sampler2DMS<vector<T,N>,sampleCount> sampler)
{
    vector<int,2> result;
    int sampleCount;
    int numberOfLevels;
    sampler.GetDimensions(result.x, result.y, sampleCount);
    return result;
}

__generic<T:__BuiltinArithmeticType, let N:int, let sampleCount:int>
[ForceInline]
public ivec3 textureSize(Sampler2DMSArray<vector<T,N>,sampleCount> sampler)
{
    vector<int,3> result;
    int sampleCount;
    int numberOfLevels;
    sampler.GetDimensions(result.x, result.y, result.z, sampleCount);
    return result;
}

// -------------------
// textureQueryLod
// -------------------

__generic<T, let isArray:int, let sampleCount:int, let isShadow:int, let format:int>
[ForceInline]
public vec2 textureQueryLod(__TextureImpl<
        T,
        __Shape1D,
        isArray,
        0, // isMS
        sampleCount,
        0, // access
        isShadow,
        1, // isCombined
        format
    > sampler, float p)
{
    return vec2(
        sampler.CalculateLevelOfDetail(p),
        sampler.CalculateLevelOfDetailUnclamped(p)
        );
}

__generic<T, Shape: __ITextureShape, let isArray:int, let sampleCount:int, let isShadow:int, let format:int>
[ForceInline]
public vec2 textureQueryLod(__TextureImpl<T,
        Shape,
        isArray,
        0, // isMS
        sampleCount,
        0, // access
        isShadow,
        1, // isCombined
        format
    > sampler, vector<float,Shape.dimensions> p)
{
    return vec2(
        sampler.CalculateLevelOfDetail(p),
        sampler.CalculateLevelOfDetailUnclamped(p)
        );
}

// -------------------
// textureQueryLevels
// -------------------

__generic<T:__BuiltinArithmeticType, let N:int>
[ForceInline]
public int textureQueryLevels(Sampler1D<vector<T,N>> sampler)
{
    int width;
    int numberOfLevels;
    sampler.GetDimensions(0, width, numberOfLevels);
    return numberOfLevels;
}

__generic<T:__BuiltinArithmeticType, let N:int>
[ForceInline]
public int textureQueryLevels(Sampler2D<vector<T,N>> sampler)
{
    vector<int,2> dim;
    int numberOfLevels;
    sampler.GetDimensions(0, dim.x, dim.y, numberOfLevels);
    return numberOfLevels;
}

__generic<T:__BuiltinArithmeticType, let N:int>
[ForceInline]
public int textureQueryLevels(Sampler3D<vector<T,N>> sampler)
{
    vector<int,3> dim;
    int numberOfLevels;
    sampler.GetDimensions(0, dim.x, dim.y, dim.z, numberOfLevels);
    return numberOfLevels;
}

__generic<T:__BuiltinArithmeticType, let N:int>
[ForceInline]
public int textureQueryLevels(SamplerCube<vector<T,N>> sampler)
{
    vector<int,2> dim;
    int numberOfLevels;
    sampler.GetDimensions(0, dim.x, dim.y, numberOfLevels);
    return numberOfLevels;
}

__generic<T:__BuiltinArithmeticType, let N:int>
[ForceInline]
public int textureQueryLevels(Sampler1DArray<vector<T,N>> sampler)
{
    vector<int,2> dim;
    int numberOfLevels;
    sampler.GetDimensions(0, dim.x, dim.y, numberOfLevels);
    return numberOfLevels;
}

__generic<T:__BuiltinArithmeticType, let N:int>
[ForceInline]
public int textureQueryLevels(Sampler2DArray<vector<T,N>> sampler)
{
    vector<int,3> dim;
    int numberOfLevels;
    sampler.GetDimensions(0, dim.x, dim.y, dim.z, numberOfLevels);
    return numberOfLevels;
}

__generic<T:__BuiltinArithmeticType, let N:int>
[ForceInline]
public int textureQueryLevels(SamplerCubeArray<vector<T,N>> sampler)
{
    vector<int,3> dim;
    int numberOfLevels;
    sampler.GetDimensions(0, dim.x, dim.y, dim.z, numberOfLevels);
    return numberOfLevels;
}

[ForceInline]
public int textureQueryLevels(sampler1DShadow sampler)
{
    int dim;
    int numberOfLevels;
    sampler.GetDimensions(0, dim, numberOfLevels);
    return numberOfLevels;
}

[ForceInline]
public int textureQueryLevels(sampler2DShadow sampler)
{
    vector<int,2> dim;
    int numberOfLevels;
    sampler.GetDimensions(0, dim.x, dim.y, numberOfLevels);
    return numberOfLevels;
}

[ForceInline]
public int textureQueryLevels(samplerCubeShadow sampler)
{
    vector<int,2> dim;
    int numberOfLevels;
    sampler.GetDimensions(0, dim.x, dim.y, numberOfLevels);
    return numberOfLevels;
}

[ForceInline]
public int textureQueryLevels(sampler1DArrayShadow sampler)
{
    vector<int,2> dim;
    int numberOfLevels;
    sampler.GetDimensions(0, dim.x, dim.y, numberOfLevels);
    return numberOfLevels;
}

[ForceInline]
public int textureQueryLevels(sampler2DArrayShadow sampler)
{
    vector<int,3> dim;
    int numberOfLevels;
    sampler.GetDimensions(0, dim.x, dim.y, dim.z, numberOfLevels);
    return numberOfLevels;
}

[ForceInline]
public int textureQueryLevels(samplerCubeArrayShadow sampler)
{
    vector<int,3> dim;
    int numberOfLevels;
    sampler.GetDimensions(0, dim.x, dim.y, dim.z, numberOfLevels);
    return numberOfLevels;
}

// -------------------
// textureSamples
// -------------------

__generic<T:__BuiltinArithmeticType, let N:int, let sampleCount:int>
[ForceInline]
public int textureSamples(Sampler2DMS<vector<T,N>,sampleCount> sampler)
{
    vector<int,2> dim;
    int sampleCount;
    int numberOfLevels;
    sampler.GetDimensions( dim.x, dim.y, sampleCount);
    return sampleCount;
}

__generic<T:__BuiltinArithmeticType, let N:int, let sampleCount:int>
[ForceInline]
public int textureSamples(Sampler2DMSArray<vector<T,N>,sampleCount> sampler)
{
    vector<int,3> dim;
    int sampleCount;
    int numberOfLevels;
    sampler.GetDimensions(dim.x, dim.y, dim.z, sampleCount);
    return sampleCount;
}

//
// Section 8.9.2. Texel Lookup Functions
//

// -------------------
// texture
// -------------------

__generic<T:__BuiltinArithmeticType, let N:int>
[ForceInline]
public vector<T,4> texture(Sampler1D<vector<T,N>> sampler, float p)
{
    return __vectorReshape<4>(sampler.Sample(p));
}

__generic<T:__BuiltinArithmeticType, let N:int>
[ForceInline]
public vector<T,4> texture(Sampler1D<vector<T,N>> sampler, float p, constexpr float bias)
{
    return __vectorReshape<4>(sampler.SampleBias(p, bias));
}

__generic<T:__BuiltinArithmeticType, let N:int, Shape: __ITextureShape, let isArray:int, let sampleCount:int, let format:int>
[ForceInline]
public vector<T,4> texture(__TextureImpl<
        vector<T,N>,
        Shape,
        isArray,
        0, // isMS
        sampleCount,
        0, // access
        0, // isShadow
        1, // isCombined
        format
    > sampler, vector<float,Shape.dimensions+isArray> p)
{
    return __vectorReshape<4>(sampler.Sample(p));
}

__generic<T:__BuiltinArithmeticType, let N:int, Shape: __ITextureShape, let isArray:int, let sampleCount:int, let format:int>
[ForceInline]
public vector<T,4> texture(__TextureImpl<
        vector<T,N>,
        Shape,
        isArray,
        0, // isMS
        sampleCount,
        0, // access
        0, // isShadow
        1, // isCombined
        format
    > sampler, vector<float,Shape.dimensions+isArray> p, constexpr float bias)
{
    return __vectorReshape<4>(sampler.SampleBias(p, bias));
}

[ForceInline]
public float texture(sampler1DShadow sampler, vec3 p)
{
    return sampler.SampleCmp(p.x, p.z);
}

[ForceInline]
public float texture(sampler1DShadow sampler, vec3 p, float bias)
{
    // TODO: Need to apply bias
    return sampler.SampleCmp(p.x, p.z);
}

[ForceInline]
public float texture(sampler2DShadow sampler, vec3 p)
{
    return sampler.SampleCmp(p.xy, p.z);
}

[ForceInline]
public float texture(sampler2DShadow sampler, vec3 p, float bias)
{
    // TODO: Need to apply bias
    return sampler.SampleCmp(p.xy, p.z);
}

[ForceInline]
public float texture(samplerCubeShadow sampler, vec4 p)
{
    return sampler.SampleCmp(p.xyz, p.w);
}

[ForceInline]
public float texture(samplerCubeShadow sampler, vec4 p, float bias)
{
    // TODO: Need to apply bias
    return sampler.SampleCmp(p.xyz, p.w);
}

[ForceInline]
public float texture(sampler1DArrayShadow sampler, vec3 p)
{
    return sampler.SampleCmp(p.xy, p.z);
}

[ForceInline]
public float texture(sampler1DArrayShadow sampler, vec3 p, float bias)
{
    // TODO: Need to apply bias
    return sampler.SampleCmp(p.xy, p.z);
}

[ForceInline]
public float texture(sampler2DArrayShadow sampler, vec4 p)
{
    return sampler.SampleCmp(p.xyz, p.w);
}

[ForceInline]
public float texture(samplerCubeArrayShadow sampler, vec4 p, float compare)
{
    return sampler.SampleCmp(p, compare);
}

// -------------------
// textureProj
// -------------------

__generic<T:__BuiltinArithmeticType, let N:int>
[ForceInline]
public vector<T,4> textureProj(Sampler1D<vector<T,N>> sampler, vec2 p)
{
    return texture(sampler, p.x / p.y);
}

__generic<T:__BuiltinArithmeticType, let N:int>
[ForceInline]
public vector<T,4> textureProj(Sampler1D<vector<T,N>> sampler, vec2 p, float bias)
{
    return texture(sampler, p.x / p.y, bias);
}

__generic<T:__BuiltinArithmeticType, let N:int>
[ForceInline]
public vector<T,4> textureProj(Sampler1D<vector<T,N>> sampler, vec4 p)
{
    return texture(sampler, p.x / p.w);
}

__generic<T:__BuiltinArithmeticType, let N:int>
[ForceInline]
public vector<T,4> textureProj(Sampler1D<vector<T,N>> sampler, vec4 p, float bias)
{
    return texture(sampler, p.x / p.w, bias);
}

__generic<T:__BuiltinArithmeticType, let N:int>
[ForceInline]
public vector<T,4> textureProj(Sampler2D<vector<T,N>> sampler, vec3 p)
{
    return texture(sampler, p.xy / p.z);
}

__generic<T:__BuiltinArithmeticType, let N:int>
[ForceInline]
public vector<T,4> textureProj(Sampler2D<vector<T,N>> sampler, vec3 p, float bias)
{
    return texture(sampler, p.xy / p.z, bias);
}

__generic<T:__BuiltinArithmeticType, let N:int>
[ForceInline]
public vector<T,4> textureProj(Sampler2D<vector<T,N>> sampler, vec4 p)
{
    return texture(sampler, p.xy / p.w);
}

__generic<T:__BuiltinArithmeticType, let N:int>
[ForceInline]
public vector<T,4> textureProj(Sampler2D<vector<T,N>> sampler, vec4 p, float bias)
{
    return texture(sampler, p.xy / p.w, bias);
}

__generic<T:__BuiltinArithmeticType, let N:int>
[ForceInline]
public vector<T,4> textureProj(Sampler3D<vector<T,N>> sampler, vec4 p)
{
    return texture(sampler, p.xyz / p.w);
}

__generic<T:__BuiltinArithmeticType, let N:int>
[ForceInline]
public vector<T,4> textureProj(Sampler3D<vector<T,N>> sampler, vec4 p, float bias)
{
    return texture(sampler, p.xyz / p.w, bias);
}

[ForceInline]
public float textureProj(sampler1DShadow sampler, vec4 p)
{
    return texture(sampler, p.xyz / p.w);
}

[ForceInline]
public float textureProj(sampler1DShadow sampler, vec4 p, float bias)
{
    return texture(sampler, p.xyz / p.w, bias);
}

[ForceInline]
public float textureProj(sampler2DShadow sampler, vec4 p)
{
    return texture(sampler, p.xyz / p.w);
}

[ForceInline]
public float textureProj(sampler2DShadow sampler, vec4 p, float bias)
{
    return texture(sampler, p.xyz / p.w, bias);
}

// -------------------
// textureLod
// -------------------

__generic<T:__BuiltinArithmeticType, let N:int>
[ForceInline]
public vector<T,4> textureLod(Sampler1D<vector<T,N>> sampler, float p, float lod)
{
    return __vectorReshape<4>(sampler.SampleLevel(p, lod));
}

__generic<T:__BuiltinArithmeticType, let N:int, Shape: __ITextureShape, let isArray:int, let sampleCount:int, let format:int>
[ForceInline]
public vector<T,4> textureLod(__TextureImpl<
        vector<T,N>,
        Shape,
        isArray,
        0, // isMS
        sampleCount,
        0, // access
        0, // isShadow
        1, // isCombined
        format
    > sampler, vector<float,Shape.dimensions+isArray> p, float lod)
{
    return __vectorReshape<4>(sampler.SampleLevel(p, lod));
}

[ForceInline]
public float textureLod(sampler2DShadow sampler, vec3 p, float lod)
{
    // TODO: Need to apply lod
    return sampler.SampleCmp(p.xy, p.z);
}

[ForceInline]
public float textureLod(sampler1DShadow sampler, vec3 p, float lod)
{
    // TODO: Need to apply lod
    return sampler.SampleCmp(p.x, p.z);
}

[ForceInline]
public float textureLod(sampler1DArrayShadow sampler, vec3 p, float lod)
{
    // TODO: Need to apply lod
    return sampler.SampleCmp(p.xy, p.z);
}

// -------------------
// textureOffset
// -------------------

__generic<T:__BuiltinArithmeticType, let N:int>
[ForceInline]
public vector<T,4> textureOffset(Sampler1D<vector<T,N>> sampler, float p, constexpr int offset, float bias = 0.0)
{
    return __vectorReshape<4>(sampler.SampleBias(p, bias, offset));
}

__generic<T:__BuiltinArithmeticType, let N:int>
[ForceInline]
public vector<T,4> textureOffset(Sampler2D<vector<T,N>> sampler, vec2 p, constexpr ivec2 offset, float bias = 0.0)
{
    return __vectorReshape<4>(sampler.SampleBias(p, bias, offset));
}

__generic<T:__BuiltinArithmeticType, let N:int>
[ForceInline]
public vector<T,4> textureOffset(Sampler3D<vector<T,N>> sampler, vec3 p, constexpr ivec3 offset, float bias = 0.0)
{
    return __vectorReshape<4>(sampler.SampleBias(p, bias, offset));
}

[ForceInline]
public float textureOffset(sampler2DShadow sampler, vec3 p, constexpr ivec2 offset, float bias = 0.0)
{
    // TODO: Need to apply bias
    return sampler.SampleCmp(p.xy, p.z, offset);
}

[ForceInline]
public float textureOffset(sampler1DShadow sampler, vec3 p, constexpr int offset, float bias = 0.0)
{
    // TODO: Need to apply bias
    return sampler.SampleCmp(p.x, p.z, offset);
}

__generic<T:__BuiltinArithmeticType, let N:int>
[ForceInline]
public vector<T,4> textureOffset(Sampler1DArray<vector<T,N>> sampler, vec2 p, constexpr int offset, float bias = 0.0)
{
    return __vectorReshape<4>(sampler.SampleBias(p, bias, offset));
}

__generic<T:__BuiltinArithmeticType, let N:int>
[ForceInline]
public vector<T,4> textureOffset(Sampler2DArray<vector<T,N>> sampler, vec3 p, constexpr ivec2 offset, float bias = 0.0)
{
    return __vectorReshape<4>(sampler.SampleBias(p, bias, offset));
}

[ForceInline]
public float textureOffset(sampler1DArrayShadow sampler, vec3 p, constexpr int offset, float bias = 0.0)
{
    // TODO: Need to apply bias
    return sampler.SampleCmp(p.xy, p.z, vector<int,1>(offset));
}

[ForceInline]
public float textureOffset(sampler2DArrayShadow sampler, vec4 p, constexpr ivec2 offset)
{
    return sampler.SampleCmp(p.xyz, p.w, offset);
}

// -------------------
// texelFetch
// -------------------

__generic<T:__BuiltinArithmeticType, let N:int>
[ForceInline]
public vector<T,4> texelFetch(Sampler1D<vector<T,N>> sampler, int p, int lod)
{
    return __vectorReshape<4>(sampler.Load(int2(p, lod)));
}

__generic<T:__BuiltinArithmeticType, let N:int, Shape:__ITextureShape, let isArray:int, let sampleCount:int, let format:int>
[ForceInline]
public vector<T,4> texelFetch(__TextureImpl<
        vector<T,N>,
        Shape,
        isArray,
        0, // isMS
        sampleCount,
        0, // access
        0, // isShadow
        1, // isCombined
        format
    > sampler, vector<int,Shape.dimensions+isArray> p, int lod)
{
    return __vectorReshape<4>(sampler.Load(__makeVector(p,lod)));
}

__generic<T:__BuiltinArithmeticType, let N:int>
[ForceInline]
public vector<T,4> texelFetch(Sampler2DRect<vector<T,N>> sampler, ivec2 p)
{
    return __vectorReshape<4>(sampler.Load(int3(p.xy,0)));
}

__generic<T:__BuiltinArithmeticType, let N:int, let format:int>
[ForceInline]
public vector<T,4> texelFetch(SamplerBuffer<vector<T,N>,format> sampler, int p)
{
    return __vectorReshape<4>(sampler.Load(p));
}

__generic<T:__BuiltinArithmeticType, let N:int, let isArray:int, let sampleCount:int, let format:int>
[ForceInline]
public vector<T,4> texelFetch(__TextureImpl<
        vector<T,N>,
        __Shape2D,
        isArray,
        1, // isMS
        sampleCount,
        0, // access
        0, // isShadow
        1, // isCombined
        format
    > sampler, vector<int,2+isArray> p, int lod)
{
    return __vectorReshape<4>(sampler.Load(p, lod));
}

// -------------------
// texelFetchOffset
// -------------------

__generic<T:__BuiltinArithmeticType, let N:int>
[ForceInline]
public vector<T,4> texelFetchOffset(Sampler1D<vector<T,N>> sampler, int p, int lod, constexpr int offset)
{
    return texelFetch(sampler, p + offset, lod);
}

__generic<T:__BuiltinArithmeticType, let N:int, Shape:__ITextureShape, let sampleCount:int, let format:int>
[ForceInline]
public vector<T,4> texelFetchOffset(__TextureImpl<
        vector<T,N>,
        Shape,
        0, // isArray
        0, // isMS
        sampleCount,
        0, // access
        0, // isShadow
        1, // isCombined
        format
    > sampler, vector<int,Shape.dimensions> p, int lod, constexpr vector<int,Shape.dimensions> offset)
{
    return texelFetch(sampler, p + offset, lod);
}

__generic<T:__BuiltinArithmeticType, let N:int, Shape:__ITextureShape, let sampleCount:int, let format:int>
[ForceInline]
public vector<T,4> texelFetchOffset(__TextureImpl<
        vector<T,N>,
        Shape,
        1, // isArray
        0, // isMS
        sampleCount,
        0, // access
        0, // isShadow
        1, // isCombined
        format
    > sampler, vector<int,Shape.dimensions+1> p, int lod, constexpr vector<int,Shape.dimensions> offset)
{
    return texelFetch(sampler, p + __makeVector(offset,0), lod);
}

__generic<T:__BuiltinArithmeticType, let N:int>
[ForceInline]
public vector<T,4> texelFetchOffset(Sampler2DRect<vector<T,N>> sampler, ivec2 p, constexpr ivec2 offset)
{
    return texelFetch(sampler, p + offset);
}

// -------------------
// textureProjOffset
// -------------------

__generic<T:__BuiltinArithmeticType, let N:int>
[ForceInline]
public vector<T,4> textureProjOffset(Sampler1D<vector<T,N>> sampler, vec2 p, constexpr int offset, float bias = 0.0)
{
    return textureOffset(sampler, p.x / p.y, offset, bias);
}

__generic<T:__BuiltinArithmeticType, let N:int>
[ForceInline]
public vector<T,4> textureProjOffset(Sampler1D<vector<T,N>> sampler, vec4 p, constexpr int offset, float bias = 0.0)
{
    return textureOffset(sampler, p.x / p.w, offset, bias);
}

__generic<T:__BuiltinArithmeticType, let N:int>
[ForceInline]
public vector<T,4> textureProjOffset(Sampler2D<vector<T,N>> sampler, vec3 p, constexpr ivec2 offset, float bias = 0.0)
{
    return textureOffset(sampler, p.xy / p.z, offset, bias);
}

__generic<T:__BuiltinArithmeticType, let N:int>
[ForceInline]
public vector<T,4> textureProjOffset(Sampler2D<vector<T,N>> sampler, vec4 p, constexpr ivec2 offset, float bias = 0.0)
{
    return textureOffset(sampler, p.xy / p.w, offset, bias);
}

__generic<T:__BuiltinArithmeticType, let N:int>
[ForceInline]
public vector<T,4> textureProjOffset(Sampler3D<vector<T,N>> sampler, vec4 p, constexpr ivec3 offset, float bias = 0.0)
{
    return textureOffset(sampler, p.xyz / p.w, offset, bias);
}

[ForceInline]
public float textureProjOffset(sampler1DShadow sampler, vec4 p, constexpr int offset, float bias = 0.0)
{
    return textureOffset(sampler, p.xyz / p.w, offset, bias);
}

[ForceInline]
public float textureProjOffset(sampler2DShadow sampler, vec4 p, constexpr ivec2 offset, float bias = 0.0)
{
    return textureOffset(sampler, p.xyz / p.w, offset, bias);
}

// -------------------
// textureLodOffset
// -------------------

__generic<T:__BuiltinArithmeticType, let N:int>
[ForceInline]
public vector<T,4> textureLodOffset(Sampler1D<vector<T,N>> sampler, float p, float lod, constexpr int offset)
{
    return __vectorReshape<4>(sampler.SampleLevel(p, lod, offset));
}

__generic<T:__BuiltinArithmeticType, let N:int, Shape:__ITextureShape, let isArray:int, let sampleCount:int, let format:int>
[ForceInline]
public vector<T,4> textureLodOffset(__TextureImpl<
        vector<T,N>,
        Shape,
        isArray,
        0, // isMS
        sampleCount,
        0, // access
        0, // isShadow
        1, // isCombined
        format
    > sampler, vector<float,Shape.dimensions+isArray> p, float lod, constexpr vector<int,Shape.planeDimensions> offset)
{
    return __vectorReshape<4>(sampler.SampleLevel(p, lod, offset));
}

[ForceInline]
public float textureLodOffset(sampler1DShadow sampler, vec3 p, float lod, constexpr int offset)
{
    // TODO: Need to apply lod
    return sampler.SampleCmpLevelZero(p.x, p.z, offset);
}

[ForceInline]
public float textureLodOffset(sampler2DShadow sampler, vec3 p, float lod, constexpr ivec2 offset)
{
    // TODO: Need to apply lod
    return sampler.SampleCmpLevelZero(p.xy, p.z, offset);
}

[ForceInline]
public float textureLodOffset(sampler1DArrayShadow sampler, vec3 p, float lod, constexpr int offset)
{
    // TODO: Need to apply lod
    return sampler.SampleCmpLevelZero(p.xy, p.z, offset);
}

// -------------------
// textureProjLod
// -------------------

__generic<T:__BuiltinArithmeticType, let N:int>
[ForceInline]
public vector<T,4> textureProjLod(Sampler1D<vector<T,N>> sampler, vec2 p, float lod)
{
    return textureLod(sampler, p.x / p.y, lod);
}

__generic<T:__BuiltinArithmeticType, let N:int>
[ForceInline]
public vector<T,4> textureProjLod(Sampler1D<vector<T,N>> sampler, vec4 p, float lod)
{
    return textureLod(sampler, p.x / p.w, lod);
}

__generic<T:__BuiltinArithmeticType, let N:int>
[ForceInline]
public vector<T,4> textureProjLod(Sampler2D<vector<T,N>> sampler, vec3 p, float lod)
{
    return textureLod(sampler, p.xy / p.z, lod);
}

__generic<T:__BuiltinArithmeticType, let N:int>
[ForceInline]
public vector<T,4> textureProjLod(Sampler2D<vector<T,N>> sampler, vec4 p, float lod)
{
    return textureLod(sampler, p.xy / p.w, lod);
}

__generic<T:__BuiltinArithmeticType, let N:int>
[ForceInline]
public vector<T,4> textureProjLod(Sampler3D<vector<T,N>> sampler, vec4 p, float lod)
{
    return textureLod(sampler, p.xyz / p.w, lod);
}

[ForceInline]
public float textureProjLod(sampler1DShadow sampler, vec4 p, float lod)
{
    return textureLod(sampler, p.xyz / p.w, lod);
}

[ForceInline]
public float textureProjLod(sampler2DShadow sampler, vec4 p, float lod)
{
    return textureLod(sampler, p.xyz / p.w, lod);
}

// -------------------
// textureProjLodOffset
// -------------------

__generic<T:__BuiltinArithmeticType, let N:int>
[ForceInline]
public vector<T,4> textureProjLodOffset(Sampler1D<vector<T,N>> sampler, vec2 p, float lod, constexpr int offset)
{
    return textureLodOffset(sampler, p.x / p.y, lod, offset);
}

__generic<T:__BuiltinArithmeticType, let N:int>
[ForceInline]
public vector<T,4> textureProjLodOffset(Sampler1D<vector<T,N>> sampler, vec4 p, float lod, constexpr int offset)
{
    return textureLodOffset(sampler, p.x / p.w, lod, offset);
}

__generic<T:__BuiltinArithmeticType, let N:int>
[ForceInline]
public vector<T,4> textureProjLodOffset(Sampler2D<vector<T,N>> sampler, vec3 p, float lod, constexpr ivec2 offset)
{
    return textureLodOffset(sampler, p.xy / p.z, lod, offset);
}

__generic<T:__BuiltinArithmeticType, let N:int>
[ForceInline]
public vector<T,4> textureProjLodOffset(Sampler2D<vector<T,N>> sampler, vec4 p, float lod, constexpr ivec2 offset)
{
    return textureLodOffset(sampler, p.xy / p.w, lod, offset);
}

__generic<T:__BuiltinArithmeticType, let N:int>
[ForceInline]
public vector<T,4> textureProjLodOffset(Sampler3D<vector<T,N>> sampler, vec4 p, float lod, constexpr ivec3 offset)
{
    return textureLodOffset(sampler, p.xyz / p.w, lod, offset);
}

[ForceInline]
public float textureProjLodOffset(sampler1DShadow sampler, vec4 p, float lod, constexpr int offset)
{
    return textureLodOffset(sampler, p.xyz / p.w, lod, offset);
}

[ForceInline]
public float textureProjLodOffset(sampler2DShadow sampler, vec4 p, float lod, constexpr ivec2 offset)
{
    return textureLodOffset(sampler, p.xyz / p.w, lod, offset);
}

// -------------------
// textureGrad
// -------------------

__generic<T:__BuiltinArithmeticType, let N:int>
[ForceInline]
public vector<T,4> textureGrad(Sampler1D<vector<T,N>> sampler, float p, float dPdx, float dPdy)
{
    return __vectorReshape<4>(sampler.SampleGrad(p, dPdx, dPdy));
}

__generic<T:__BuiltinArithmeticType, let N:int, Shape:__ITextureShape, let isArray:int, let sampleCount:int, let format:int>
[ForceInline]
public vector<T,4> textureGrad(__TextureImpl<
        vector<T,N>,
        Shape,
        isArray,
        0, // isMS
        sampleCount,
        0, // access
        0, // isShadow
        1, // isCombined
        format
    > sampler, vector<float,Shape.dimensions+isArray> p, vector<float,Shape.dimensions> dPdx, vector<float,Shape.dimensions> dPdy)
{
    return __vectorReshape<4>(sampler.SampleGrad(p, dPdx, dPdy));
}

[ForceInline]
public float textureGrad(sampler1DShadow sampler, vec3 p, float dPdx, float dPdy)
{
    // TODO: Not implemented
    return 0;
}

[ForceInline]
public float textureGrad(sampler1DArrayShadow sampler, vec3 p, float dPdx, float dPdy)
{
    // TODO: Not implemented
    return 0;
}

[ForceInline]
public float textureGrad(sampler2DShadow sampler, vec3 p, vec2 dPdx, vec2 dPdy)
{
    // TODO: Not implemented on HLSL side yet.
    return 0;
}

[ForceInline]
public float textureGrad(samplerCubeShadow sampler, vec4 p, vec3 dPdx, vec3 dPdy)
{
    // TODO: Not implemented on HLSL side yet.
    return 0;
}

[ForceInline]
public float textureGrad(sampler2DArrayShadow sampler, vec4 p, vec2 dPdx, vec2 dPdy)
{
    // TODO: Not implemented on HLSL side yet.
    return 0;
}

// -------------------
// textureGradOffset
// -------------------

__generic<T:__BuiltinArithmeticType, let N:int>
[ForceInline]
public vector<T,4> textureGradOffset(Sampler1D<vector<T,N>> sampler, float p, float dPdx, float dPdy, constexpr int offset)
{
    return __vectorReshape<4>(sampler.SampleGrad(p, dPdx, dPdy, offset));
}

__generic<T:__BuiltinArithmeticType, let N:int, Shape:__ITextureShape, let isArray:int, let sampleCount:int, let format:int>
[ForceInline]
public vector<T,4> textureGradOffset(__TextureImpl<
        vector<T,N>,
        Shape,
        isArray,
        0, // isMS
        sampleCount,
        0, // access
        0, // isShadow
        1, // isCombined
        format
    > sampler, vector<float,Shape.dimensions+isArray> p, vector<float,Shape.dimensions> dPdx, vector<float,Shape.dimensions> dPdy, constexpr vector<int,Shape.dimensions> offset)
{
    return __vectorReshape<4>(sampler.SampleGrad(p, dPdx, dPdy, offset));
}

[ForceInline]
public float textureGradOffset(sampler1DShadow sampler, vec3 p, float dPdx, float dPdy, constexpr int offset)
{
    // TODO: Not implemented on HLSL side yet.
    return 0;
}

[ForceInline]
public float textureGradOffset(sampler2DShadow sampler, vec3 p, vec2 dPdx, vec2 dPdy, constexpr ivec2 offset)
{
    // TODO: Not implemented on HLSL side yet.
    return 0;
}

[ForceInline]
public float textureGradOffset(sampler1DArrayShadow sampler, vec3 p, float dPdx, float dPdy, constexpr int offset)
{
    // TODO: Not implemented on HLSL side yet.
    return 0;
}

[ForceInline]
public float textureGradOffset(sampler2DArrayShadow sampler, vec4 p, vec2 dPdx, vec2 dPdy, constexpr ivec2 offset)
{
    // TODO: Not implemented on HLSL side yet.
    return 0;
}

// -------------------
// textureProjGrad
// -------------------

__generic<T:__BuiltinArithmeticType, let N:int>
[ForceInline]
public vector<T,4> textureProjGrad(Sampler1D<vector<T,N>> sampler, vec2 p, float dPdx, float dPdy)
{
    return textureGrad(sampler, p.x / p.y, dPdx, dPdy);
}

__generic<T:__BuiltinArithmeticType, let N:int>
[ForceInline]
public vector<T,4> textureProjGrad(Sampler1D<vector<T,N>> sampler, vec4 p, float dPdx, float dPdy)
{
    return textureGrad(sampler, p.x / p.w, dPdx, dPdy);
}

__generic<T:__BuiltinArithmeticType, let N:int>
[ForceInline]
public vector<T,4> textureProjGrad(Sampler2D<vector<T,N>> sampler, vec3 p, vec2 dPdx, vec2 dPdy)
{
    return textureGrad(sampler, p.xy / p.z, dPdx, dPdy);
}

__generic<T:__BuiltinArithmeticType, let N:int>
[ForceInline]
public vector<T,4> textureProjGrad(Sampler2D<vector<T,N>> sampler, vec4 p, vec2 dPdx, vec2 dPdy)
{
    return textureGrad(sampler, p.xy / p.w, dPdx, dPdy);
}

__generic<T:__BuiltinArithmeticType, let N:int>
[ForceInline]
public vector<T,4> textureProjGrad(Sampler3D<vector<T,N>> sampler, vec4 p, vec3 dPdx, vec3 dPdy)
{
    return textureGrad(sampler, p.xyz / p.w, dPdx, dPdy);
}

[ForceInline]
public float textureProjGrad(sampler1DShadow sampler, vec4 p, float dPdx, float dPdy)
{
    return textureGrad(sampler, p.xyz / p.w, dPdx, dPdy);
}

[ForceInline]
public float textureProjGrad(sampler2DShadow sampler, vec4 p, vec2 dPdx, vec2 dPdy)
{
    return textureGrad(sampler, p.xyz / p.w, dPdx, dPdy);
}

// -------------------
// textureProjGradOffset
// -------------------

__generic<T:__BuiltinArithmeticType, let N:int>
[ForceInline]
public vector<T,4> textureProjGradOffset(Sampler1D<vector<T,N>> sampler, vec2 p, float dPdx, float dPdy, constexpr int offset)
{
    return textureGradOffset(sampler, p.x / p.y, dPdx, dPdy, offset);
}

__generic<T:__BuiltinArithmeticType, let N:int>
[ForceInline]
public vector<T,4> textureProjGradOffset(Sampler1D<vector<T,N>> sampler, vec4 p, float dPdx, float dPdy, constexpr int offset)
{
    return textureGradOffset(sampler, p.x / p.w, dPdx, dPdy, offset);
}

__generic<T:__BuiltinArithmeticType, let N:int>
[ForceInline]
public vector<T,4> textureProjGradOffset(Sampler2D<vector<T,N>> sampler, vec3 p, vec2 dPdx, vec2 dPdy, constexpr ivec2 offset)
{
    return textureGradOffset(sampler, p.xy / p.z, dPdx, dPdy, offset);
}

__generic<T:__BuiltinArithmeticType, let N:int>
[ForceInline]
public vector<T,4> textureProjGradOffset(Sampler2D<vector<T,N>> sampler, vec4 p, vec2 dPdx, vec2 dPdy, constexpr ivec2 offset)
{
    return textureGradOffset(sampler, p.xy / p.w, dPdx, dPdy, offset);
}

__generic<T:__BuiltinArithmeticType, let N:int>
[ForceInline]
public vector<T,4> textureProjGradOffset(Sampler3D<vector<T,N>> sampler, vec4 p, vec3 dPdx, vec3 dPdy, constexpr ivec3 offset)
{
    return textureGradOffset(sampler, p.xyz / p.w, dPdx, dPdy, offset);
}

[ForceInline]
public float textureProjGradOffset(sampler1DShadow sampler, vec4 p, float dPdx, float dPdy, constexpr int offset)
{
    return textureGradOffset(sampler, p.xyz / p.w, dPdx, dPdy, offset);
}

[ForceInline]
public float textureProjGradOffset(sampler2DShadow sampler, vec4 p, vec2 dPdx, vec2 dPdy, constexpr ivec2 offset)
{
    return textureGradOffset(sampler, p.xyz / p.w, dPdx, dPdy, offset);
}

//
// Section 8.9.4. Texture Gather Functions
//

// -------------------
// textureGather
// -------------------

__generic<T:__BuiltinArithmeticType, let N:int, Shape:__ITextureShape, let isArray:int, let sampleCount:int, let format:int>
[ForceInline]
public vector<T,4> textureGather(__TextureImpl<
        vector<T,N>,
        Shape,
        isArray,
        0, // isMS
        sampleCount,
        0, // access
        0, // isShadow
        1, // isCombined
        format
    > sampler, vector<float,Shape.dimensions+isArray> p, int comp = 0)
{
    switch (comp)
    {
        case 1: return sampler.GatherGreen(p);
        case 2: return sampler.GatherBlue(p);
        case 3: return sampler.GatherAlpha(p);
    }
    return sampler.GatherRed(p);
}

__generic<Shape:__ITextureShape, let isArray:int, let sampleCount:int, let format:int>
[ForceInline]
public vec4 textureGather(__TextureImpl<
        float,
        Shape,
        isArray,
        0, // isMS
        sampleCount,
        0, // access
        1, // isShadow
        1, // isCombined
        format
    > sampler, vector<float,Shape.dimensions+isArray> p, float refZ)
{
    return sampler.GatherCmp(p, refZ);
}

// -------------------
// textureGatherOffset
// -------------------

__generic<T:__BuiltinArithmeticType, let N:int, let isArray:int, let sampleCount:int, let format:int>
[ForceInline]
public vector<T,4> textureGatherOffset(__TextureImpl<
        vector<T,N>,
        __Shape2D,
        isArray,
        0, // isMS
        sampleCount,
        0, // access
        0, // isShadow
        1, // isCombined
        format
    > sampler, vector<float,2+isArray> p, constexpr vector<int,2> offset, int comp = 0)
{
    switch (comp)
    {
        case 1: return sampler.GatherGreen(p, offset);
        case 2: return sampler.GatherBlue(p, offset);
        case 3: return sampler.GatherAlpha(p, offset);
    }
    return sampler.Gather(p, offset);
}

__generic<let isArray:int, let sampleCount:int, let format:int>
[ForceInline]
public vec4 textureGatherOffset(__TextureImpl<
        float,
        __Shape2D,
        isArray,
        0, // isMS
        sampleCount,
        0, // access
        1, // isShadow
        1, // isCombined
        format
    > sampler, vector<float,2+isArray> p, float refZ, constexpr vector<int,2> offset)
{
    return sampler.GatherCmp(p, refZ, offset);
}

// -------------------
// textureGatherOffsets
// -------------------

__generic<T:__BuiltinArithmeticType, let N:int, let isArray:int, let sampleCount:int, let format:int>
[ForceInline]
public vector<T,4> textureGatherOffsets(__TextureImpl<
        vector<T,N>,
        __Shape2D,
        isArray,
        0, // isMS
        sampleCount,
        0, // access
        0, // isShadow
        1, // isCombined
        format
    > sampler, vector<float,2+isArray> p, constexpr vector<int,2> offsets[4], int comp = 0)
{
    switch (comp)
    {
        case 1: return sampler.GatherGreen(p, offsets[0], offsets[1], offsets[2], offsets[3]);
        case 2: return sampler.GatherBlue(p, offsets[0], offsets[1], offsets[2], offsets[3]);
        case 3: return sampler.GatherAlpha(p, offsets[0], offsets[1], offsets[2], offsets[3]);
    }
    return sampler.Gather(p, offsets[0], offsets[1], offsets[2], offsets[3]);
}

__generic<let isArray:int, let sampleCount:int, let format:int>
[ForceInline]
public vec4 textureGatherOffsets(__TextureImpl<
        float,
        __Shape2D,
        isArray,
        0, // isMS
        sampleCount,
        0, // access
        1, // isShadow
        1, // isCombined
        format
    > sampler, vector<float,2+isArray> p, float refZ, constexpr vector<int,2> offsets[4])
{
    return sampler.GatherCmp(p, refZ, offsets[0], offsets[1], offsets[2], offsets[3]);
}

//
// Section 8.9.5. Compatibility Profile Texture Functions
//

public vec4 texture1D(sampler1D sampler, float coord)
{
    return texture(sampler, coord);
}

public vec4 texture1D(sampler1D sampler, float coord, float bias)
{
    return texture(sampler, coord, bias);
}

public vec4 texture1DProj(sampler1D sampler, vec2 coord)
{
    return textureProj(sampler, coord);
}

public vec4 texture1DProj(sampler1D sampler, vec2 coord, float bias)
{
    return textureProj(sampler, coord, bias);
}

public vec4 texture1DProj(sampler1D sampler, vec4 coord)
{
    return textureProj(sampler, coord);
}

public vec4 texture1DProj(sampler1D sampler, vec4 coord, float bias)
{
    return textureProj(sampler, coord, bias);
}

public vec4 texture1DLod(sampler1D sampler, float coord, float lod)
{
    return textureLod(sampler, coord, lod);
}

public vec4 texture1DProjLod(sampler1D sampler, vec2 coord, float lod)
{
    return textureProjLod(sampler, coord, lod);
}

public vec4 texture1DProjLod(sampler1D sampler, vec4 coord, float lod)
{
    return textureProjLod(sampler, coord, lod);
}

public vec4 texture2D(sampler2D sampler, vec2 coord)
{
    return texture(sampler, coord);
}

public vec4 texture2D(sampler2D sampler, vec2 coord, float bias)
{
    return texture(sampler, coord, bias);
}

public vec4 texture2DProj(sampler2D sampler, vec3 coord)
{
    return textureProj(sampler, coord);
}

public vec4 texture2DProj(sampler2D sampler, vec3 coord, float bias)
{
    return textureProj(sampler, coord, bias);
}

public vec4 texture2DProj(sampler2D sampler, vec4 coord)
{
    return textureProj(sampler, coord);
}

public vec4 texture2DProj(sampler2D sampler, vec4 coord, float bias)
{
    return textureProj(sampler, coord, bias);
}

public vec4 texture2DLod(sampler2D sampler, vec2 coord, float lod)
{
    return textureLod(sampler, coord, lod);
}

public vec4 texture2DProjLod(sampler2D sampler, vec3 coord, float lod)
{
    return textureProjLod(sampler, coord, lod);
}

public vec4 texture2DProjLod(sampler2D sampler, vec4 coord, float lod)
{
    return textureProjLod(sampler, coord, lod);
}

public vec4 texture3D(sampler3D sampler, vec3 coord)
{
    return texture(sampler, coord);
}

public vec4 texture3D(sampler3D sampler, vec3 coord, float bias)
{
    return texture(sampler, coord, bias);
}

public vec4 texture3DProj(sampler3D sampler, vec4 coord)
{
    return textureProj(sampler, coord);
}

public vec4 texture3DProj(sampler3D sampler, vec4 coord, float bias)
{
    return textureProj(sampler, coord, bias);
}

public vec4 texture3DLod(sampler3D sampler, vec3 coord, float lod)
{
    return textureLod(sampler, coord, lod);
}

public vec4 texture3DProjLod(sampler3D sampler, vec4 coord, float lod)
{
    return textureProjLod(sampler, coord, lod);
}

public vec4 textureCube(samplerCube sampler, vec3 coord)
{
    return texture(sampler, coord);
}

public vec4 textureCube(samplerCube sampler, vec3 coord, float bias)
{
    return texture(sampler, coord, bias);
}

public vec4 textureCubeLod(samplerCube sampler, vec3 coord, float lod)
{
    return textureLod(sampler, coord, lod);
}

public vec4 shadow1D(sampler1DShadow sampler, vec3 coord)
{
    return texture(sampler, coord);
}

public vec4 shadow1D(sampler1DShadow sampler, vec3 coord, float bias)
{
    return texture(sampler, coord, bias);
}

public vec4 shadow2D(sampler2DShadow sampler, vec3 coord)
{
    return texture(sampler, coord);
}

public vec4 shadow2D(sampler2DShadow sampler, vec3 coord, float bias)
{
    return texture(sampler, coord, bias);
}

public vec4 shadow1DProj(sampler1DShadow sampler, vec4 coord)
{
    return textureProj(sampler, coord);
}

public vec4 shadow1DProj(sampler1DShadow sampler, vec4 coord, float bias)
{
    return textureProj(sampler, coord, bias);
}

public vec4 shadow2DProj(sampler2DShadow sampler, vec4 coord)
{
    return textureProj(sampler, coord);
}

public vec4 shadow2DProj(sampler2DShadow sampler, vec4 coord, float bias)
{
    return textureProj(sampler, coord, bias);
}

public vec4 shadow1DLod(sampler1DShadow sampler, vec3 coord, float lod)
{
    return textureLod(sampler, coord, lod);
}

public vec4 shadow2DLod(sampler2DShadow sampler, vec3 coord, float lod)
{
    return textureLod(sampler, coord, lod);
}

public vec4 shadow1DProjLod(sampler1DShadow sampler, vec4 coord, float lod)
{
    return textureProjLod(sampler, coord, lod);
}

public vec4 shadow2DProjLod(sampler2DShadow sampler, vec4 coord, float lod)
{
    return textureProjLod(sampler, coord, lod);
}

//
// Ray tracing
//

public typealias rayQueryEXT = RayQuery;

__glsl_extension(GL_EXT_ray_query)
__glsl_version(460)
[ForceInline]
public void rayQueryConfirmIntersectionEXT(inout rayQueryEXT q)
{
    q.CommitNonOpaqueTriangleHit();
}

__glsl_extension(GL_EXT_ray_query)
__glsl_version(460)
[ForceInline]
public bool rayQueryProceedEXT(inout rayQueryEXT q)
{
    return q.Proceed();
}

__glsl_extension(GL_EXT_ray_query)
__glsl_version(460)
[__NoSideEffect]
public uint rayQueryGetIntersectionTypeEXT(rayQueryEXT q, bool committed)
{
    if (committed)
    {
        q.CommittedStatus();
    }
    else
    {
        q.CandidateType();
    }
    return 0;
}


//
// Subgroup
//

__glsl_extension(KHR_shader_subgroup)
__glsl_version(450)
public void subgroupBarrier()
{
    //__subgroupBarrier();
}

__glsl_extension(KHR_shader_subgroup)
__glsl_version(450)
public void subgroupMemoryBarrier()
{
}

