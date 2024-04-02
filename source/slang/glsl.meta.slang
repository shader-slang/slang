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

[require(glsl)]
[require(spirv)]
public property uint3 gl_NumWorkGroups {

    get {
        __target_switch
        {
        case glsl:
            __intrinsic_asm "(gl_NumWorkGroups)";
        case spirv:
            return spirv_asm {
                    result:$$uint3 = OpLoad builtin(NumWorkgroups:uint3);
                };
        }
    }
}

[require(glsl)]
[require(spirv)]
public property uint3 gl_WorkGroupSize {

    get {
        __target_switch
        {
        case glsl:
            __intrinsic_asm "(gl_WorkGroupSize)";
        case spirv:
            return spirv_asm {
                    result:$$uint3 = OpLoad builtin(WorkgroupSize:uint3);
                };
        }
    }
}

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

//// gimageDim Implementation

public static const int GL_TEXTURE_CUBE_MAP_POSITIVE_X = 0;
public static const int GL_TEXTURE_CUBE_MAP_NEGATIVE_X = 1;
public static const int GL_TEXTURE_CUBE_MAP_POSITIVE_Y = 2;
public static const int GL_TEXTURE_CUBE_MAP_NEGATIVE_Y = 3;
public static const int GL_TEXTURE_CUBE_MAP_POSITIVE_Z = 4;
public static const int GL_TEXTURE_CUBE_MAP_NEGATIVE_Z = 5;

__generic<T : __BuiltinType>
[ForceInline] void typeRequireChecks_image_atomic_tier1()
{
    __target_switch
    {
    case glsl:
    {
        if (__type_equals<T, float>())
            __requireGLSLExtension("GL_EXT_shader_atomic_float");
    }
    case spirv:
        if (__type_equals<T, float>())
        {
            spirv_asm
            {
                OpExtension "SPV_EXT_shader_atomic_float_add";
                OpCapability AtomicFloat32AddEXT
            };
        }
    }
}

__generic<T : __BuiltinType>
[ForceInline] void typeRequireChecks_image_atomic_tier2()
{
    __target_switch
    {
    case glsl:
    {
        if (__type_equals<T, float>())
            __requireGLSLExtension("GL_EXT_shader_atomic_float2");
    }
    case spirv:
        if (__type_equals<T, float>())
        {
            spirv_asm
            {
                OpExtension "SPV_EXT_shader_atomic_float_min_max";
                OpCapability AtomicFloat32MinMaxEXT
            };
        }
    }
}

__generic<T : __BuiltinType>
[ForceInline] void typeRequireChecks_image_atomic_int64()
{
    __target_switch
    {
    case spirv:
        if (__type_equals<T, uint64_t>()
            || __type_equals<T, int64_t>())
        {
            spirv_asm
            {
                OpCapability Int64Atomics
            };
        }
        return;
    }
}

${{{{
{
    struct ImageTypeInfo
    {
        const char *prefix;
        const char *type;
        bool isInt;
        const char *SPVTypePrefix;
        const char *SPVSubTypePrefix;
        const char *SPVAtomicSuffix;
    };
    static const ImageTypeInfo kTypeWithPrefix[] =
        {
            { "", "float", false, "F", "F", "EXT"},
            { "u", "uint", true, "I", "U", ""},
            { "i", "int", true, "I", "S", ""},
            { "u64", "uint64_t", true, "I", "U", ""},
            { "i64", "int64_t", true, "I", "S", ""},
        };
    struct ShapeTypeInfo
    {
        const char *suffix;
        int imageSizeIVecDim;
        int imageCoordIVecIndexerDim;
        const char *shape;
        int isArray;
        int isMS;
        bool isRect;
    }
    static const kShapeType[] =
        {
            { "1D", 1, 1,"__Shape1D", 0, 0, 0},

            { "2D", 2, 2, "__Shape2D", 0, 0, 0},
            { "3D", 3, 3, "__Shape3D", 0, 0, 0},
            { "Cube", 2, 3, "__ShapeCube", 0, 0, 0},
            { "2DRect", 2, 2, "__Shape2D", 0, 0, 1},
            { "1DArray", 2, 2, "__Shape1D", 1, 0, 0},

            { "2DArray", 3, 3, "__Shape2D", 1, 0, 0},
            { "CubeArray", 3, 3, "__ShapeCube", 1, 0, 0},
            { "Buffer", 1, 1, "__ShapeBuffer", 0, 0, 0},
            { "2DMS", 2, 2, "__Shape2D", 0, 1, 0},
            { "2DMSArray", 3, 3, "__Shape2D", 1, 1, 0},
        };
    for (const auto& targetType : kTypeWithPrefix)
    for (const auto& targetShape : kShapeType)
    {
        // gimageDim is the format of image buffers, we
        // need to resolve a type name from the format:
        // targetType.prefix is the 'g'
        // targetShape.suffix is the 'Dim'
        StringBuilder fullTypeNameBuilder;
        fullTypeNameBuilder << targetType.prefix << "image" << targetShape.suffix;
        auto fullTypeName = fullTypeNameBuilder.toString();

        // We need a string to resolve for the imageSize return type
        // since each shape has a different return which may not be
        // a vec
        StringBuilder imageSizeIntOrIVecBuilder;
        if (targetShape.imageSizeIVecDim == 1)
            imageSizeIntOrIVecBuilder << "int";
        else
            imageSizeIntOrIVecBuilder << "ivec" << targetShape.imageSizeIVecDim;
        auto imageSizeIntOrIVec = imageSizeIntOrIVecBuilder.toString();

        // Following OpenGL/Vulkan semantics, IMAGE_PARAMS string represents
        // the changing list of parameters per gimageDim type
        // https://registry.khronos.org/OpenGL/specs/gl/GLSLangSpec.4.60.pdf
        StringBuilder IMAGE_PARAMSBuilder;
        IMAGE_PARAMSBuilder << fullTypeName << " image,";
        int paramC = targetShape.imageCoordIVecIndexerDim;
        if (paramC == 1)
            IMAGE_PARAMSBuilder << "int";
        else
            IMAGE_PARAMSBuilder << "ivec" << paramC;
        IMAGE_PARAMSBuilder << " P";
        if (targetShape.isMS)
        {
            IMAGE_PARAMSBuilder << ", int sample";
        }
        auto IMAGE_PARAMS = IMAGE_PARAMSBuilder.toString();

        // SPIR-V requires a 'sample' parameter for all texture
        // functions. We need to handle a dynamic creation of
        // 'sample' if 'sample' parameter is not in IMAGE_PARAMS
        StringBuilder SPV_PREFIX_IMAGE_PARAMSBuilder;
        StringBuilder SPV_SUFFIX_IMAGE_PARAMSBuilder;
        StringBuilder SPV_DEFAULT_SAMPLE_VAR_IF_MISSINGBuilder;

        SPV_PREFIX_IMAGE_PARAMSBuilder << "$image $P";
        SPV_SUFFIX_IMAGE_PARAMSBuilder;
        if (targetShape.isMS)
        {
            SPV_SUFFIX_IMAGE_PARAMSBuilder << " Sample $sample";
        }
        else
        {
            SPV_DEFAULT_SAMPLE_VAR_IF_MISSINGBuilder << "let sample = 0;";
        }

        auto SPV_PREFIX_IMAGE_PARAMS = SPV_PREFIX_IMAGE_PARAMSBuilder.toString();
        auto SPV_SUFFIX_IMAGE_PARAMS = SPV_SUFFIX_IMAGE_PARAMSBuilder.toString();
        auto SPV_DEFAULT_SAMPLE_VAR_IF_MISSING = SPV_DEFAULT_SAMPLE_VAR_IF_MISSINGBuilder.toString();
    }}}}

        __generic<let format:int=0>
        public typealias $(fullTypeName) = __TextureImpl<
            $(targetType.type),
            $(targetShape.shape),
            $(targetShape.isArray), // isArray
            $(targetShape.isMS), // isMS
            0, // sampleCount
            1, // access
            0, // isShadow
            0, // isCombined
            format
        >;

${{{{
        if(targetShape.isRect)
            continue;
}}}}

        // readonly writeonly in GLSL means an object only allows information queries.
        [require(spirv)]
        [require(glsl)]
        [__readNone]
        [ForceInline] public $(imageSizeIntOrIVec) imageSize(readonly writeonly $(fullTypeName) image)
        {
            __target_switch
            {
            case glsl: __intrinsic_asm "imageSize($*0)";
            case spirv:
                return spirv_asm
                {
                    OpCapability ImageQuery;
                    result:$$$(imageSizeIntOrIVec) = OpImageQuerySize $image;
                };
            }
        }
        [require(spirv)]
        [require(glsl)]
        [__NoSideEffect]
        [ForceInline] public $(targetType.prefix)vec4 imageLoad(readonly $(IMAGE_PARAMS))
        {
            __target_switch
            {
            case glsl: __intrinsic_asm "imageLoad($*0)";
            case spirv:
            {
                $(SPV_DEFAULT_SAMPLE_VAR_IF_MISSING)
                return spirv_asm
                {
                    result:$$$(targetType.prefix)vec4 = OpImageRead $(SPV_PREFIX_IMAGE_PARAMS) $(SPV_SUFFIX_IMAGE_PARAMS);
                };
            }
            }
        }
        [require(spirv)]
        [require(glsl)]
        [ForceInline] public void imageStore(writeonly $(IMAGE_PARAMS), $(targetType.prefix)vec4 data)
        {
            __target_switch
            {
            case glsl: __intrinsic_asm "imageStore($*0)";
            case spirv:
            {
                $(SPV_DEFAULT_SAMPLE_VAR_IF_MISSING)
                return spirv_asm
                {
                    OpImageWrite $(SPV_PREFIX_IMAGE_PARAMS) $data $(SPV_SUFFIX_IMAGE_PARAMS);
                };
            }
            }
        }

${{{{
        if (targetShape.isMS)
        {
}}}}
            // readonly writeonly in GLSL means an object only allows information queries.
            [require(spirv)]
            [require(glsl)]
            [__readNone]
            [ForceInline] public int imageSamples(readonly writeonly $(fullTypeName) image)
            {
                __target_switch
                {
                case glsl: __intrinsic_asm "imageSamples($*0)";
                case spirv:
                    return spirv_asm
                    {
                        OpCapability ImageQuery;
                        result:$$int = OpImageQuerySamples $image;
                    };
                }
            }
${{{{
        }
}}}}
        [require(spirv)]
        [require(glsl)]
        [ForceInline] public $(targetType.type) imageAtomicAdd($(IMAGE_PARAMS), $(targetType.type) data)
        {
            typeRequireChecks_image_atomic_tier1<$(targetType.type)>();
            __target_switch
            {
            case glsl: __intrinsic_asm "imageAtomicAdd($*0)";
            case spirv:
            {
                let imageP = __getLegalizedSPIRVGlobalParamAddr(image);
                typeRequireChecks_image_atomic_int64<$(targetType.type)>();
                $(SPV_DEFAULT_SAMPLE_VAR_IF_MISSING)
                return spirv_asm
                {
                    OpCapability ImageQuery;
                    %ptrType = OpTypePointer Image $$$(targetType.type);
                    %ptr:%ptrType = OpImageTexelPointer $imageP $P $sample;
                    result:$$$(targetType.type) = OpAtomic$(targetType.SPVTypePrefix)Add$(targetType.SPVAtomicSuffix) %ptr Device None $data
                };
            }
            }
        }
        [require(spirv)]
        [require(glsl)]
        [ForceInline] public $(targetType.type) imageAtomicExchange($(IMAGE_PARAMS), $(targetType.type) data)
        {
            typeRequireChecks_image_atomic_tier1<$(targetType.type)>();
            __target_switch
            {
            case glsl: __intrinsic_asm "imageAtomicExchange($*0)";
            case spirv:
            {
                let imageP = __getLegalizedSPIRVGlobalParamAddr(image);
                typeRequireChecks_image_atomic_int64<$(targetType.type)>();
                $(SPV_DEFAULT_SAMPLE_VAR_IF_MISSING)
                return spirv_asm
                {
                    OpCapability ImageQuery;
                    %ptrType = OpTypePointer Image $$$(targetType.type);
                    %ptr:%ptrType = OpImageTexelPointer $imageP $P $sample;
                    result:$$$(targetType.type) = OpAtomicExchange %ptr Device ImageMemory $data
                };
            }
            }
        }
        [require(spirv)]
        [require(glsl)]
        [ForceInline] public $(targetType.type) imageAtomicMin($(IMAGE_PARAMS), $(targetType.type) data)
        {
            typeRequireChecks_image_atomic_tier2<$(targetType.type)>();
            __target_switch
            {
            case glsl: __intrinsic_asm "imageAtomicMin($*0)";
            case spirv:
            {
                let imageP = __getLegalizedSPIRVGlobalParamAddr(image);
                typeRequireChecks_image_atomic_int64<$(targetType.type)>();
                $(SPV_DEFAULT_SAMPLE_VAR_IF_MISSING)
                return spirv_asm
                {
                    OpCapability ImageQuery;
                    %ptrType = OpTypePointer Image $$$(targetType.type);
                    %ptr:%ptrType = OpImageTexelPointer $imageP $P $sample;
                    result:$$$(targetType.type) = OpAtomic$(targetType.SPVSubTypePrefix)Min$(targetType.SPVAtomicSuffix) %ptr Device ImageMemory $data
                };
            }
            }
        }
        [require(spirv)]
        [require(glsl)]
        [ForceInline] public $(targetType.type) imageAtomicMax($(IMAGE_PARAMS), $(targetType.type) data)
        {
            typeRequireChecks_image_atomic_tier2<$(targetType.type)>();
            __target_switch
            {
            case glsl: __intrinsic_asm "imageAtomicMax($*0)";
            case spirv:
            {
                let imageP = __getLegalizedSPIRVGlobalParamAddr(image);
                typeRequireChecks_image_atomic_int64<$(targetType.type)>();
                $(SPV_DEFAULT_SAMPLE_VAR_IF_MISSING)
                return spirv_asm
                {
                    OpCapability ImageQuery;
                    %ptrType = OpTypePointer Image $$$(targetType.type);
                    %ptr:%ptrType = OpImageTexelPointer $imageP $P $sample;
                    result:$$$(targetType.type) = OpAtomic$(targetType.SPVSubTypePrefix)Max$(targetType.SPVAtomicSuffix) %ptr Device ImageMemory $data
                };
            }
            }
        }
${{{{
        if (!targetType.isInt)
            continue;
}}}}
        [require(spirv)]
        [require(glsl)]
        [ForceInline] public $(targetType.type) imageAtomicAnd($(IMAGE_PARAMS), $(targetType.type) data)
        {
            __target_switch
            {
            case glsl: __intrinsic_asm "imageAtomicAnd($*0)";
            case spirv:
            {
                let imageP = __getLegalizedSPIRVGlobalParamAddr(image);
                typeRequireChecks_image_atomic_int64<$(targetType.type)>();
                $(SPV_DEFAULT_SAMPLE_VAR_IF_MISSING)
                return spirv_asm
                {
                    OpCapability ImageQuery;
                    %ptrType = OpTypePointer Image $$$(targetType.type);
                    %ptr:%ptrType = OpImageTexelPointer $imageP $P $sample;
                    result:$$$(targetType.type) = OpAtomicAnd %ptr Device ImageMemory $data
                };
            }
            }
        }
        [require(spirv)]
        [require(glsl)]
        [ForceInline] public $(targetType.type) imageAtomicOr($(IMAGE_PARAMS), $(targetType.type) data)
        {
            __target_switch
            {
            case glsl: __intrinsic_asm "imageAtomicOr($*0)";
            case spirv:
            {
                let imageP = __getLegalizedSPIRVGlobalParamAddr(image);
                typeRequireChecks_image_atomic_int64<$(targetType.type)>();
                $(SPV_DEFAULT_SAMPLE_VAR_IF_MISSING)
                return spirv_asm
                {
                    OpCapability ImageQuery;
                    %ptrType = OpTypePointer Image $$$(targetType.type);
                    %ptr:%ptrType = OpImageTexelPointer $imageP $P $sample;
                    result:$$$(targetType.type) = OpAtomicOr %ptr Device ImageMemory $data
                };
            }
            }
        }
        [require(spirv)]
        [require(glsl)]
        [ForceInline] public $(targetType.type) imageAtomicXor($(IMAGE_PARAMS), $(targetType.type) data)
        {
            __target_switch
            {
            case glsl: __intrinsic_asm "imageAtomicXor($*0)";
            case spirv:
            {
                let imageP = __getLegalizedSPIRVGlobalParamAddr(image);
                typeRequireChecks_image_atomic_int64<$(targetType.type)>();
                $(SPV_DEFAULT_SAMPLE_VAR_IF_MISSING)
                return spirv_asm
                {
                    OpCapability ImageQuery;
                    %ptrType = OpTypePointer Image $$$(targetType.type);
                    %ptr:%ptrType = OpImageTexelPointer $imageP $P $sample;
                    result:$$$(targetType.type) = OpAtomicXor %ptr Device ImageMemory $data
                };
            }
            }
        }
        [require(spirv)]
        [require(glsl)]
        [ForceInline] public $(targetType.type) imageAtomicCompSwap($(IMAGE_PARAMS), $(targetType.type) compare, $(targetType.type) data)
        {
            __target_switch
            {
            case glsl: __intrinsic_asm "imageAtomicCompSwap($*0)";
            case spirv:
            {
                let imageP = __getLegalizedSPIRVGlobalParamAddr(image);
                typeRequireChecks_image_atomic_int64<$(targetType.type)>();
                $(SPV_DEFAULT_SAMPLE_VAR_IF_MISSING)
                return spirv_asm
                {
                    OpCapability ImageQuery;
                    %ptrType = OpTypePointer Image $$$(targetType.type);
                    %ptr:%ptrType = OpImageTexelPointer $imageP $P $sample;
                    result:$$$(targetType.type) = OpAtomicCompareExchange %ptr Device ImageMemory ImageMemory $data $compare
                };
            }
            }
        }
${{{{
    }
}
}}}}

//// RayTracing

// Ray Tracing variables

public typealias rayQueryEXT = RayQuery;
public typealias accelerationStructureEXT = RaytracingAccelerationStructure;
public typealias accelerationStructureNV = RaytracingAccelerationStructure;

public typealias hitObjectNV = HitObject;

//GL_EXT_ray_tracing BuiltIn's 

void requireGLSLExtForRayTracingBuiltin() 
{
    __target_switch
    {
    case glsl:
        __requireGLSLExtension("GL_EXT_ray_tracing");
        __intrinsic_asm "";
    }
}

__spirv_version(1.4) 
void setupExtForRayTracingBuiltIn() 
{
    __target_switch
    {
    case glsl:
        requireGLSLExtForRayTracingBuiltin();
    case spirv:
        return;
    }
}

[require(glsl, raytracing)]
[require(spirv, raytracing)]
public property uint3 gl_LaunchIDNV 
{
    get 
    {
        setupExtForRayTracingBuiltIn();
        __target_switch
        {
        case glsl:
        {
            __intrinsic_asm "(gl_LaunchIDNV)";
        }
        case spirv:
        {
            return spirv_asm 
            {
                result:$$uint3 = OpLoad builtin(LaunchIdNV:uint3);
            };
        }
        }    
    }
}

[require(glsl, raytracing)]
[require(spirv, raytracing)]
public property uint3 gl_LaunchIDEXT 
{
    get 
    {
        setupExtForRayTracingBuiltIn();
        return DispatchRaysIndex();
    }
}

[require(glsl, raytracing)]
[require(spirv, raytracing)]
public property uint3 gl_LaunchSizeNV 
{
    get 
    {
        setupExtForRayTracingBuiltIn();
        __target_switch
        {
        case glsl:
        {
            __intrinsic_asm "(gl_LaunchSizeNV)";
        }
        case spirv:
        {
            return spirv_asm 
            {
                result:$$uint3 = OpLoad builtin(LaunchSizeNV:uint3);
            };
        }
        }
    }
}

[require(glsl, raytracing)]
[require(spirv, raytracing)]
public property uint3 gl_LaunchSizeEXT 
{
    get 
    {
        setupExtForRayTracingBuiltIn();
        return DispatchRaysDimensions();
    }
}

[require(glsl, raytracing)]
[require(spirv, raytracing)]
public property int gl_PrimitiveID 
{
    get 
    {
        setupExtForRayTracingBuiltIn();
        return PrimitiveIndex();
    }
}

[require(glsl, raytracing)]
[require(spirv, raytracing)]
public property int gl_InstanceID 
{
    get 
    {
        setupExtForRayTracingBuiltIn();
        return InstanceIndex();
    }
}

[require(glsl, raytracing)]
[require(spirv, raytracing)]
public property int gl_InstanceCustomIndexEXT 
{
    get 
    {
        setupExtForRayTracingBuiltIn();
        return InstanceID();
    }
}

[require(glsl, raytracing)]
[require(spirv, raytracing)]
public property int gl_GeometryIndexEXT 
{
    get
    {
        setupExtForRayTracingBuiltIn();
        return GeometryIndex();
    }
}

[require(glsl, raytracing)]
[require(spirv, raytracing)]
public property vec3 gl_WorldRayOriginEXT 
{
    get
    {
        setupExtForRayTracingBuiltIn();
        return WorldRayOrigin();
    }
}

[require(glsl, raytracing)]
[require(spirv, raytracing)]
public property vec3 gl_WorldRayDirectionEXT 
{
    get
    {
        setupExtForRayTracingBuiltIn();
        return WorldRayDirection();
    }
}

[require(glsl, raytracing)]
[require(spirv, raytracing)]
public property vec3 gl_ObjectRayOriginEXT 
{
    get
    {
        setupExtForRayTracingBuiltIn();
        return ObjectRayOrigin();
    }
}

[require(glsl, raytracing)]
[require(spirv, raytracing)]
public property vec3 gl_ObjectRayDirectionEXT 
{
    get
    {
        setupExtForRayTracingBuiltIn();
        return ObjectRayDirection();
    }
}

[require(glsl, raytracing)]
[require(spirv, raytracing)]
public property float gl_RayTminEXT 
{
    get
    {
        setupExtForRayTracingBuiltIn();
        return RayTMin();
    }
}

[require(glsl, raytracing)]
[require(spirv, raytracing)]
public property float gl_RayTmaxEXT 
{
    get
    {
        setupExtForRayTracingBuiltIn();
        return RayTCurrent();
    }
}

[require(glsl, raytracing)]
[require(spirv, raytracing)]
public property uint gl_IncomingRayFlagsEXT 
{
    get
    {
        setupExtForRayTracingBuiltIn();
        return RayFlags();
    }
}

[require(glsl, raytracing)]
[require(spirv, raytracing)]
public property float gl_HitTEXT 
{
    get
    {
        setupExtForRayTracingBuiltIn();
        __target_switch
        {
        case glsl:
        {
            __intrinsic_asm "(gl_HitTEXT)";
        }
        case spirv:
        {
            return spirv_asm 
            {
                result:$$float = OpLoad builtin(RayTmaxKHR:float);
            };
        }
        }
    }
}

[require(glsl, raytracing)]
[require(spirv, raytracing)]
public property uint gl_HitKindEXT 
{
    get
    {
        setupExtForRayTracingBuiltIn();
        return HitKind();
    }
}

[require(glsl, raytracing)]
[require(spirv, raytracing)]
public property mat4x3 gl_ObjectToWorldEXT 
{
    get
    {
        setupExtForRayTracingBuiltIn();
        __target_switch
        {
        case glsl:
        {
            __intrinsic_asm "(gl_ObjectToWorldEXT)";
        }
        case spirv:
        {
            return spirv_asm 
            {
                %mat:$$mat3x4 = OpLoad builtin(ObjectToWorldKHR:mat3x4);
                result:$$mat4x3 = OpTranspose %mat
            };
        }
        }
    }
}

[require(glsl, raytracing)]
[require(spirv, raytracing)]
public property mat3x4 gl_ObjectToWorld3x4EXT 
{
    get
    {
        setupExtForRayTracingBuiltIn();
        __target_switch
        {
        case glsl:
        {
            __intrinsic_asm "(gl_ObjectToWorld3x4EXT)";
        }
        case spirv:
        {
            return spirv_asm 
            {
                result:$$mat3x4 = OpLoad builtin(ObjectToWorldKHR:mat3x4)
            };
        }
        }
    }
}

[require(glsl, raytracing)]
[require(spirv, raytracing)]
public property mat4x3 gl_WorldToObjectEXT 
{
    get
    {
        setupExtForRayTracingBuiltIn();
        __target_switch
        {
        case glsl:
        {
            __intrinsic_asm "(gl_WorldToObjectEXT)";
        }
        case spirv:
        {
            return spirv_asm 
            {
                %mat:$$mat3x4 = OpLoad builtin(WorldToObjectKHR:mat3x4);
                result:$$mat4x3 = OpTranspose %mat
            };
        }
        }
    }
}

[require(glsl, raytracing)]
[require(spirv, raytracing)]
public property mat3x4 gl_WorldToObject3x4EXT 
{
    get
    {
        setupExtForRayTracingBuiltIn();
        __target_switch
        {
        case glsl:
        {
            __intrinsic_asm "(gl_WorldToObject3x4EXT)";
        }
        case spirv:
        {
            return spirv_asm 
            {
                result:$$mat3x4 = OpLoad builtin(WorldToObjectKHR:mat3x4)
            };
        }
        }
    }
}

// GL_EXT_ray_tracing functions

__glsl_extension(GL_EXT_ray_query)
[require(glsl, raytracing)]
[require(spirv, raytracing)]
public void traceRayEXT(
    accelerationStructureEXT topLevel,
    uint rayFlags,
    uint cullMask,
    uint sbtRecordOffset,
    uint sbtRecordStride,
    uint missIndex,
    vec3 origin,
    float Tmin,
    vec3 direction,
    float Tmax,
    constexpr int payload)
{
    __target_switch
    {
    case glsl:
    {
        __traceRay(
            topLevel,
            rayFlags,
            cullMask,
            sbtRecordOffset,
            sbtRecordStride,
            missIndex,
            origin,
            Tmin,
            direction,
            Tmax,
            payload);
    }
    case spirv:
    {
        spirv_asm 
        {
            OpTraceRayKHR 
                /**/ $topLevel
                /**/ $rayFlags
                /**/ $cullMask
                /**/ $sbtRecordOffset
                /**/ $sbtRecordStride
                /**/ $missIndex
                /**/ $origin
                /**/ $Tmin
                /**/ $direction
                /**/ $Tmax
                /**/ __rayPayloadFromLocation(payload);
        };
    }
    }
}

__glsl_extension(GL_EXT_ray_query)
[require(glsl, raytracing)]
[require(spirv, raytracing)]
public bool reportIntersectionEXT(float hitT, uint hitKind)
{
    return __reportIntersection(hitT, hitKind);
}

__glsl_extension(GL_EXT_ray_query)
[require(glsl, raytracing)]
[require(spirv, raytracing)]
public void executeCallableEXT(
    uint sbtRecordIndex, 
    int callable /*callableDataEXT and callableDataInEXT*/)
{
    __target_switch
    {
    case glsl:
    {
        __executeCallable(sbtRecordIndex, callable);
    }
    case spirv:
    {
        spirv_asm {
            OpExecuteCallableKHR $sbtRecordIndex __rayCallableFromLocation(callable)
        };
    }
    }
}

// GL_EXT_ray_tracing constants

public static const uint gl_HitKindFrontFacingTriangleEXT = 0xFEU;
public static const uint gl_HitKindBackFacingTriangleEXT = 0xFFU;

/// GL_EXT_ray_query

// GL_EXT_ray_query constants

public static const uint gl_RayFlagsNoneEXT = 0U;
public static const uint gl_RayFlagsOpaqueEXT = 1U;
public static const uint gl_RayFlagsNoOpaqueEXT = 2U;
public static const uint gl_RayFlagsTerminateOnFirstHitEXT = 4U;
public static const uint gl_RayFlagsSkipClosestHitShaderEXT = 8U;
public static const uint gl_RayFlagsCullBackFacingTrianglesEXT = 16U;
public static const uint gl_RayFlagsCullFrontFacingTrianglesEXT = 32U;
public static const uint gl_RayFlagsCullOpaqueEXT = 64U;
public static const uint gl_RayFlagsCullNoOpaqueEXT = 128U;

public static const uint gl_RayQueryCommittedIntersectionNoneEXT = 0U;
public static const uint gl_RayQueryCommittedIntersectionTriangleEXT = 1U;
public static const uint gl_RayQueryCommittedIntersectionGeneratedEXT = 2U;

public static const uint gl_RayQueryCandidateIntersectionTriangleEXT = 0U;
public static const uint gl_RayQueryCandidateIntersectionAABBEXT = 1U;

// GL_EXT_ray_query functions

__glsl_extension(GL_EXT_ray_query)
[require(glsl, rayquery)]
[require(spirv, rayquery)]
[ForceInline]
public void rayQueryInitializeEXT(
    inout rayQueryEXT q,
    accelerationStructureEXT topLevel,
    uint rayFlags, uint cullMask, vec3 origin,
    float tMin, vec3 direction, float tMax)
{
    q.TraceRayInline(
        topLevel, 
        rayFlags, 
        cullMask, 
        { origin, tMin, direction, tMax });
}

__glsl_extension(GL_EXT_ray_query)
[require(glsl, rayquery)]
[require(spirv, rayquery)]
[ForceInline]
public bool rayQueryProceedEXT(inout rayQueryEXT q)
{
    return q.Proceed();
}

__glsl_extension(GL_EXT_ray_query)
[require(glsl, rayquery)]
[require(spirv, rayquery)]
[mutating]
[ForceInline]
public void rayQueryTerminateEXT(inout rayQueryEXT q)
{
    q.Abort();
}

__glsl_extension(GL_EXT_ray_query)
[require(glsl, rayquery)]
[require(spirv, rayquery)]
[ForceInline]
public void rayQueryGenerateIntersectionEXT(inout rayQueryEXT q, float tHit)
{
    q.CommitProceduralPrimitiveHit(tHit);
}

__glsl_extension(GL_EXT_ray_query)
[require(glsl, rayquery)]
[require(spirv, rayquery)]
[ForceInline]
public void rayQueryConfirmIntersectionEXT(inout rayQueryEXT q)
{
    q.CommitNonOpaqueTriangleHit();
}

__glsl_extension(GL_EXT_ray_query)
[require(glsl, rayquery)]
[require(spirv, rayquery)]
[__NoSideEffect]
[ForceInline]
public uint rayQueryGetIntersectionTypeEXT(rayQueryEXT q, bool committed)
{
    if (committed)
    {
        return q.CommittedStatus();
    }
    else
    {
        return q.CandidateType();
    }
}

__glsl_extension(GL_EXT_ray_query)
[require(glsl, rayquery)]
[require(spirv, rayquery)]
[ForceInline]
public float rayQueryGetRayTMinEXT(rayQueryEXT q)
{
    return q.RayTMin();
}

__glsl_extension(GL_EXT_ray_query)
[require(glsl, rayquery)]
[require(spirv, rayquery)]
[ForceInline]
public uint rayQueryGetRayFlagsEXT(rayQueryEXT q)
{
    return q.RayFlags();
}

__glsl_extension(GL_EXT_ray_query)
[require(glsl, rayquery)]
[require(spirv, rayquery)]
[ForceInline]
public vec3 rayQueryGetWorldRayOriginEXT(rayQueryEXT q)
{
    return q.WorldRayOrigin();
}

__glsl_extension(GL_EXT_ray_query)
[require(glsl, rayquery)]
[require(spirv, rayquery)]
[ForceInline]
public vec3 rayQueryGetWorldRayDirectionEXT(rayQueryEXT q)
{
    return q.WorldRayDirection();
}

__glsl_extension(GL_EXT_ray_query)
[require(glsl, rayquery)]
[require(spirv, rayquery)]
[ForceInline]
public float rayQueryGetIntersectionTEXT(rayQueryEXT q, bool committed) 
{
    if (committed) 
    {
        return q.CommittedRayT();
    }
    else 
    {
        return q.CandidateTriangleRayT();
    }
}

__glsl_extension(GL_EXT_ray_query)
[require(glsl, rayquery)]
[require(spirv, rayquery)]
[ForceInline]
public int rayQueryGetIntersectionInstanceCustomIndexEXT(rayQueryEXT q, bool committed) 
{
    if (committed) 
    {
        return q.CommittedRayInstanceCustomIndex();
    }
    else 
    {
        return q.CandidateRayInstanceCustomIndex();;
    }
}

__glsl_extension(GL_EXT_ray_query)
[require(glsl, rayquery)]
[require(spirv, rayquery)]
[ForceInline]
public int rayQueryGetIntersectionInstanceIdEXT(rayQueryEXT q, bool committed) 
{
    if (committed) 
    {
        return q.CommittedRayInstanceId();
    }
    else 
    {
        return q.CandidateRayInstanceId();
    }
}

__glsl_extension(GL_EXT_ray_query)
[require(glsl, rayquery)]
[require(spirv, rayquery)]
[ForceInline]
public uint rayQueryGetIntersectionInstanceShaderBindingTableRecordOffsetEXT(rayQueryEXT q, bool committed) 
{
    if (committed) 
    {
        return q.CommittedRayInstanceShaderBindingTableRecordOffset();
    }
    else 
    {
        return q.CandidateRayInstanceShaderBindingTableRecordOffset();    
    }
}

__glsl_extension(GL_EXT_ray_query)
[require(glsl, rayquery)]
[require(spirv, rayquery)]
[ForceInline]
public int rayQueryGetIntersectionGeometryIndexEXT(rayQueryEXT q, bool committed) 
{
    if (committed) 
    {
        return q.CommittedRayGeometryIndex();
    }
    else 
    {
        return q.CandidateRayGeometryIndex();
    }
}

__glsl_extension(GL_EXT_ray_query)
[require(glsl, rayquery)]
[require(spirv, rayquery)]
[ForceInline]
public int rayQueryGetIntersectionPrimitiveIndexEXT(rayQueryEXT q, bool committed) 
{
    if (committed) 
    {
        return q.CommittedRayPrimitiveIndex();
    }
    else 
    {
        return q.CandidateRayPrimitiveIndex();
    }
}

__glsl_extension(GL_EXT_ray_query)
[require(glsl, rayquery)]
[require(spirv, rayquery)]
[ForceInline]
public vec2 rayQueryGetIntersectionBarycentricsEXT(rayQueryEXT q, bool committed) 
{
    if (committed) 
    {
        return q.CommittedRayBarycentrics();
    }
    else 
    {
        return q.CandidateRayBarycentrics();
    }
}

__glsl_extension(GL_EXT_ray_query)
[require(glsl, rayquery)]
[require(spirv, rayquery)]
[ForceInline]
public bool rayQueryGetIntersectionFrontFaceEXT(rayQueryEXT q, bool committed) 
{
    if (committed) 
    {
        return q.CommittedRayFrontFace();
    }
    else 
    {
        return q.CandidateRayFrontFace();
    }
}

__glsl_extension(GL_EXT_ray_query)
[require(glsl, rayquery)]
[require(spirv, rayquery)]
[ForceInline]
public bool rayQueryGetIntersectionCandidateAABBOpaqueEXT(rayQueryEXT q) 
{
    return q.CandidateProceduralPrimitiveNonOpaque();
}

[require(glsl, rayquery)]
[require(spirv, rayquery)]
[ForceInline]
public vec3 rayQueryGetIntersectionObjectRayDirectionEXT(rayQueryEXT q, bool committed) 
{
    if (committed) 
    {
        return q.CommittedRayObjectRayDirection();
    }
    else 
    {
        return q.CandidateRayObjectRayDirection();
    }
}

__glsl_extension(GL_EXT_ray_query)
[require(glsl, rayquery)]
[require(spirv, rayquery)]
[ForceInline]
public vec3 rayQueryGetIntersectionObjectRayOriginEXT(rayQueryEXT q, bool committed) 
{
    if (committed) 
    {
        return q.CommittedRayObjectRayOrigin();
    }
    else 
    {
        return q.CandidateRayObjectRayOrigin();
    }
}

__glsl_extension(GL_EXT_ray_query)
[require(glsl, rayquery)]
[require(spirv, rayquery)]
[ForceInline]
public mat4x3 rayQueryGetIntersectionObjectToWorldEXT(rayQueryEXT q, bool committed) 
{
    if (committed) 
    {
        return transpose(q.CommittedRayObjectToWorld());
    }
    else 
    {
        return transpose(q.CandidateRayObjectToWorld());
    }
}

__glsl_extension(GL_EXT_ray_query)
[require(glsl, rayquery)]
[require(spirv, rayquery)]
[ForceInline]
public mat4x3 rayQueryGetIntersectionWorldToObjectEXT(rayQueryEXT q, bool committed) 
{
    if (committed) 
    {
        return transpose(q.CommittedRayWorldToObject());
    }
    else 
    {
        return transpose(q.CandidateRayWorldToObject());
    }
}

/// GL_NV_shader_invocation_reorder

__glsl_extension(GL_EXT_ray_tracing)
__glsl_extension(GL_NV_shader_invocation_reorder)
__glsl_extension(GLSL_EXT_buffer_reference_uvec2)
[require(glsl, ser)]
[require(spirv, ser)]
[ForceInline]
public void hitObjectTraceRayNV(
    inout hitObjectNV hitObject,
    accelerationStructureEXT topLevel,
    uint rayFlags,
    uint cullMask,
    uint sbtRecordOffset,
    uint sbtRecordStride,
    uint missIndex,
    vec3 origin,
    float Tmin,
    vec3 direction,
    float Tmax,
    constexpr int payload) 
    {
    __target_switch
    {
    case glsl:
    {
        HitObject::__glslTraceRay(
            hitObject,
            topLevel,
            rayFlags,
            cullMask,
            sbtRecordOffset,
            sbtRecordStride,
            missIndex,
            origin,
            Tmin,
            direction,
            Tmax,
            payload);
    }
    case spirv:
    {
        spirv_asm
        {
            OpHitObjectTraceRayNV
                &hitObject
                /**/ $topLevel
                /**/ $rayFlags
                /**/ $cullMask
                /**/ $sbtRecordOffset
                /**/ $sbtRecordStride
                /**/ $missIndex
                /**/ $origin
                /**/ $Tmin
                /**/ $direction
                /**/ $Tmax
                /**/ __rayPayloadFromLocation(payload)
        };
    }
    }
}

__glsl_extension(GL_EXT_ray_tracing)
__glsl_extension(GL_NV_shader_invocation_reorder)
__glsl_extension(GLSL_EXT_buffer_reference_uvec2)
__glsl_extension(GL_NV_ray_tracing_motion_blur)
[require(glsl, ser_motion)]
[require(spirv, ser_motion)]
[ForceInline]
public void hitObjectTraceRayMotionNV(
    inout hitObjectNV hitObject, 
    accelerationStructureEXT topLevel,
    uint rayFlags,
    uint cullMask,
    uint sbtRecordOffset,
    uint sbtRecordStride,
    uint missIndex,
    vec3 origin,
    float Tmin,
    vec3 direction,
    float Tmax,
    float currentTime,
    constexpr int payload)
    {
    __target_switch
    {
    case glsl:
    {
        HitObject::__glslTraceMotionRay(
            hitObject,
            topLevel,
            rayFlags,
            cullMask,
            sbtRecordOffset,
            sbtRecordStride,
            missIndex,
            origin,
            Tmin,
            direction,
            Tmax,
            currentTime,
            payload);
    }
    case spirv:
    {
        spirv_asm
        {
            OpCapability RayTracingMotionBlurNV;
            OpExtension "SPV_NV_ray_tracing_motion_blur";
            OpHitObjectTraceRayMotionNV
                /**/ &hitObject 
                /**/ $topLevel
                /**/ $rayFlags
                /**/ $cullMask
                /**/ $sbtRecordOffset
                /**/ $sbtRecordStride
                /**/ $missIndex
                /**/ $origin
                /**/ $Tmin
                /**/ $direction
                /**/ $Tmax
                /**/ $currentTime
                /**/ __rayPayloadFromLocation(payload)
        };
    }
    }
}

__glsl_extension(GL_EXT_ray_tracing)
__glsl_extension(GL_NV_shader_invocation_reorder)
__glsl_extension(GLSL_EXT_buffer_reference_uvec2)
[require(glsl, ser)]
[require(spirv, ser)]
[ForceInline]
public void hitObjectRecordHitNV(
    inout hitObjectNV hitObject,
    accelerationStructureEXT topLevel,
    int instanceid,
    int primitiveid,
    int geometryindex,
    uint hitKind,
    uint sbtRecordOffset,
    uint sbtRecordStride,
    vec3 origin,
    float Tmin,
    vec3 direction,
    float Tmax,
    constexpr int attributeLocation)
    {
    __target_switch
    {
    case glsl:
    {
        HitObject::__glslMakeHit(
            hitObject,
            topLevel,
            instanceid,
            primitiveid,
            geometryindex,
            hitKind,
            sbtRecordOffset,
            sbtRecordStride,
            origin,
            Tmin,
            direction,
            Tmax,
            attributeLocation);
    }
    case spirv:
    {
        spirv_asm
        {
            OpHitObjectRecordHitNV
                /**/ &hitObject
                /**/ $topLevel
                /**/ $instanceid
                /**/ $primitiveid
                /**/ $geometryindex
                /**/ $hitKind
                /**/ $sbtRecordOffset
                /**/ $sbtRecordStride
                /**/ $origin
                /**/ $Tmin
                /**/ $direction
                /**/ $Tmax
                /**/ __rayAttributeFromLocation(attributeLocation)
        };
    }
    }
}

__glsl_extension(GL_EXT_ray_tracing)
__glsl_extension(GL_NV_shader_invocation_reorder)
__glsl_extension(GLSL_EXT_buffer_reference_uvec2)
__glsl_extension(GL_NV_ray_tracing_motion_blur)
[require(glsl, ser_motion)]
[require(spirv, ser_motion)]
[ForceInline]
public void hitObjectRecordHitMotionNV(
    inout hitObjectNV hitObject,
    accelerationStructureEXT topLevel,
    int instanceid,
    int primitiveid,
    int geometryindex,
    uint hitKind,
    uint sbtRecordOffset,
    uint sbtRecordStride,
    vec3 origin,
    float Tmin,
    vec3 direction,
    float Tmax,
    float currentTime,
    constexpr int attributeLocation)
    {
    __target_switch
    {
    case glsl:
    {
        HitObject::__glslMakeMotionHit(
            hitObject,
            topLevel,
            instanceid,
            primitiveid,
            geometryindex,
            hitKind,
            sbtRecordOffset,
            sbtRecordStride,
            origin,
            Tmin,
            direction,
            Tmax,
            currentTime,
            attributeLocation);
    }
    case spirv:
    {
        spirv_asm
        {
            OpHitObjectRecordHitMotionNV
                /**/ &hitObject
                /**/ $topLevel
                /**/ $instanceid
                /**/ $primitiveid
                /**/ $geometryindex
                /**/ $hitKind
                /**/ $sbtRecordOffset
                /**/ $sbtRecordStride
                /**/ $origin
                /**/ $Tmin
                /**/ $direction
                /**/ $Tmax
                /**/ $currentTime
                /**/ __rayAttributeFromLocation(attributeLocation)
        };
    }
    }
}

__glsl_extension(GL_EXT_ray_tracing)
__glsl_extension(GL_NV_shader_invocation_reorder)
__glsl_extension(GLSL_EXT_buffer_reference_uvec2)
[require(glsl, ser)]
[require(spirv, ser)]
[ForceInline]
public void hitObjectRecordHitWithIndexNV(
    inout hitObjectNV hitObject,
    accelerationStructureEXT topLevel,
    int instanceid,
    int primitiveid,
    int geometryindex,
    uint hitKind,
    uint sbtRecordIndex,
    vec3 origin,
    float Tmin,
    vec3 direction,
    float Tmax,
    constexpr int attributeLocation)
{
    __target_switch
    {
    case glsl:
    {
        HitObject::__glslMakeHitWithIndex(
            hitObject,
            topLevel,
            instanceid,
            primitiveid,
            geometryindex,
            hitKind,
            sbtRecordIndex,
            origin,
            Tmin,
            direction,
            Tmax,
            attributeLocation);
    }
    case spirv:
    {
        spirv_asm
        {
            OpHitObjectRecordHitWithIndexNV
                /**/ &hitObject
                /**/ $topLevel
                /**/ $instanceid
                /**/ $primitiveid
                /**/ $geometryindex
                /**/ $hitKind
                /**/ $sbtRecordIndex
                /**/ $origin
                /**/ $Tmin
                /**/ $direction
                /**/ $Tmax
                /**/ __rayAttributeFromLocation(attributeLocation)
        };
    }
    }
}

__glsl_extension(GL_EXT_ray_tracing)
__glsl_extension(GL_NV_shader_invocation_reorder)
__glsl_extension(GLSL_EXT_buffer_reference_uvec2)
__glsl_extension(GL_NV_ray_tracing_motion_blur)
[require(glsl, ser_motion)]
[require(spirv, ser_motion)]
[ForceInline]
public void hitObjectRecordHitWithIndexMotionNV(
    inout hitObjectNV hitObject,
    accelerationStructureEXT topLevel,
    int instanceid,
    int primitiveid,
    int geometryindex,
    uint hitKind,
    uint sbtRecordIndex,
    vec3 origin,
    float Tmin,
    vec3 direction,
    float Tmax,
    float currentTime,
    constexpr int attributeLocation)
    {
    __target_switch
    {
    case glsl:
    {
        HitObject::__glslMakeMotionHitWithIndex(
            hitObject,
            topLevel,
            instanceid,
            primitiveid,
            geometryindex,
            hitKind,
            sbtRecordIndex,
            origin,
            Tmin,
            direction,
            Tmax,
            currentTime,
            attributeLocation);
    }
    case spirv:
    {
        spirv_asm
        {
            OpCapability RayTracingMotionBlurNV;
            OpExtension "SPV_NV_ray_tracing_motion_blur";
            OpHitObjectRecordHitWithIndexMotionNV
                /**/ &hitObject
                /**/ $topLevel
                /**/ $instanceid
                /**/ $primitiveid
                /**/ $geometryindex
                /**/ $hitKind
                /**/ $sbtRecordIndex
                /**/ $origin
                /**/ $Tmin
                /**/ $direction
                /**/ $Tmax
                /**/ $currentTime
                /**/ __rayAttributeFromLocation(attributeLocation)
        };
    }
    }
}

__glsl_extension(GL_EXT_ray_tracing)
__glsl_extension(GL_NV_shader_invocation_reorder)
__glsl_extension(GLSL_EXT_buffer_reference_uvec2)
[require(glsl, ser)]
[require(spirv, ser)]
[ForceInline]
public void hitObjectRecordMissNV(
    inout hitObjectNV hitObject, 
    uint sbtRecordIndex, 
    vec3 origin,
    float Tmin,
    vec3 direction,
    float Tmax)
{
    hitObject = HitObject::MakeMiss(
        sbtRecordIndex,
        { origin, Tmin, direction, Tmax }
        );
}

__glsl_extension(GL_EXT_ray_tracing)
__glsl_extension(GL_NV_shader_invocation_reorder)
__glsl_extension(GLSL_EXT_buffer_reference_uvec2)
__glsl_extension(GL_NV_ray_tracing_motion_blur)
[require(glsl, ser_motion)]
[require(spirv, ser_motion)]
[ForceInline]
public void hitObjectRecordMissMotionNV(
    inout hitObjectNV hitObject, 
    uint sbtRecordIndex, 
    vec3 origin,
    float Tmin,
    vec3 direction,
    float Tmax,
    float currentTime)
{
    hitObject = HitObject::MakeMotionMiss(
        sbtRecordIndex,
        { origin, Tmin, direction, Tmax },
        currentTime
        );
}

__glsl_extension(GL_EXT_ray_tracing)
__glsl_extension(GL_NV_shader_invocation_reorder)
__glsl_extension(GLSL_EXT_buffer_reference_uvec2)
[require(glsl, ser)]
[require(spirv, ser)]
[ForceInline]
public void hitObjectRecordEmptyNV(hitObjectNV hitObject)
{
    hitObject = HitObject::MakeNop();
}

__glsl_extension(GL_EXT_ray_tracing)
__glsl_extension(GL_NV_shader_invocation_reorder)
__glsl_extension(GLSL_EXT_buffer_reference_uvec2)
[require(glsl, ser)]
[require(spirv, ser)]
[ForceInline]
public void hitObjectExecuteShaderNV(
    inout hitObjectNV hitObject, 
    constexpr int payload)
{    
    __target_switch 
    {
    case glsl:
    {
        HitObject::__glslInvoke(hitObject, payload);
    }
    case spirv:
    {
        spirv_asm 
        {
            OpHitObjectExecuteShaderNV &hitObject __rayPayloadFromLocation(payload)
        };
    }
    }
}

__glsl_extension(GL_EXT_ray_tracing)
__glsl_extension(GL_NV_shader_invocation_reorder)
__glsl_extension(GLSL_EXT_buffer_reference_uvec2)
[require(glsl, ser)]
[require(spirv, ser)]
[ForceInline]
public bool hitObjectIsEmptyNV(hitObjectNV hitObject)
{
    return hitObject.IsNop();
}

__glsl_extension(GL_EXT_ray_tracing)
__glsl_extension(GL_NV_shader_invocation_reorder)
__glsl_extension(GLSL_EXT_buffer_reference_uvec2)
[require(glsl, ser)]
[require(spirv, ser)]
[ForceInline]
public bool hitObjectIsMissNV(hitObjectNV hitObject)
{
    return hitObject.IsMiss();
}

__glsl_extension(GL_EXT_ray_tracing)
__glsl_extension(GL_NV_shader_invocation_reorder)
__glsl_extension(GLSL_EXT_buffer_reference_uvec2)
[require(glsl, ser)]
[require(spirv, ser)]
[ForceInline]
public bool hitObjectIsHitNV(hitObjectNV hitObject)
{
    return hitObject.IsHit();
}

__glsl_extension(GL_EXT_ray_tracing)
__glsl_extension(GL_NV_shader_invocation_reorder)
__glsl_extension(GLSL_EXT_buffer_reference_uvec2)
[require(glsl, ser)]
[require(spirv, ser)]
[ForceInline]
public float hitObjectGetRayTMinNV(hitObjectNV hitObject)
{
    return hitObject.GetRayDesc().TMin;  
}

__glsl_extension(GL_EXT_ray_tracing)
__glsl_extension(GL_NV_shader_invocation_reorder)
__glsl_extension(GLSL_EXT_buffer_reference_uvec2)
[require(glsl, ser)]
[require(spirv, ser)]
[ForceInline]
public float hitObjectGetRayTMaxNV(hitObjectNV hitObject)
{
    return hitObject.GetRayDesc().TMax; 
}

__glsl_extension(GL_EXT_ray_tracing)
__glsl_extension(GL_NV_shader_invocation_reorder)
__glsl_extension(GLSL_EXT_buffer_reference_uvec2)
[require(glsl, ser)]
[require(spirv, ser)]
[ForceInline]
public vec3 hitObjectGetWorldRayOriginNV(hitObjectNV hitObject)
{
    return hitObject.GetRayDesc().Origin;
}

__glsl_extension(GL_EXT_ray_tracing)
__glsl_extension(GL_NV_shader_invocation_reorder)
__glsl_extension(GLSL_EXT_buffer_reference_uvec2)
[require(glsl, ser)]
[require(spirv, ser)]
[ForceInline]
public vec3 hitObjectGetWorldRayDirectionNV(hitObjectNV hitObject)
{
    return hitObject.GetRayDesc().Direction; 
}

__glsl_extension(GL_EXT_ray_tracing)
__glsl_extension(GL_NV_shader_invocation_reorder)
__glsl_extension(GLSL_EXT_buffer_reference_uvec2)
[require(glsl, ser)]
[require(spirv, ser)]
[ForceInline]
public vec3 hitObjectGetObjectRayOriginNV(hitObjectNV hitObject)
{
    return hitObject.GetObjectRayOrigin();
}

__glsl_extension(GL_EXT_ray_tracing)
__glsl_extension(GL_NV_shader_invocation_reorder)
__glsl_extension(GLSL_EXT_buffer_reference_uvec2)
[require(glsl, ser)]
[require(spirv, ser)]
[ForceInline]
public vec3 hitObjectGetObjectRayDirectionNV(hitObjectNV hitObject)
{
    return hitObject.GetObjectRayDirection();
}

__glsl_extension(GL_EXT_ray_tracing)
__glsl_extension(GL_NV_shader_invocation_reorder)
__glsl_extension(GLSL_EXT_buffer_reference_uvec2)
[require(glsl, ser)]
[require(spirv, ser)]
[ForceInline]
public mat4x3 hitObjectGetObjectToWorldNV(hitObjectNV hitObject)
{
    return transpose(hitObject.GetObjectToWorld());
}

__glsl_extension(GL_EXT_ray_tracing)
__glsl_extension(GL_NV_shader_invocation_reorder)
__glsl_extension(GLSL_EXT_buffer_reference_uvec2)
[require(glsl, ser)]
[require(spirv, ser)]
[ForceInline]
public mat4x3 hitObjectGetWorldToObjectNV(hitObjectNV hitObject)
{
    return transpose(hitObject.GetWorldToObject()); 
}

__glsl_extension(GL_EXT_ray_tracing)
__glsl_extension(GL_NV_shader_invocation_reorder)
__glsl_extension(GLSL_EXT_buffer_reference_uvec2)
[require(glsl, ser)]
[require(spirv, ser)]
[ForceInline]
public int hitObjectGetInstanceCustomIndexNV(hitObjectNV hitObject)
{
    return hitObject.GetInstanceID();
}

__glsl_extension(GL_EXT_ray_tracing)
__glsl_extension(GL_NV_shader_invocation_reorder)
__glsl_extension(GLSL_EXT_buffer_reference_uvec2)
[require(glsl, ser)]
[require(spirv, ser)]
[ForceInline]
public int hitObjectGetInstanceIdNV(hitObjectNV hitObject)
{
    return hitObject.GetInstanceIndex();
}

[require(glsl, ser)]
[require(spirv, ser)]
[ForceInline]
public int hitObjectGetGeometryIndexNV(hitObjectNV hitObject)
{
    return hitObject.GetGeometryIndex();
}

__glsl_extension(GL_EXT_ray_tracing)
__glsl_extension(GL_NV_shader_invocation_reorder)
__glsl_extension(GLSL_EXT_buffer_reference_uvec2)
[require(glsl, ser)]
[require(spirv, ser)]
[ForceInline]
public int hitObjectGetPrimitiveIndexNV(hitObjectNV hitObject)
{
    return hitObject.GetPrimitiveIndex();
}

__glsl_extension(GL_EXT_ray_tracing)
__glsl_extension(GL_NV_shader_invocation_reorder)
__glsl_extension(GLSL_EXT_buffer_reference_uvec2)
[require(glsl, ser)]
[require(spirv, ser)]
[ForceInline]
public uint hitObjectGetHitKindNV(hitObjectNV hitObject)
{
    return hitObject.GetHitKind();
}

__glsl_extension(GL_EXT_ray_tracing)
__glsl_extension(GL_NV_shader_invocation_reorder)
__glsl_extension(GLSL_EXT_buffer_reference_uvec2)
[require(glsl, ser)]
[require(spirv, ser)]
[ForceInline]
public void hitObjectGetAttributesNV(
    inout hitObjectNV hitObject, 
    constexpr int attributeLocation)
{
    __target_switch
    {
    case glsl:
    {
        __intrinsic_asm "hitObjectGetAttributesNV($0, $1)";
    }
    case spirv:
    {
        spirv_asm 
        {
            OpCapability ShaderInvocationReorderNV; 
            OpHitObjectGetAttributesNV &hitObject __rayAttributeFromLocation(attributeLocation)
        };
    }
    }
}

__glsl_extension(GL_EXT_ray_tracing)
__glsl_extension(GL_NV_shader_invocation_reorder)
__glsl_extension(GLSL_EXT_buffer_reference_uvec2)
[require(glsl, ser)]
[require(spirv, ser)]
[ForceInline]
public uvec2 hitObjectGetShaderRecordBufferHandleNV(hitObjectNV hitObject)
{
    return hitObject.GetShaderRecordBufferHandle();
}

__glsl_extension(GL_EXT_ray_tracing)
__glsl_extension(GL_NV_shader_invocation_reorder)
__glsl_extension(GLSL_EXT_buffer_reference_uvec2)
[require(glsl, ser)]
[require(spirv, ser)]
[ForceInline]
public uint hitObjectGetShaderBindingTableRecordIndexNV(hitObjectNV hitObject)
{
    return hitObject.GetShaderTableIndex();
}

__glsl_extension(GL_EXT_ray_tracing)
__glsl_extension(GL_NV_shader_invocation_reorder)
__glsl_extension(GLSL_EXT_buffer_reference_uvec2)
[require(glsl, ser)]
[require(spirv, ser)]
[ForceInline]
public float hitObjectGetCurrentTimeNV(hitObjectNV hitObject)
{
    return hitObject.GetCurrentTime();
}

__glsl_extension(GL_EXT_ray_tracing)
__glsl_extension(GL_NV_shader_invocation_reorder)
__glsl_extension(GLSL_EXT_buffer_reference_uvec2)
[require(glsl, ser)]
[require(spirv, ser)]
[ForceInline]
public void reorderThreadNV(uint hint, uint bits)
{
    ReorderThread(hint, bits);
}

__glsl_extension(GL_EXT_ray_tracing)
__glsl_extension(GL_NV_shader_invocation_reorder)
__glsl_extension(GLSL_EXT_buffer_reference_uvec2)
[require(glsl, ser)]
[require(spirv, ser)]
[ForceInline]
public void reorderThreadNV(hitObjectNV hitObject)
{
    ReorderThread(hitObject);
}

__glsl_extension(GL_EXT_ray_tracing)
__glsl_extension(GL_NV_shader_invocation_reorder)
__glsl_extension(GLSL_EXT_buffer_reference_uvec2)
[require(glsl, ser)]
[require(spirv, ser)]
[ForceInline]
public void reorderThreadNV(hitObjectNV hitObject, uint hint, uint bits)
{
    ReorderThread(hitObject, hint, bits);
}

/// GL_NV_ray_tracing_motion_blur

[require(glsl, raytracing_motionblur)]
[require(spirv, raytracing_motionblur)]
public property float gl_CurrentRayTimeNV
{
    get
    {
        __target_switch
        {
        case glsl:
            __intrinsic_asm "(gl_CurrentRayTimeNV)";
        case spirv:
            return spirv_asm
            {
                OpCapability RayTracingMotionBlurNV;
                OpExtension "SPV_NV_ray_tracing_motion_blur";
                result:$$float = OpLoad builtin(CurrentRayTimeNV:float);
            };
        }
    }
}

__glsl_extension(GL_EXT_ray_tracing) 
__glsl_extension(GL_NV_ray_tracing_motion_blur)
[require(glsl, raytracing_motionblur)]
[require(spirv, raytracing_motionblur)]
[ForceInline]
public void traceRayMotionNV(
    accelerationStructureEXT topLevel,
    uint rayFlags,
    uint cullMask,
    uint sbtRecordOffset,
    uint sbtRecordStride,
    uint missIndex,
    vec3 origin,
    float Tmin,
    vec3 direction,
    float Tmax,
    float currentTime,
    constexpr int payload)
{
    __target_switch 
    {
    case glsl:
    {
        __traceMotionRay(
            topLevel,
            rayFlags,
            cullMask,
            sbtRecordOffset,
            sbtRecordStride,
            missIndex,
            origin,
            Tmin,
            direction,
            Tmax,
            currentTime,
            payload);
    }
    case spirv:
    {
        spirv_asm 
        {
            OpCapability RayTracingMotionBlurNV;
            OpExtension "SPV_NV_ray_tracing_motion_blur";
            OpTraceRayMotionNV
                /**/ $topLevel
                /**/ $rayFlags
                /**/ $cullMask
                /**/ $sbtRecordOffset
                /**/ $sbtRecordStride
                /**/ $missIndex
                /**/ $origin
                /**/ $Tmin
                /**/ $direction
                /**/ $Tmax
                /**/ $currentTime
                /**/ __rayPayloadFromLocation(payload)
        };
    }
    }
}

__generic<T : __BuiltinType>
[ForceInline]
void typeRequireChecks_shader_subgroup_GLSL() {
    // the following is a seperate function call, since else the `__requireGLSLExtension` and associated __intrinsic_asm is ignored if the calling function also calls an __intrinsic_asm
    __target_switch
    {
    case glsl:
        if (__type_equals<T, half>()
            || __type_equals<T, float16_t>()
            ) __requireGLSLExtension("GL_EXT_shader_subgroup_extended_types_float16");
        else if (__type_equals<T, uint8_t>()
            || __type_equals<T, int8_t>()
            ) __requireGLSLExtension("GL_EXT_shader_subgroup_extended_types_int8");
        else if (__type_equals<T, uint16_t>()
            || __type_equals<T, int16_t>()
            ) __requireGLSLExtension("GL_EXT_shader_subgroup_extended_types_int16");
        else if (__type_equals<T, uint64_t>()
            || __type_equals<T, int64_t>()
            ) __requireGLSLExtension("GL_EXT_shader_subgroup_extended_types_int64");

        __intrinsic_asm "";
    }
}

__generic<T : __BuiltinType>
void shader_subgroup_preamble() {
    // checks needed for shader_subgroup functions; __requireGLSLExtension does not work 
    // (does not add the ext specified correctly to the compile output; using extended type
    // will result in error for using the type)
    __target_switch
    {
    case glsl:
        typeRequireChecks_shader_subgroup_GLSL<T>();
    case spirv:
        return;
    }

} 

// GL_KHR_shader_subgroup_basic Built-in Variables

void requireGLSLExtForSubgroupBasicBuiltin() {
    __target_switch
    {
    case glsl:
        __requireGLSLExtension("GL_KHR_shader_subgroup_basic");
        __intrinsic_asm "";
    }
}

__spirv_version(1.3) 
void setupExtForSubgroupBasicBuiltIn() {
    __target_switch
    {
    case glsl:
        requireGLSLExtForSubgroupBasicBuiltin();
    case spirv:
        return;
    }
}

void requireGLSLExtForSubgroupBallotBuiltin() {
    __target_switch
    {
    case glsl:
        __requireGLSLExtension("GL_KHR_shader_subgroup_ballot");
        __intrinsic_asm "";
    }
}

__spirv_version(1.3) 
void setupExtForSubgroupBallotBuiltIn() {
    __target_switch
    {
    case glsl:
        requireGLSLExtForSubgroupBallotBuiltin();
    case spirv:
        return;
    }
}

[require(glsl)]
[require(spirv)]
public property uint gl_NumSubgroups {
    
    get {
        setupExtForSubgroupBasicBuiltIn();
        __target_switch
        {
        case glsl:
            __intrinsic_asm "(gl_NumSubgroups)";
        case spirv:
            return spirv_asm {
                    OpCapability GroupNonUniform;
                    result:$$uint = OpLoad builtin(NumSubgroups:uint);
                };
        }

    }
}

[require(glsl)]
[require(spirv)]
public property uint gl_SubgroupID
{
    get {
        setupExtForSubgroupBasicBuiltIn();
        __target_switch
        {
        case glsl:
            __intrinsic_asm "(gl_SubgroupID)";
        case spirv:
            return spirv_asm {
                        OpCapability GroupNonUniform;
                        result:$$uint = OpLoad builtin(SubgroupId:uint);
                    };
        }
    }
}

[require(glsl)]
[require(spirv)]
public property uint gl_SubgroupSize
{
    get {
        setupExtForSubgroupBasicBuiltIn();
        return WaveGetLaneCount();
    }
}

[require(glsl)]
[require(spirv)]
public property uint gl_SubgroupInvocationID
{
    get {
        setupExtForSubgroupBasicBuiltIn();
        return WaveGetLaneIndex();
    }
}

[require(glsl)]
[require(spirv)]
public property uvec4 gl_SubgroupEqMask
{
    get {
        setupExtForSubgroupBasicBuiltIn();
        setupExtForSubgroupBallotBuiltIn();
        __target_switch
        {
        case glsl:
            __intrinsic_asm "(gl_SubgroupEqMask)";
        case spirv:
            return spirv_asm {
                        OpCapability GroupNonUniformBallot;
                        result:$$uvec4 = OpLoad builtin(SubgroupEqMask:uvec4);
                    };
        }
    }
}

[require(glsl)]
[require(spirv)]
public property uvec4 gl_SubgroupGeMask
{
    get {
        setupExtForSubgroupBasicBuiltIn();
        setupExtForSubgroupBallotBuiltIn();
        __target_switch
        {
        case glsl:
            __intrinsic_asm "(gl_SubgroupGeMask)";
        case spirv:
            return spirv_asm {
                        OpCapability GroupNonUniformBallot;
                        result:$$uvec4 = OpLoad builtin(SubgroupGeMask:uvec4);
                    };
        }
    }
}

[require(glsl)]
[require(spirv)]
public property uvec4 gl_SubgroupGtMask
{
    get {
        setupExtForSubgroupBasicBuiltIn();
        setupExtForSubgroupBallotBuiltIn();
        __target_switch
        {
        case glsl:
            __intrinsic_asm "(gl_SubgroupGtMask)";
        case spirv:
            return spirv_asm {
                        OpCapability GroupNonUniformBallot;
                        result:$$uvec4 = OpLoad builtin(SubgroupGtMask:uvec4);
                    };
        }
    }
}

[require(glsl)]
[require(spirv)]
public property uvec4 gl_SubgroupLeMask
{
    get {
        setupExtForSubgroupBasicBuiltIn();
        setupExtForSubgroupBallotBuiltIn();
        __target_switch
        {
        case glsl:
            __intrinsic_asm "(gl_SubgroupLeMask)";
        case spirv:
            return spirv_asm {
                        OpCapability GroupNonUniformBallot;
                        result:$$uvec4 = OpLoad builtin(SubgroupLeMask:uvec4);
                    };
        }
    }
}

[require(glsl)]
[require(spirv)]
public property uvec4 gl_SubgroupLtMask
{
    get {
        setupExtForSubgroupBasicBuiltIn();
        setupExtForSubgroupBallotBuiltIn();
        __target_switch
        {
        case glsl:
            __intrinsic_asm "(gl_SubgroupLtMask)";
        case spirv:
            return spirv_asm {
                        OpCapability GroupNonUniformBallot;
                        result:$$uvec4 = OpLoad builtin(SubgroupLtMask:uvec4);
                    };
        }
    }
}

// GL_KHR_shader_subgroup_basic

__glsl_extension(GL_KHR_shader_subgroup_basic) [require(glsl)] 
__spirv_version(1.3) [require(spirv)]
[ForceInline] public void subgroupBarrier()
{
    __target_switch
    {
    case cuda:
        __intrinsic_asm "__syncwarp()";
    case hlsl:
        __intrinsic_asm "AllMemoryBarrierWithGroupSync()";
    case glsl:
        __intrinsic_asm "subgroupBarrier()";
    case spirv:
        spirv_asm {
            OpCapability Shader;
            OpControlBarrier Subgroup Subgroup AcquireRelease|SubgroupMemory|ImageMemory|UniformMemory
        };

    }
}

__glsl_extension(GL_KHR_shader_subgroup_basic) [require(glsl)] 
__spirv_version(1.3) [require(spirv)]
[ForceInline] public void subgroupMemoryBarrier()
{
    __target_switch
    {
    case cuda:
        __intrinsic_asm "__threadfence_block()";
    case hlsl:
        __intrinsic_asm "AllMemoryBarrier()";
    case glsl:
        __intrinsic_asm "subgroupMemoryBarrier()";
    case spirv:
        spirv_asm {
            OpCapability Shader;
            OpMemoryBarrier Subgroup AcquireRelease|SubgroupMemory|ImageMemory|UniformMemory
        };

    }
}

__glsl_extension(GL_KHR_shader_subgroup_basic) [require(glsl)] 
__spirv_version(1.3) [require(spirv)]
[ForceInline] public void subgroupMemoryBarrierBuffer()
{
    // the following implementation is NOT the same as DeviceMemoryBarrier
    // HLSL lacks the same granularity of blocking on subgroup memory within a subgroup
    __target_switch
    {
    case cuda:
        __intrinsic_asm "__threadfence_block()";
    case hlsl:
        __intrinsic_asm "DeviceMemoryBarrier()";
    case glsl:
        __intrinsic_asm "subgroupMemoryBarrierBuffer()";
    case spirv:
        spirv_asm {
            OpCapability Shader;
            OpMemoryBarrier Subgroup AcquireRelease|UniformMemory
        };

    }
}

__glsl_extension(GL_KHR_shader_subgroup_basic) [require(glsl)] 
__spirv_version(1.3) [require(spirv)]
[ForceInline] public void subgroupMemoryBarrierImage()
{
    __target_switch
    {
    case cuda:
        __intrinsic_asm "__threadfence_block()";
    case hlsl:
        __intrinsic_asm "DeviceMemoryBarrier()";
    case glsl:
        __intrinsic_asm "subgroupMemoryBarrierImage()";
    case spirv:
        spirv_asm {
            OpMemoryBarrier Subgroup AcquireRelease|ImageMemory
        };

    }
}

__glsl_extension(GL_KHR_shader_subgroup_basic) [require(glsl)] 
__spirv_version(1.3) [require(spirv)]
[ForceInline] public void subgroupMemoryBarrierShared()
{
    __target_switch
    {
    case cuda:
        __intrinsic_asm "__threadfence_block()";
    case hlsl:
        __intrinsic_asm "GroupMemoryBarrier()";
    case glsl:
        __intrinsic_asm "subgroupMemoryBarrierShared()";
    case spirv:
        spirv_asm {
            // SubgroupMemory triggers vulkan validation layer error; 
            // WorkgroupMemory is the next level of granularity 
            OpMemoryBarrier Subgroup AcquireRelease|WorkgroupMemory
        };

    }
}

__glsl_extension(GL_KHR_shader_subgroup_basic) [require(glsl)] 
__spirv_version(1.3) [require(spirv)]
[ForceInline] public bool subgroupElect()
{
    __target_switch
    {
    case cuda:
        __intrinsic_asm "( (__activemask() & (__activemask()*-1)) == _getLaneId())";    
    case glsl:
    case spirv:
    case hlsl:
        return WaveIsFirstLane();

    }
}

// GL_KHR_shader_subgroup_vote

__glsl_extension(GL_KHR_shader_subgroup_vote) [require(glsl)] 
__spirv_version(1.3) [require(spirv)]
[ForceInline] public bool subgroupAll(bool value)
{

    return WaveActiveAllTrue(value);

}

__glsl_extension(GL_KHR_shader_subgroup_vote) [require(glsl)] 
__spirv_version(1.3) [require(spirv)]
[ForceInline] public bool subgroupAny(bool value)
{
    return WaveActiveAnyTrue(value);
    
}

__generic<T : __BuiltinType>
__glsl_extension(GL_KHR_shader_subgroup_vote) [require(glsl)] 
__spirv_version(1.3) [require(spirv)]
[ForceInline] public bool subgroupAllEqual(T value)
{
    shader_subgroup_preamble<T>();
    return WaveActiveAllEqual(value);
}

__generic<T : __BuiltinType, let N : int>
__glsl_extension(GL_KHR_shader_subgroup_vote) [require(glsl)] 
__spirv_version(1.3) [require(spirv)]
[ForceInline] public bool subgroupAllEqual(vector<T,N> value)
{
    shader_subgroup_preamble<T>();
    return WaveActiveAllEqual(value);
}

// GL_KHR_shader_subgroup_arithmetic

__generic<T : __BuiltinArithmeticType>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic) [require(glsl)] 
__spirv_version(1.3) [require(spirv)]
[ForceInline] public T subgroupAdd(T value)
{
    shader_subgroup_preamble<T>();
    return WaveActiveSum(value);
}

__generic<T : __BuiltinArithmeticType>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic) [require(glsl)] 
__spirv_version(1.3) [require(spirv)]
[ForceInline] public T subgroupMul(T value)
{
    shader_subgroup_preamble<T>();
    return WaveActiveProduct(value);
}

__generic<T : __BuiltinArithmeticType>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic) [require(glsl)] 
__spirv_version(1.3) [require(spirv)]
[ForceInline] public T subgroupMin(T value)
{
    shader_subgroup_preamble<T>();
    return WaveActiveMin(value);
}

__generic<T : __BuiltinArithmeticType>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic) [require(glsl)] 
__spirv_version(1.3) [require(spirv)]
[ForceInline] public T subgroupMax(T value)
{
    shader_subgroup_preamble<T>();
    return WaveActiveMax(value);
}

__generic<T : __BuiltinLogicalType>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic) [require(glsl)] 
__spirv_version(1.3) [require(spirv)]
[ForceInline] public T subgroupAnd(T value)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl: __intrinsic_asm "subgroupAnd($0)";
    case spirv:
        if (__isBool<T>()) {
            return spirv_asm {
                OpCapability GroupNonUniformArithmetic;
                OpGroupNonUniformLogicalAnd $$T result Subgroup 0 $value
            };
        }
        else {
            return spirv_asm {
                OpCapability GroupNonUniformArithmetic;
                OpGroupNonUniformBitwiseAnd $$T result Subgroup 0 $value
            };
        }
    }
}

__generic<T : __BuiltinLogicalType>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic) [require(glsl)] 
__spirv_version(1.3) [require(spirv)]
[ForceInline] public T subgroupOr(T value)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl: __intrinsic_asm "subgroupOr($0)";
    case spirv:
        if (__isBool<T>()) {
            return spirv_asm {
                OpCapability GroupNonUniformArithmetic;
                OpGroupNonUniformLogicalOr $$T result Subgroup 0 $value
            };
        }
        else {
            return spirv_asm {
                OpCapability GroupNonUniformArithmetic;
                OpGroupNonUniformBitwiseOr $$T result Subgroup 0 $value
            };
        }
    }
}

__generic<T : __BuiltinLogicalType>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic) [require(glsl)] 
__spirv_version(1.3) [require(spirv)]
[ForceInline] public T subgroupXor(T value)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl: __intrinsic_asm "subgroupXor($0)";
    case spirv:
        if (__isBool<T>()) {
            return spirv_asm {
                OpCapability GroupNonUniformArithmetic;
                OpGroupNonUniformLogicalXor $$T result Subgroup 0 $value
            };
        }
        else {
            return spirv_asm {
                OpCapability GroupNonUniformArithmetic;
                OpGroupNonUniformBitwiseXor $$T result Subgroup 0 $value
            };
        }
    }
}

__generic<T : __BuiltinArithmeticType>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic) [require(glsl)] 
__spirv_version(1.3) [require(spirv)]
[ForceInline] public T subgroupInclusiveAdd(T value)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl:
        __intrinsic_asm "subgroupInclusiveAdd($0)";
    case spirv:
        if (__isFloat<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformFAdd $$T result Subgroup InclusiveScan $value};
        else if (__isInt<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformIAdd $$T result Subgroup InclusiveScan $value};
        else return value;
    }
}

__generic<T : __BuiltinArithmeticType>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic) [require(glsl)] 
__spirv_version(1.3) [require(spirv)]
[ForceInline] public T subgroupInclusiveMul(T value)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl:
        __intrinsic_asm "subgroupInclusiveMul($0)";
    case spirv:
        if (__isFloat<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformFMul $$T result Subgroup InclusiveScan $value};
        else if (__isInt<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformIMul $$T result Subgroup InclusiveScan $value};
        else return value;
    }
}

__generic<T : __BuiltinArithmeticType>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic) [require(glsl)] 
__spirv_version(1.3) [require(spirv)]
[ForceInline] public T subgroupInclusiveMin(T value)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl:
        __intrinsic_asm "subgroupInclusiveMin($0)";
    case spirv:
        if (__isFloat<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformFMin $$T result Subgroup InclusiveScan $value};
        else if (__isSignedInt<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformSMin $$T result Subgroup InclusiveScan $value};
        else if (__isUnsignedInt<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformUMin $$T result Subgroup InclusiveScan $value};
        else return value;
    }
}

__generic<T : __BuiltinArithmeticType>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic) [require(glsl)] 
__spirv_version(1.3) [require(spirv)]
[ForceInline] public T subgroupInclusiveMax(T value)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl:
        __intrinsic_asm "subgroupInclusiveMax($0)";
    case spirv:
        if (__isFloat<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformFMax $$T result Subgroup InclusiveScan $value};
        else if (__isSignedInt<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformSMax $$T result Subgroup InclusiveScan $value};
        else if (__isUnsignedInt<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformUMax $$T result Subgroup InclusiveScan $value};
        else return value;
    }
}

__generic<T : __BuiltinLogicalType>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic) [require(glsl)] 
__spirv_version(1.3) [require(spirv)]
[ForceInline] public T subgroupInclusiveAnd(T value)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl: __intrinsic_asm "subgroupInclusiveAnd($0)";
    case spirv:
        if (__isBool<T>()) {
            return spirv_asm {
                OpCapability GroupNonUniformArithmetic;
                OpGroupNonUniformLogicalAnd $$T result Subgroup InclusiveScan $value
            };
        }
        else {
            return spirv_asm {
                OpCapability GroupNonUniformArithmetic;
                OpGroupNonUniformBitwiseAnd $$T result Subgroup InclusiveScan $value
            };
        }
    }
}

__generic<T : __BuiltinLogicalType>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic) [require(glsl)] 
__spirv_version(1.3) [require(spirv)]
[ForceInline] public T subgroupInclusiveOr(T value)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl: __intrinsic_asm "subgroupInclusiveOr($0)";
    case spirv:
        if (__isBool<T>()) {
            return spirv_asm {
                OpCapability GroupNonUniformArithmetic;
                OpGroupNonUniformLogicalOr $$T result Subgroup InclusiveScan $value
            };
        }
        else {
            return spirv_asm {
                OpCapability GroupNonUniformArithmetic;
                OpGroupNonUniformBitwiseOr $$T result Subgroup InclusiveScan $value
            };
        }
    }
}

__generic<T : __BuiltinLogicalType>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic) [require(glsl)] 
__spirv_version(1.3) [require(spirv)]
[ForceInline] public T subgroupInclusiveXor(T value)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl:
        __intrinsic_asm "subgroupInclusiveXor($0)";
    case spirv:
        if (__isBool<T>()) return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformLogicalXor $$T result Subgroup InclusiveScan $value};
        else return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformBitwiseXor $$T result Subgroup InclusiveScan $value};
    }
    return T(0);
}

__generic<T : __BuiltinArithmeticType>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic) [require(glsl)] 
__spirv_version(1.3) [require(spirv)]
[ForceInline] public T subgroupExclusiveAdd(T value)
{
    shader_subgroup_preamble<T>();
    return WavePrefixSum(value);
}


__generic<T : __BuiltinArithmeticType>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic) [require(glsl)] 
__spirv_version(1.3) [require(spirv)]
[ForceInline] public T subgroupExclusiveMul(T value)
{
    shader_subgroup_preamble<T>();
    return WavePrefixProduct(value);
}

__generic<T : __BuiltinArithmeticType>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic) [require(glsl)] 
__spirv_version(1.3) [require(spirv)]
[ForceInline] public T subgroupExclusiveMin(T value)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl:
        __intrinsic_asm "subgroupExclusiveMin($0)";
    case spirv:
        if (__isFloat<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformFMin $$T result Subgroup ExclusiveScan $value};
        else if (__isSignedInt<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformSMin $$T result Subgroup ExclusiveScan $value};
        else if (__isUnsignedInt<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformUMin $$T result Subgroup ExclusiveScan $value};
        else return value;
    }
}

__generic<T : __BuiltinArithmeticType>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic) [require(glsl)] 
__spirv_version(1.3) [require(spirv)]
[ForceInline] public T subgroupExclusiveMax(T value)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl:
        __intrinsic_asm "subgroupExclusiveMax($0)";
    case spirv:
        if (__isFloat<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformFMax $$T result Subgroup ExclusiveScan $value};
        else if (__isSignedInt<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformSMax $$T result Subgroup ExclusiveScan $value};
        else if (__isUnsignedInt<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformUMax $$T result Subgroup ExclusiveScan $value};
        else return value;
    }
}

__generic<T : __BuiltinLogicalType>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic) [require(glsl)] 
__spirv_version(1.3) [require(spirv)]
[ForceInline] public T subgroupExclusiveAnd(T value)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl: __intrinsic_asm "subgroupExclusiveAnd($0)";
    case spirv:
        if (__isBool<T>()) return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformLogicalAnd $$T result Subgroup ExclusiveScan $value};
        else return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformBitwiseAnd $$T result Subgroup ExclusiveScan $value};
    }
}

__generic<T : __BuiltinLogicalType>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic) [require(glsl)] 
__spirv_version(1.3) [require(spirv)]
[ForceInline] public T subgroupExclusiveOr(T value)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl: __intrinsic_asm "subgroupExclusiveOr($0)";
    case spirv:
        if (__isBool<T>()) return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformLogicalOr $$T result Subgroup ExclusiveScan $value};
        else return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformBitwiseOr $$T result Subgroup ExclusiveScan $value};
    }
}

__generic<T : __BuiltinLogicalType>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic) [require(glsl)] 
__spirv_version(1.3) [require(spirv)]
[ForceInline] public T subgroupExclusiveXor(T value)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl: __intrinsic_asm "subgroupExclusiveXor($0)";
    case spirv:
        if (__isBool<T>()) return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformLogicalXor $$T result Subgroup ExclusiveScan $value};
        else return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformBitwiseXor $$T result Subgroup ExclusiveScan $value};
    }
}

// GL_KHR_shader_subgroup_arithmetic
//note: this is a seperate section because it is so huge that the only reasonable way to implement this is to just regex replace code

__generic<T : __BuiltinArithmeticType, let N : int>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic) [require(glsl)] 
__spirv_version(1.3) [require(spirv)]
[ForceInline] public vector<T,N> subgroupAdd(vector<T,N> value)
{
    shader_subgroup_preamble<T>();
    return WaveActiveSum(value);
}

__generic<T : __BuiltinArithmeticType, let N : int>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic) [require(glsl)] 
__spirv_version(1.3) [require(spirv)]
[ForceInline] public vector<T,N> subgroupMul(vector<T,N> value)
{
    shader_subgroup_preamble<T>();
    return WaveActiveProduct(value);
}

__generic<T : __BuiltinArithmeticType, let N : int>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic) [require(glsl)] 
__spirv_version(1.3) [require(spirv)]
[ForceInline] public vector<T,N> subgroupMin(vector<T,N> value)
{
    shader_subgroup_preamble<T>();
    return WaveActiveMin(value);
}

__generic<T : __BuiltinArithmeticType, let N : int>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic) [require(glsl)] 
__spirv_version(1.3) [require(spirv)]
[ForceInline] public vector<T,N> subgroupMax(vector<T,N> value)
{
    shader_subgroup_preamble<T>();
    return WaveActiveMax(value);
}

__generic<T : __BuiltinLogicalType, let N : int>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic) [require(glsl)] 
__spirv_version(1.3) [require(spirv)]
[ForceInline] public vector<T,N> subgroupAnd(vector<T,N> value)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl: __intrinsic_asm "subgroupAnd($0)";
    case spirv:
        if (__isBool<T>()) {
            return spirv_asm {
                OpCapability GroupNonUniformArithmetic;
                OpGroupNonUniformLogicalAnd $$vector<T,N> result Subgroup 0 $value
            };
        }
        else {
            return spirv_asm {
                OpCapability GroupNonUniformArithmetic;
                OpGroupNonUniformBitwiseAnd $$vector<T,N> result Subgroup 0 $value
            };
        }

    }
}

__generic<T : __BuiltinLogicalType, let N : int>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic) [require(glsl)] 
__spirv_version(1.3) [require(spirv)]
[ForceInline] public vector<T,N> subgroupOr(vector<T,N> value)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl: __intrinsic_asm "subgroupOr($0)";
    case spirv:
        if (__isBool<T>()) {
            return spirv_asm {
                OpCapability GroupNonUniformArithmetic;
                OpGroupNonUniformLogicalOr $$vector<T,N> result Subgroup 0 $value
            };
        }
        else {
            return spirv_asm {
                OpCapability GroupNonUniformArithmetic;
                OpGroupNonUniformBitwiseOr $$vector<T,N> result Subgroup 0 $value
            };
        }

    }
}

__generic<T : __BuiltinLogicalType, let N : int>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic) [require(glsl)] 
__spirv_version(1.3) [require(spirv)]
[ForceInline] public vector<T,N> subgroupXor(vector<T,N> value)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl: __intrinsic_asm "subgroupXor($0)";
    case spirv:
        if (__isBool<T>()) {
            return spirv_asm {
                OpCapability GroupNonUniformArithmetic;
                OpGroupNonUniformLogicalXor $$vector<T,N> result Subgroup 0 $value
            };
        }
        else {
            return spirv_asm {
                OpCapability GroupNonUniformArithmetic;
                OpGroupNonUniformBitwiseXor $$vector<T,N> result Subgroup 0 $value
            };
        }
    }
}

__generic<T : __BuiltinArithmeticType, let N : int>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic) [require(glsl)] 
__spirv_version(1.3) [require(spirv)]
[ForceInline] public vector<T,N> subgroupInclusiveAdd(vector<T,N> value)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl:
        __intrinsic_asm "subgroupInclusiveAdd($0)";
    case spirv:
        if (__isFloat<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformFAdd $$vector<T,N> result Subgroup InclusiveScan $value};
        else if (__isInt<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformIAdd $$vector<T,N> result Subgroup InclusiveScan $value};
        else return value;
    }
}

__generic<T : __BuiltinArithmeticType, let N : int>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic) [require(glsl)] 
__spirv_version(1.3) [require(spirv)]
[ForceInline] public vector<T,N> subgroupInclusiveMul(vector<T,N> value)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl:
        __intrinsic_asm "subgroupInclusiveMul($0)";
    case spirv:
        if (__isFloat<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformFMul $$vector<T,N> result Subgroup InclusiveScan $value};
        else if (__isInt<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformIMul $$vector<T,N> result Subgroup InclusiveScan $value};
        else return value;
    }
}

__generic<T : __BuiltinArithmeticType, let N : int>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic) [require(glsl)] 
__spirv_version(1.3) [require(spirv)]
[ForceInline] public vector<T,N> subgroupInclusiveMin(vector<T,N> value)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl:
        __intrinsic_asm "subgroupInclusiveMin($0)";
    case spirv:
        if (__isFloat<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformFMin $$vector<T,N> result Subgroup InclusiveScan $value};
        else if (__isSignedInt<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformSMin $$vector<T,N> result Subgroup InclusiveScan $value};
        else if (__isUnsignedInt<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformUMin $$vector<T,N> result Subgroup InclusiveScan $value};
        else return value;
    }
}

__generic<T : __BuiltinArithmeticType, let N : int>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic) [require(glsl)] 
__spirv_version(1.3) [require(spirv)]
[ForceInline] public vector<T,N> subgroupInclusiveMax(vector<T,N> value)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl:
        __intrinsic_asm "subgroupInclusiveMax($0)";
    case spirv:
        if (__isFloat<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformFMax $$vector<T,N> result Subgroup InclusiveScan $value};
        else if (__isSignedInt<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformSMax $$vector<T,N> result Subgroup InclusiveScan $value};
        else if (__isUnsignedInt<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformUMax $$vector<T,N> result Subgroup InclusiveScan $value};
        else return value;
    }
}

__generic<T : __BuiltinLogicalType, let N : int>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic) [require(glsl)] 
__spirv_version(1.3) [require(spirv)]
[ForceInline] public vector<T,N> subgroupInclusiveAnd(vector<T,N> value)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl:
        __intrinsic_asm "subgroupInclusiveAnd($0)";
    case spirv:
        if (__isBool<T>()) return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformLogicalAnd $$vector<T,N> result Subgroup InclusiveScan $value};
        else return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformBitwiseAnd $$vector<T,N> result Subgroup InclusiveScan $value};
    }
}

__generic<T : __BuiltinLogicalType, let N : int>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic) [require(glsl)] 
__spirv_version(1.3) [require(spirv)]
[ForceInline] public vector<T,N> subgroupInclusiveOr(vector<T,N> value)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl:
        __intrinsic_asm "subgroupInclusiveOr($0)";
    case spirv:
        if (__isBool<T>()) return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformLogicalOr $$vector<T,N> result Subgroup InclusiveScan $value};
        else return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformBitwiseOr $$vector<T,N> result Subgroup InclusiveScan $value};
    }
}

__generic<T : __BuiltinLogicalType, let N : int>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic) [require(glsl)] 
__spirv_version(1.3) [require(spirv)]
[ForceInline] public vector<T,N> subgroupInclusiveXor(vector<T,N> value)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl:
        __intrinsic_asm "subgroupInclusiveXor($0)";
    case spirv:
        if (__isBool<T>()) return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformLogicalXor $$vector<T,N> result Subgroup InclusiveScan $value};
        else return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformBitwiseXor $$vector<T,N> result Subgroup InclusiveScan $value};
    }
}

__generic<T : __BuiltinArithmeticType, let N : int>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic) [require(glsl)] 
__spirv_version(1.3) [require(spirv)]
[ForceInline] public vector<T,N> subgroupExclusiveAdd(vector<T,N> value)
{
    shader_subgroup_preamble<T>();
    return WavePrefixSum(value);
}


__generic<T : __BuiltinArithmeticType, let N : int>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic) [require(glsl)] 
__spirv_version(1.3) [require(spirv)]
[ForceInline] public vector<T,N> subgroupExclusiveMul(vector<T,N> value)
{
    shader_subgroup_preamble<T>();
    return WavePrefixProduct(value);
}

__generic<T : __BuiltinArithmeticType, let N : int>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic) [require(glsl)] 
__spirv_version(1.3) [require(spirv)]
[ForceInline] public vector<T,N> subgroupExclusiveMin(vector<T,N> value)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl:
        __intrinsic_asm "subgroupExclusiveMin($0)";
    case spirv:
        if (__isFloat<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformFMin $$vector<T,N> result Subgroup ExclusiveScan $value};
        else if (__isSignedInt<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformSMin $$vector<T,N> result Subgroup ExclusiveScan $value};
        else if (__isUnsignedInt<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformUMin $$vector<T,N> result Subgroup ExclusiveScan $value};
        else return value;
    }
}

__generic<T : __BuiltinArithmeticType, let N : int>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic) [require(glsl)] 
__spirv_version(1.3) [require(spirv)]
[ForceInline] public vector<T,N> subgroupExclusiveMax(vector<T,N> value)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl:
        __intrinsic_asm "subgroupExclusiveMax($0)";
    case spirv:
        if (__isFloat<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformFMax $$vector<T,N> result Subgroup ExclusiveScan $value};
        else if (__isSignedInt<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformSMax $$vector<T,N> result Subgroup ExclusiveScan $value};
        else if (__isUnsignedInt<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformUMax $$vector<T,N> result Subgroup ExclusiveScan $value};
        else return value;
    }
}

__generic<T : __BuiltinLogicalType, let N : int>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic) [require(glsl)] 
__spirv_version(1.3) [require(spirv)]
[ForceInline] public vector<T,N> subgroupExclusiveAnd(vector<T,N> value)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl: __intrinsic_asm "subgroupExclusiveAnd($0)";
    case spirv:
        if (__isBool<T>()) return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformLogicalAnd $$vector<T,N> result Subgroup ExclusiveScan $value};
        else return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformBitwiseAnd $$vector<T,N> result Subgroup ExclusiveScan $value};
    }
}

__generic<T : __BuiltinLogicalType, let N : int>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic) [require(glsl)] 
__spirv_version(1.3) [require(spirv)]
[ForceInline] public vector<T,N> subgroupExclusiveOr(vector<T,N> value)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl: __intrinsic_asm "subgroupExclusiveOr($0)";
    case spirv:
        if (__isBool<T>()) return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformLogicalOr $$vector<T,N> result Subgroup ExclusiveScan $value};
        else return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformBitwiseOr $$vector<T,N> result Subgroup ExclusiveScan $value};
    }
}

__generic<T : __BuiltinLogicalType, let N : int>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic) [require(glsl)] 
__spirv_version(1.3) [require(spirv)]
[ForceInline] public vector<T,N> subgroupExclusiveXor(vector<T,N> value)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl: __intrinsic_asm "subgroupExclusiveXor($0)";
    case spirv:
        if (__isBool<T>()) return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformLogicalXor $$vector<T,N> result Subgroup ExclusiveScan $value};
        else return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformBitwiseXor $$vector<T,N> result Subgroup ExclusiveScan $value};
    }
}

// GL_KHR_shader_subgroup_ballot

__generic<T : __BuiltinType>
__glsl_extension(GL_KHR_shader_subgroup_ballot) [require(glsl)] 
__spirv_version(1.3) [require(spirv)]
[ForceInline] public T subgroupBroadcast(T value, uint id)
{
    shader_subgroup_preamble<T>();
    return WaveMaskBroadcastLaneAt(WaveGetActiveMask(), value, id);
}

__generic<T : __BuiltinType, let N : int>
__glsl_extension(GL_KHR_shader_subgroup_ballot) [require(glsl)] 
__spirv_version(1.3) [require(spirv)]
[ForceInline] public vector<T,N> subgroupBroadcast(vector<T,N> value, uint id)
{
    shader_subgroup_preamble<T>();
    return WaveMaskBroadcastLaneAt(WaveGetActiveMask(), value, id);
}

__generic<T : __BuiltinType>
__glsl_extension(GL_KHR_shader_subgroup_ballot) [require(glsl)] 
__spirv_version(1.3) [require(spirv)]
[ForceInline] public T subgroupBroadcastFirst(T value)
{
    shader_subgroup_preamble<T>();
    return WaveMaskReadLaneFirst(WaveGetActiveMask(), value);
}

__generic<T : __BuiltinType, let N : int>
__glsl_extension(GL_KHR_shader_subgroup_ballot) [require(glsl)] 
__spirv_version(1.3) [require(spirv)]
[ForceInline] public vector<T,N> subgroupBroadcastFirst(vector<T,N> value)
{
    shader_subgroup_preamble<T>();
    return WaveMaskReadLaneFirst(WaveGetActiveMask(), value);
}

// WaveMaskBallot is not the same; it force trunc's
__glsl_extension(GL_KHR_shader_subgroup_ballot) [require(glsl)] 
__spirv_version(1.3) [require(spirv)]
[ForceInline] public uvec4 subgroupBallot(bool value)
{
    return WaveActiveBallot(value);
}

// logic for HLSL and CUDA which lack InverseBalloc
// CUDA: works exclusivly 32 waves, therefore only need comp x
// HLSL:{
// 1. index into comp I want: index = trunc(float(lane)*(1/32))
// 2. lane & value[index]
// note: 1/32 wil be converted to multiplication
// we do 1/32 since 1 uint stores 32 threads 
// note 2: we have a waveLaneCount check because based on wave lane count we can determine if we can do a 
// fast path or slow path (know index is 0 or non 0)
// }
__glsl_extension(GL_KHR_shader_subgroup_ballot) [require(glsl)] 
__spirv_version(1.3) [require(spirv)]
[ForceInline] public bool subgroupInverseBallot(uvec4 value)
{
    __target_switch
    {
    case cuda:
        // only has 32 warps
        __intrinsic_asm "(($0).x >> (_getLaneId()) & 1)";
    case hlsl:
        // much like _WaveCountBits, but here we hope that we hit case 0; we can then avoid the expensive logic
        const uint waveLaneCount = WaveGetLaneCount();
        switch ((waveLaneCount - 1) / 32)
        {
        case 0:
            __intrinsic_asm "(($0)[0] >> WaveGetLaneIndex()) & 1)";
        case 1:
        case 2:
        case 3:
            __intrinsic_asm "((($0)[uint(float(WaveGetLaneIndex())*0.03125f)] >> WaveGetLaneIndex()) & 1)";
        }
    case glsl:
        __intrinsic_asm "subgroupInverseBallot($0)";
    case spirv:
        return spirv_asm {
                OpCapability GroupNonUniformBallot; 
                OpGroupNonUniformInverseBallot $$bool result Subgroup $value
        };
    }
    return false;
}

// same logic as subgroupInverseBallot
__glsl_extension(GL_KHR_shader_subgroup_ballot) [require(glsl)] 
__spirv_version(1.3) [require(spirv)]
[ForceInline] public bool subgroupBallotBitExtract(uvec4 value, uint index)
{
    __target_switch
    {
    case cuda:
        __intrinsic_asm "($1 & ($0).x) != 0";
    case hlsl:
        const uint waveLaneCount = WaveGetLaneCount();
        switch ((waveLaneCount - 1) / 32)
        {
        case 0:
            __intrinsic_asm "($0)[0] & ($1)";
        case 1:
        case 2:
        case 3:
            __intrinsic_asm "($0)[uint(float($1)*0.03125f)] & ($1)";
        }
    case glsl:
        __intrinsic_asm "subgroupBallotBitExtract($0, $1)";
    case spirv:
        return spirv_asm {
                OpCapability GroupNonUniformBallot; 
                OpGroupNonUniformBallotBitExtract $$bool result Subgroup $value $index
        };
    }
    return false;
}


// the count is only supposed to use uvec4 values within bottom bits of subgroup launched, not a simple countbits
__glsl_extension(GL_KHR_shader_subgroup_ballot) [require(glsl)] 
__spirv_version(1.3) [require(spirv)]
[ForceInline] public uint subgroupBallotBitCount(uvec4 value)
{
    __target_switch
    {
    case glsl:
        __intrinsic_asm "subgroupBallotBitCount($0)";
    case spirv:
        return spirv_asm {
            OpCapability GroupNonUniformBallot; 
            OpGroupNonUniformBallotBitCount $$uint result Subgroup Reduce $value
        };
    }
}

__glsl_extension(GL_KHR_shader_subgroup_ballot) [require(glsl)] 
__spirv_version(1.3) [require(spirv)]
[ForceInline] public uint subgroupBallotInclusiveBitCount(uvec4 value)
{
    __target_switch
    {
    case glsl:
        __intrinsic_asm "subgroupBallotInclusiveBitCount($0)";
    case spirv:
        return spirv_asm {
            OpCapability GroupNonUniformBallot; 
            OpGroupNonUniformBallotBitCount $$uint result Subgroup InclusiveScan $value
        };
    }
}

__glsl_extension(GL_KHR_shader_subgroup_ballot) [require(glsl)] 
__spirv_version(1.3) [require(spirv)]
[ForceInline] public uint subgroupBallotExclusiveBitCount(uvec4 value)
{
    __target_switch
    {
    case glsl:
        __intrinsic_asm "subgroupBallotExclusiveBitCount($0)";
    case spirv:
        return spirv_asm {
            OpCapability GroupNonUniformBallot; 
            OpGroupNonUniformBallotBitCount $$uint result Subgroup ExclusiveScan $value
        };
    }
}

__glsl_extension(GL_KHR_shader_subgroup_ballot) [require(glsl)] 
__spirv_version(1.3) [require(spirv)]
[ForceInline] public uint subgroupBallotFindLSB(uvec4 value)
{
    __target_switch
    {
    case glsl:
        __intrinsic_asm "subgroupBallotFindLSB($0)";
    case spirv:
        return spirv_asm {
            OpCapability GroupNonUniformBallot; 
            OpGroupNonUniformBallotFindLSB $$uint result Subgroup $value
        };
    }
}

__glsl_extension(GL_KHR_shader_subgroup_ballot) [require(glsl)] 
__spirv_version(1.3) [require(spirv)]
[ForceInline] public uint subgroupBallotFindMSB(uvec4 value)
{
    __target_switch
    {
    case glsl:
        __intrinsic_asm "subgroupBallotFindMSB($0)";
    case spirv:
        return spirv_asm {
            OpCapability GroupNonUniformBallot; 
            OpGroupNonUniformBallotFindMSB $$uint result Subgroup $value
        };
    }
}

// GL_KHR_shader_subgroup_shuffle

__generic<T : __BuiltinType>
__glsl_extension(GL_KHR_shader_subgroup_shuffle) [require(glsl)] 
__spirv_version(1.3) [require(spirv)]
[ForceInline] public T subgroupShuffle(T value, uint index)
{
    shader_subgroup_preamble<T>();
    return WaveShuffle(value, index);
}

__generic<T : __BuiltinType>
__glsl_extension(GL_KHR_shader_subgroup_shuffle) [require(glsl)] 
__spirv_version(1.3) [require(spirv)]
[ForceInline] public T subgroupShuffleXor(T value, uint mask)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl:
        __intrinsic_asm "subgroupShuffleXor($0,$1)";
    case spirv:
        return spirv_asm {
            OpCapability GroupNonUniformBallot; 
            OpGroupNonUniformShuffleXor $$T result Subgroup $value $mask
        };
    }
}

__generic<T : __BuiltinType, let N : int>
__glsl_extension(GL_KHR_shader_subgroup_shuffle) [require(glsl)] 
__spirv_version(1.3) [require(spirv)]
[ForceInline] public vector<T,N> subgroupShuffle(vector<T,N> value, uint index)
{
    shader_subgroup_preamble<T>();
    return WaveShuffle(value, index);
}

__generic<T : __BuiltinType, let N : int>
__glsl_extension(GL_KHR_shader_subgroup_shuffle) [require(glsl)] 
__spirv_version(1.3) [require(spirv)]
[ForceInline] public vector<T,N> subgroupShuffleXor(vector<T,N> value, uint mask)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl:
        __intrinsic_asm "subgroupShuffleXor($0,$1)";
    case spirv:
        return spirv_asm {
            OpCapability GroupNonUniformBallot; 
            OpGroupNonUniformShuffleXor $$vector<T,N> result Subgroup $value $mask
        };
    }
}


// GL_KHR_shader_subgroup_shuffle_relative

__generic<T : __BuiltinType>
__glsl_extension(GL_KHR_shader_subgroup_shuffle_relative) [require(glsl)] 
__spirv_version(1.3) [require(spirv)]
[ForceInline] public T subgroupShuffleUp(T value, uint delta)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl:
        __intrinsic_asm "subgroupShuffleUp($0, $1)";
    case spirv:
        return spirv_asm {
            OpCapability GroupNonUniformShuffleRelative;
            OpGroupNonUniformShuffleUp $$T result Subgroup $value $delta
        };
    }
}

__generic<T : __BuiltinType>
__glsl_extension(GL_KHR_shader_subgroup_shuffle_relative) [require(glsl)] 
__spirv_version(1.3) [require(spirv)]
[ForceInline] public T subgroupShuffleDown(T value, uint delta)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl:
        __intrinsic_asm "subgroupShuffleDown($0, $1)";
    case spirv:
        return spirv_asm {
            OpCapability GroupNonUniformShuffleRelative; 
            OpGroupNonUniformShuffleDown $$T result Subgroup $value $delta
        };
    }
}


__generic<T : __BuiltinType, let N : int>
__glsl_extension(GL_KHR_shader_subgroup_shuffle_relative) [require(glsl)] 
__spirv_version(1.3) [require(spirv)]
[ForceInline] public vector<T,N> subgroupShuffleUp(vector<T,N> value, uint delta)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl:
        __intrinsic_asm "subgroupShuffleUp($0, $1)";
    case spirv:
        return spirv_asm {
            OpCapability GroupNonUniformShuffleRelative;
            OpGroupNonUniformShuffleUp $$vector<T,N> result Subgroup $value $delta
        };
    }
}

__generic<T : __BuiltinType, let N : int>
__glsl_extension(GL_KHR_shader_subgroup_shuffle_relative) [require(glsl)] 
__spirv_version(1.3) [require(spirv)]
[ForceInline] public vector<T,N> subgroupShuffleDown(vector<T,N> value, uint delta)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl:
        __intrinsic_asm "subgroupShuffleDown($0, $1)";
    case spirv:
        return spirv_asm {
            OpCapability GroupNonUniformShuffleRelative;
            OpGroupNonUniformShuffleDown $$vector<T,N> result Subgroup $value $delta
        };
    }
}
// GL_KHR_shader_subgroup_clustered

__generic<T : __BuiltinArithmeticType>
__glsl_extension(GL_KHR_shader_subgroup_clustered) [require(glsl)] 
__spirv_version(1.3) [require(spirv)]
[ForceInline] public T subgroupClusteredAdd(T value, uint clusterSize)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl:
        __intrinsic_asm "subgroupClusteredAdd($0, $1)";
    case spirv:
        if (__isFloat<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpCapability GroupNonUniformClustered; OpGroupNonUniformFAdd $$T result Subgroup ClusteredReduce $value $clusterSize};
        else if (__isInt<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpCapability GroupNonUniformClustered; OpGroupNonUniformIAdd $$T result Subgroup ClusteredReduce $value $clusterSize};
        else return value;
    }
}

__generic<T : __BuiltinArithmeticType>
__glsl_extension(GL_KHR_shader_subgroup_clustered) [require(glsl)] 
__spirv_version(1.3) [require(spirv)]
[ForceInline] public T subgroupClusteredMul(T value, uint clusterSize)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl:
        __intrinsic_asm "subgroupClusteredMul($0, $1)";
    case spirv:
        if (__isFloat<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpCapability GroupNonUniformClustered; OpGroupNonUniformFMul $$T result Subgroup ClusteredReduce $value $clusterSize};
        else if (__isInt<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpCapability GroupNonUniformClustered; OpGroupNonUniformIMul $$T result Subgroup ClusteredReduce $value $clusterSize};
        else return value;
    } 
}

__generic<T : __BuiltinArithmeticType>
__glsl_extension(GL_KHR_shader_subgroup_clustered) [require(glsl)] 
__spirv_version(1.3) [require(spirv)]
[ForceInline] public T subgroupClusteredMin(T value, uint clusterSize)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl:
        __intrinsic_asm "subgroupClusteredMin($0, $1)";
    case spirv:
        if (__isFloat<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpCapability GroupNonUniformClustered; OpGroupNonUniformFMin $$T result Subgroup ClusteredReduce $value $clusterSize};
        else if (__isSignedInt<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpCapability GroupNonUniformClustered; OpGroupNonUniformSMin $$T result Subgroup ClusteredReduce $value $clusterSize};
        else if (__isUnsignedInt<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpCapability GroupNonUniformClustered; OpGroupNonUniformUMin $$T result Subgroup ClusteredReduce $value $clusterSize};
        else return value;
    }
}

__generic<T : __BuiltinArithmeticType>
__glsl_extension(GL_KHR_shader_subgroup_clustered) [require(glsl)] 
__spirv_version(1.3) [require(spirv)]
[ForceInline] public T subgroupClusteredMax(T value, uint clusterSize)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl:
        __intrinsic_asm "subgroupClusteredMax($0, $1)";
    case spirv:
        if (__isFloat<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpCapability GroupNonUniformClustered; OpGroupNonUniformFMax $$T result Subgroup ClusteredReduce $value $clusterSize};
        else if (__isSignedInt<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpCapability GroupNonUniformClustered; OpGroupNonUniformSMax $$T result Subgroup ClusteredReduce $value $clusterSize};
        else if (__isUnsignedInt<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpCapability GroupNonUniformClustered;  OpGroupNonUniformUMax $$T result Subgroup ClusteredReduce $value $clusterSize};
        else return value;
    }
}

__generic<T : __BuiltinLogicalType>
__glsl_extension(GL_KHR_shader_subgroup_clustered) [require(glsl)] 
__spirv_version(1.3) [require(spirv)]
[ForceInline] public T subgroupClusteredAnd(T value, uint clusterSize)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl:
        __intrinsic_asm "subgroupClusteredAnd($0, $1)";
    case spirv:
        if (__isBool<T>()) return spirv_asm {OpCapability GroupNonUniformArithmetic; OpCapability GroupNonUniformClustered; OpGroupNonUniformLogicalAnd $$T result Subgroup ClusteredReduce $value $clusterSize};
        else return spirv_asm {OpCapability GroupNonUniformArithmetic; OpCapability GroupNonUniformClustered; OpGroupNonUniformBitwiseAnd $$T result Subgroup ClusteredReduce $value $clusterSize};
    }
}

__generic<T : __BuiltinLogicalType>
__glsl_extension(GL_KHR_shader_subgroup_clustered) [require(glsl)] 
__spirv_version(1.3) [require(spirv)]
[ForceInline] public T subgroupClusteredOr(T value, uint clusterSize)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl:
        __intrinsic_asm "subgroupClusteredOr($0, $1)";
    case spirv:
        if (__isBool<T>()) return spirv_asm {OpCapability GroupNonUniformArithmetic; OpCapability GroupNonUniformClustered; OpGroupNonUniformLogicalOr $$T result Subgroup ClusteredReduce $value $clusterSize};
        else return spirv_asm {OpCapability GroupNonUniformArithmetic; OpCapability GroupNonUniformClustered; OpGroupNonUniformBitwiseOr $$T result Subgroup ClusteredReduce $value $clusterSize};
    }
}



__generic<T : __BuiltinLogicalType>
__glsl_extension(GL_KHR_shader_subgroup_clustered) [require(glsl)] 
__spirv_version(1.3) [require(spirv)]
[ForceInline] public T subgroupClusteredXor(T value, uint clusterSize)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl:
        __intrinsic_asm "subgroupClusteredXor($0, $1)";
    case spirv:
        if (__isBool<T>()) return spirv_asm {OpCapability GroupNonUniformArithmetic; OpCapability GroupNonUniformClustered; OpGroupNonUniformLogicalXor $$T result Subgroup ClusteredReduce $value $clusterSize};
        else return spirv_asm {OpCapability GroupNonUniformArithmetic; OpCapability GroupNonUniformClustered; OpGroupNonUniformBitwiseXor $$T result Subgroup ClusteredReduce $value $clusterSize};
    }
}



__generic<T : __BuiltinArithmeticType, let N : int>
__glsl_extension(GL_KHR_shader_subgroup_clustered) [require(glsl)] 
__spirv_version(1.3) [require(spirv)]
[ForceInline] public vector<T,N> subgroupClusteredAdd(vector<T,N> value, uint clusterSize)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl:
        __intrinsic_asm "subgroupClusteredAdd($0, $1)";
    case spirv:
        if (__isFloat<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpCapability GroupNonUniformClustered; 
            OpGroupNonUniformFAdd $$vector<T,N> result Subgroup ClusteredReduce $value $clusterSize};
        else if (__isInt<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpCapability GroupNonUniformClustered; OpGroupNonUniformIAdd $$vector<T,N> result Subgroup ClusteredReduce $value $clusterSize};
        else return value;
    }
}

__generic<T : __BuiltinArithmeticType, let N : int>
__glsl_extension(GL_KHR_shader_subgroup_clustered) [require(glsl)] 
__spirv_version(1.3) [require(spirv)]
[ForceInline] public vector<T,N> subgroupClusteredMul(vector<T,N> value, uint clusterSize)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl:
        __intrinsic_asm "subgroupClusteredMul($0, $1)";
    case spirv:
        if (__isFloat<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpCapability GroupNonUniformClustered; OpGroupNonUniformFMul $$vector<T,N> result Subgroup ClusteredReduce $value $clusterSize};
        else if (__isInt<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpCapability GroupNonUniformClustered; OpGroupNonUniformIMul $$vector<T,N> result Subgroup ClusteredReduce $value $clusterSize};
        else return value;
    } 
}

__generic<T : __BuiltinArithmeticType, let N : int>
__glsl_extension(GL_KHR_shader_subgroup_clustered) [require(glsl)] 
__spirv_version(1.3) [require(spirv)]
[ForceInline] public vector<T,N> subgroupClusteredMin(vector<T,N> value, uint clusterSize)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl:
        __intrinsic_asm "subgroupClusteredMin($0, $1)";
    case spirv:
        if (__isFloat<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpCapability GroupNonUniformClustered; OpGroupNonUniformFMin $$vector<T,N> result Subgroup ClusteredReduce $value $clusterSize};
        else if (__isSignedInt<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpCapability GroupNonUniformClustered; OpGroupNonUniformSMin $$vector<T,N> result Subgroup ClusteredReduce $value $clusterSize};
        else if (__isUnsignedInt<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpCapability GroupNonUniformClustered; OpGroupNonUniformUMin $$vector<T,N> result Subgroup ClusteredReduce $value $clusterSize};
        else return value;
    }
}

__generic<T : __BuiltinArithmeticType, let N : int>
__glsl_extension(GL_KHR_shader_subgroup_clustered) [require(glsl)] 
__spirv_version(1.3) [require(spirv)]
[ForceInline] public vector<T,N> subgroupClusteredMax(vector<T,N> value, uint clusterSize)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl:
        __intrinsic_asm "subgroupClusteredMax($0, $1)";
    case spirv:
        if (__isFloat<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpCapability GroupNonUniformClustered; OpGroupNonUniformFMax $$vector<T,N> result Subgroup ClusteredReduce $value $clusterSize};
        else if (__isSignedInt<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpCapability GroupNonUniformClustered; OpGroupNonUniformSMax $$vector<T,N> result Subgroup ClusteredReduce $value $clusterSize};
        else if (__isUnsignedInt<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpCapability GroupNonUniformClustered;  OpGroupNonUniformUMax $$vector<T,N> result Subgroup ClusteredReduce $value $clusterSize};
        else return value;
    }
}

__generic<T : __BuiltinLogicalType, let N : int>
__glsl_extension(GL_KHR_shader_subgroup_clustered) [require(glsl)] 
__spirv_version(1.3) [require(spirv)]
[ForceInline] public vector<T,N> subgroupClusteredAnd(vector<T,N> value, uint clusterSize)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl:
        __intrinsic_asm "subgroupClusteredAnd($0, $1)";
    case spirv:
        if (__isBool<T>()) return spirv_asm {OpCapability GroupNonUniformArithmetic; OpCapability GroupNonUniformClustered; OpGroupNonUniformLogicalAnd $$vector<T,N> result Subgroup ClusteredReduce $value $clusterSize};
        else return spirv_asm {OpCapability GroupNonUniformArithmetic; OpCapability GroupNonUniformClustered; OpGroupNonUniformBitwiseAnd $$vector<T,N> result Subgroup ClusteredReduce $value $clusterSize};
    }
}

__generic<T : __BuiltinLogicalType, let N : int>
__glsl_extension(GL_KHR_shader_subgroup_clustered) [require(glsl)] 
__spirv_version(1.3) [require(spirv)]
[ForceInline] public vector<T,N> subgroupClusteredOr(vector<T,N> value, uint clusterSize)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl:
        __intrinsic_asm "subgroupClusteredOr($0, $1)";
    case spirv:
        if (__isBool<T>()) return spirv_asm {OpCapability GroupNonUniformArithmetic; OpCapability GroupNonUniformClustered; OpGroupNonUniformLogicalOr $$vector<T,N> result Subgroup ClusteredReduce $value $clusterSize};
        else return spirv_asm {OpCapability GroupNonUniformArithmetic; OpCapability GroupNonUniformClustered; OpGroupNonUniformBitwiseOr $$vector<T,N> result Subgroup ClusteredReduce $value $clusterSize};
    }
}

__generic<T : __BuiltinLogicalType, let N : int>
__glsl_extension(GL_KHR_shader_subgroup_clustered) [require(glsl)] 
__spirv_version(1.3) [require(spirv)]
[ForceInline] public vector<T,N> subgroupClusteredXor(vector<T,N> value, uint clusterSize)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl:
        __intrinsic_asm "subgroupClusteredXor($0, $1)";
    case spirv:
        if (__isBool<T>()) return spirv_asm {OpCapability GroupNonUniformArithmetic; OpCapability GroupNonUniformClustered; OpGroupNonUniformLogicalXor $$vector<T,N> result Subgroup ClusteredReduce $value $clusterSize};
        else return spirv_asm {OpCapability GroupNonUniformArithmetic; OpCapability GroupNonUniformClustered; OpGroupNonUniformBitwiseXor $$vector<T,N> result Subgroup ClusteredReduce $value $clusterSize};
    }
}

// GL_KHR_shader_subgroup_quad

__generic<T : __BuiltinType>
__glsl_extension(GL_KHR_shader_subgroup_quad) [require(glsl)] 
__spirv_version(1.3) [require(spirv)]
[ForceInline] public T subgroupQuadBroadcast(T value, uint id)
{
    shader_subgroup_preamble<T>();
    return QuadReadLaneAt(value, id);
}

__generic<T : __BuiltinType>
__glsl_extension(GL_KHR_shader_subgroup_quad) [require(glsl)] 
__spirv_version(1.3) [require(spirv)]
[ForceInline] public T subgroupQuadSwapHorizontal(T value)
{
    shader_subgroup_preamble<T>();
    return QuadReadAcrossX(value);
}

__generic<T : __BuiltinType>
__glsl_extension(GL_KHR_shader_subgroup_quad) [require(glsl)] 
__spirv_version(1.3) [require(spirv)]
[ForceInline] public T subgroupQuadSwapVertical(T value)
{
    shader_subgroup_preamble<T>();
    return QuadReadAcrossY(value);
}

__generic<T : __BuiltinType>
__glsl_extension(GL_KHR_shader_subgroup_quad) [require(glsl)] 
__spirv_version(1.3) [require(spirv)]
[ForceInline] public T subgroupQuadSwapDiagonal(T value)
{
    shader_subgroup_preamble<T>();
    return QuadReadAcrossDiagonal(value);
}


__generic<T : __BuiltinType, let N : int>
__glsl_extension(GL_KHR_shader_subgroup_quad) [require(glsl)] 
__spirv_version(1.3) [require(spirv)]
[ForceInline] public vector<T,N> subgroupQuadBroadcast(vector<T,N> value, uint id)
{
    shader_subgroup_preamble<T>();
    return QuadReadLaneAt(value, id);
}

__generic<T : __BuiltinType, let N : int>
__glsl_extension(GL_KHR_shader_subgroup_quad) [require(glsl)] 
__spirv_version(1.3) [require(spirv)]
[ForceInline] public vector<T,N> subgroupQuadSwapHorizontal(vector<T,N> value)
{
    shader_subgroup_preamble<T>();
    return QuadReadAcrossX(value);
}

__generic<T : __BuiltinType, let N : int>
__glsl_extension(GL_KHR_shader_subgroup_quad) [require(glsl)] 
__spirv_version(1.3) [require(spirv)]
[ForceInline] public vector<T,N> subgroupQuadSwapVertical(vector<T,N> value)
{
    shader_subgroup_preamble<T>();
    return QuadReadAcrossY(value);
}

__generic<T : __BuiltinType, let N : int>
__glsl_extension(GL_KHR_shader_subgroup_quad) [require(glsl)] 
__spirv_version(1.3) [require(spirv)]
[ForceInline] public vector<T,N> subgroupQuadSwapDiagonal(vector<T,N> value)
{
    shader_subgroup_preamble<T>();
    return QuadReadAcrossDiagonal(value);
}

//// GLSL atomic

// The following type internally is a Shader Storage Buffer 
// as per GL_EXT_vulkan_glsl_relaxed
__magic_type(GLSLAtomicUintType)
__intrinsic_type($(kIROp_GLSLAtomicUintType))
public struct atomic_uint
{
};

// tier of float refers to atomic extension support of float1 or float2.
// if we are inside a atomic_float function we will run the check for float1 tier
// types and operations to enable the according ext needed for these operations

__generic<T : __BuiltinType>
[ForceInline] void typeRequireChecks_atomic_using_float0_tier() 
{
    __target_switch
    {
    case glsl:
    {
        if (__type_equals<T, uint64_t>() || __type_equals<T, int64_t>()) 
            __requireGLSLExtension("GL_EXT_shader_atomic_int64");  
    }
    case spirv:
        return; 
    }
}
__generic<T : __BuiltinType>
[ForceInline] void typeRequireChecks_atomic_using_float1_tier() 
{
    __target_switch
    {
    case glsl:
    {
        if (__type_equals<T, float>()) 
            __requireGLSLExtension("GL_EXT_shader_atomic_float");
        else if (__type_equals<T, half>() || __type_equals<T, float16_t>())
        {
            __requireGLSLExtension("GL_EXT_shader_atomic_float2");
            __requireGLSLExtension("GL_EXT_shader_explicit_arithmetic_types");
        }
        else if (__type_equals<T, double>()) 
            __requireGLSLExtension("GL_EXT_shader_atomic_float");
        else if (__type_equals<T, uint64_t>() || __type_equals<T, int64_t>())
            __requireGLSLExtension("GL_EXT_shader_atomic_int64");
    }
    case spirv:
        return;
    }
}
__generic<T : __BuiltinType>
[ForceInline] void typeRequireChecks_atomic_using_float2_tier() 
{
    __target_switch
    {
    case glsl:
    {    
        if (__type_equals<T, float>()) 
            __requireGLSLExtension("GL_EXT_shader_atomic_float2");
        else if (__type_equals<T, half>() || __type_equals<T, float16_t>())
        {
            __requireGLSLExtension("GL_EXT_shader_atomic_float2");
            __requireGLSLExtension("GL_EXT_shader_explicit_arithmetic_types");
        }
        else if (__type_equals<T, double>()) 
            __requireGLSLExtension("GL_EXT_shader_atomic_float2");
        else if (__type_equals<T, uint64_t>() || __type_equals<T, int64_t>()) 
             __requireGLSLExtension("GL_EXT_shader_atomic_int64");        
    }
    case spirv: 
        return; 
    }
}

__generic<T : __BuiltinType>
void typeRequireChecks_atomic_using_add()
{
    __target_switch
    {
    case glsl:
        return;
    case spirv:
    {
        if (__type_equals<T, float>())
        {
            spirv_asm
            {
                OpExtension "SPV_EXT_shader_atomic_float_add"; 
                OpCapability AtomicFloat32AddEXT
            };
        }
        else if (__type_equals<T, half>() 
            || __type_equals<T, float16_t>())
        {
            spirv_asm
            {
                OpExtension "SPV_EXT_shader_atomic_float_add"; 
                OpCapability AtomicFloat16AddEXT
            };
        }
        else if (__type_equals<T, double>())
        {
            spirv_asm
            {
                OpExtension "SPV_EXT_shader_atomic_float_add"; 
                OpCapability AtomicFloat64AddEXT
            };
        }
        else if (__type_equals<T, uint64_t>()
            || __type_equals<T, int64_t>())
        {
            spirv_asm
            {
                OpCapability Int64Atomics
            };
        }
    }    
    }
}
__generic<T : __BuiltinType>
void typeRequireChecks_atomic_using_MinMax()
{
    __target_switch
    {
    case glsl:
        return;
    case spirv:
    {
        if (__type_equals<T, float>())
        {
            spirv_asm
            {
                OpExtension "SPV_EXT_shader_atomic_float_add"; 
                OpCapability AtomicFloat32MinMaxEXT
            };
        }
        else if (__type_equals<T, half>() 
            || __type_equals<T, float16_t>())
        {
            spirv_asm
            {
                OpExtension "SPV_EXT_shader_atomic_float_add"; 
                OpCapability AtomicFloat16MinMaxEXT
            };
        }
        else if (__type_equals<T, double>())
        {
            spirv_asm
            {
                OpExtension "SPV_EXT_shader_atomic_float_add"; 
                OpCapability AtomicFloat64MinMaxEXT
            };
        }
        else if (__type_equals<T, uint64_t>()
            || __type_equals<T, int64_t>())
        {
            spirv_asm
            { 
                OpCapability Int64Atomics
            };
        }
    }
    }
}
__generic<T : __BuiltinType>
[ForceInline] void typeRequireChecks_atomic_using_Logical_CAS() 
{
    __target_switch
    {
    case glsl:
        return;
    case spirv:
    {
        if (__type_equals<T, uint64_t>()
            || __type_equals<T, int64_t>())
        {
            spirv_asm
            { 
                OpCapability Int64Atomics
            };
        }
    }
    }
}

${{{{
static const struct {
    const char* name;
    const char* classType;
    const char *subclassType;
    const char *suffix;
    const bool isFloat;
} atomics[] =
    {
        { 
            "uint", "I", "U", "", false
        },
        { 
            "uint64_t", "I", "U", "", false
        },
        { 
            "int", "I", "S", "", false
        },
        { 
            "int64_t", "I", "S", "", false
        },
        { 
            "float16_t", "F", "F", "EXT", true
        },
        { 
            "float", "F", "F", "EXT", true
        },
        { 
            "double", "F", "F", "EXT", true
        },
    };
for (const auto& item : atomics) 
{
}}}}

__spirv_version(1.0)[require(spirv)]
__glsl_version(430) [require(glsl)]
[ForceInline] public $(item.name) atomicAdd(inout $(item.name) mem, $(item.name) data)
{
    typeRequireChecks_atomic_using_float1_tier<$(item.name)>();
    typeRequireChecks_atomic_using_add<$(item.name)>();
    __target_switch
    {
    case glsl: __intrinsic_asm "atomicAdd($0, $1)";
    case spirv:
        return spirv_asm
        {
            OpAtomic$(item.classType)Add$(item.suffix) $$$(item.name) result &mem Device UniformMemory $data
        };
    }
}

__spirv_version(1.0)[require(spirv)]
__glsl_version(430) [require(glsl)]
[ForceInline] public $(item.name) atomicMin(inout $(item.name) mem, $(item.name) data)
{
    typeRequireChecks_atomic_using_float2_tier<$(item.name)>();
    typeRequireChecks_atomic_using_MinMax<$(item.name)>();
    __target_switch
    {
    case glsl: __intrinsic_asm "atomicMin($0, $1)";
    case spirv:
        return spirv_asm
        {
            OpAtomic$(item.subclassType)Min$(item.suffix) $$$(item.name) result &mem Device UniformMemory $data
        };
    }
}

__spirv_version(1.0)[require(spirv)]
__glsl_version(430) [require(glsl)]
[ForceInline] public $(item.name) atomicMax(inout $(item.name) mem, $(item.name) data)
{
    typeRequireChecks_atomic_using_float2_tier<$(item.name)>();
    typeRequireChecks_atomic_using_MinMax<$(item.name)>();
    __target_switch
    {
    case glsl: __intrinsic_asm "atomicMax($0, $1)";
    case spirv:
        return spirv_asm
        {
            OpAtomic$(item.subclassType)Max$(item.suffix) $$$(item.name) result &mem Device UniformMemory $data
        };
    }
}

__spirv_version(1.0)[require(spirv)]
__glsl_version(430) [require(glsl)]
[ForceInline] public $(item.name) atomicExchange(inout $(item.name) mem, $(item.name) data)
{
    typeRequireChecks_atomic_using_float1_tier<$(item.name)>();
    __target_switch
    {
    case glsl: __intrinsic_asm "atomicExchange($0, $1)";
    case spirv:
        return spirv_asm
        {
            OpAtomicExchange $$$(item.name) result &mem Device UniformMemory $data
        };
    }
}

${{{{
if(item.isFloat) 
    continue;
}}}}

__spirv_version(1.0)[require(spirv)]
__glsl_version(430) [require(glsl)]
[ForceInline] public $(item.name) atomicAnd(inout $(item.name) mem, $(item.name) data)
{
    typeRequireChecks_atomic_using_float0_tier<$(item.name)>();
    typeRequireChecks_atomic_using_Logical_CAS<$(item.name)>();
    __target_switch
    {
    case glsl:
    {
        __intrinsic_asm "atomicAnd($0, $1)";
    }
    case spirv:
        return spirv_asm
        {
            OpAtomicAnd $$$(item.name) result &mem Device UniformMemory $data
        };
    }
}

__spirv_version(1.0)[require(spirv)]
__glsl_version(430) [require(glsl)]
[ForceInline] public $(item.name) atomicOr(inout $(item.name) mem, $(item.name) data)
{
    typeRequireChecks_atomic_using_float0_tier<$(item.name)>();
    typeRequireChecks_atomic_using_Logical_CAS<$(item.name)>();
    __target_switch
    {
    case glsl: __intrinsic_asm "atomicOr($0, $1)";
    case spirv:
        return spirv_asm
        {
            OpAtomicOr $$$(item.name) result &mem Device UniformMemory $data
        };
    }
}

__spirv_version(1.0)[require(spirv)]
__glsl_version(430) [require(glsl)]
[ForceInline] public $(item.name) atomicXor(inout $(item.name) mem, $(item.name) data)
{
    typeRequireChecks_atomic_using_float0_tier<$(item.name)>();
    typeRequireChecks_atomic_using_Logical_CAS<$(item.name)>();
    __target_switch
    {
    case glsl: __intrinsic_asm "atomicXor($0, $1)";
    case spirv:
        return spirv_asm
        {
            OpAtomicXor $$$(item.name) result &mem Device UniformMemory $data
        };
    }
}

__spirv_version(1.0)[require(spirv)]
__glsl_version(430) [require(glsl)]
[ForceInline] public $(item.name) atomicCompSwap(inout $(item.name) mem, $(item.name) compare, $(item.name) data)
{
    typeRequireChecks_atomic_using_float0_tier<$(item.name)>();
    typeRequireChecks_atomic_using_Logical_CAS<$(item.name)>();
    __target_switch
    {
    case glsl: __intrinsic_asm "atomicCompSwap($0, $1, $2)";
    case spirv:
        return spirv_asm
        {
            result:$$$(item.name) = OpAtomicCompareExchange &mem Device None None $data $compare
        };
    }
}

${{{{
}
}}}}

// all atomic_uint functions are mangled at compile time, 
// all types are converted into a field address of a 'uint'
// relative to the layout(offset) of the atomic_uint 
__spirv_version(1.0)[require(spirv)]
__glsl_version(430) [require(glsl)]
[ForceInline] public uint atomicCounterIncrement(atomic_uint c)
{

    __target_switch
    {
    case glsl: __intrinsic_asm "atomicAdd($0, 1)";
    case spirv:
    {
        return spirv_asm
        {
            OpAtomicIIncrement $$uint result $c Device UniformMemory
        };
    }
    }
}

__glsl_version(430) [require(glsl)]
[ForceInline] public uint atomicCounterDecrement_GLSL_helper(atomic_uint c)
{
    __target_switch
    {
    case glsl:
    {
        __intrinsic_asm "atomicExchange($0,$0-1)";
    }
    }
}

__spirv_version(1.0)[require(spirv)]
__glsl_version(430) [require(glsl)]
[ForceInline] public uint atomicCounter(atomic_uint c)
{
    __target_switch
    {
    case glsl:
    {
        __intrinsic_asm "($0)";
    }
    case spirv:
    {
        return spirv_asm
        {
            OpLoad $$uint result $c 
        };
    }
    }
}

__spirv_version(1.0)[require(spirv)]
__glsl_version(430) [require(glsl)]
[ForceInline] public uint atomicCounterDecrement(atomic_uint c)
{
    __target_switch
    {
    case glsl:
    {
        atomicCounterDecrement_GLSL_helper(c);
        return atomicCounter(c);
    }
    case spirv:
    {
        // spirv OpAtomicIDecrement returns pre-sub-1, glsl returns the new value
        // we want a discarded side effect and then return the new value
        return spirv_asm
        {
            %discardedValue:$$uint = OpAtomicIDecrement $c Device UniformMemory;
            OpLoad $$uint result $c
        };
    }
    }   
}

__spirv_version(1.0)[require(spirv)]
__glsl_version(430) [require(glsl)]
[ForceInline] public uint atomicCounterAdd(atomic_uint c, uint data)
{
    __target_switch
    {
    case glsl: __intrinsic_asm "atomicAdd($0, $1)";
    case spirv:
    {
        return spirv_asm
        {
            OpAtomicIAdd $$uint result $c Device UniformMemory $data
        };
    }
    }
}

__spirv_version(1.0)[require(spirv)]
__glsl_version(430) [require(glsl)]
[ForceInline] public uint atomicCounterSubtract(atomic_uint c, uint data)
{
    __target_switch
    {
    case glsl:
    {
        __intrinsic_asm "atomicExchange($0,$0-$1)";
    }
    case spirv:
    {
        return spirv_asm
        {
            OpAtomicISub $$uint result $c Device UniformMemory $data
        };
    }
    }   
}

__spirv_version(1.0)[require(spirv)]
__glsl_version(430) [require(glsl)]
[ForceInline] public uint atomicCounterMin(atomic_uint c, uint data)
{
    __target_switch
    {
    case glsl: __intrinsic_asm "atomicMin($0, $1)";
    case spirv:
    {
        return spirv_asm
        {
            OpAtomicUMin $$uint result $c Device UniformMemory $data
        };
    }
    }
}

__spirv_version(1.0)[require(spirv)]
__glsl_version(430) [require(glsl)]
[ForceInline] public uint atomicCounterMax(atomic_uint c, uint data)
{
    __target_switch
    {
    case glsl: __intrinsic_asm "atomicMax($0, $1)";
    case spirv:
    {
        return spirv_asm
        {
            OpAtomicUMax $$uint result $c Device UniformMemory $data
        };
    }
    }
}

__spirv_version(1.0)[require(spirv)]
__glsl_version(430) [require(glsl)]
[ForceInline] public uint atomicCounterAnd(atomic_uint c, uint data)
{
    __target_switch
    {
    case glsl: __intrinsic_asm "atomicAnd($0, $1)";
    case spirv:
    {
        return spirv_asm
        {
            OpAtomicAnd $$uint result $c Device UniformMemory $data
        };
    }
    }   
}

__spirv_version(1.0)[require(spirv)]
__glsl_version(430) [require(glsl)]
[ForceInline] public uint atomicCounterOr(atomic_uint c, uint data)
{
    __target_switch
    {
    case glsl: __intrinsic_asm "atomicOr($0, $1)";
    case spirv:
    {
        return spirv_asm
        {
            OpAtomicOr $$uint result $c Device UniformMemory $data
        };
    }
    }
}

__spirv_version(1.0)[require(spirv)]
__glsl_version(430) [require(glsl)]
[ForceInline] public uint atomicCounterXor(atomic_uint c, uint data)
{
    __target_switch
    {
    case glsl: __intrinsic_asm "atomicXor($0, $1)";
    case spirv:
    {
        return spirv_asm
        {
            OpAtomicXor $$uint result $c Device UniformMemory $data
        };
    }
    }
}

__spirv_version(1.0)[require(spirv)]
__glsl_version(430) [require(glsl)]
[ForceInline] public uint atomicCounterExchange(atomic_uint c, uint data)
{
    __target_switch
    {
    case glsl: __intrinsic_asm "atomicExchange($0, $1)";
    case spirv:
    {
        return spirv_asm
        {
            OpAtomicExchange $$uint result $c Device UniformMemory $data
        };
    }
    }
}

__spirv_version(1.0)[require(spirv)]
__glsl_version(430) [require(glsl)]
[ForceInline] public uint atomicCounterCompSwap(atomic_uint c, uint compare, uint data)
{
    __target_switch
    {
    case glsl: __intrinsic_asm "atomicCompSwap($0, $1, $2)";
    case spirv:
    {
        return spirv_asm
        {
            OpAtomicCompareExchange $$uint result $c Device UniformMemory UniformMemory $data $compare
        };
    }
    }
}

/// Section 8.14. Fragment Processing Functions

[require(any_gfx_target, fragmentprocessing)]
[__NoSideEffect]
[ForceInline]
public float dFdx(float p)
{
    return ddx(p);
}
[require(any_gfx_target, fragmentprocessing)]
__generic<let N : int>
[__NoSideEffect]
[ForceInline]
public vector<float, N> dFdx(vector<float, N> p)
{
    return ddx(p);
}

[require(any_gfx_target, fragmentprocessing)]
[__NoSideEffect]
[ForceInline]
public float dFdy(float p)
{
    return ddy(p);
}
[require(any_gfx_target, fragmentprocessing)]
__generic<let N : int>
[__NoSideEffect]
[ForceInline]
public vector<float, N> dFdy(vector<float, N> p)
{
    return ddy(p);
}

[require(any_gfx_target, fragmentprocessing_derivativecontrol)]
[__NoSideEffect]
[ForceInline]
public float dFdxFine(float p)
{
    return ddx_fine(p);
}
[require(any_gfx_target, fragmentprocessing_derivativecontrol)]
__generic<let N : int>
[__NoSideEffect]
[ForceInline]
public vector<float, N> dFdxFine(vector<float, N> p)
{
    return ddx_fine(p);
}

[require(any_gfx_target, fragmentprocessing_derivativecontrol)]
[__NoSideEffect]
[ForceInline]
public float dFdyFine(float p)
{
    return ddy_fine(p);
}
[require(any_gfx_target, fragmentprocessing_derivativecontrol)]
__generic<let N : int>
[__NoSideEffect]
[ForceInline]
public vector<float, N> dFdyFine(vector<float, N> p)
{
    return ddy_fine(p);
}

[require(any_gfx_target, fragmentprocessing_derivativecontrol)]
[__NoSideEffect]
[ForceInline]
public float dFdxCoarse(float p)
{
    return ddx_coarse(p);
}
[require(any_gfx_target, fragmentprocessing_derivativecontrol)]
__generic<let N : int>
[__NoSideEffect]
[ForceInline]
public vector<float, N> dFdxCoarse(vector<float, N> p)
{
    return ddx_coarse(p);
}

[require(any_gfx_target, fragmentprocessing_derivativecontrol)]
[__NoSideEffect]
[ForceInline]
public float dFdyCoarse(float p)
{
    return ddy_coarse(p);
}
[require(any_gfx_target, fragmentprocessing_derivativecontrol)]
__generic<let N : int>
[__NoSideEffect]
[ForceInline]
public vector<float, N> dFdyCoarse(vector<float, N> p)
{
    return ddy_coarse(p);
}

[require(any_gfx_target, fragmentprocessing_derivativecontrol)]
[__NoSideEffect]
[ForceInline]
public float fwidthFine(float p)
{
    __target_switch
    {
    case hlsl:
    {
        return abs(ddx_fine(p)) + abs(ddy_fine(p));
    }
    case glsl: __intrinsic_asm "fwidthFine($0)";
    case spirv:
    {
        return spirv_asm
        {
            OpCapability DerivativeControl;
            OpFwidthFine $$float result $p;
        };
    }
    }
}
[require(any_gfx_target, fragmentprocessing_derivativecontrol)]
__generic<let N : int>
[__NoSideEffect]
[ForceInline]
public vector<float, N> fwidthFine(vector<float, N> p)
{
    __target_switch
    {
    case hlsl:
    {
        return abs(ddx_fine(p)) + abs(ddy_fine(p));
    }
    case glsl: __intrinsic_asm "fwidthFine($0)";
    case spirv:
    {
        return spirv_asm
        {
            OpCapability DerivativeControl;
            OpFwidthFine $$vector<float, N> result $p;
        };
    }
    }
}

[require(any_gfx_target, fragmentprocessing_derivativecontrol)]
[__NoSideEffect]
[ForceInline]
public float fwidthCoarse(float p)
{
    __target_switch
    {
    case hlsl:
    {
        return abs(ddx_coarse(p)) + abs(ddy_coarse(p));
    }
    case glsl: __intrinsic_asm "fwidthCoarse($0)";
    case spirv:
    {
        return spirv_asm
        {
            OpCapability DerivativeControl;
            OpFwidthCoarse $$float result $p;
        };
    }
    }
}
[require(any_gfx_target, fragmentprocessing_derivativecontrol)]
__generic<let N : int>
[__NoSideEffect]
[ForceInline]
public vector<float, N> fwidthCoarse(vector<float, N> p)
{
    __target_switch
    {
    case hlsl:
    {
        return abs(ddx_coarse(p)) + abs(ddy_coarse(p));
    }
    case glsl: __intrinsic_asm "fwidthCoarse($0)";
    case spirv:
    {
        return spirv_asm
        {
            OpCapability DerivativeControl;
            OpFwidthCoarse $$vector<float, N> result $p;
        };
    }
    }
}

[require(any_gfx_target, fragmentprocessing)]
[__NoSideEffect]
[__GLSLRequireShaderInputParameter(0)]
public float interpolateAtCentroid(__ref float interpolant)
{
    __target_switch
    {
    case hlsl:
    {
        return EvaluateAttributeAtCentroid(interpolant);
    }
    case glsl: __intrinsic_asm "interpolateAtCentroid($0)";
    case spirv:
    {
        return spirv_asm
        {
            OpCapability InterpolationFunction;
            OpExtInst $$float result glsl450 InterpolateAtCentroid &interpolant
        };
    }
    }
}
[require(any_gfx_target, fragmentprocessing)]
__generic<let N : int>
[__NoSideEffect]
[__GLSLRequireShaderInputParameter(0)]
public vector<float, N> interpolateAtCentroid(__ref vector<float, N> interpolant)
{
    __target_switch
    {
    case hlsl:
    {
        return EvaluateAttributeAtCentroid(interpolant);
    }
    case glsl: __intrinsic_asm "interpolateAtCentroid($0)";
    case spirv:
    {
        return spirv_asm
        {
            OpCapability InterpolationFunction;
            OpExtInst $$vector<float, N> result glsl450 InterpolateAtCentroid &interpolant
        };
    }
    }
}

[require(any_gfx_target, fragmentprocessing)]
[__NoSideEffect]
[__GLSLRequireShaderInputParameter(0)]
public float interpolateAtSample(__ref float interpolant, int sample)
{
    __target_switch
    {
    case hlsl:
    {
        return EvaluateAttributeAtSample(interpolant, sample);
    }
    case glsl: __intrinsic_asm "interpolateAtSample($0,$1)";
    case spirv:
    {
        return spirv_asm
        {
            OpCapability InterpolationFunction;
            OpExtInst $$float result glsl450 InterpolateAtSample &interpolant $sample
        };
    }
    }
}
[require(any_gfx_target, fragmentprocessing)]
__generic<let N : int>
[__NoSideEffect]
[__GLSLRequireShaderInputParameter(0)]
public vector<float, N> interpolateAtSample(__ref vector<float, N> interpolant, int sample)
{
    __target_switch
    {
    case hlsl:
    {
        return EvaluateAttributeAtSample(interpolant, sample);
    }
    case glsl: __intrinsic_asm "interpolateAtSample($0,$1)";
    case spirv:
    {
        return spirv_asm
        {
            OpCapability InterpolationFunction;
            OpExtInst $$vector<float, N> result glsl450 InterpolateAtSample &interpolant $sample
        };
    }
    }
}

[require(glsl_spirv, fragmentprocessing)]
[__NoSideEffect]
[__GLSLRequireShaderInputParameter(0)]
public float interpolateAtOffset(__ref float interpolant, vec2 offset)
{
    __target_switch
    {
    case glsl: __intrinsic_asm "interpolateAtOffset($0,$1)";
    case spirv:
    {
        return spirv_asm
        {
            OpCapability InterpolationFunction;
            OpExtInst $$float result glsl450 InterpolateAtOffset &interpolant $offset
        };
    }
    }
}
[require(glsl_spirv, fragmentprocessing)]
__generic<let N : int>
[__NoSideEffect]
[__GLSLRequireShaderInputParameter(0)]
public vector<float, N> interpolateAtOffset(__ref vector<float, N> interpolant, vec2 offset)
{
    __target_switch
    {
    case glsl: __intrinsic_asm "interpolateAtOffset($0,$1)";
    case spirv:
    {
        return spirv_asm
        {
            OpCapability InterpolationFunction;
            OpExtInst $$vector<float, N> result glsl450 InterpolateAtOffset &interpolant $offset
        };
    }
    }
}

/// Section 8.15. Noise Functions (deprecated)

[deprecated("Always returns 0")]
[__NoSideEffect]
[ForceInline]
public float noise1(float x)
{
    return 0.0f;
}
[deprecated("Always returns 0")]
__generic<let N : int>
[__NoSideEffect]
[ForceInline]
public float noise1(vector<float, N> x)
{
    return 0.0f;
}

[deprecated("Always returns 0")]
[__NoSideEffect]
[ForceInline]
public vec2 noise2(float x)
{
    return vec2(0.0f);
}
[deprecated("Always returns 0")]
__generic<let N : int>
[__NoSideEffect]
[ForceInline]
public vec2 noise2(vector<float, N> x)
{
    return vec2(0.0f);
}

[deprecated("Always returns 0")]
[__NoSideEffect]
[ForceInline]
public vec3 noise3(float x)
{
    return vec3(0.0f);
}
[deprecated("Always returns 0")]
__generic<let N : int>
[__NoSideEffect]
[ForceInline]
public vec3 noise3(vector<float, N> x)
{
    return vec3(0.0f);
}

[deprecated("Always returns 0")]
[__NoSideEffect]
[ForceInline]
public vec4 noise4(float x)
{
    return vec4(0.0f);
}
[deprecated("Always returns 0")]
__generic<let N : int>
[__NoSideEffect]
[ForceInline]
public vec4 noise4(vector<float, N> x)
{
    return vec4(0.0f);
}

/// Section 8.16. Shader Invocation Control Functions

// TODO: for tessellation control shader: barrier() function may only
// be placed inside the main() of the shader and may not be called within
// control flow.
// TODO: if called after a return, error.

[require(any_gfx_target, compute_tess_gfx)]
[ForceInline]
public void barrier()
{
    __target_switch
    {
    case hlsl:
    {
        GroupMemoryBarrier();
    }
    case glsl: __intrinsic_asm "barrier()";
    case spirv:
    {
        spirv_asm
        {
            OpControlBarrier Workgroup Workgroup AcquireRelease|WorkgroupMemory
        };
    }
    }
}

/// Section 8.17. Shader Memory Control Functions

[require(shadermemorycontrol)]
[ForceInline]
public void memoryBarrier()
{
    __target_switch
    {
    case hlsl: __intrinsic_asm "AllMemoryBarrier()";
    case glsl: __intrinsic_asm "memoryBarrier()";
    case spirv:
    {
        spirv_asm
        {
            OpCapability Shader;
            OpMemoryBarrier Device AcquireRelease|WorkgroupMemory|ImageMemory|UniformMemory
        };
    }
    }
}

// Does not exist in vulkan. The equivlent is now `memoryBarrierBuffer`
// for vulkan since GL_EXT_vulkan_glsl_relaxed states that a atomic_counter
// is a storage buffer object in implementation.

// glslang will compile with `AtomicCounterMemory` since it does not use
// the `AtomicStorage` OpCapability which is required for `AtomicCounterMemory`.
// this is invalid/undefined spir-v for vulkan targets and should not be followed
[require(any_gfx_target, shadermemorycontrol_compute)]
[ForceInline]
public void memoryBarrierAtomicCounter()
{
    __target_switch
    {
    case hlsl: __intrinsic_asm "AllMemoryBarrier()";
    case glsl: __intrinsic_asm "memoryBarrierBuffer()";
    case spirv:
    {
        spirv_asm
        {
            OpCapability Shader;
            OpMemoryBarrier Device AcquireRelease|UniformMemory
        };
    }
    }
}
[require(shadermemorycontrol)]
[ForceInline]
public void memoryBarrierBuffer()
{
    __target_switch
    {
    case hlsl: __intrinsic_asm "DeviceMemoryBarrier()";
    case glsl: __intrinsic_asm "memoryBarrierBuffer()";
    case spirv:
    {
        spirv_asm
        {
            OpCapability Shader;
            OpMemoryBarrier Device AcquireRelease|UniformMemory
        };
    }
    }
}

[require(any_gfx_target, shadermemorycontrol_compute)]
[ForceInline]
public void memoryBarrierShared()
{
    __target_switch
    {
    case hlsl: __intrinsic_asm "GroupMemoryBarrier()";
    case glsl: __intrinsic_asm "memoryBarrierShared()";
    case spirv:
    {
        spirv_asm
        {
            OpCapability Shader;
            OpMemoryBarrier Device AcquireRelease|WorkgroupMemory
        };
    }
    }
}
[require(shadermemorycontrol)]
[ForceInline]
public void memoryBarrierImage()
{
    __target_switch
    {
    case hlsl: __intrinsic_asm "DeviceMemoryBarrier()";
    case glsl: __intrinsic_asm "memoryBarrierImage()";
    case spirv:
    {
        spirv_asm
        {
            OpMemoryBarrier Device AcquireRelease|ImageMemory
        };
    }
    }
}
[require(any_gfx_target, shadermemorycontrol_compute)]
[ForceInline]
public void groupMemoryBarrier()
{
    __target_switch
    {
    case hlsl: __intrinsic_asm "GroupMemoryBarrier()";
    case glsl: __intrinsic_asm "groupMemoryBarrier()";
    case spirv:
    {
        spirv_asm
        {
            OpMemoryBarrier Workgroup AcquireRelease|WorkgroupMemory|ImageMemory|UniformMemory
        };
    }
    }
}

/// Section 8.18. Subpass-Input Functions

${{{{
struct SubpassLoadEntriesType
{
    const char* typeName;
    const char* prefix;
};
static const SubpassLoadEntriesType kSubpassLoadEntries[] =
    {
        { "vec4", "" },
        { "uvec4", "u" },
        { "ivec4", "i" },
    };
for (const auto& subpassLoadEntry : kSubpassLoadEntries)
{
}}}}
    [require(glsl_spirv, subpass)]
    public typealias $(subpassLoadEntry.prefix)subpassInput = __SubpassImpl<
        $(subpassLoadEntry.typeName),
        0 // isMS
    >;

    [require(glsl_spirv, subpass)]
    public typealias $(subpassLoadEntry.prefix)subpassInputMS = __SubpassImpl<
        $(subpassLoadEntry.typeName),
        1 // isMS
    >;
${{{{
}
}}}}

    __generic<T>
    [require(glsl_spirv, subpass)]
    [__NoSideEffect]
    [ForceInline]
    public T subpassLoad(__SubpassImpl<T,0> subpass)
    {
        __target_switch
        {
        case hlsl: return subpass.SubpassLoad();
        case glsl: __intrinsic_asm "subpassLoad($0)";
        case spirv:
        {
            let zeroVec = ivec2(0);
            return spirv_asm
            {
                OpCapability StorageImageReadWithoutFormat;
                result:$$T = OpImageRead $subpass $zeroVec
            };
        }
        }
    }
    __generic<T>
    [require(glsl_spirv, subpass)]
    [__NoSideEffect]
    [ForceInline]
    public T subpassLoad(__SubpassImpl<T,1> subpass, int sample)
    {
        __target_switch
        {
        case hlsl: return subpass.SubpassLoad(sample);
        case glsl: __intrinsic_asm "subpassLoad($0, $1)";
        case spirv:
        {
            let zeroVec = ivec2(0);
            return spirv_asm
            {
                OpCapability StorageImageReadWithoutFormat;
                result:$$T = OpImageRead $subpass $zeroVec Sample $sample
            };
        }
        }
    }

/// Section 8.19. Shader Invocation Group Functions

// Invocation functions compiles into SubgroupVoteKHR operations.
// Instead of using these, we directly emit the non deprecated
// alternatives
__glsl_version(460)
__spirv_version(1.3)
[require(shaderinvocationgroup)]
[NonUniformReturn]
[ForceInline]
public bool anyInvocation(bool value)
{
    return WaveActiveAnyTrue(value);
}
__glsl_version(460)
__spirv_version(1.3)
[require(shaderinvocationgroup)]
[NonUniformReturn]
[ForceInline]
public bool allInvocations(bool value)
{
    return WaveActiveAllTrue(value);
}
__glsl_version(460)
__spirv_version(1.3)
[require(shaderinvocationgroup)]
[NonUniformReturn]
[ForceInline]
public bool allInvocationsEqual(bool value)
{
    return WaveActiveAllEqual(value);
}