// slang-stdlib.cpp

#include "slang-stdlib.h"
#include "syntax.h"

#define STRINGIZE(x) STRINGIZE2(x)
#define STRINGIZE2(x) #x
#define LINE_STRING STRINGIZE(__LINE__)

enum { kCoreLibIncludeStringLine = __LINE__ + 1 };
const char* kCoreLibIncludeStringChunks[] = { R"=(

// A type that can be used as an operand for builtins
interface __BuiltinType {}

// A type that can be used for arithmetic operations
interface __BuiltinArithmeticType : __BuiltinType {}

// A type that logically has a sign (positive/negative/zero)
interface __BuiltinSignedArithmeticType : __BuiltinArithmeticType {}

// A type that can represent integers
interface __BuiltinIntegerType : __BuiltinArithmeticType {}

// A type that can represent non-integers
interface __BuiltinRealType : __BuiltinArithmeticType {}

// A type that uses a floating-point representation
interface __BuiltinFloatingPointType : __BuiltinRealType, __BuiltinSignedArithmeticType {}

__generic<T,U> __intrinsic_op(Sequence) U operator,(T left, U right);

__generic<T> __intrinsic_op(Select) T operator?:(bool condition, T ifTrue, T ifFalse);
__generic<T, let N : int> __intrinsic_op(Select) vector<T,N> operator?:(vector<bool,N> condition, vector<T,N> ifTrue, vector<T,N> ifFalse);

)=" };


enum { kHLSLLibIncludeStringLine = __LINE__+1 };
const char * kHLSLLibIncludeStringChunks[] = { R"=(

typedef uint UINT;

__generic<T> __magic_type(HLSLAppendStructuredBufferType) struct AppendStructuredBuffer
{
    __intrinsic void Append(T value);

    __intrinsic void GetDimensions(
        out uint numStructs,
        out uint stride);
};

__generic<T> __magic_type(HLSLBufferType) struct Buffer
{
    __intrinsic void GetDimensions(
        out uint dim);

    __intrinsic T Load(int location);
    __intrinsic T Load(int location, out uint status);

    __intrinsic __subscript(uint index) -> T;
};

__magic_type(HLSLByteAddressBufferType) struct ByteAddressBuffer
{
    __intrinsic void GetDimensions(
        out uint dim);

    __intrinsic uint Load(int location);
    __intrinsic uint Load(int location, out uint status);

    __intrinsic uint2 Load2(int location);
    __intrinsic uint2 Load2(int location, out uint status);

    __intrinsic uint3 Load3(int location);
    __intrinsic uint3 Load3(int location, out uint status);

    __intrinsic uint4 Load4(int location);
    __intrinsic uint4 Load4(int location, out uint status);
};

__generic<T> __magic_type(HLSLStructuredBufferType) struct StructuredBuffer
{
    __intrinsic void GetDimensions(
        out uint numStructs,
        out uint stride);

    __intrinsic T Load(int location);
    __intrinsic T Load(int location, out uint status);

    __intrinsic __subscript(uint index) -> T;
};

__generic<T> __magic_type(HLSLConsumeStructuredBufferType) struct ConsumeStructuredBuffer
{
    __intrinsic T Consume();

    __intrinsic void GetDimensions(
        out uint numStructs,
        out uint stride);
};

__generic<T, let N : int> __magic_type(HLSLInputPatchType) struct InputPatch
{
    __intrinsic __subscript(uint index) -> T;
};

__generic<T, let N : int> __magic_type(HLSLOutputPatchType) struct OutputPatch
{
    __intrinsic __subscript(uint index) -> T { set; }
};

__generic<T> __magic_type(HLSLRWBufferType) struct RWBuffer
{
    // Note(tfoley): duplication with declaration of `Buffer`

    __intrinsic void GetDimensions(
        out uint dim);

    __intrinsic T Load(int location);
    __intrinsic T Load(int location, out uint status);

    __intrinsic __subscript(uint index) -> T { get; set; }
};

__magic_type(HLSLRWByteAddressBufferType) struct RWByteAddressBuffer
{
    // Note(tfoley): supports alll operations from `ByteAddressBuffer`
    // TODO(tfoley): can this be made a sub-type?

    __intrinsic void GetDimensions(
        out uint dim);

    __intrinsic uint Load(int location);
    __intrinsic uint Load(int location, out uint status);

    __intrinsic uint2 Load2(int location);
    __intrinsic uint2 Load2(int location, out uint status);

    __intrinsic uint3 Load3(int location);
    __intrinsic uint3 Load3(int location, out uint status);

    __intrinsic uint4 Load4(int location);
    __intrinsic uint4 Load4(int location, out uint status);

    // Added operations:

    __intrinsic void InterlockedAdd(
        UINT dest,
        UINT value,
        out UINT original_value);
    __intrinsic void InterlockedAdd(
        UINT dest,
        UINT value);

    __intrinsic void InterlockedAnd(
        UINT dest,
        UINT value,
        out UINT original_value);
    __intrinsic void InterlockedAnd(
        UINT dest,
        UINT value);

    __intrinsic void InterlockedCompareExchange(
        UINT dest,
        UINT compare_value,
        UINT value,
        out UINT original_value);
    __intrinsic void InterlockedCompareExchange(
        UINT dest,
        UINT compare_value,
        UINT value);

    __intrinsic void InterlockedCompareStore(
        UINT dest,
        UINT compare_value,
        UINT value);
    __intrinsic void InterlockedCompareStore(
        UINT dest,
        UINT compare_value);

    __intrinsic void InterlockedExchange(
        UINT dest,
        UINT value,
        out UINT original_value);
    __intrinsic void InterlockedExchange(
        UINT dest,
        UINT value);

    __intrinsic void InterlockedMax(
        UINT dest,
        UINT value,
        out UINT original_value);
    __intrinsic void InterlockedMax(
        UINT dest,
        UINT value);

    __intrinsic void InterlockedMin(
        UINT dest,
        UINT value,
        out UINT original_value);
    __intrinsic void InterlockedMin(
        UINT dest,
        UINT value);

    __intrinsic void InterlockedOr(
        UINT dest,
        UINT value,
        out UINT original_value);
    __intrinsic void InterlockedOr(
        UINT dest,
        UINT value);

    __intrinsic void InterlockedXor(
        UINT dest,
        UINT value,
        out UINT original_value);
    __intrinsic void InterlockedXor(
        UINT dest,
        UINT value);

    __intrinsic void Store(
        uint address,
        uint value);

    __intrinsic void Store2(
        uint address,
        uint2 value);

    __intrinsic void Store3(
        uint address,
        uint3 value);

    __intrinsic void Store4(
        uint address,
        uint4 value);
};

__generic<T> __magic_type(HLSLRWStructuredBufferType) struct RWStructuredBuffer
{
    __intrinsic uint DecrementCounter();

    __intrinsic void GetDimensions(
        out uint numStructs,
        out uint stride);

    __intrinsic void IncrementCounter();

    __intrinsic T Load(int location);
    __intrinsic T Load(int location, out uint status);

    __intrinsic __subscript(uint index) -> T { get; set; }
};

__generic<T> __magic_type(HLSLPointStreamType) struct PointStream
{
    void Append(T value);
    void RestartStrip();
};

__generic<T> __magic_type(HLSLLineStreamType) struct LineStream
{
    void Append(T value);
    void RestartStrip();
};

__generic<T> __magic_type(HLSLLineStreamType) struct TriangleStream
{
    void Append(T value);
    void RestartStrip();
};

)=", R"=(

// Note(tfoley): Trying to systematically add all the HLSL builtins

// Try to terminate the current draw or dispatch call (HLSL SM 4.0)
__intrinsic void abort();

// Absolute value (HLSL SM 1.0)
__generic<T : __BuiltinSignedArithmeticType> __intrinsic T abs(T x);
__generic<T : __BuiltinSignedArithmeticType, let N : int> __intrinsic vector<T,N> abs(vector<T,N> x);
__generic<T : __BuiltinSignedArithmeticType, let N : int, let M : int> __intrinsic matrix<T,N,M> abs(matrix<T,N,M> x);

// Inverse cosine (HLSL SM 1.0)
__generic<T : __BuiltinFloatingPointType> __intrinsic T acos(T x);
__generic<T : __BuiltinFloatingPointType, let N : int> __intrinsic vector<T,N> acos(vector<T,N> x);
__generic<T : __BuiltinFloatingPointType, let N : int, let M : int> __intrinsic matrix<T,N,M> acos(matrix<T,N,M> x);

// Test if all components are non-zero (HLSL SM 1.0)
__generic<T : __BuiltinType> __intrinsic T all(T x);
__generic<T : __BuiltinType, let N : int> __intrinsic vector<T,N> all(vector<T,N> x);
__generic<T : __BuiltinType, let N : int, let M : int> __intrinsic matrix<T,N,M> all(matrix<T,N,M> x);

// Barrier for writes to all memory spaces (HLSL SM 5.0)
__intrinsic void AllMemoryBarrier();

// Thread-group sync and barrier for writes to all memory spaces (HLSL SM 5.0)
__intrinsic void AllMemoryBarrierWithGroupSync();

// Test if any components is non-zero (HLSL SM 1.0)
__generic<T : __BuiltinType> __intrinsic T any(T x);
__generic<T : __BuiltinType, let N : int> __intrinsic vector<T,N> any(vector<T,N> x);
__generic<T : __BuiltinType, let N : int, let M : int> __intrinsic matrix<T,N,M> any(matrix<T,N,M> x);


// Reinterpret bits as a double (HLSL SM 5.0)
__intrinsic double asdouble(uint lowbits, uint highbits);

// Reinterpret bits as a float (HLSL SM 4.0)
__intrinsic float asfloat( int x);
__intrinsic float asfloat(uint x);
__generic<let N : int> __intrinsic vector<float,N> asfloat(vector< int,N> x);
__generic<let N : int> __intrinsic vector<float,N> asfloat(vector<uint,N> x);
__generic<let N : int, let M : int> __intrinsic matrix<float,N,M> asfloat(matrix< int,N,M> x);
__generic<let N : int, let M : int> __intrinsic matrix<float,N,M> asfloat(matrix<uint,N,M> x);


// Inverse sine (HLSL SM 1.0)
__generic<T : __BuiltinFloatingPointType> __intrinsic T asin(T x);
__generic<T : __BuiltinFloatingPointType, let N : int> __intrinsic vector<T,N> asin(vector<T,N> x);
__generic<T : __BuiltinFloatingPointType, let N : int, let M : int> __intrinsic matrix<T,N,M> asin(matrix<T,N,M> x);

// Reinterpret bits as an int (HLSL SM 4.0)
__intrinsic int asint(float x);
__intrinsic int asint(uint x);
__generic<let N : int> __intrinsic vector<int,N> asint(vector<float,N> x);
__generic<let N : int> __intrinsic vector<int,N> asint(vector<uint,N> x);
__generic<let N : int, let M : int> __intrinsic matrix<int,N,M> asint(matrix<float,N,M> x);
__generic<let N : int, let M : int> __intrinsic matrix<int,N,M> asint(matrix<uint,N,M> x);

// Reinterpret bits of double as a uint (HLSL SM 5.0)
__intrinsic void asuint(double value, out uint lowbits, out uint highbits);

// Reinterpret bits as a uint (HLSL SM 4.0)
__intrinsic uint asuint(float x);
__intrinsic uint asuint(int x);
__generic<let N : int> __intrinsic vector<uint,N> asuint(vector<float,N> x);
__generic<let N : int> __intrinsic vector<uint,N> asuint(vector<int,N> x);
__generic<let N : int, let M : int> __intrinsic matrix<uint,N,M> asuint(matrix<float,N,M> x);
__generic<let N : int, let M : int> __intrinsic matrix<uint,N,M> asuint(matrix<int,N,M> x);

// Inverse tangent (HLSL SM 1.0)
__generic<T : __BuiltinFloatingPointType> __intrinsic T atan(T x);
__generic<T : __BuiltinFloatingPointType, let N : int> __intrinsic vector<T,N> atan(vector<T,N> x);
__generic<T : __BuiltinFloatingPointType, let N : int, let M : int> __intrinsic matrix<T,N,M> atan(matrix<T,N,M> x);

__generic<T : __BuiltinFloatingPointType>
__intrinsic(glsl,"atan($0,$1)")
__intrinsic
T atan2(T y, T x);

__generic<T : __BuiltinFloatingPointType, let N : int>
__intrinsic(glsl,"atan($0,$1)")
__intrinsic
vector<T,N> atan2(vector<T,N> y, vector<T,N> x);

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__intrinsic(glsl,"atan($0,$1)")
__intrinsic
matrix<T,N,M> atan2(matrix<T,N,M> y, matrix<T,N,M> x);

// Ceiling (HLSL SM 1.0)
__generic<T : __BuiltinFloatingPointType> __intrinsic T ceil(T x);
__generic<T : __BuiltinFloatingPointType, let N : int> __intrinsic vector<T,N> ceil(vector<T,N> x);
__generic<T : __BuiltinFloatingPointType, let N : int, let M : int> __intrinsic matrix<T,N,M> ceil(matrix<T,N,M> x);


// Check access status to tiled resource
__intrinsic bool CheckAccessFullyMapped(uint status);

// Clamp (HLSL SM 1.0)
__generic<T : __BuiltinArithmeticType> __intrinsic T clamp(T x, T min, T max);
__generic<T : __BuiltinArithmeticType, let N : int> __intrinsic vector<T,N> clamp(vector<T,N> x, vector<T,N> min, vector<T,N> max);
__generic<T : __BuiltinArithmeticType, let N : int, let M : int> __intrinsic matrix<T,N,M> clamp(matrix<T,N,M> x, matrix<T,N,M> min, matrix<T,N,M> max);

// Clip (discard) fragment conditionally
__generic<T : __BuiltinFloatingPointType> __intrinsic void clip(T x);
__generic<T : __BuiltinFloatingPointType, let N : int> __intrinsic void clip(vector<T,N> x);
__generic<T : __BuiltinFloatingPointType, let N : int, let M : int> __intrinsic void clip(matrix<T,N,M> x);

// Cosine
__generic<T : __BuiltinFloatingPointType> __intrinsic T cos(T x);
__generic<T : __BuiltinFloatingPointType, let N : int> __intrinsic vector<T,N> cos(vector<T,N> x);
__generic<T : __BuiltinFloatingPointType, let N : int, let M : int> __intrinsic matrix<T,N,M> cos(matrix<T,N,M> x);

// Hyperbolic cosine
__generic<T : __BuiltinFloatingPointType> __intrinsic T cosh(T x);
__generic<T : __BuiltinFloatingPointType, let N : int> __intrinsic vector<T,N> cosh(vector<T,N> x);
__generic<T : __BuiltinFloatingPointType, let N : int, let M : int> __intrinsic matrix<T,N,M> cosh(matrix<T,N,M> x);

// Population count
__intrinsic uint countbits(uint value);

// Cross product
__generic<T : __BuiltinArithmeticType> __intrinsic vector<T,3> cross(vector<T,3> x, vector<T,3> y);

// Convert encoded color
__intrinsic int4 D3DCOLORtoUBYTE4(float4 x);

// Partial-difference derivatives
__generic<T : __BuiltinFloatingPointType>
__intrinsic(glsl, dFdx)
__intrinsic
T ddx(T x);
__generic<T : __BuiltinFloatingPointType, let N : int>
__intrinsic(glsl, dFdx)
__intrinsic
vector<T,N> ddx(vector<T,N> x);
__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__intrinsic(glsl, dFdx)
__intrinsic
matrix<T,N,M> ddx(matrix<T,N,M> x);

__generic<T : __BuiltinFloatingPointType>
__intrinsic(glsl, dFdxCoarse)
__intrinsic
T ddx_coarse(T x);
__generic<T : __BuiltinFloatingPointType, let N : int>
__intrinsic(glsl, dFdxCoarse)
__intrinsic
vector<T,N> ddx_coarse(vector<T,N> x);
__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__intrinsic(glsl, dFdxCoarse)
__intrinsic
matrix<T,N,M> ddx_coarse(matrix<T,N,M> x);

__generic<T : __BuiltinFloatingPointType>
__intrinsic(glsl, dFdxFine)
__intrinsic
T ddx_fine(T x);
__generic<T : __BuiltinFloatingPointType, let N : int>
__intrinsic(glsl, dFdxFine)
__intrinsic
vector<T,N> ddx_fine(vector<T,N> x);
__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__intrinsic(glsl, dFdxFine)
__intrinsic
matrix<T,N,M> ddx_fine(matrix<T,N,M> x);

__generic<T : __BuiltinFloatingPointType>
__intrinsic(glsl, dFdy)
__intrinsic
T ddy(T x);
__generic<T : __BuiltinFloatingPointType, let N : int>
__intrinsic(glsl, dFdy)
__intrinsic
vector<T,N> ddy(vector<T,N> x);
__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__intrinsic(glsl, dFdy)
__intrinsic
 matrix<T,N,M> ddy(matrix<T,N,M> x);

__generic<T : __BuiltinFloatingPointType>
__intrinsic(glsl, dFdyCoarse)
__intrinsic
T ddy_coarse(T x);
__generic<T : __BuiltinFloatingPointType, let N : int>
__intrinsic(glsl, dFdyCoarse)
__intrinsic
vector<T,N> ddy_coarse(vector<T,N> x);
__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__intrinsic(glsl, dFdyCoarse)
__intrinsic
matrix<T,N,M> ddy_coarse(matrix<T,N,M> x);

__generic<T : __BuiltinFloatingPointType>
__intrinsic(glsl, dFdyFine)
__intrinsic
T ddy_fine(T x);
__generic<T : __BuiltinFloatingPointType, let N : int>
__intrinsic(glsl, dFdyFine)
__intrinsic
vector<T,N> ddy_fine(vector<T,N> x);
__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__intrinsic(glsl, dFdyFine)
__intrinsic
matrix<T,N,M> ddy_fine(matrix<T,N,M> x);


// Radians to degrees
__generic<T : __BuiltinFloatingPointType> __intrinsic T degrees(T x);
__generic<T : __BuiltinFloatingPointType, let N : int> __intrinsic vector<T,N> degrees(vector<T,N> x);
__generic<T : __BuiltinFloatingPointType, let N : int, let M : int> __intrinsic matrix<T,N,M> degrees(matrix<T,N,M> x);

// Matrix determinant

__generic<T : __BuiltinFloatingPointType, let N : int> __intrinsic T determinant(matrix<T,N,N> m);

// Barrier for device memory
__intrinsic void DeviceMemoryBarrier();
__intrinsic void DeviceMemoryBarrierWithGroupSync();

// Vector distance

__generic<T : __BuiltinFloatingPointType, let N : int> __intrinsic T distance(vector<T,N> x, vector<T,N> y);

// Vector dot product

__generic<T : __BuiltinArithmeticType, let N : int> __intrinsic T dot(vector<T,N> x, vector<T,N> y);

// Helper for computing distance terms for lighting (obsolete)

__generic<T : __BuiltinFloatingPointType> __intrinsic vector<T,4> dst(vector<T,4> x, vector<T,4> y);

// Error message

// __intrinsic void errorf( string format, ... );

// Attribute evaluation

__generic<T : __BuiltinArithmeticType> __intrinsic T EvaluateAttributeAtCentroid(T x);
__generic<T : __BuiltinArithmeticType, let N : int> __intrinsic vector<T,N> EvaluateAttributeAtCentroid(vector<T,N> x);
__generic<T : __BuiltinArithmeticType, let N : int, let M : int> __intrinsic matrix<T,N,M> EvaluateAttributeAtCentroid(matrix<T,N,M> x);

__generic<T : __BuiltinArithmeticType> __intrinsic T EvaluateAttributeAtSample(T x, uint sampleindex);
__generic<T : __BuiltinArithmeticType, let N : int> __intrinsic vector<T,N> EvaluateAttributeAtSample(vector<T,N> x, uint sampleindex);
__generic<T : __BuiltinArithmeticType, let N : int, let M : int> __intrinsic matrix<T,N,M> EvaluateAttributeAtSample(matrix<T,N,M> x, uint sampleindex);

__generic<T : __BuiltinArithmeticType> __intrinsic T EvaluateAttributeSnapped(T x, int2 offset);
__generic<T : __BuiltinArithmeticType, let N : int> __intrinsic vector<T,N> EvaluateAttributeSnapped(vector<T,N> x, int2 offset);
__generic<T : __BuiltinArithmeticType, let N : int, let M : int> __intrinsic matrix<T,N,M> EvaluateAttributeSnapped(matrix<T,N,M> x, int2 offset);

// Base-e exponent
__generic<T : __BuiltinFloatingPointType> __intrinsic T exp(T x);
__generic<T : __BuiltinFloatingPointType, let N : int> __intrinsic vector<T,N> exp(vector<T,N> x);
__generic<T : __BuiltinFloatingPointType, let N : int, let M : int> __intrinsic matrix<T,N,M> exp(matrix<T,N,M> x);

// Base-2 exponent
__generic<T : __BuiltinFloatingPointType> __intrinsic T exp2(T x);
__generic<T : __BuiltinFloatingPointType, let N : int> __intrinsic vector<T,N> exp2(vector<T,N> x);
__generic<T : __BuiltinFloatingPointType, let N : int, let M : int> __intrinsic matrix<T,N,M> exp2(matrix<T,N,M> x);

// Convert 16-bit float stored in low bits of integer
__intrinsic float f16tof32(uint value);
__generic<let N : int> __intrinsic vector<float,N> f16tof32(vector<uint,N> value);

// Convert to 16-bit float stored in low bits of integer
__intrinsic uint f32tof16(float value);
__generic<let N : int> __intrinsic vector<uint,N> f32tof16(vector<float,N> value);

// Flip surface normal to face forward, if needed
__generic<T : __BuiltinFloatingPointType, let N : int> __intrinsic vector<T,N> faceforward(vector<T,N> n, vector<T,N> i, vector<T,N> ng);

// Find first set bit starting at high bit and working down
__intrinsic int firstbithigh(int value);
__generic<let N : int> __intrinsic vector<int,N> firstbithigh(vector<int,N> value);

__intrinsic uint firstbithigh(uint value);
__generic<let N : int> __intrinsic vector<uint,N> firstbithigh(vector<uint,N> value);

// Find first set bit starting at low bit and working up
__intrinsic int firstbitlow(int value);
__generic<let N : int> __intrinsic vector<int,N> firstbitlow(vector<int,N> value);

__intrinsic uint firstbitlow(uint value);
__generic<let N : int> __intrinsic vector<uint,N> firstbitlow(vector<uint,N> value);

// Floor (HLSL SM 1.0)
__generic<T : __BuiltinFloatingPointType> __intrinsic T floor(T x);
__generic<T : __BuiltinFloatingPointType, let N : int> __intrinsic vector<T,N> floor(vector<T,N> x);
__generic<T : __BuiltinFloatingPointType, let N : int, let M : int> __intrinsic matrix<T,N,M> floor(matrix<T,N,M> x);

// Fused multiply-add for doubles
__intrinsic double fma(double a, double b, double c);
__generic<let N : int> __intrinsic vector<double, N> fma(vector<double, N> a, vector<double, N> b, vector<double, N> c);
__generic<let N : int, let M : int> __intrinsic matrix<double,N,M> fma(matrix<double,N,M> a, matrix<double,N,M> b, matrix<double,N,M> c);

// Floating point remainder of x/y
__generic<T : __BuiltinFloatingPointType> __intrinsic T fmod(T x, T y);
__generic<T : __BuiltinFloatingPointType, let N : int> __intrinsic vector<T,N> fmod(vector<T,N> x, vector<T,N> y);
__generic<T : __BuiltinFloatingPointType, let N : int, let M : int> __intrinsic matrix<T,N,M> fmod(matrix<T,N,M> x, matrix<T,N,M> y);

// Fractional part
__generic<T : __BuiltinFloatingPointType> __intrinsic T frac(T x);
__generic<T : __BuiltinFloatingPointType, let N : int> __intrinsic vector<T,N> frac(vector<T,N> x);
__generic<T : __BuiltinFloatingPointType, let N : int, let M : int> __intrinsic matrix<T,N,M> frac(matrix<T,N,M> x);

// Split float into mantissa and exponent
__generic<T : __BuiltinFloatingPointType> __intrinsic T frexp(T x, out T exp);
__generic<T : __BuiltinFloatingPointType, let N : int> __intrinsic vector<T,N> frexp(vector<T,N> x, out vector<T,N> exp);
__generic<T : __BuiltinFloatingPointType, let N : int, let M : int> __intrinsic matrix<T,N,M> frexp(matrix<T,N,M> x, out matrix<T,N,M> exp);

// Texture filter width
__generic<T : __BuiltinFloatingPointType> __intrinsic T fwidth(T x);
__generic<T : __BuiltinFloatingPointType, let N : int> __intrinsic vector<T,N> fwidth(vector<T,N> x);
__generic<T : __BuiltinFloatingPointType, let N : int, let M : int> __intrinsic matrix<T,N,M> fwidth(matrix<T,N,M> x);

)=", R"=(

// Get number of samples in render target
__intrinsic uint GetRenderTargetSampleCount();

// Get position of given sample
__intrinsic float2 GetRenderTargetSamplePosition(int Index);

// Group memory barrier
__intrinsic void GroupMemoryBarrier();
__intrinsic void GroupMemoryBarrierWithGroupSync();

// Atomics
__intrinsic void InterlockedAdd(in out  int dest,  int value, out  int original_value);
__intrinsic void InterlockedAdd(in out uint dest, uint value, out uint original_value);

__intrinsic void InterlockedAnd(in out  int dest,  int value, out  int original_value);
__intrinsic void InterlockedAnd(in out uint dest, uint value, out uint original_value);

__intrinsic void InterlockedCompareExchange(in out  int dest,  int compare_value,  int value, out  int original_value);
__intrinsic void InterlockedCompareExchange(in out uint dest, uint compare_value, uint value, out uint original_value);

__intrinsic void InterlockedCompareStore(in out  int dest,  int compare_value,  int value);
__intrinsic void InterlockedCompareStore(in out uint dest, uint compare_value, uint value);

__intrinsic void InterlockedExchange(in out  int dest,  int value, out  int original_value);
__intrinsic void InterlockedExchange(in out uint dest, uint value, out uint original_value);

__intrinsic void InterlockedMax(in out  int dest,  int value, out  int original_value);
__intrinsic void InterlockedMax(in out uint dest, uint value, out uint original_value);

__intrinsic void InterlockedMin(in out  int dest,  int value, out  int original_value);
__intrinsic void InterlockedMin(in out uint dest, uint value, out uint original_value);

__intrinsic void InterlockedOr(in out  int dest,  int value, out  int original_value);
__intrinsic void InterlockedOr(in out uint dest, uint value, out uint original_value);

__intrinsic void InterlockedXor(in out  int dest,  int value, out  int original_value);
__intrinsic void InterlockedXor(in out uint dest, uint value, out uint original_value);

// Is floating-point value finite?
__generic<T : __BuiltinFloatingPointType> __intrinsic bool isfinite(T x);
__generic<T : __BuiltinFloatingPointType, let N : int> __intrinsic vector<bool,N> isfinite(vector<T,N> x);
__generic<T : __BuiltinFloatingPointType, let N : int, let M : int> __intrinsic matrix<bool,N,M> isfinite(matrix<T,N,M> x);

// Is floating-point value infinite?
__generic<T : __BuiltinFloatingPointType> __intrinsic bool isinf(T x);
__generic<T : __BuiltinFloatingPointType, let N : int> __intrinsic vector<bool,N> isinf(vector<T,N> x);
__generic<T : __BuiltinFloatingPointType, let N : int, let M : int> __intrinsic matrix<bool,N,M> isinf(matrix<T,N,M> x);

// Is floating-point value not-a-number?
__generic<T : __BuiltinFloatingPointType> __intrinsic bool isnan(T x);
__generic<T : __BuiltinFloatingPointType, let N : int> __intrinsic vector<bool,N> isnan(vector<T,N> x);
__generic<T : __BuiltinFloatingPointType, let N : int, let M : int> __intrinsic matrix<bool,N,M> isnan(matrix<T,N,M> x);

// Construct float from mantissa and exponent
__generic<T : __BuiltinFloatingPointType> __intrinsic T ldexp(T x, T exp);
__generic<T : __BuiltinFloatingPointType, let N : int> __intrinsic vector<T,N> ldexp(vector<T,N> x, vector<T,N> exp);
__generic<T : __BuiltinFloatingPointType, let N : int, let M : int> __intrinsic matrix<T,N,M> ldexp(matrix<T,N,M> x, matrix<T,N,M> exp);

// Vector length
__generic<T : __BuiltinFloatingPointType, let N : int> __intrinsic T length(vector<T,N> x);

// Linear interpolation
__generic<T : __BuiltinFloatingPointType>
__intrinsic(glsl, mix)
__intrinsic
T lerp(T x, T y, T s);

__generic<T : __BuiltinFloatingPointType, let N : int>
__intrinsic(glsl, mix)
__intrinsic
vector<T,N> lerp(vector<T,N> x, vector<T,N> y, vector<T,N> s);

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__intrinsic(glsl, mix)
__intrinsic
matrix<T,N,M> lerp(matrix<T,N,M> x, matrix<T,N,M> y, matrix<T,N,M> s);

// Legacy lighting function (obsolete)
__intrinsic float4 lit(float n_dot_l, float n_dot_h, float m);

// Base-e logarithm
__generic<T : __BuiltinFloatingPointType> __intrinsic T log(T x);
__generic<T : __BuiltinFloatingPointType, let N : int> __intrinsic vector<T,N> log(vector<T,N> x);
__generic<T : __BuiltinFloatingPointType, let N : int, let M : int> __intrinsic matrix<T,N,M> log(matrix<T,N,M> x);

// Base-10 logarithm
__generic<T : __BuiltinFloatingPointType> __intrinsic T log10(T x);
__generic<T : __BuiltinFloatingPointType, let N : int> __intrinsic vector<T,N> log10(vector<T,N> x);
__generic<T : __BuiltinFloatingPointType, let N : int, let M : int> __intrinsic matrix<T,N,M> log10(matrix<T,N,M> x);

// Base-2 logarithm
__generic<T : __BuiltinFloatingPointType> __intrinsic T log2(T x);
__generic<T : __BuiltinFloatingPointType, let N : int> __intrinsic vector<T,N> log2(vector<T,N> x);
__generic<T : __BuiltinFloatingPointType, let N : int, let M : int> __intrinsic matrix<T,N,M> log2(matrix<T,N,M> x);

// multiply-add
__generic<T : __BuiltinArithmeticType> __intrinsic T mad(T mvalue, T avalue, T bvalue);
__generic<T : __BuiltinArithmeticType, let N : int> __intrinsic vector<T,N> mad(vector<T,N> mvalue, vector<T,N> avalue, vector<T,N> bvalue);
__generic<T : __BuiltinArithmeticType, let N : int, let M : int> __intrinsic matrix<T,N,M> mad(matrix<T,N,M> mvalue, matrix<T,N,M> avalue, matrix<T,N,M> bvalue);

// maximum
__generic<T : __BuiltinArithmeticType> __intrinsic T max(T x, T y);
__generic<T : __BuiltinArithmeticType, let N : int> __intrinsic vector<T,N> max(vector<T,N> x, vector<T,N> y);
__generic<T : __BuiltinArithmeticType, let N : int, let M : int> __intrinsic matrix<T,N,M> max(matrix<T,N,M> x, matrix<T,N,M> y);

// minimum
__generic<T : __BuiltinArithmeticType> __intrinsic T min(T x, T y);
__generic<T : __BuiltinArithmeticType, let N : int> __intrinsic vector<T,N> min(vector<T,N> x, vector<T,N> y);
__generic<T : __BuiltinArithmeticType, let N : int, let M : int> __intrinsic matrix<T,N,M> min(matrix<T,N,M> x, matrix<T,N,M> y);

// split into integer and fractional parts (both with same sign)
__generic<T : __BuiltinFloatingPointType> __intrinsic T modf(T x, out T ip);
__generic<T : __BuiltinFloatingPointType, let N : int> __intrinsic vector<T,N> modf(vector<T,N> x, out vector<T,N> ip);
__generic<T : __BuiltinFloatingPointType, let N : int, let M : int> __intrinsic matrix<T,N,M> modf(matrix<T,N,M> x, out matrix<T,N,M> ip);

// msad4 (whatever that is)
__intrinsic uint4 msad4(uint reference, uint2 source, uint4 accum);

// General inner products

// scalar-scalar
__generic<T : __BuiltinArithmeticType> __intrinsic_op(Mul_Scalar_Scalar) T mul(T x, T y);

// scalar-vector and vector-scalar
__generic<T : __BuiltinArithmeticType, let N : int> __intrinsic_op(Mul_Vector_Scalar) vector<T,N> mul(vector<T,N> x, T y);
__generic<T : __BuiltinArithmeticType, let N : int> __intrinsic_op(Mul_Scalar_Vector) vector<T,N> mul(T x, vector<T,N> y);

// scalar-matrix and matrix-scalar
__generic<T : __BuiltinArithmeticType, let N : int, let M :int> __intrinsic_op(Mul_Matrix_Scalar) matrix<T,N,M> mul(matrix<T,N,M> x, T y);
__generic<T : __BuiltinArithmeticType, let N : int, let M :int> __intrinsic_op(Mul_Scalar_Matrix) matrix<T,N,M> mul(T x, matrix<T,N,M> y);

// vector-vector (dot product)
__generic<T : __BuiltinArithmeticType, let N : int> __intrinsic_op(InnerProduct_Vector_Vector) T mul(vector<T,N> x, vector<T,N> y);

// vector-matrix
__generic<T : __BuiltinArithmeticType, let N : int, let M : int> __intrinsic_op(InnerProduct_Vector_Matrix) vector<T,M> mul(vector<T,N> x, matrix<T,N,M> y);

// matrix-vector
__generic<T : __BuiltinArithmeticType, let N : int, let M : int> __intrinsic_op(InnerProduct_Matrix_Vector) vector<T,N> mul(matrix<T,N,M> x, vector<T,M> y);

// matrix-matrix
__generic<T : __BuiltinArithmeticType, let R : int, let N : int, let C : int> __intrinsic_op(InnerProduct_Matrix_Matrix) matrix<T,R,C> mul(matrix<T,R,N> x, matrix<T,N,C> y);

// noise (deprecated)
__intrinsic float noise(float x);
__generic<let N : int> __intrinsic float noise(vector<float, N> x);

// Normalize a vector
__generic<T : __BuiltinFloatingPointType, let N : int> __intrinsic vector<T,N> normalize(vector<T,N> x);

// Raise to a power
__generic<T : __BuiltinFloatingPointType> __intrinsic T pow(T x, T y);
__generic<T : __BuiltinFloatingPointType, let N : int> __intrinsic vector<T,N> pow(vector<T,N> x, vector<T,N> y);
__generic<T : __BuiltinFloatingPointType, let N : int, let M : int> __intrinsic matrix<T,N,M> pow(matrix<T,N,M> x, matrix<T,N,M> y);

// Output message

// __intrinsic void printf( string format, ... );

// Tessellation factor fixup routines

__intrinsic void Process2DQuadTessFactorsAvg(
    in  float4 RawEdgeFactors,
    in  float2 InsideScale,
    out float4 RoundedEdgeTessFactors,
    out float2 RoundedInsideTessFactors,
    out float2 UnroundedInsideTessFactors);

__intrinsic void Process2DQuadTessFactorsMax(
    in  float4 RawEdgeFactors,
    in  float2 InsideScale,
    out float4 RoundedEdgeTessFactors,
    out float2 RoundedInsideTessFactors,
    out float2 UnroundedInsideTessFactors);

__intrinsic void Process2DQuadTessFactorsMin(
    in  float4 RawEdgeFactors,
    in  float2 InsideScale,
    out float4 RoundedEdgeTessFactors,
    out float2 RoundedInsideTessFactors,
    out float2 UnroundedInsideTessFactors);

__intrinsic void ProcessIsolineTessFactors(
    in  float RawDetailFactor,
    in  float RawDensityFactor,
    out float RoundedDetailFactor,
    out float RoundedDensityFactor);

__intrinsic void ProcessQuadTessFactorsAvg(
    in  float4 RawEdgeFactors,
    in  float InsideScale,
    out float4 RoundedEdgeTessFactors,
    out float2 RoundedInsideTessFactors,
    out float2 UnroundedInsideTessFactors);

__intrinsic void ProcessQuadTessFactorsMax(
    in  float4 RawEdgeFactors,
    in  float InsideScale,
    out float4 RoundedEdgeTessFactors,
    out float2 RoundedInsideTessFactors,
    out float2 UnroundedInsideTessFactors);

__intrinsic void ProcessQuadTessFactorsMin(
    in  float4 RawEdgeFactors,
    in  float InsideScale,
    out float4 RoundedEdgeTessFactors,
    out float2 RoundedInsideTessFactors,
    out float2 UnroundedInsideTessFactors);

__intrinsic void ProcessTriTessFactorsAvg(
    in  float3 RawEdgeFactors,
    in  float InsideScale,
    out float3 RoundedEdgeTessFactors,
    out float RoundedInsideTessFactor,
    out float UnroundedInsideTessFactor);

__intrinsic void ProcessTriTessFactorsMax(
    in  float3 RawEdgeFactors,
    in  float InsideScale,
    out float3 RoundedEdgeTessFactors,
    out float RoundedInsideTessFactor,
    out float UnroundedInsideTessFactor);

__intrinsic void ProcessTriTessFactorsMin(
    in  float3 RawEdgeFactors,
    in  float InsideScale,
    out float3 RoundedEdgeTessFactors,
    out float RoundedInsideTessFactors,
    out float UnroundedInsideTessFactors);

// Degrees to radians
__generic<T : __BuiltinFloatingPointType> __intrinsic T radians(T x);
__generic<T : __BuiltinFloatingPointType, let N : int> __intrinsic vector<T,N> radians(vector<T,N> x);
__generic<T : __BuiltinFloatingPointType, let N : int, let M : int> __intrinsic matrix<T,N,M> radians(matrix<T,N,M> x);

// Approximate reciprocal
__generic<T : __BuiltinFloatingPointType> __intrinsic T rcp(T x);
__generic<T : __BuiltinFloatingPointType, let N : int> __intrinsic vector<T,N> rcp(vector<T,N> x);
__generic<T : __BuiltinFloatingPointType, let N : int, let M : int> __intrinsic matrix<T,N,M> rcp(matrix<T,N,M> x);

// Reflect incident vector across plane with given normal
__generic<T : __BuiltinFloatingPointType, let N : int>
__intrinsic
vector<T,N> reflect(vector<T,N> i, vector<T,N> n);

// Refract incident vector given surface normal and index of refraction
__generic<T : __BuiltinFloatingPointType, let N : int>
__intrinsic
vector<T,N> refract(vector<T,N> i, vector<T,N> n, float eta);

// Reverse order of bits
__intrinsic uint reversebits(uint value);
__generic<let N : int> vector<uint,N> reversebits(vector<uint,N> value);

// Round-to-nearest
__generic<T : __BuiltinFloatingPointType> __intrinsic T round(T x);
__generic<T : __BuiltinFloatingPointType, let N : int> __intrinsic vector<T,N> round(vector<T,N> x);
__generic<T : __BuiltinFloatingPointType, let N : int, let M : int> __intrinsic matrix<T,N,M> round(matrix<T,N,M> x);

// Reciprocal of square root
__generic<T : __BuiltinFloatingPointType> __intrinsic T rsqrt(T x);
__generic<T : __BuiltinFloatingPointType, let N : int> __intrinsic vector<T,N> rsqrt(vector<T,N> x);
__generic<T : __BuiltinFloatingPointType, let N : int, let M : int> __intrinsic matrix<T,N,M> rsqrt(matrix<T,N,M> x);

// Clamp value to [0,1] range
__generic<T : __BuiltinFloatingPointType>
__intrinsic(glsl, "clamp($0, 0, 1)") __intrinsic
T saturate(T x);

__generic<T : __BuiltinFloatingPointType, let N : int>
__intrinsic(glsl, "clamp($0, 0, 1)") __intrinsic
vector<T,N> saturate(vector<T,N> x);

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__intrinsic(glsl, "clamp($0, 0, 1)") __intrinsic
matrix<T,N,M> saturate(matrix<T,N,M> x);


// Extract sign of value
__generic<T : __BuiltinSignedArithmeticType> __intrinsic int sign(T x);
__generic<T : __BuiltinSignedArithmeticType, let N : int> __intrinsic vector<int,N> sign(vector<T,N> x);
__generic<T : __BuiltinSignedArithmeticType, let N : int, let M : int> __intrinsic matrix<int,N,M> sign(matrix<T,N,M> x);

)=", R"=(


// Sine
__generic<T : __BuiltinFloatingPointType> __intrinsic T sin(T x);
__generic<T : __BuiltinFloatingPointType, let N : int> __intrinsic vector<T,N> sin(vector<T,N> x);
__generic<T : __BuiltinFloatingPointType, let N : int, let M : int> __intrinsic matrix<T,N,M> sin(matrix<T,N,M> x);

// Sine and cosine
__generic<T : __BuiltinFloatingPointType, let N : int> __intrinsic void sincos(T x, out T s, out T c);
__generic<T : __BuiltinFloatingPointType, let N : int> __intrinsic void sincos(vector<T,N> x, out vector<T,N> s, out vector<T,N> c);
__generic<T : __BuiltinFloatingPointType, let N : int, let M : int> __intrinsic void sincos(matrix<T,N,M> x, out matrix<T,N,M> s, out matrix<T,N,M> c);

// Hyperbolic Sine
__generic<T : __BuiltinFloatingPointType> __intrinsic T sinh(T x);
__generic<T : __BuiltinFloatingPointType, let N : int> __intrinsic vector<T,N> sinh(vector<T,N> x);
__generic<T : __BuiltinFloatingPointType, let N : int, let M : int> __intrinsic matrix<T,N,M> sinh(matrix<T,N,M> x);

// Smooth step (Hermite interpolation)
__generic<T : __BuiltinFloatingPointType> __intrinsic T smoothstep(T min, T max, T x);
__generic<T : __BuiltinFloatingPointType, let N : int> __intrinsic vector<T,N> smoothstep(vector<T,N> min, vector<T,N> max, vector<T,N> x);
__generic<T : __BuiltinFloatingPointType, let N : int, let M : int> __intrinsic matrix<T,N,M> smoothstep(matrix<T,N,M> min, matrix<T,N,M> max, matrix<T,N,M> x);

// Square root
__generic<T : __BuiltinFloatingPointType> __intrinsic T sqrt(T x);
__generic<T : __BuiltinFloatingPointType, let N : int> __intrinsic vector<T,N> sqrt(vector<T,N> x);
__generic<T : __BuiltinFloatingPointType, let N : int, let M : int> __intrinsic matrix<T,N,M> sqrt(matrix<T,N,M> x);

// Step function
__generic<T : __BuiltinFloatingPointType> __intrinsic T step(T y, T x);
__generic<T : __BuiltinFloatingPointType, let N : int> __intrinsic vector<T,N> step(vector<T,N> y, vector<T,N> x);
__generic<T : __BuiltinFloatingPointType, let N : int, let M : int> __intrinsic matrix<T,N,M> step(matrix<T,N,M> y, matrix<T,N,M> x);

// Tangent
__generic<T : __BuiltinFloatingPointType> __intrinsic T tan(T x);
__generic<T : __BuiltinFloatingPointType, let N : int> __intrinsic vector<T,N> tan(vector<T,N> x);
__generic<T : __BuiltinFloatingPointType, let N : int, let M : int> __intrinsic matrix<T,N,M> tan(matrix<T,N,M> x);

// Hyperbolic tangent
__generic<T : __BuiltinFloatingPointType> __intrinsic T tanh(T x);
__generic<T : __BuiltinFloatingPointType, let N : int> __intrinsic vector<T,N> tanh(vector<T,N> x);
__generic<T : __BuiltinFloatingPointType, let N : int, let M : int> __intrinsic matrix<T,N,M> tanh(matrix<T,N,M> x);

// Legacy texture-fetch operations

/*
__intrinsic float4 tex1D(sampler1D s, float t);
__intrinsic float4 tex1D(sampler1D s, float t, float ddx, float ddy);
__intrinsic float4 tex1Dbias(sampler1D s, float4 t);
__intrinsic float4 tex1Dgrad(sampler1D s, float t, float ddx, float ddy);
__intrinsic float4 tex1Dlod(sampler1D s, float4 t);
__intrinsic float4 tex1Dproj(sampler1D s, float4 t);

__intrinsic float4 tex2D(sampler2D s, float2 t);
__intrinsic float4 tex2D(sampler2D s, float2 t, float2 ddx, float2 ddy);
__intrinsic float4 tex2Dbias(sampler2D s, float4 t);
__intrinsic float4 tex2Dgrad(sampler2D s, float2 t, float2 ddx, float2 ddy);
__intrinsic float4 tex2Dlod(sampler2D s, float4 t);
__intrinsic float4 tex2Dproj(sampler2D s, float4 t);

__intrinsic float4 tex3D(sampler3D s, float3 t);
__intrinsic float4 tex3D(sampler3D s, float3 t, float3 ddx, float3 ddy);
__intrinsic float4 tex3Dbias(sampler3D s, float4 t);
__intrinsic float4 tex3Dgrad(sampler3D s, float3 t, float3 ddx, float3 ddy);
__intrinsic float4 tex3Dlod(sampler3D s, float4 t);
__intrinsic float4 tex3Dproj(sampler3D s, float4 t);

__intrinsic float4 texCUBE(samplerCUBE s, float3 t);
__intrinsic float4 texCUBE(samplerCUBE s, float3 t, float3 ddx, float3 ddy);
__intrinsic float4 texCUBEbias(samplerCUBE s, float4 t);
__intrinsic float4 texCUBEgrad(samplerCUBE s, float3 t, float3 ddx, float3 ddy);
__intrinsic float4 texCUBElod(samplerCUBE s, float4 t);
__intrinsic float4 texCUBEproj(samplerCUBE s, float4 t);
*/

// Matrix transpose
__generic<T : __BuiltinType, let N : int, let M : int> __intrinsic matrix<T,M,N> transpose(matrix<T,N,M> x);

// Truncate to integer
__generic<T : __BuiltinFloatingPointType> __intrinsic T trunc(T x);
__generic<T : __BuiltinFloatingPointType, let N : int> __intrinsic vector<T,N> trunc(vector<T,N> x);
__generic<T : __BuiltinFloatingPointType, let N : int, let M : int> __intrinsic matrix<T,N,M> trunc(matrix<T,N,M> x);


)=", R"=(

// Shader model 6.0 stuff

__intrinsic uint GlobalOrderedCountIncrement(uint countToAppendForThisLane);

__generic<T : __BuiltinType> __intrinsic T QuadReadLaneAt(T sourceValue, int quadLaneID);
__generic<T : __BuiltinType, let N : int> __intrinsic vector<T,N> QuadReadLaneAt(vector<T,N> sourceValue, int quadLaneID);
__generic<T : __BuiltinType, let N : int, let M : int> __intrinsic matrix<T,N,M> QuadReadLaneAt(matrix<T,N,M> sourceValue, int quadLaneID);

__generic<T : __BuiltinType> __intrinsic T QuadSwapX(T localValue);
__generic<T : __BuiltinType, let N : int> __intrinsic vector<T,N> QuadSwapX(vector<T,N> localValue);
__generic<T : __BuiltinType, let N : int, let M : int> __intrinsic matrix<T,N,M> QuadSwapX(matrix<T,N,M> localValue);

__generic<T : __BuiltinType> __intrinsic T QuadSwapY(T localValue);
__generic<T : __BuiltinType, let N : int> __intrinsic vector<T,N> QuadSwapY(vector<T,N> localValue);
__generic<T : __BuiltinType, let N : int, let M : int> __intrinsic matrix<T,N,M> QuadSwapY(matrix<T,N,M> localValue);

__generic<T : __BuiltinIntegerType> __intrinsic T WaveAllBitAnd(T expr);
__generic<T : __BuiltinIntegerType, let N : int> __intrinsic vector<T,N> WaveAllBitAnd(vector<T,N> expr);
__generic<T : __BuiltinIntegerType, let N : int, let M : int> __intrinsic matrix<T,N,M> WaveAllBitAnd(matrix<T,N,M> expr);

__generic<T : __BuiltinIntegerType> __intrinsic T WaveAllBitOr(T expr);
__generic<T : __BuiltinIntegerType, let N : int> __intrinsic vector<T,N> WaveAllBitOr(vector<T,N> expr);
__generic<T : __BuiltinIntegerType, let N : int, let M : int> __intrinsic matrix<T,N,M> WaveAllBitOr(matrix<T,N,M> expr);

__generic<T : __BuiltinIntegerType> __intrinsic T WaveAllBitXor(T expr);
__generic<T : __BuiltinIntegerType, let N : int> __intrinsic vector<T,N> WaveAllBitXor(vector<T,N> expr);
__generic<T : __BuiltinIntegerType, let N : int, let M : int> __intrinsic matrix<T,N,M> WaveAllBitXor(matrix<T,N,M> expr);

__generic<T : __BuiltinArithmeticType> __intrinsic T WaveAllMax(T expr);
__generic<T : __BuiltinArithmeticType, let N : int> __intrinsic vector<T,N> WaveAllMax(vector<T,N> expr);
__generic<T : __BuiltinArithmeticType, let N : int, let M : int> __intrinsic matrix<T,N,M> WaveAllMax(matrix<T,N,M> expr);

__generic<T : __BuiltinArithmeticType> __intrinsic T WaveAllMin(T expr);
__generic<T : __BuiltinArithmeticType, let N : int> __intrinsic vector<T,N> WaveAllMin(vector<T,N> expr);
__generic<T : __BuiltinArithmeticType, let N : int, let M : int> __intrinsic matrix<T,N,M> WaveAllMin(matrix<T,N,M> expr);

__generic<T : __BuiltinArithmeticType> __intrinsic T WaveAllProduct(T expr);
__generic<T : __BuiltinArithmeticType, let N : int> __intrinsic vector<T,N> WaveAllProduct(vector<T,N> expr);
__generic<T : __BuiltinArithmeticType, let N : int, let M : int> __intrinsic matrix<T,N,M> WaveAllProduct(matrix<T,N,M> expr);

__generic<T : __BuiltinArithmeticType> __intrinsic T WaveAllSum(T expr);
__generic<T : __BuiltinArithmeticType, let N : int> __intrinsic vector<T,N> WaveAllSum(vector<T,N> expr);
__generic<T : __BuiltinArithmeticType, let N : int, let M : int> __intrinsic matrix<T,N,M> WaveAllSum(matrix<T,N,M> expr);

__intrinsic bool WaveAllEqual(bool expr);
__intrinsic bool WaveAllTrue(bool expr);
__intrinsic bool WaveAnyTrue(bool expr);

uint64_t WaveBallot(bool expr);

uint WaveGetLaneCount();
uint WaveGetLaneIndex();
uint WaveGetOrderedIndex();

bool WaveIsHelperLane();

bool WaveOnce();

__generic<T : __BuiltinArithmeticType> __intrinsic T WavePrefixProduct(T expr);
__generic<T : __BuiltinArithmeticType, let N : int> __intrinsic vector<T,N> WavePrefixProduct(vector<T,N> expr);
__generic<T : __BuiltinArithmeticType, let N : int, let M : int> __intrinsic matrix<T,N,M> WavePrefixProduct(matrix<T,N,M> expr);

__generic<T : __BuiltinArithmeticType> __intrinsic T WavePrefixSum(T expr);
__generic<T : __BuiltinArithmeticType, let N : int> __intrinsic vector<T,N> WavePrefixSum(vector<T,N> expr);
__generic<T : __BuiltinArithmeticType, let N : int, let M : int> __intrinsic matrix<T,N,M> WavePrefixSum(matrix<T,N,M> expr);

__generic<T : __BuiltinType> __intrinsic T WaveReadFirstLane(T expr);
__generic<T : __BuiltinType, let N : int> __intrinsic vector<T,N> WaveReadFirstLane(vector<T,N> expr);
__generic<T : __BuiltinType, let N : int, let M : int> __intrinsic matrix<T,N,M> WaveReadFirstLane(matrix<T,N,M> expr);

__generic<T : __BuiltinType> __intrinsic T WaveReadLaneAt(T expr, int laneIndex);
__generic<T : __BuiltinType, let N : int> __intrinsic vector<T,N> WaveReadLaneAt(vector<T,N> expr, int laneIndex);
__generic<T : __BuiltinType, let N : int, let M : int> __intrinsic matrix<T,N,M> WaveReadLaneAt(matrix<T,N,M> expr, int laneIndex);


)=", R"=(

// `typedef`s to help with the fact that HLSL has been sorta-kinda case insensitive at various points
typedef Texture2D texture2D;

#line default
)=" };


namespace Slang
{
    static String stdlibPath;

    String getStdlibPath()
    {
        if(stdlibPath.Length() != 0)
            return stdlibPath;

        StringBuilder pathBuilder;
        for( auto cc = __FILE__; *cc; ++cc )
        {
            switch( *cc )
            {
            case '\n':
            case '\t':
            case '\\':
                pathBuilder << "\\";
            default:
                pathBuilder << *cc;
                break;
            }
        }
        stdlibPath = pathBuilder.ProduceString();

        return stdlibPath;
    }

    // Cached code for the various libraries
    String coreLibraryCode;
    String slangLibraryCode;
    String hlslLibraryCode;
    String glslLibraryCode;

    enum
    {
        SINT_MASK   = 1 << 0,
        FLOAT_MASK  = 1 << 1,
        COMPARISON  = 1 << 2,
        BOOL_MASK   = 1 << 3,
        UINT_MASK   = 1 << 4,
        ASSIGNMENT  = 1 << 5,
        POSTFIX     = 1 << 6,

        INT_MASK = SINT_MASK | UINT_MASK,
        ARITHMETIC_MASK = INT_MASK | FLOAT_MASK,
        LOGICAL_MASK = INT_MASK | BOOL_MASK,
        ANY_MASK = INT_MASK | FLOAT_MASK | BOOL_MASK,
    };

    static const struct {
        char const* name;
        BaseType	tag;
        unsigned    flags;
    } kBaseTypes[] = {
        { "void",	BaseType::Void,     0 },
        { "int",	BaseType::Int,      SINT_MASK },
        { "half",	BaseType::Half,     FLOAT_MASK },
        { "float",	BaseType::Float,    FLOAT_MASK },
        { "double",	BaseType::Double,   FLOAT_MASK },
        { "uint",	BaseType::UInt,     UINT_MASK },
        { "bool",	BaseType::Bool,     BOOL_MASK },
        { "uint64_t", BaseType::UInt64, UINT_MASK },
    };

    struct OpInfo { IntrinsicOp opCode; char const* opName; unsigned flags; };

    static const OpInfo unaryOps[] = {
        { IntrinsicOp::Pos,     "+",    ARITHMETIC_MASK },
        { IntrinsicOp::Neg,     "-",    ARITHMETIC_MASK },
        { IntrinsicOp::Not,     "!",    ANY_MASK        },
        { IntrinsicOp::BitNot,  "~",    INT_MASK        },
        { IntrinsicOp::PreInc,  "++",   ARITHMETIC_MASK | ASSIGNMENT },
        { IntrinsicOp::PreDec,  "--",   ARITHMETIC_MASK | ASSIGNMENT },
        { IntrinsicOp::PostInc, "++",   ARITHMETIC_MASK | ASSIGNMENT | POSTFIX },
        { IntrinsicOp::PostDec, "--",   ARITHMETIC_MASK | ASSIGNMENT | POSTFIX },
    };

    static const OpInfo binaryOps[] = {
        { IntrinsicOp::Add,     "+",    ARITHMETIC_MASK },
        { IntrinsicOp::Sub,     "-",    ARITHMETIC_MASK },
        { IntrinsicOp::Mul,     "*",    ARITHMETIC_MASK },
        { IntrinsicOp::Div,     "/",    ARITHMETIC_MASK },
        { IntrinsicOp::Mod,     "%",    INT_MASK },

        { IntrinsicOp::And,     "&&",   LOGICAL_MASK },
        { IntrinsicOp::Or,      "||",   LOGICAL_MASK },

        { IntrinsicOp::BitAnd,  "&",    LOGICAL_MASK },
        { IntrinsicOp::BitOr,   "|",    LOGICAL_MASK },
        { IntrinsicOp::BitXor,  "^",    LOGICAL_MASK },

        { IntrinsicOp::Lsh,     "<<",   INT_MASK },
        { IntrinsicOp::Rsh,     ">>",   INT_MASK },

        { IntrinsicOp::Eql,     "==",   ANY_MASK | COMPARISON },
        { IntrinsicOp::Neq,     "!=",   ANY_MASK | COMPARISON },

        { IntrinsicOp::Greater, ">",    ARITHMETIC_MASK | COMPARISON },
        { IntrinsicOp::Less,    "<",    ARITHMETIC_MASK | COMPARISON },
        { IntrinsicOp::Geq,     ">=",   ARITHMETIC_MASK | COMPARISON },
        { IntrinsicOp::Leq,     "<=",   ARITHMETIC_MASK | COMPARISON },

        { IntrinsicOp::AddAssign,     "+=",    ASSIGNMENT | ARITHMETIC_MASK },
        { IntrinsicOp::SubAssign,     "-=",    ASSIGNMENT | ARITHMETIC_MASK },
        { IntrinsicOp::MulAssign,     "*=",    ASSIGNMENT | ARITHMETIC_MASK },
        { IntrinsicOp::DivAssign,     "/=",    ASSIGNMENT | ARITHMETIC_MASK },
        { IntrinsicOp::ModAssign,     "%=",    ASSIGNMENT | ARITHMETIC_MASK },
        { IntrinsicOp::AndAssign,     "&=",    ASSIGNMENT | LOGICAL_MASK },
        { IntrinsicOp::OrAssign,      "|=",    ASSIGNMENT | LOGICAL_MASK },
        { IntrinsicOp::XorAssign,     "^=",    ASSIGNMENT | LOGICAL_MASK },
        { IntrinsicOp::LshAssign,     "<<=",   ASSIGNMENT | INT_MASK },
        { IntrinsicOp::RshAssign,     ">>=",   ASSIGNMENT | INT_MASK },
    };


    String getCoreLibraryCode()
    {
        if (coreLibraryCode.Length() > 0)
            return coreLibraryCode;

        StringBuilder sb;

        // generate operator overloads


        String path = getStdlibPath();

#define EMIT_LINE_DIRECTIVE() sb << "#line " << (__LINE__+1) << " \"" << path << "\"\n"

        // Generate declarations for all the base types

        static const int kBaseTypeCount = sizeof(kBaseTypes) / sizeof(kBaseTypes[0]);
        for (int tt = 0; tt < kBaseTypeCount; ++tt)
        {
            EMIT_LINE_DIRECTIVE();
            sb << "__builtin_type(" << int(kBaseTypes[tt].tag) << ") struct " << kBaseTypes[tt].name;

            // Declare interface conformances for this type

            sb << "\n    : __BuiltinType\n";

            switch (kBaseTypes[tt].tag)
            {
            case BaseType::Float:
                sb << "\n    , __BuiltinFloatingPointType\n";
                sb << "\n    ,  __BuiltinRealType\n";
                // fall through to:
            case BaseType::Int:
                sb << "\n    ,  __BuiltinSignedArithmeticType\n";
                // fall through to:
            case BaseType::UInt:
            case BaseType::UInt64:
                sb << "\n    ,  __BuiltinArithmeticType\n";
                // fall through to:
            case BaseType::Bool:
                sb << "\n    ,  __BuiltinType\n";
                break;

            default:
                break;
            }

            sb << "\n{\n";


            // Declare initializers to convert from various other types
            for (int ss = 0; ss < kBaseTypeCount; ++ss)
            {
                if (kBaseTypes[ss].tag == BaseType::Void)
                    continue;

                EMIT_LINE_DIRECTIVE();
                sb << "__init(" << kBaseTypes[ss].name << " value);\n";
            }

            sb << "};\n";
        }

        // Declare vector and matrix types

        sb << "__generic<T = float, let N : int = 4> __magic_type(Vector) struct vector\n{\n";
        sb << "    typedef T Element;\n";
        sb << "    __init(T value);\n"; // initialize from single scalar
        sb << "};\n";
        sb << "__generic<T = float, let R : int = 4, let C : int = 4> __magic_type(Matrix) struct matrix {};\n";

        static const struct {
            char const* name;
            char const* glslPrefix;
        } kTypes[] =
        {
            {"float", ""},
            {"int", "i"},
            {"uint", "u"},
            {"bool", "b"},
        };
        static const int kTypeCount = sizeof(kTypes) / sizeof(kTypes[0]);

        for (int tt = 0; tt < kTypeCount; ++tt)
        {
            // Declare HLSL vector types
            for (int ii = 1; ii <= 4; ++ii)
            {
                sb << "typedef vector<" << kTypes[tt].name << "," << ii << "> " << kTypes[tt].name << ii << ";\n";
            }

            // Declare HLSL matrix types
            for (int rr = 2; rr <= 4; ++rr)
            for (int cc = 2; cc <= 4; ++cc)
            {
                sb << "typedef matrix<" << kTypes[tt].name << "," << rr << "," << cc << "> " << kTypes[tt].name << rr << "x" << cc << ";\n";
            }
        }

        // Declare additional built-in generic types
//        EMIT_LINE_DIRECTIVE();
        sb << "__generic<T> __magic_type(ConstantBuffer) struct ConstantBuffer {};\n";
        sb << "__generic<T> __magic_type(TextureBuffer) struct TextureBuffer {};\n";


        static const char* kComponentNames[]{ "x", "y", "z", "w" };
        static const char* kVectorNames[]{ "", "x", "xy", "xyz", "xyzw" };

        // Need to add constructors to the types above
        for (int N = 2; N <= 4; ++N)
        {
            sb << "__generic<T> __extension vector<T, " << N << ">\n{\n";

            // initialize from N scalars
            sb << "__init(";
            for (int ii = 0; ii < N; ++ii)
            {
                if (ii != 0) sb << ", ";
                sb << "T " << kComponentNames[ii];
            }
            sb << ");\n";

            // Initialize from an M-vector and then scalars
            for (int M = 2; M < N; ++M)
            {
                sb << "__init(vector<T," << M << "> " << kVectorNames[M];
                for (int ii = M; ii < N; ++ii)
                {
                    sb << ", T " << kComponentNames[ii];
                }
                sb << ");\n";
            }

            // initialize from another vector of the same size
            //
            // TODO(tfoley): this overlaps with implicit conversions.
            // We should look for a way that we can define implicit
            // conversions directly in the stdlib instead...
            sb << "__generic<U> __init(vector<U," << N << ">);\n";

            // Initialize from two vectors, of size M and N-M
            for(int M = 2; M <= (N-2); ++M)
            {
                int K = N - M;
                assert(K >= 2);

                sb << "__init(vector<T," << M << "> " << kVectorNames[M];
                sb << ", vector<T," << K << "> ";
                for (int ii = 0; ii < K; ++ii)
                {
                    sb << kComponentNames[ii];
                }
                sb << ");\n";
            }

            sb << "}\n";
        }

        for( int R = 2; R <= 4; ++R )
        for( int C = 2; C <= 4; ++C )
        {
            sb << "__generic<T> __extension matrix<T, " << R << "," << C << ">\n{\n";

            // initialize from R*C scalars
            sb << "__init(";
            for( int ii = 0; ii < R; ++ii )
            for( int jj = 0; jj < C; ++jj )
            {
                if ((ii+jj) != 0) sb << ", ";
                sb << "T m" << ii << jj;
            }
            sb << ");\n";

            // Initialize from R C-vectors
            sb << "__init(";
            for (int ii = 0; ii < R; ++ii)
            {
                if(ii != 0) sb << ", ";
                sb << "vector<T," << C << "> row" << ii;
            }
            sb << ");\n";


            // initialize from another matrix of the same size
            //
            // TODO(tfoley): See comment about how this overlaps
            // with implicit conversion, in the `vector` case above
            sb << "__generic<U> __init(matrix<U," << R << ", " << C << ">);\n";

            sb << "}\n";
        }

        // Declare built-in texture and sampler types

        sb << "__magic_type(SamplerState," << int(SamplerStateType::Flavor::SamplerState) << ") struct SamplerState {};";
        sb << "__magic_type(SamplerState," << int(SamplerStateType::Flavor::SamplerComparisonState) << ") struct SamplerComparisonState {};";

        // TODO(tfoley): Need to handle `RW*` variants of texture types as well...
        static const struct {
            char const*			name;
            TextureType::Shape	baseShape;
            int					coordCount;
        } kBaseTextureTypes[] = {
            { "Texture1D",		TextureType::Shape1D,	1 },
            { "Texture2D",		TextureType::Shape2D,	2 },
            { "Texture3D",		TextureType::Shape3D,	3 },
            { "TextureCube",	TextureType::ShapeCube,	3 },
        };
        static const int kBaseTextureTypeCount = sizeof(kBaseTextureTypes) / sizeof(kBaseTextureTypes[0]);


        static const struct {
            char const*         name;
            SlangResourceAccess access;
        } kBaseTextureAccessLevels[] = {
            { "",                   SLANG_RESOURCE_ACCESS_READ },
            { "RW",                 SLANG_RESOURCE_ACCESS_READ_WRITE },
            { "RasterizerOrdered",  SLANG_RESOURCE_ACCESS_RASTER_ORDERED },
        };
        static const int kBaseTextureAccessLevelCount = sizeof(kBaseTextureAccessLevels) / sizeof(kBaseTextureAccessLevels[0]);

        for (int tt = 0; tt < kBaseTextureTypeCount; ++tt)
        {
            char const* name = kBaseTextureTypes[tt].name;
            TextureType::Shape baseShape = kBaseTextureTypes[tt].baseShape;

            for (int isArray = 0; isArray < 2; ++isArray)
            {
                // Arrays of 3D textures aren't allowed
                if (isArray && baseShape == TextureType::Shape3D) continue;

                for (int isMultisample = 0; isMultisample < 2; ++isMultisample)
                for (int accessLevel = 0; accessLevel < kBaseTextureAccessLevelCount; ++accessLevel)
                {
                    auto access = kBaseTextureAccessLevels[accessLevel].access;

                    // TODO: any constraints to enforce on what gets to be multisampled?

                    unsigned flavor = baseShape;
                    if (isArray)		flavor |= TextureType::ArrayFlag;
                    if (isMultisample)	flavor |= TextureType::MultisampleFlag;
//                        if (isShadow)		flavor |= TextureType::ShadowFlag;

                    flavor |= (access << 8);

                    // emit a generic signature
                    // TODO: allow for multisample count to come in as well...
                    sb << "__generic<T = float4> ";

                    sb << "__magic_type(Texture," << int(flavor) << ") struct ";
                    sb << kBaseTextureAccessLevels[accessLevel].name;
                    sb << name;
                    if (isMultisample) sb << "MS";
                    if (isArray) sb << "Array";
//                        if (isShadow) sb << "Shadow";
                    sb << "\n{";

                    if( !isMultisample )
                    {
                        sb << "float CalculateLevelOfDetail(SamplerState s, ";
                        sb << "float" << kBaseTextureTypes[tt].coordCount << " location);\n";

                        sb << "float CalculateLevelOfDetailUnclamped(SamplerState s, ";
                        sb << "float" << kBaseTextureTypes[tt].coordCount << " location);\n";
                    }

                    // `GetDimensions`

                    for(int isFloat = 0; isFloat < 2; ++isFloat)
                    for(int includeMipInfo = 0; includeMipInfo < 2; ++includeMipInfo)
                    {
                        char const* t = isFloat ? "out float " : "out uint ";

                        sb << "void GetDimensions(";
                        if(includeMipInfo)
                            sb << "uint mipLevel, ";

                        switch(baseShape)
                        {
                        case TextureType::Shape1D:
                            sb << t << "width";
                            break;

                        case TextureType::Shape2D:
                        case TextureType::ShapeCube:
                            sb << t << "width,";
                            sb << t << "height";
                            break;

                        case TextureType::Shape3D:
                            sb << t << "width,";
                            sb << t << "height,";
                            sb << t << "depth";
                            break;

                        default:
                            assert(!"unexpected");
                            break;
                        }

                        if(isArray)
                        {
                            sb << ", " << t << "elements";
                        }

                        if(isMultisample)
                        {
                            sb << ", " << t << "sampleCount";
                        }

                        if(includeMipInfo)
                            sb << ", " << t << "numberOfLevels";

                        sb << ");\n";
                    }

                    // `GetSamplePosition()`
                    if( isMultisample )
                    {
                        sb << "float2 GetSamplePosition(int s);\n";
                    }

                    // `Load()`

                    if( kBaseTextureTypes[tt].coordCount + isArray < 4 )
                    {
                        int loadCoordCount = kBaseTextureTypes[tt].coordCount + isArray + (isMultisample?0:1);

                        // When translating to GLSL, we need to break apart the `location` argument.
                        //
                        // TODO: this should realy be handled by having this member actually get lowered!
                        static const char* kGLSLLoadCoordsSwizzle[] = { "", "", "x", "xy", "xyz", "xyzw" };
                        static const char* kGLSLLoadLODSwizzle[]    = { "", "", "y", "z", "w", "error" };

                        if (isMultisample)
                        {
                            sb << "__intrinsic(glsl, \"texelFetch($P, $0, $1)\")\n";
                        }
                        else
                        {
                            sb << "__intrinsic(glsl, \"texelFetch($P, ($0)." << kGLSLLoadCoordsSwizzle[loadCoordCount] << ", ($0)." << kGLSLLoadLODSwizzle[loadCoordCount] << ")\")\n";
                        }
                        sb << "__intrinsic\n";
                        sb << "T Load(";
                        sb << "int" << loadCoordCount << " location";
                        if(isMultisample)
                        {
                            sb << ", int sampleIndex";
                        }
                        sb << ");\n";

                        if (isMultisample)
                        {
                            sb << "__intrinsic(glsl, \"texelFetchOffset($P, $0, $1, $2)\")\n";
                        }
                        else
                        {
                            sb << "__intrinsic(glsl, \"texelFetch($P, ($0)." << kGLSLLoadCoordsSwizzle[loadCoordCount] << ", ($0)." << kGLSLLoadLODSwizzle[loadCoordCount] << ", $1)\")\n";
                        }
                        sb << "__intrinsic\n";
                        sb << "T Load(";
                        sb << "int" << loadCoordCount << " location";
                        if(isMultisample)
                        {
                            sb << ", int sampleIndex";
                        }
                        sb << ", int" << loadCoordCount << " offset";
                        sb << ");\n";


                        sb << "T Load(";
                        sb << "int" << loadCoordCount << " location";
                        if(isMultisample)
                        {
                            sb << ", int sampleIndex";
                        }
                        sb << ", int" << kBaseTextureTypes[tt].coordCount << " offset";
                        sb << ", out uint status";
                        sb << ");\n";
                    }

                    if(baseShape != TextureType::ShapeCube)
                    {
                        // subscript operator
                        sb << "__intrinsic __subscript(uint" << kBaseTextureTypes[tt].coordCount + isArray << " location) -> T;\n";
                    }

                    if( !isMultisample )
                    {
                        // `Sample()`

                        sb << "__intrinsic(glsl, \"texture($p, $1)\")\n";
                        sb << "__intrinsic\n";
                        sb << "T Sample(SamplerState s, ";
                        sb << "float" << kBaseTextureTypes[tt].coordCount + isArray << " location);\n";

                        if( baseShape != TextureType::ShapeCube )
                        {
                            sb << "__intrinsic(glsl, \"textureOffset($p, $1)\")\n";
                            sb << "__intrinsic\n";
                            sb << "T Sample(SamplerState s, ";
                            sb << "float" << kBaseTextureTypes[tt].coordCount + isArray << " location, ";
                            sb << "int" << kBaseTextureTypes[tt].coordCount << " offset);\n";
                        }

                        sb << "T Sample(SamplerState s, ";
                        sb << "float" << kBaseTextureTypes[tt].coordCount + isArray << " location, ";
                        if( baseShape != TextureType::ShapeCube )
                        {
                            sb << "int" << kBaseTextureTypes[tt].coordCount << " offset, ";
                        }
                        sb << "float clamp);\n";

                        sb << "T Sample(SamplerState s, ";
                        sb << "float" << kBaseTextureTypes[tt].coordCount + isArray << " location, ";
                        if( baseShape != TextureType::ShapeCube )
                        {
                            sb << "int" << kBaseTextureTypes[tt].coordCount << " offset, ";
                        }
                        sb << "float clamp, out uint status);\n";


                        // `SampleBias()`
                        sb << "__intrinsic(glsl, \"texture($p, $1, $2)\")\n";
                        sb << "__intrinsic\n";
                        sb << "T SampleBias(SamplerState s, ";
                        sb << "float" << kBaseTextureTypes[tt].coordCount + isArray << " location, float bias);\n";

                        if( baseShape != TextureType::ShapeCube )
                        {
                            sb << "__intrinsic(glsl, \"textureOffset($p, $1, $2, $3)\")\n";
                            sb << "__intrinsic\n";
                            sb << "T SampleBias(SamplerState s, ";
                            sb << "float" << kBaseTextureTypes[tt].coordCount + isArray << " location, float bias, ";
                            sb << "int" << kBaseTextureTypes[tt].coordCount << " offset);\n";
                        }

                        // `SampleCmp()` and `SampleCmpLevelZero`
                        sb << "T SampleCmp(SamplerComparisonState s, ";
                        sb << "float" << kBaseTextureTypes[tt].coordCount + isArray << " location, ";
                        sb << "float compareValue";
                        sb << ");\n";

                        sb << "T SampleCmpLevelZero(SamplerComparisonState s, ";
                        sb << "float" << kBaseTextureTypes[tt].coordCount + isArray << " location, ";
                        sb << "float compareValue";
                        sb << ");\n";

                        if( baseShape != TextureType::ShapeCube )
                        {
                            // Note(tfoley): MSDN seems confused, and claims that the `offset`
                            // parameter for `SampleCmp` is available for everything but 3D
                            // textures, while `Sample` and `SampleBias` are consistent in
                            // saying they only exclude `offset` for cube maps (which makes
                            // sense). I'm going to assume the documentation for `SampleCmp`
                            // is just wrong.

                            sb << "T SampleCmp(SamplerState s, ";
                            sb << "float" << kBaseTextureTypes[tt].coordCount + isArray << " location, ";
                            sb << "float compareValue, ";
                            sb << "int" << kBaseTextureTypes[tt].coordCount << " offset);\n";

                            sb << "T SampleCmpLevelZero(SamplerState s, ";
                            sb << "float" << kBaseTextureTypes[tt].coordCount + isArray << " location, ";
                            sb << "float compareValue, ";
                            sb << "int" << kBaseTextureTypes[tt].coordCount << " offset);\n";
                        }

                        sb << "T SampleGrad(SamplerState s, ";
                        sb << "float" << kBaseTextureTypes[tt].coordCount + isArray << " location, ";
                        sb << "float" << kBaseTextureTypes[tt].coordCount << " gradX, ";
                        sb << "float" << kBaseTextureTypes[tt].coordCount << " gradY";
                        sb << ");\n";

                        if( baseShape != TextureType::ShapeCube )
                        {
                            sb << "T SampleGrad(SamplerState s, ";
                            sb << "float" << kBaseTextureTypes[tt].coordCount + isArray << " location, ";
                            sb << "float" << kBaseTextureTypes[tt].coordCount << " gradX, ";
                            sb << "float" << kBaseTextureTypes[tt].coordCount << " gradY, ";
                            sb << "int" << kBaseTextureTypes[tt].coordCount << " offset);\n";
                        }

                        // `SampleLevel`

                        sb << "__intrinsic(glsl, \"textureLod($p, $1, $2)\")\n";
                        sb << "__intrinsic\n";
                        sb << "T SampleLevel(SamplerState s, ";
                        sb << "float" << kBaseTextureTypes[tt].coordCount + isArray << " location, ";
                        sb << "float level);\n";

                        if( baseShape != TextureType::ShapeCube )
                        {
                            sb << "__intrinsic(glsl, \"textureLodOffset($p, $1, $2, $3)\")\n";
                            sb << "__intrinsic\n";
                            sb << "T SampleLevel(SamplerState s, ";
                            sb << "float" << kBaseTextureTypes[tt].coordCount + isArray << " location, ";
                            sb << "float level, ";
                            sb << "int" << kBaseTextureTypes[tt].coordCount << " offset);\n";
                        }
                    }

                    sb << "\n};\n";

                    // `Gather*()` operations are handled via an `extension` declaration,
                    // because this lets us capture the element type of the texture.
                    //
                    // TODO: longer-term there should be something like a `TextureElementType`
                    // interface, that both scalars and vectors implement, that then exposes
                    // a `Scalar` associated type, and `Gather` can return `vector<T.Scalar, 4>`.
                    //
                    static const struct {
                        char const* genericPrefix;
                        char const* elementType;
                    } kGatherExtensionCases[] = {
                        { "__generic<T, let N : int>", "vector<T,N>" },

                        // TODO: need a case here for scalars `T`, but also
                        // need to ensure that case doesn't accidentally match
                        // for `T = vector<...>`, which requires actual checking
                        // of constraints on generic parameters.
                    };
                    for(auto cc : kGatherExtensionCases)
                    {
                        // TODO: this should really be an `if` around the entire `Gather` logic
                        if (isMultisample) break;

                        EMIT_LINE_DIRECTIVE();
                        sb << cc.genericPrefix << " __extension ";
                        sb << kBaseTextureAccessLevels[accessLevel].name;
                        sb << name;
                        if (isArray) sb << "Array";
                        sb << "<" << cc.elementType << " >";
                        sb << "\n{\n";


                        // `Gather`
                        // (tricky because it returns a 4-vector of the element type
                        // of the texture components...)
                        //
                        // TODO: is it actually correct to restrict these so that, e.g.,
                        // `GatherAlpha()` isn't allowed on `Texture2D<float3>` because
                        // it nominally doesn't have an alpha component?
                        static const struct {
                            int componentIndex;
                            char const* componentName;
                        } kGatherComponets[] = {
                            { 0, "" },
                            { 0, "Red" },
                            { 1, "Green" },
                            { 2, "Blue" },
                            { 3, "Alpha" },
                        };

                        for(auto kk : kGatherComponets)
                        {
                            auto componentName = kk.componentName;

                            EMIT_LINE_DIRECTIVE();
                            sb << "vector<T, 4> Gather" << componentName << "(SamplerState s, ";
                            sb << "float" << kBaseTextureTypes[tt].coordCount << " location);\n";

                            EMIT_LINE_DIRECTIVE();
                            sb << "vector<T, 4> Gather" << componentName << "(SamplerState s, ";
                            sb << "float" << kBaseTextureTypes[tt].coordCount << " location, ";
                            sb << "int" << kBaseTextureTypes[tt].coordCount << " offset);\n";

                            EMIT_LINE_DIRECTIVE();
                            sb << "vector<T, 4> Gather" << componentName << "(SamplerState s, ";
                            sb << "float" << kBaseTextureTypes[tt].coordCount << " location, ";
                            sb << "int" << kBaseTextureTypes[tt].coordCount << " offset, ";
                            sb << "out uint status);\n";

                            EMIT_LINE_DIRECTIVE();
                            sb << "vector<T, 4> Gather" << componentName << "(SamplerState s, ";
                            sb << "float" << kBaseTextureTypes[tt].coordCount << " location, ";
                            sb << "int" << kBaseTextureTypes[tt].coordCount << " offset1, ";
                            sb << "int" << kBaseTextureTypes[tt].coordCount << " offset2, ";
                            sb << "int" << kBaseTextureTypes[tt].coordCount << " offset3, ";
                            sb << "int" << kBaseTextureTypes[tt].coordCount << " offset4);\n";

                            EMIT_LINE_DIRECTIVE();
                            sb << "vector<T, 4> Gather" << componentName << "(SamplerState s, ";
                            sb << "float" << kBaseTextureTypes[tt].coordCount << " location, ";
                            sb << "int" << kBaseTextureTypes[tt].coordCount << " offset1, ";
                            sb << "int" << kBaseTextureTypes[tt].coordCount << " offset2, ";
                            sb << "int" << kBaseTextureTypes[tt].coordCount << " offset3, ";
                            sb << "int" << kBaseTextureTypes[tt].coordCount << " offset4, ";
                            sb << "out uint status);\n";
                        }

                        EMIT_LINE_DIRECTIVE();
                        sb << "\n}\n";
                    }
                }
            }
        }


        for (auto op : unaryOps)
        {
            for (auto type : kBaseTypes)
            {
                if ((type.flags & op.flags) == 0)
                    continue;

                char const* fixity = (op.flags & POSTFIX) != 0 ? "__postfix " : "__prefix ";
                char const* qual = (op.flags & ASSIGNMENT) != 0 ? "in out " : "";

                // scalar version
                sb << fixity;
                sb << "__intrinsic_op(" << int(op.opCode) << ") " << type.name << " operator" << op.opName << "(" << qual << type.name << " value);\n";

                // vector version
                sb << "__generic<let N : int> ";
                sb << fixity;
                sb << "__intrinsic_op(" << int(op.opCode) << ") vector<" << type.name << ",N> operator" << op.opName << "(" << qual << "vector<" << type.name << ",N> value);\n";

                // matrix version
                sb << "__generic<let N : int, let M : int> ";
                sb << fixity;
                sb << "__intrinsic_op(" << int(op.opCode) << ") matrix<" << type.name << ",N,M> operator" << op.opName << "(" << qual << "matrix<" << type.name << ",N,M> value);\n";
            }
        }

        for (auto op : binaryOps)
        {
            for (auto type : kBaseTypes)
            {
                if ((type.flags & op.flags) == 0)
                    continue;

                char const* leftType = type.name;
                char const* rightType = leftType;
                char const* resultType = leftType;

                if (op.flags & COMPARISON) resultType = "bool";

                char const* leftQual = "";
                if(op.flags & ASSIGNMENT) leftQual = "in out ";

                // TODO: handle `SHIFT`

                // scalar version
                sb << "__intrinsic_op(" << int(op.opCode) << ") " << resultType << " operator" << op.opName << "(" << leftQual << leftType << " left, " << rightType << " right);\n";

                // vector version
                sb << "__generic<let N : int> ";
                sb << "__intrinsic_op(" << int(op.opCode) << ") vector<" << resultType << ",N> operator" << op.opName << "(" << leftQual << "vector<" << leftType << ",N> left, vector<" << rightType << ",N> right);\n";

                // matrix version

                // skip matrix-matrix multiply operations here, so that GLSL doesn't see them
                switch (op.opCode)
                {
                case IntrinsicOp::Mul:
                case IntrinsicOp::MulAssign:
                    break;

                default:
                    sb << "__generic<let N : int, let M : int> ";
                    sb << "__intrinsic_op(" << int(op.opCode) << ") matrix<" << resultType << ",N,M> operator" << op.opName << "(" << leftQual << "matrix<" << leftType << ",N,M> left, matrix<" << rightType << ",N,M> right);\n";
                    break;
                }
            }
        }

        // Output a suitable `#line` directive to point at our raw stdlib code above
        sb << "\n#line " << kCoreLibIncludeStringLine << " \"" << path << "\"\n";

        int chunkCount = sizeof(kCoreLibIncludeStringChunks) / sizeof(kCoreLibIncludeStringChunks[0]);
        for (int cc = 0; cc < chunkCount; ++cc)
        {
            sb << kCoreLibIncludeStringChunks[cc];
        }

        coreLibraryCode = sb.ProduceString();
        return coreLibraryCode;
    }

    String getHLSLLibraryCode()
    {
        if (hlslLibraryCode.Length() > 0)
            return hlslLibraryCode;

        StringBuilder sb;


//        sb << "__generic<T> __magic_type(PackedBuffer) struct PackedBuffer {};\n";
//        sb << "__generic<T> __magic_type(Uniform) struct Uniform {};\n";
//        sb << "__generic<T> __magic_type(Patch) struct Patch {};\n";

        // Component-wise multiplication ops
        for(auto op : binaryOps)
        {
            switch (op.opCode)
            {
            default:
                continue;

            case IntrinsicOp::Mul:
            case IntrinsicOp::MulAssign:
                break;
            }

            for (auto type : kBaseTypes)
            {
                if ((type.flags & op.flags) == 0)
                    continue;

                char const* leftType = type.name;
                char const* rightType = leftType;
                char const* resultType = leftType;

                char const* leftQual = "";
                if(op.flags & ASSIGNMENT) leftQual = "in out ";

                sb << "__generic<let N : int, let M : int> ";
                sb << "__intrinsic_op(" << int(op.opCode) << ") matrix<" << resultType << ",N,M> operator" << op.opName << "(" << leftQual << "matrix<" << leftType << ",N,M> left, matrix<" << rightType << ",N,M> right);\n";
            }
        }

        // Output a suitable `#line` directive to point at our raw stdlib code above
        sb << "\n#line " << kHLSLLibIncludeStringLine << " \"" << getStdlibPath() << "\"\n";

        int chunkCount = sizeof(kHLSLLibIncludeStringChunks) / sizeof(kHLSLLibIncludeStringChunks[0]);
        for (int cc = 0; cc < chunkCount; ++cc)
        {
            sb << kHLSLLibIncludeStringChunks[cc];
        }

        hlslLibraryCode = sb.ProduceString();
        return hlslLibraryCode;
    }


    // GLSL-specific library code

    String getGLSLLibraryCode()
    {
        if(glslLibraryCode.Length() != 0)
            return glslLibraryCode;

        String path = getStdlibPath();

        StringBuilder sb;

        static const struct {
            char const* name;
            char const* glslPrefix;
        } kTypes[] =
        {
            {"float", ""},
            {"int", "i"},
            {"uint", "u"},
            {"bool", "b"},
        };
        static const int kTypeCount = sizeof(kTypes) / sizeof(kTypes[0]);

        for( int tt = 0; tt < kTypeCount; ++tt )
        {
            // Declare GLSL aliases for HLSL types
            for (int vv = 2; vv <= 4; ++vv)
            {
                sb << "typedef vector<" << kTypes[tt].name << "," << vv << "> " << kTypes[tt].glslPrefix << "vec" << vv << ";\n";
                sb << "typedef matrix<" << kTypes[tt].name << "," << vv << "," << vv << "> " << kTypes[tt].glslPrefix << "mat" << vv << ";\n";
            }
            for (int rr = 2; rr <= 4; ++rr)
            for (int cc = 2; cc <= 4; ++cc)
            {
                sb << "typedef matrix<" << kTypes[tt].name << "," << rr << "," << cc << "> " << kTypes[tt].glslPrefix << "mat" << rr << "x" << cc << ";\n";
            }
        }

        // Multiplication operations for vectors + matrices

        // scalar-vector and vector-scalar
        sb << "__generic<T : __BuiltinArithmeticType, let N : int> __intrinsic_op(Mul) vector<T,N> operator*(vector<T,N> x, T y);\n";
        sb << "__generic<T : __BuiltinArithmeticType, let N : int> __intrinsic_op(Mul) vector<T,N> operator*(T x, vector<T,N> y);\n";

        // scalar-matrix and matrix-scalar
        sb << "__generic<T : __BuiltinArithmeticType, let N : int, let M :int> __intrinsic_op(Mul) matrix<T,N,M> operator*(matrix<T,N,M> x, T y);\n";
        sb << "__generic<T : __BuiltinArithmeticType, let N : int, let M :int> __intrinsic_op(Mul) matrix<T,N,M> operator*(T x, matrix<T,N,M> y);\n";

        // vector-vector (dot product)
        sb << "__generic<T : __BuiltinArithmeticType, let N : int> __intrinsic_op(Mul) T operator*(vector<T,N> x, vector<T,N> y);\n";

        // vector-matrix
        sb << "__generic<T : __BuiltinArithmeticType, let N : int, let M : int> __intrinsic_op(Mul) vector<T,M> operator*(vector<T,N> x, matrix<T,N,M> y);\n";

        // matrix-vector
        sb << "__generic<T : __BuiltinArithmeticType, let N : int, let M : int> __intrinsic_op(Mul) vector<T,N> operator*(matrix<T,N,M> x, vector<T,M> y);\n";

        // matrix-matrix
        sb << "__generic<T : __BuiltinArithmeticType, let R : int, let N : int, let C : int> __intrinsic_op(Mul) matrix<T,R,C> operator*(matrix<T,R,N> x, matrix<T,N,C> y);\n";



        //

        // TODO(tfoley): Need to handle `RW*` variants of texture types as well...
        static const struct {
            char const*			name;
            TextureType::Shape	baseShape;
            int					coordCount;
        } kBaseTextureTypes[] = {
            { "1D",		TextureType::Shape1D,	1 },
            { "2D",		TextureType::Shape2D,	2 },
            { "3D",		TextureType::Shape3D,	3 },
            { "Cube",	TextureType::ShapeCube,	3 },
            { "Buffer", TextureType::ShapeBuffer,   1 },
        };
        static const int kBaseTextureTypeCount = sizeof(kBaseTextureTypes) / sizeof(kBaseTextureTypes[0]);


        static const struct {
            char const*         name;
            SlangResourceAccess access;
        } kBaseTextureAccessLevels[] = {
            { "",                   SLANG_RESOURCE_ACCESS_READ },
            { "RW",                 SLANG_RESOURCE_ACCESS_READ_WRITE },
            { "RasterizerOrdered",  SLANG_RESOURCE_ACCESS_RASTER_ORDERED },
        };
        static const int kBaseTextureAccessLevelCount = sizeof(kBaseTextureAccessLevels) / sizeof(kBaseTextureAccessLevels[0]);

        for (int tt = 0; tt < kBaseTextureTypeCount; ++tt)
        {
            char const* shapeName = kBaseTextureTypes[tt].name;
            TextureType::Shape baseShape = kBaseTextureTypes[tt].baseShape;

            for (int isArray = 0; isArray < 2; ++isArray)
            {
                // Arrays of 3D textures aren't allowed
                if (isArray && baseShape == TextureType::Shape3D) continue;

                for (int isMultisample = 0; isMultisample < 2; ++isMultisample)
                {
                    auto readAccess = SLANG_RESOURCE_ACCESS_READ;
                    auto readWriteAccess = SLANG_RESOURCE_ACCESS_READ_WRITE;

                    // TODO: any constraints to enforce on what gets to be multisampled?

                        
                    unsigned flavor = baseShape;
                    if (isArray)		flavor |= TextureType::ArrayFlag;
                    if (isMultisample)	flavor |= TextureType::MultisampleFlag;
//                        if (isShadow)		flavor |= TextureType::ShadowFlag;



                    unsigned readFlavor = flavor | (readAccess << 8);
                    unsigned readWriteFlavor = flavor | (readWriteAccess << 8);

                    StringBuilder nameBuilder;
                    nameBuilder << shapeName;
                    if (isMultisample) nameBuilder << "MS";
                    if (isArray) nameBuilder << "Array";
                    auto name = nameBuilder.ProduceString();

                    sb << "__generic<T> ";
                    sb << "__magic_type(TextureSampler," << int(readFlavor) << ") struct ";
                    sb << "__sampler" << name;
                    sb << " {};\n";

                    sb << "__generic<T> ";
                    sb << "__magic_type(Texture," << int(readFlavor) << ") struct ";
                    sb << "__texture" << name;
                    sb << " {};\n";

                    sb << "__generic<T> ";
                    sb << "__magic_type(GLSLImageType," << int(readWriteFlavor) << ") struct ";
                    sb << "__image" << name;
                    sb << " {};\n";

                    // TODO(tfoley): flesh this out for all the available prefixes
                    static const struct
                    {
                        char const* prefix;
                        char const* elementType;
                    } kTextureElementTypes[] = {
                        { "", "vec4" },
                        { "i", "ivec4" },
                        { "u", "uvec4" },
                        { nullptr, nullptr },
                    };
                    for( auto ee = kTextureElementTypes; ee->prefix; ++ee )
                    {
                        sb << "typedef __sampler" << name << "<" << ee->elementType << "> " << ee->prefix << "sampler" << name << ";\n";
                        sb << "typedef __texture" << name << "<" << ee->elementType << "> " << ee->prefix << "texture" << name << ";\n";
                        sb << "typedef __image" << name << "<" << ee->elementType << "> " << ee->prefix << "image" << name << ";\n";
                    }
                }
            }
        }

        sb << "__generic<T> __magic_type(GLSLInputParameterBlockType) struct __GLSLInputParameterBlock {};\n";
        sb << "__generic<T> __magic_type(GLSLOutputParameterBlockType) struct __GLSLOutputParameterBlock {};\n";
        sb << "__generic<T> __magic_type(GLSLShaderStorageBufferType) struct __GLSLShaderStorageBuffer {};\n";

        sb << "__magic_type(SamplerState," << int(SamplerStateType::Flavor::SamplerState) << ") struct sampler {};";

        sb << "__magic_type(GLSLInputAttachmentType) struct subpassInput {};";

        // Define additional keywords
        sb << "__modifier(GLSLBufferModifier)       buffer;\n";
        sb << "__modifier(GLSLWriteOnlyModifier)    writeonly;\n";
        sb << "__modifier(GLSLReadOnlyModifier)     readonly;\n";
        sb << "__modifier(GLSLPatchModifier)        patch;\n";

        sb << "__modifier(SimpleModifier)           flat;\n";
        sb << "__modifier(SimpleModifier)           highp;\n";

        glslLibraryCode = sb.ProduceString();
        return glslLibraryCode;
    }



    //

    void finalizeShaderLibrary()
    {
        stdlibPath = String();

        coreLibraryCode = String();
        hlslLibraryCode = String();
        glslLibraryCode = String();
    }

}
