// Slang HLSL compatibility library

typedef uint UINT;

__generic<T>
__magic_type(HLSLAppendStructuredBufferType)
__intrinsic_type($(kIROp_HLSLAppendStructuredBufferType))
struct AppendStructuredBuffer
{
    void Append(T value);

    void GetDimensions(
        out uint numStructs,
        out uint stride);
};

__magic_type(HLSLByteAddressBufferType)
__intrinsic_type($(kIROp_HLSLByteAddressBufferType))
struct ByteAddressBuffer
{
    __target_intrinsic(glsl, "$1 = $0._data.length() * 4")
    void GetDimensions(
        out uint dim);

    __target_intrinsic(glsl, "$0._data[$1/4]")
    uint Load(int location);

    uint Load(int location, out uint status);

    __target_intrinsic(glsl, "uvec2($0._data[$1/4], $0._data[$1/4+1])")
    uint2 Load2(int location);

    uint2 Load2(int location, out uint status);

    __target_intrinsic(glsl, "uvec3($0._data[$1/4], $0._data[$1/4+1], $0._data[$1/4+2])")
    uint3 Load3(int location);

    uint3 Load3(int location, out uint status);

    __target_intrinsic(glsl, "uvec4($0._data[$1/4], $0._data[$1/4+1], $0._data[$1/4+2], $0._data[$1/4+3])")
    uint4 Load4(int location);

    uint4 Load4(int location, out uint status);
};

__generic<T>
__magic_type(HLSLStructuredBufferType)
__intrinsic_type($(kIROp_HLSLStructuredBufferType))
struct StructuredBuffer
{
    void GetDimensions(
        out uint numStructs,
        out uint stride);

    T Load(int location);
    T Load(int location, out uint status);

    __subscript(uint index) -> T
    {
        __target_intrinsic(glsl, "$0._data[$1]")
        get;
    };
};

__generic<T>
__magic_type(HLSLConsumeStructuredBufferType)
__intrinsic_type($(kIROp_HLSLConsumeStructuredBufferType))
struct ConsumeStructuredBuffer
{
    T Consume();

    void GetDimensions(
        out uint numStructs,
        out uint stride);
};

__generic<T, let N : int>
__magic_type(HLSLInputPatchType)
__intrinsic_type($(kIROp_HLSLInputPatchType))
struct InputPatch
{
    __subscript(uint index) -> T;
};

__generic<T, let N : int>
__magic_type(HLSLOutputPatchType)
__intrinsic_type($(kIROp_HLSLOutputPatchType))
struct OutputPatch
{
    __subscript(uint index) -> T;
};

${{{{
static const struct {
    IROp op;
    char const* name;
} kMutableByteAddressBufferCases[] =
{
    { kIROp_HLSLRWByteAddressBufferType,                "RWByteAddressBuffer" },
    { kIROp_HLSLRasterizerOrderedByteAddressBufferType, "RasterizerOrderedByteAddressBuffer" },
};
for(auto item : kMutableByteAddressBufferCases) {
}}}}

__magic_type(HLSL$(item.name)Type)
__intrinsic_type($(item.op))
struct $(item.name)
{
    // Note(tfoley): supports all operations from `ByteAddressBuffer`
    // TODO(tfoley): can this be made a sub-type?

    __target_intrinsic(glsl, "$1 = $0._data.length() * 4")
    void GetDimensions(
        out uint dim);

    __target_intrinsic(glsl, "$0._data[$1/4]")
    uint Load(int location);

    uint Load(int location, out uint status);

    __target_intrinsic(glsl, "uvec2($0._data[$1/4], $0._data[$1/4+1])")
    uint2 Load2(int location);

    uint2 Load2(int location, out uint status);

    __target_intrinsic(glsl, "uvec3($0._data[$1/4], $0._data[$1/4+1], $0._data[$1/4+2])")
    uint3 Load3(int location);

    uint3 Load3(int location, out uint status);

    __target_intrinsic(glsl, "uvec4($0._data[$1/4], $0._data[$1/4+1], $0._data[$1/4+2], $0._data[$1/4+3])")
    uint4 Load4(int location);

    uint4 Load4(int location, out uint status);

    // Added operations:

    __target_intrinsic(glsl, "($3 = atomicAdd($0._data[$1/4], $2))")
    void InterlockedAdd(
        UINT dest,
        UINT value,
        out UINT original_value);

    __target_intrinsic(glsl, "atomicAdd($0._data[$1/4], $2)")
    void InterlockedAdd(
        UINT dest,
        UINT value);

    __target_intrinsic(glsl, "($3 = atomicAnd($0._data[$1/4], $2))")
    void InterlockedAnd(
        UINT dest,
        UINT value,
        out UINT original_value);

    __target_intrinsic(glsl, "atomicAnd($0._data[$1/4], $2)")
    void InterlockedAnd(
        UINT dest,
        UINT value);

    __target_intrinsic(glsl, "($4 = atomicCompSwap($0._data[$1/4], $2, $3))")
    void InterlockedCompareExchange(
        UINT dest,
        UINT compare_value,
        UINT value,
        out UINT original_value);

    __target_intrinsic(glsl, "atomicCompSwap($0._data[$1/4], $2, $3)")
    void InterlockedCompareStore(
        UINT dest,
        UINT compare_value,
        UINT value);

    __target_intrinsic(glsl, "($3 = atomicExchange($0._data[$1/4], $2))")
    void InterlockedExchange(
        UINT dest,
        UINT value,
        out UINT original_value);

    __target_intrinsic(glsl, "($3 = atomicMax($0._data[$1/4], $2))")
    void InterlockedMax(
        UINT dest,
        UINT value,
        out UINT original_value);

    __target_intrinsic(glsl, "atomicMax($0._data[$1/4], $2)")
    void InterlockedMax(
        UINT dest,
        UINT value);

    __target_intrinsic(glsl, "($3 = atomicMin($0._data[$1/4], $2))")
    void InterlockedMin(
        UINT dest,
        UINT value,
        out UINT original_value);

    __target_intrinsic(glsl, "atomicMin($0._data[$1/4], $2)")
    void InterlockedMin(
        UINT dest,
        UINT value);

    __target_intrinsic(glsl, "($3 = atomicOr($0._data[$1/4], $2))")
    void InterlockedOr(
        UINT dest,
        UINT value,
        out UINT original_value);

    __target_intrinsic(glsl, "atomicOr($0._data[$1/4], $2)")
    void InterlockedOr(
        UINT dest,
        UINT value);

    __target_intrinsic(glsl, "($3 = atomicXor($0._data[$1/4], $2))")
    void InterlockedXor(
        UINT dest,
        UINT value,
        out UINT original_value);

    __target_intrinsic(glsl, "atomicXor($0._data[$1/4], $2)")
    void InterlockedXor(
        UINT dest,
        UINT value);

    __target_intrinsic(glsl, "$0._data[$1/4] = $2")
    void Store(
        uint address,
        uint value);

    __target_intrinsic(glsl, "$0._data[$1/4] = $2.x, $0._data[$1/4+1] = $2.y")
    void Store2(
        uint address,
        uint2 value);

    __target_intrinsic(glsl, "$0._data[$1/4] = $2.x, $0._data[$1/4+1] = $2.y, $0._data[$1/4+2] = $2.z")
    void Store3(
        uint address,
        uint3 value);

    __target_intrinsic(glsl, "$0._data[$1/4] = $2.x, $0._data[$1/4+1] = $2.y, $0._data[$1/4+2] = $2.z, $0._data[$1/4+3] = $2.w")
    void Store4(
        uint address,
        uint4 value);
};

${{{{
}
}}}}

${{{{
static const struct {
    IROp op;
    char const* name;
} kMutableStructuredBufferCases[] =
{
    { kIROp_HLSLRWStructuredBufferType,                "RWStructuredBuffer" },
    { kIROp_HLSLRasterizerOrderedStructuredBufferType, "RasterizerOrderedStructuredBuffer" },
};
for(auto item : kMutableStructuredBufferCases) {
}}}}


__generic<T>
__magic_type(HLSL$(item.name)Type)
__intrinsic_type($(item.op))
struct $(item.name)
{
    uint DecrementCounter();

    void GetDimensions(
        out uint numStructs,
        out uint stride);

    uint IncrementCounter();

    T Load(int location);
    T Load(int location, out uint status);

    __subscript(uint index) -> T
    {
        __target_intrinsic(glsl, "$0._data[$1]")
        ref;
    }
};

${{{{
}
}}}}

__generic<T>
__magic_type(HLSLPointStreamType)
__intrinsic_type($(kIROp_HLSLPointStreamType))
struct PointStream
{
    __target_intrinsic(glsl, "EmitVertex()")
    void Append(T value);

    __target_intrinsic(glsl, "EndPrimitive()")
    void RestartStrip();
};

__generic<T>
__magic_type(HLSLLineStreamType)
__intrinsic_type($(kIROp_HLSLLineStreamType))
struct LineStream
{
    __target_intrinsic(glsl, "EmitVertex()")
    void Append(T value);

    __target_intrinsic(glsl, "EndPrimitive()")
    void RestartStrip();
};

__generic<T>
__magic_type(HLSLTriangleStreamType)
__intrinsic_type($(kIROp_HLSLTriangleStreamType))
struct TriangleStream
{
    __target_intrinsic(glsl, "EmitVertex()")
    void Append(T value);

    __target_intrinsic(glsl, "EndPrimitive()")
    void RestartStrip();
};

// Note(tfoley): Trying to systematically add all the HLSL builtins

// Try to terminate the current draw or dispatch call (HLSL SM 4.0)
void abort();

// Absolute value (HLSL SM 1.0)
__generic<T : __BuiltinSignedArithmeticType> T abs(T x);
__generic<T : __BuiltinSignedArithmeticType, let N : int> vector<T,N> abs(vector<T,N> x);
__generic<T : __BuiltinSignedArithmeticType, let N : int, let M : int> matrix<T,N,M> abs(matrix<T,N,M> x);

// Inverse cosine (HLSL SM 1.0)
__generic<T : __BuiltinFloatingPointType> T acos(T x);
__generic<T : __BuiltinFloatingPointType, let N : int> vector<T,N> acos(vector<T,N> x);
__generic<T : __BuiltinFloatingPointType, let N : int, let M : int> matrix<T,N,M> acos(matrix<T,N,M> x);

// Test if all components are non-zero (HLSL SM 1.0)
__generic<T : __BuiltinType> bool all(T x);
__generic<T : __BuiltinType, let N : int> bool all(vector<T,N> x);
__generic<T : __BuiltinType, let N : int, let M : int> bool all(matrix<T,N,M> x);

// Barrier for writes to all memory spaces (HLSL SM 5.0)
__target_intrinsic(glsl, "memoryBarrier(), groupMemoryBarrier(), memoryBarrierImage(), memoryBarrierBuffer()")
void AllMemoryBarrier();

// Thread-group sync and barrier for writes to all memory spaces (HLSL SM 5.0)
__target_intrinsic(glsl, "memoryBarrier(), groupMemoryBarrier(), memoryBarrierImage(), memoryBarrierBuffer(), barrier()")
void AllMemoryBarrierWithGroupSync();

// Test if any components is non-zero (HLSL SM 1.0)

__generic<T : __BuiltinType>
__target_intrinsic(glsl, "bool($0)")
bool any(T x);

__generic<T : __BuiltinType, let N : int>
__target_intrinsic(glsl, "any(bvec$N0($0))")
bool any(vector<T,N> x);

__generic<T : __BuiltinType, let N : int, let M : int>
// TODO: need to define GLSL mapping
bool any(matrix<T,N,M> x);


// Reinterpret bits as a double (HLSL SM 5.0)
double asdouble(uint lowbits, uint highbits);

// Reinterpret bits as a float (HLSL SM 4.0)
float asfloat( int x);
float asfloat(uint x);
__generic<let N : int> vector<float,N> asfloat(vector< int,N> x);
__generic<let N : int> vector<float,N> asfloat(vector<uint,N> x);
__generic<let N : int, let M : int> matrix<float,N,M> asfloat(matrix< int,N,M> x);
__generic<let N : int, let M : int> matrix<float,N,M> asfloat(matrix<uint,N,M> x);


// Inverse sine (HLSL SM 1.0)
__generic<T : __BuiltinFloatingPointType> T asin(T x);
__generic<T : __BuiltinFloatingPointType, let N : int> vector<T,N> asin(vector<T,N> x);
__generic<T : __BuiltinFloatingPointType, let N : int, let M : int> matrix<T,N,M> asin(matrix<T,N,M> x);

// Reinterpret bits as an int (HLSL SM 4.0)
int asint(float x);
int asint(uint x);
__generic<let N : int> vector<int,N> asint(vector<float,N> x);
__generic<let N : int> vector<int,N> asint(vector<uint,N> x);
__generic<let N : int, let M : int> matrix<int,N,M> asint(matrix<float,N,M> x);
__generic<let N : int, let M : int> matrix<int,N,M> asint(matrix<uint,N,M> x);

// Reinterpret bits of double as a uint (HLSL SM 5.0)
void asuint(double value, out uint lowbits, out uint highbits);

// Reinterpret bits as a uint (HLSL SM 4.0)
uint asuint(float x);
uint asuint(int x);
__generic<let N : int> vector<uint,N> asuint(vector<float,N> x);
__generic<let N : int> vector<uint,N> asuint(vector<int,N> x);
__generic<let N : int, let M : int> matrix<uint,N,M> asuint(matrix<float,N,M> x);
__generic<let N : int, let M : int> matrix<uint,N,M> asuint(matrix<int,N,M> x);

// Inverse tangent (HLSL SM 1.0)
__generic<T : __BuiltinFloatingPointType> T atan(T x);
__generic<T : __BuiltinFloatingPointType, let N : int> vector<T,N> atan(vector<T,N> x);
__generic<T : __BuiltinFloatingPointType, let N : int, let M : int> matrix<T,N,M> atan(matrix<T,N,M> x);

__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(glsl,"atan($0,$1)")
T atan2(T y, T x);

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(glsl,"atan($0,$1)")
vector<T,N> atan2(vector<T,N> y, vector<T,N> x);

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(glsl,"atan($0,$1)")
matrix<T,N,M> atan2(matrix<T,N,M> y, matrix<T,N,M> x);

// Ceiling (HLSL SM 1.0)
__generic<T : __BuiltinFloatingPointType> T ceil(T x);
__generic<T : __BuiltinFloatingPointType, let N : int> vector<T,N> ceil(vector<T,N> x);
__generic<T : __BuiltinFloatingPointType, let N : int, let M : int> matrix<T,N,M> ceil(matrix<T,N,M> x);


// Check access status to tiled resource
bool CheckAccessFullyMapped(uint status);

// Clamp (HLSL SM 1.0)
__generic<T : __BuiltinArithmeticType> T clamp(T x, T min, T max);
__generic<T : __BuiltinArithmeticType, let N : int> vector<T,N> clamp(vector<T,N> x, vector<T,N> min, vector<T,N> max);
__generic<T : __BuiltinArithmeticType, let N : int, let M : int> matrix<T,N,M> clamp(matrix<T,N,M> x, matrix<T,N,M> min, matrix<T,N,M> max);

// Clip (discard) fragment conditionally
__generic<T : __BuiltinFloatingPointType> void clip(T x);
__generic<T : __BuiltinFloatingPointType, let N : int> void clip(vector<T,N> x);
__generic<T : __BuiltinFloatingPointType, let N : int, let M : int> void clip(matrix<T,N,M> x);

// Cosine
__generic<T : __BuiltinFloatingPointType> T cos(T x);
__generic<T : __BuiltinFloatingPointType, let N : int> vector<T,N> cos(vector<T,N> x);
__generic<T : __BuiltinFloatingPointType, let N : int, let M : int> matrix<T,N,M> cos(matrix<T,N,M> x);

// Hyperbolic cosine
__generic<T : __BuiltinFloatingPointType> T cosh(T x);
__generic<T : __BuiltinFloatingPointType, let N : int> vector<T,N> cosh(vector<T,N> x);
__generic<T : __BuiltinFloatingPointType, let N : int, let M : int> matrix<T,N,M> cosh(matrix<T,N,M> x);

// Population count
__target_intrinsic(glsl, "bitCount")
uint countbits(uint value);

// Cross product
__generic<T : __BuiltinArithmeticType> vector<T,3> cross(vector<T,3> x, vector<T,3> y);

// Convert encoded color
int4 D3DCOLORtoUBYTE4(float4 x);

// Partial-difference derivatives
__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(glsl, dFdx)
T ddx(T x);

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(glsl, dFdx)
vector<T,N> ddx(vector<T,N> x);

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(glsl, dFdx)
matrix<T,N,M> ddx(matrix<T,N,M> x);

__generic<T : __BuiltinFloatingPointType>
__glsl_extension(GL_ARB_derivative_control)
__target_intrinsic(glsl, dFdxCoarse)
T ddx_coarse(T x);

__generic<T : __BuiltinFloatingPointType, let N : int>
__glsl_extension(GL_ARB_derivative_control)
__target_intrinsic(glsl, dFdxCoarse)
vector<T,N> ddx_coarse(vector<T,N> x);

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__glsl_extension(GL_ARB_derivative_control)
__target_intrinsic(glsl, dFdxCoarse)
matrix<T,N,M> ddx_coarse(matrix<T,N,M> x);

__generic<T : __BuiltinFloatingPointType>
__glsl_extension(GL_ARB_derivative_control)
__target_intrinsic(glsl, dFdxFine)
T ddx_fine(T x);

__generic<T : __BuiltinFloatingPointType, let N : int>
__glsl_extension(GL_ARB_derivative_control)
__target_intrinsic(glsl, dFdxFine)
vector<T,N> ddx_fine(vector<T,N> x);

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__glsl_extension(GL_ARB_derivative_control)
__target_intrinsic(glsl, dFdxFine)
matrix<T,N,M> ddx_fine(matrix<T,N,M> x);

__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(glsl, dFdy)
T ddy(T x);

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(glsl, dFdy)
vector<T,N> ddy(vector<T,N> x);

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(glsl, dFdy)
 matrix<T,N,M> ddy(matrix<T,N,M> x);

__generic<T : __BuiltinFloatingPointType>
__glsl_extension(GL_ARB_derivative_control)
__target_intrinsic(glsl, dFdyCoarse)
T ddy_coarse(T x);

__generic<T : __BuiltinFloatingPointType, let N : int>
__glsl_extension(GL_ARB_derivative_control)
__target_intrinsic(glsl, dFdyCoarse)
vector<T,N> ddy_coarse(vector<T,N> x);

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__glsl_extension(GL_ARB_derivative_control)
__target_intrinsic(glsl, dFdyCoarse)
matrix<T,N,M> ddy_coarse(matrix<T,N,M> x);

__generic<T : __BuiltinFloatingPointType>
__glsl_extension(GL_ARB_derivative_control)
__target_intrinsic(glsl, dFdyFine)
T ddy_fine(T x);

__generic<T : __BuiltinFloatingPointType, let N : int>
__glsl_extension(GL_ARB_derivative_control)
__target_intrinsic(glsl, dFdyFine)
vector<T,N> ddy_fine(vector<T,N> x);

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__glsl_extension(GL_ARB_derivative_control)
__target_intrinsic(glsl, dFdyFine)
matrix<T,N,M> ddy_fine(matrix<T,N,M> x);


// Radians to degrees
__generic<T : __BuiltinFloatingPointType> T degrees(T x);
__generic<T : __BuiltinFloatingPointType, let N : int> vector<T,N> degrees(vector<T,N> x);
__generic<T : __BuiltinFloatingPointType, let N : int, let M : int> matrix<T,N,M> degrees(matrix<T,N,M> x);

// Matrix determinant

__generic<T : __BuiltinFloatingPointType, let N : int> T determinant(matrix<T,N,N> m);

// Barrier for device memory
__target_intrinsic(glsl, "memoryBarrier(), memoryBarrierImage(), memoryBarrierBuffer()")
void DeviceMemoryBarrier();

__target_intrinsic(glsl, "memoryBarrier(), memoryBarrierImage(), memoryBarrierBuffer(), barrier()")
void DeviceMemoryBarrierWithGroupSync();

// Vector distance

__generic<T : __BuiltinFloatingPointType, let N : int> T distance(vector<T,N> x, vector<T,N> y);

// Vector dot product

__generic<T : __BuiltinArithmeticType, let N : int> T dot(vector<T,N> x, vector<T,N> y);

// Helper for computing distance terms for lighting (obsolete)

__generic<T : __BuiltinFloatingPointType> vector<T,4> dst(vector<T,4> x, vector<T,4> y);

// Error message

// void errorf( string format, ... );

// Attribute evaluation

// TODO: The matrix cases of these functions won't actuall work
// when compiled to GLSL, since they only support scalar/vector

// TODO: Should these be constrains to `__BuiltinFloatingPointType`?

__generic<T : __BuiltinArithmeticType>
__target_intrinsic(glsl, interpolateAtCentroid)
T EvaluateAttributeAtCentroid(T x);

__generic<T : __BuiltinArithmeticType, let N : int>
__target_intrinsic(glsl, interpolateAtCentroid)
vector<T,N> EvaluateAttributeAtCentroid(vector<T,N> x);

__generic<T : __BuiltinArithmeticType, let N : int, let M : int>
__target_intrinsic(glsl, interpolateAtCentroid)
matrix<T,N,M> EvaluateAttributeAtCentroid(matrix<T,N,M> x);

__generic<T : __BuiltinArithmeticType>
__target_intrinsic(glsl, "interpolateAtSample($0, int($1))")
T EvaluateAttributeAtSample(T x, uint sampleindex);

__generic<T : __BuiltinArithmeticType, let N : int>
__target_intrinsic(glsl, "interpolateAtSample($0, int($1))")
vector<T,N> EvaluateAttributeAtSample(vector<T,N> x, uint sampleindex);

__generic<T : __BuiltinArithmeticType, let N : int, let M : int>
__target_intrinsic(glsl, "interpolateAtSample($0, int($1))")
matrix<T,N,M> EvaluateAttributeAtSample(matrix<T,N,M> x, uint sampleindex);

__generic<T : __BuiltinArithmeticType>
__target_intrinsic(glsl, "interpolateAtOffset($0, vec2($1) / 16.0f)")
T EvaluateAttributeSnapped(T x, int2 offset);

__generic<T : __BuiltinArithmeticType, let N : int>
__target_intrinsic(glsl, "interpolateAtOffset($0, vec2($1) / 16.0f)")
vector<T,N> EvaluateAttributeSnapped(vector<T,N> x, int2 offset);

__generic<T : __BuiltinArithmeticType, let N : int, let M : int>
__target_intrinsic(glsl, "interpolateAtOffset($0, vec2($1) / 16.0f)")
matrix<T,N,M> EvaluateAttributeSnapped(matrix<T,N,M> x, int2 offset);

// Base-e exponent
__generic<T : __BuiltinFloatingPointType> T exp(T x);
__generic<T : __BuiltinFloatingPointType, let N : int> vector<T,N> exp(vector<T,N> x);
__generic<T : __BuiltinFloatingPointType, let N : int, let M : int> matrix<T,N,M> exp(matrix<T,N,M> x);

// Base-2 exponent
__generic<T : __BuiltinFloatingPointType> T exp2(T x);
__generic<T : __BuiltinFloatingPointType, let N : int> vector<T,N> exp2(vector<T,N> x);
__generic<T : __BuiltinFloatingPointType, let N : int, let M : int> matrix<T,N,M> exp2(matrix<T,N,M> x);

// Convert 16-bit float stored in low bits of integer
float f16tof32(uint value);
__generic<let N : int> vector<float,N> f16tof32(vector<uint,N> value);

// Convert to 16-bit float stored in low bits of integer
uint f32tof16(float value);
__generic<let N : int> vector<uint,N> f32tof16(vector<float,N> value);

// Flip surface normal to face forward, if needed
__generic<T : __BuiltinFloatingPointType, let N : int> vector<T,N> faceforward(vector<T,N> n, vector<T,N> i, vector<T,N> ng);

// Find first set bit starting at high bit and working down
__target_intrinsic(glsl,"findMSB")
int firstbithigh(int value);

__target_intrinsic(glsl,"findMSB")
__generic<let N : int> vector<int,N> firstbithigh(vector<int,N> value);

__target_intrinsic(glsl,"findMSB")
uint firstbithigh(uint value);

__target_intrinsic(glsl,"findMSB")
__generic<let N : int> vector<uint,N> firstbithigh(vector<uint,N> value);

// Find first set bit starting at low bit and working up
__target_intrinsic(glsl,"findLSB")
int firstbitlow(int value);

__target_intrinsic(glsl,"findLSB")
__generic<let N : int> vector<int,N> firstbitlow(vector<int,N> value);

__target_intrinsic(glsl,"findLSB")
uint firstbitlow(uint value);

__target_intrinsic(glsl,"findLSB")
__generic<let N : int> vector<uint,N> firstbitlow(vector<uint,N> value);

// Floor (HLSL SM 1.0)
__generic<T : __BuiltinFloatingPointType> T floor(T x);
__generic<T : __BuiltinFloatingPointType, let N : int> vector<T,N> floor(vector<T,N> x);
__generic<T : __BuiltinFloatingPointType, let N : int, let M : int> matrix<T,N,M> floor(matrix<T,N,M> x);

// Fused multiply-add for doubles
double fma(double a, double b, double c);
__generic<let N : int> vector<double, N> fma(vector<double, N> a, vector<double, N> b, vector<double, N> c);
__generic<let N : int, let M : int> matrix<double,N,M> fma(matrix<double,N,M> a, matrix<double,N,M> b, matrix<double,N,M> c);

// Floating point remainder of x/y
__generic<T : __BuiltinFloatingPointType> T fmod(T x, T y);
__generic<T : __BuiltinFloatingPointType, let N : int> vector<T,N> fmod(vector<T,N> x, vector<T,N> y);
__generic<T : __BuiltinFloatingPointType, let N : int, let M : int> matrix<T,N,M> fmod(matrix<T,N,M> x, matrix<T,N,M> y);

// Fractional part
__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(glsl, fract)
T frac(T x);

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(glsl, fract)
vector<T,N> frac(vector<T,N> x);

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(glsl, fract)
matrix<T,N,M> frac(matrix<T,N,M> x);

// Split float into mantissa and exponent
__generic<T : __BuiltinFloatingPointType> T frexp(T x, out T exp);
__generic<T : __BuiltinFloatingPointType, let N : int> vector<T,N> frexp(vector<T,N> x, out vector<T,N> exp);
__generic<T : __BuiltinFloatingPointType, let N : int, let M : int> matrix<T,N,M> frexp(matrix<T,N,M> x, out matrix<T,N,M> exp);

// Texture filter width
__generic<T : __BuiltinFloatingPointType> T fwidth(T x);
__generic<T : __BuiltinFloatingPointType, let N : int> vector<T,N> fwidth(vector<T,N> x);
__generic<T : __BuiltinFloatingPointType, let N : int, let M : int> matrix<T,N,M> fwidth(matrix<T,N,M> x);

// Get number of samples in render target
uint GetRenderTargetSampleCount();

// Get position of given sample
float2 GetRenderTargetSamplePosition(int Index);

// Group memory barrier
__target_intrinsic(glsl, "groupMemoryBarrier")
void GroupMemoryBarrier();

__target_intrinsic(glsl, "groupMemoryBarrier(), barrier()")
void GroupMemoryBarrierWithGroupSync();

// Atomics

__target_intrinsic(glsl, "$atomicAdd($A, $1)")
void InterlockedAdd(__ref  int dest,  int value);

__target_intrinsic(glsl, "$atomicAdd($A, $1)")
void InterlockedAdd(__ref uint dest, uint value);

__target_intrinsic(glsl, "($2 = $atomicAdd($A, $1))")
void InterlockedAdd(__ref  int dest,  int value, out  int original_value);

__target_intrinsic(glsl, "($2 = $atomicAdd($A, $1))")
void InterlockedAdd(__ref uint dest, uint value, out uint original_value);

__target_intrinsic(glsl, "$atomicAnd($A, $1)")
void InterlockedAnd(__ref  int dest,  int value);

__target_intrinsic(glsl, "$atomicAnd($A, $1)")
void InterlockedAnd(__ref uint dest, uint value);

__target_intrinsic(glsl, "($2 = $atomicAnd($A, $1))")
void InterlockedAnd(__ref  int dest,  int value, out  int original_value);

__target_intrinsic(glsl, "($2 = $atomicAnd($A, $1))")
void InterlockedAnd(__ref uint dest, uint value, out uint original_value);

__target_intrinsic(glsl, "($3 = $atomicCompSwap($A, $1, $2))")
void InterlockedCompareExchange(__ref  int dest,  int compare_value,  int value, out  int original_value);

__target_intrinsic(glsl, "($3 = $atomicCompSwap($A, $1, $2))")
void InterlockedCompareExchange(__ref uint dest, uint compare_value, uint value, out uint original_value);

__target_intrinsic(glsl, "$atomicCompSwap($A, $1, $2)")
void InterlockedCompareStore(__ref  int dest,  int compare_value,  int value);

__target_intrinsic(glsl, "$atomicCompSwap($A, $1, $2)")
void InterlockedCompareStore(__ref uint dest, uint compare_value, uint value);

__target_intrinsic(glsl, "($2 = $atomicExchange($A, $1))")
void InterlockedExchange(__ref  int dest,  int value, out  int original_value);

__target_intrinsic(glsl, "($2 = $atomicExchange($A, $1))")
void InterlockedExchange(__ref uint dest, uint value, out uint original_value);

__target_intrinsic(glsl, "$atomicMax($A, $1)")
void InterlockedMax(__ref  int dest,  int value);

__target_intrinsic(glsl, "$atomicMax($A, $1)")
void InterlockedMax(__ref uint dest, uint value);

__target_intrinsic(glsl, "($2 = $atomicMax($A, $1))")
void InterlockedMax(__ref  int dest,  int value, out  int original_value);

__target_intrinsic(glsl, "($2 = $atomicMax($A, $1))")
void InterlockedMax(__ref uint dest, uint value, out uint original_value);

__target_intrinsic(glsl, "$atomicMin($A, $1)")
void InterlockedMin(__ref  int dest,  int value);

__target_intrinsic(glsl, "$atomicMin($A, $1)")
void InterlockedMin(__ref uint dest, uint value);

__target_intrinsic(glsl, "($2 = $atomicMin($A, $1))")
void InterlockedMin(__ref  int dest,  int value, out  int original_value);

__target_intrinsic(glsl, "($2 = $atomicMin($A, $1))")
void InterlockedMin(__ref uint dest, uint value, out uint original_value);

__target_intrinsic(glsl, "$atomicOr($A, $1)")
void InterlockedOr(__ref  int dest,  int value);

__target_intrinsic(glsl, "$atomicOr($A, $1)")
void InterlockedOr(__ref uint dest, uint value);

__target_intrinsic(glsl, "($2 = $atomicOr($A, $1))")
void InterlockedOr(__ref  int dest,  int value, out  int original_value);

__target_intrinsic(glsl, "($2 = $atomicOr($A, $1))")
void InterlockedOr(__ref uint dest, uint value, out uint original_value);

__target_intrinsic(glsl, "$atomicXor($A, $1)")
void InterlockedXor(__ref  int dest,  int value);

__target_intrinsic(glsl, "$atomicXor($A, $1)")
void InterlockedXor(__ref uint dest, uint value);

__target_intrinsic(glsl, "($2 = $atomicXor($A, $1))")
void InterlockedXor(__ref  int dest,  int value, out  int original_value);

__target_intrinsic(glsl, "($2 = $atomicXor($A, $1))")
void InterlockedXor(__ref uint dest, uint value, out uint original_value);

// Is floating-point value finite?
__generic<T : __BuiltinFloatingPointType> bool isfinite(T x);
__generic<T : __BuiltinFloatingPointType, let N : int> vector<bool,N> isfinite(vector<T,N> x);
__generic<T : __BuiltinFloatingPointType, let N : int, let M : int> matrix<bool,N,M> isfinite(matrix<T,N,M> x);

// Is floating-point value infinite?
__generic<T : __BuiltinFloatingPointType> bool isinf(T x);
__generic<T : __BuiltinFloatingPointType, let N : int> vector<bool,N> isinf(vector<T,N> x);
__generic<T : __BuiltinFloatingPointType, let N : int, let M : int> matrix<bool,N,M> isinf(matrix<T,N,M> x);

// Is floating-point value not-a-number?
__generic<T : __BuiltinFloatingPointType> bool isnan(T x);
__generic<T : __BuiltinFloatingPointType, let N : int> vector<bool,N> isnan(vector<T,N> x);
__generic<T : __BuiltinFloatingPointType, let N : int, let M : int> matrix<bool,N,M> isnan(matrix<T,N,M> x);

// Construct float from mantissa and exponent
__generic<T : __BuiltinFloatingPointType> T ldexp(T x, T exp);
__generic<T : __BuiltinFloatingPointType, let N : int> vector<T,N> ldexp(vector<T,N> x, vector<T,N> exp);
__generic<T : __BuiltinFloatingPointType, let N : int, let M : int> matrix<T,N,M> ldexp(matrix<T,N,M> x, matrix<T,N,M> exp);

// Vector length
__generic<T : __BuiltinFloatingPointType, let N : int> T length(vector<T,N> x);

// Linear interpolation
__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(glsl, mix)
T lerp(T x, T y, T s);

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(glsl, mix)
vector<T,N> lerp(vector<T,N> x, vector<T,N> y, vector<T,N> s);

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(glsl, mix)
matrix<T,N,M> lerp(matrix<T,N,M> x, matrix<T,N,M> y, matrix<T,N,M> s);

// Legacy lighting function (obsolete)
float4 lit(float n_dot_l, float n_dot_h, float m);

// Base-e logarithm
__generic<T : __BuiltinFloatingPointType> T log(T x);
__generic<T : __BuiltinFloatingPointType, let N : int> vector<T,N> log(vector<T,N> x);
__generic<T : __BuiltinFloatingPointType, let N : int, let M : int> matrix<T,N,M> log(matrix<T,N,M> x);

// Base-10 logarithm
__generic<T : __BuiltinFloatingPointType> T log10(T x);
__generic<T : __BuiltinFloatingPointType, let N : int> vector<T,N> log10(vector<T,N> x);
__generic<T : __BuiltinFloatingPointType, let N : int, let M : int> matrix<T,N,M> log10(matrix<T,N,M> x);

// Base-2 logarithm
__generic<T : __BuiltinFloatingPointType> T log2(T x);
__generic<T : __BuiltinFloatingPointType, let N : int> vector<T,N> log2(vector<T,N> x);
__generic<T : __BuiltinFloatingPointType, let N : int, let M : int> matrix<T,N,M> log2(matrix<T,N,M> x);

// multiply-add

__target_intrinsic(glsl, fma)
__generic<T : __BuiltinArithmeticType> T mad(T mvalue, T avalue, T bvalue);

__target_intrinsic(glsl, fma)
__generic<T : __BuiltinArithmeticType, let N : int> vector<T,N> mad(vector<T,N> mvalue, vector<T,N> avalue, vector<T,N> bvalue);

__target_intrinsic(glsl, fma)
__generic<T : __BuiltinArithmeticType, let N : int, let M : int> matrix<T,N,M> mad(matrix<T,N,M> mvalue, matrix<T,N,M> avalue, matrix<T,N,M> bvalue);

// maximum
__generic<T : __BuiltinArithmeticType> T max(T x, T y);
__generic<T : __BuiltinArithmeticType, let N : int> vector<T,N> max(vector<T,N> x, vector<T,N> y);
__generic<T : __BuiltinArithmeticType, let N : int, let M : int> matrix<T,N,M> max(matrix<T,N,M> x, matrix<T,N,M> y);

// minimum
__generic<T : __BuiltinArithmeticType> T min(T x, T y);
__generic<T : __BuiltinArithmeticType, let N : int> vector<T,N> min(vector<T,N> x, vector<T,N> y);
__generic<T : __BuiltinArithmeticType, let N : int, let M : int> matrix<T,N,M> min(matrix<T,N,M> x, matrix<T,N,M> y);

// split into integer and fractional parts (both with same sign)
__generic<T : __BuiltinFloatingPointType> T modf(T x, out T ip);
__generic<T : __BuiltinFloatingPointType, let N : int> vector<T,N> modf(vector<T,N> x, out vector<T,N> ip);
__generic<T : __BuiltinFloatingPointType, let N : int, let M : int> matrix<T,N,M> modf(matrix<T,N,M> x, out matrix<T,N,M> ip);

// msad4 (whatever that is)
uint4 msad4(uint reference, uint2 source, uint4 accum);

// General inner products

// scalar-scalar
__generic<T : __BuiltinArithmeticType> T mul(T x, T y);

// scalar-vector and vector-scalar
__generic<T : __BuiltinArithmeticType, let N : int> vector<T,N> mul(vector<T,N> x, T y);
__generic<T : __BuiltinArithmeticType, let N : int> vector<T,N> mul(T x, vector<T,N> y);

// scalar-matrix and matrix-scalar
__generic<T : __BuiltinArithmeticType, let N : int, let M :int> matrix<T,N,M> mul(matrix<T,N,M> x, T y);
__generic<T : __BuiltinArithmeticType, let N : int, let M :int> matrix<T,N,M> mul(T x, matrix<T,N,M> y);

// vector-vector (dot product)
__generic<T : __BuiltinArithmeticType, let N : int> __intrinsic_op(dot) T mul(vector<T,N> x, vector<T,N> y);

// vector-matrix
__generic<T : __BuiltinArithmeticType, let N : int, let M : int> __intrinsic_op(mulVectorMatrix) vector<T,M> mul(vector<T,N> x, matrix<T,N,M> y);

// matrix-vector
__generic<T : __BuiltinArithmeticType, let N : int, let M : int> __intrinsic_op(mulMatrixVector) vector<T,N> mul(matrix<T,N,M> x, vector<T,M> y);

// matrix-matrix
__generic<T : __BuiltinArithmeticType, let R : int, let N : int, let C : int> __intrinsic_op(mulMatrixMatrix) matrix<T,R,C> mul(matrix<T,R,N> x, matrix<T,N,C> y);

// noise (deprecated)
float noise(float x);
__generic<let N : int> float noise(vector<float, N> x);

/// Indicate that an index may be non-uniform at execution time.
///
/// Shader Model 5.1 and 6.x introduce support for dynamic indexing
/// of arrays of resources, but place the restriction that *by default*
/// the implementation can assume that any value used as an index into
/// such arrays will be dynamically uniform across an entire `Draw` or `Dispatch`
/// (when using instancing, the value must be uniform across all instances;
/// it does not seem that the restriction extends to draws within a multi-draw).
///
/// In order to indicate to the implementation that it cannot make the
/// uniformity assumption, a shader programmer is required to pass the index
/// to the `NonUniformResourceIndex` function before using it as an index.
/// The function superficially acts like an identity function.
///
/// Note: a future version of Slang may take responsibility for inserting calls
/// to this function as necessary in output code, rather than make this
/// the user's responsibility, so that the default behavior of the language
/// is more semantically "correct."
__target_intrinsic(glsl, nonuniformEXT)
__glsl_extension(GL_EXT_nonuniform_qualifier)
uint NonUniformResourceIndex(uint index);

__target_intrinsic(glsl, nonuniformEXT)
__glsl_extension(GL_EXT_nonuniform_qualifier)
int NonUniformResourceIndex(int index);

// Normalize a vector
__generic<T : __BuiltinFloatingPointType, let N : int> vector<T,N> normalize(vector<T,N> x);

// Raise to a power
__generic<T : __BuiltinFloatingPointType> T pow(T x, T y);
__generic<T : __BuiltinFloatingPointType, let N : int> vector<T,N> pow(vector<T,N> x, vector<T,N> y);
__generic<T : __BuiltinFloatingPointType, let N : int, let M : int> matrix<T,N,M> pow(matrix<T,N,M> x, matrix<T,N,M> y);

// Output message

// void printf( string format, ... );

// Tessellation factor fixup routines

void Process2DQuadTessFactorsAvg(
    in  float4 RawEdgeFactors,
    in  float2 InsideScale,
    out float4 RoundedEdgeTessFactors,
    out float2 RoundedInsideTessFactors,
    out float2 UnroundedInsideTessFactors);

void Process2DQuadTessFactorsMax(
    in  float4 RawEdgeFactors,
    in  float2 InsideScale,
    out float4 RoundedEdgeTessFactors,
    out float2 RoundedInsideTessFactors,
    out float2 UnroundedInsideTessFactors);

void Process2DQuadTessFactorsMin(
    in  float4 RawEdgeFactors,
    in  float2 InsideScale,
    out float4 RoundedEdgeTessFactors,
    out float2 RoundedInsideTessFactors,
    out float2 UnroundedInsideTessFactors);

void ProcessIsolineTessFactors(
    in  float RawDetailFactor,
    in  float RawDensityFactor,
    out float RoundedDetailFactor,
    out float RoundedDensityFactor);

void ProcessQuadTessFactorsAvg(
    in  float4 RawEdgeFactors,
    in  float InsideScale,
    out float4 RoundedEdgeTessFactors,
    out float2 RoundedInsideTessFactors,
    out float2 UnroundedInsideTessFactors);

void ProcessQuadTessFactorsMax(
    in  float4 RawEdgeFactors,
    in  float InsideScale,
    out float4 RoundedEdgeTessFactors,
    out float2 RoundedInsideTessFactors,
    out float2 UnroundedInsideTessFactors);

void ProcessQuadTessFactorsMin(
    in  float4 RawEdgeFactors,
    in  float InsideScale,
    out float4 RoundedEdgeTessFactors,
    out float2 RoundedInsideTessFactors,
    out float2 UnroundedInsideTessFactors);

void ProcessTriTessFactorsAvg(
    in  float3 RawEdgeFactors,
    in  float InsideScale,
    out float3 RoundedEdgeTessFactors,
    out float RoundedInsideTessFactor,
    out float UnroundedInsideTessFactor);

void ProcessTriTessFactorsMax(
    in  float3 RawEdgeFactors,
    in  float InsideScale,
    out float3 RoundedEdgeTessFactors,
    out float RoundedInsideTessFactor,
    out float UnroundedInsideTessFactor);

void ProcessTriTessFactorsMin(
    in  float3 RawEdgeFactors,
    in  float InsideScale,
    out float3 RoundedEdgeTessFactors,
    out float RoundedInsideTessFactors,
    out float UnroundedInsideTessFactors);

// Degrees to radians
__generic<T : __BuiltinFloatingPointType> T radians(T x);
__generic<T : __BuiltinFloatingPointType, let N : int> vector<T,N> radians(vector<T,N> x);
__generic<T : __BuiltinFloatingPointType, let N : int, let M : int> matrix<T,N,M> radians(matrix<T,N,M> x);

// Approximate reciprocal
__generic<T : __BuiltinFloatingPointType> T rcp(T x);
__generic<T : __BuiltinFloatingPointType, let N : int> vector<T,N> rcp(vector<T,N> x);
__generic<T : __BuiltinFloatingPointType, let N : int, let M : int> matrix<T,N,M> rcp(matrix<T,N,M> x);

// Reflect incident vector across plane with given normal
__generic<T : __BuiltinFloatingPointType, let N : int>
vector<T,N> reflect(vector<T,N> i, vector<T,N> n);

// Refract incident vector given surface normal and index of refraction
__generic<T : __BuiltinFloatingPointType, let N : int>
vector<T,N> refract(vector<T,N> i, vector<T,N> n, float eta);

// Reverse order of bits
__target_intrinsic(glsl, "bitfieldReverse")
uint reversebits(uint value);

__target_intrinsic(glsl, "bitfieldReverse")
__generic<let N : int> vector<uint,N> reversebits(vector<uint,N> value);

// Round-to-nearest
__generic<T : __BuiltinFloatingPointType> T round(T x);
__generic<T : __BuiltinFloatingPointType, let N : int> vector<T,N> round(vector<T,N> x);
__generic<T : __BuiltinFloatingPointType, let N : int, let M : int> matrix<T,N,M> round(matrix<T,N,M> x);

// Reciprocal of square root
__generic<T : __BuiltinFloatingPointType> T rsqrt(T x);
__generic<T : __BuiltinFloatingPointType, let N : int> vector<T,N> rsqrt(vector<T,N> x);
__generic<T : __BuiltinFloatingPointType, let N : int, let M : int> matrix<T,N,M> rsqrt(matrix<T,N,M> x);

// Clamp value to [0,1] range
__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(glsl, "clamp($0, 0, 1)")
T saturate(T x);

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(glsl, "clamp($0, 0, 1)")
vector<T,N> saturate(vector<T,N> x);

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(glsl, "clamp($0, 0, 1)")
matrix<T,N,M> saturate(matrix<T,N,M> x);

__generic<T : __BuiltinFloatingPointType>
__specialized_for_target(glsl)
T saturate(T x)
{
    return clamp<T>(x, T(0), T(1));
}

__generic<T : __BuiltinFloatingPointType, let N : int>
__specialized_for_target(glsl)
vector<T,N> saturate(vector<T,N> x)
{
    return clamp<T,N>(x,
        vector<T,N>(T(0)),
        vector<T,N>(T(1)));
}

// HACK: need a helper to turn a scalar into a matrix,
// because GLSL and HLSL disagree on the semantics of
// constructing a matrix from a single scalar.
__generic<T, let N : int, let M : int>
matrix<T,N,M> __scalarToMatrix(T value);

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__specialized_for_target(glsl)
matrix<T,N,M> saturate(matrix<T,N,M> x)
{
    return clamp<T,N,M>(x,
        __scalarToMatrix<T,N,M>(T(0)),
        __scalarToMatrix<T,N,M>(T(1)));
}


// Extract sign of value
__generic<T : __BuiltinSignedArithmeticType> int sign(T x);
__generic<T : __BuiltinSignedArithmeticType, let N : int> vector<int,N> sign(vector<T,N> x);
__generic<T : __BuiltinSignedArithmeticType, let N : int, let M : int> matrix<int,N,M> sign(matrix<T,N,M> x);


// Sine
__generic<T : __BuiltinFloatingPointType> T sin(T x);
__generic<T : __BuiltinFloatingPointType, let N : int> vector<T,N> sin(vector<T,N> x);
__generic<T : __BuiltinFloatingPointType, let N : int, let M : int> matrix<T,N,M> sin(matrix<T,N,M> x);

// Sine and cosine
__generic<T : __BuiltinFloatingPointType, let N : int> void sincos(T x, out T s, out T c);
__generic<T : __BuiltinFloatingPointType, let N : int> void sincos(vector<T,N> x, out vector<T,N> s, out vector<T,N> c);
__generic<T : __BuiltinFloatingPointType, let N : int, let M : int> void sincos(matrix<T,N,M> x, out matrix<T,N,M> s, out matrix<T,N,M> c);

// Hyperbolic Sine
__generic<T : __BuiltinFloatingPointType> T sinh(T x);
__generic<T : __BuiltinFloatingPointType, let N : int> vector<T,N> sinh(vector<T,N> x);
__generic<T : __BuiltinFloatingPointType, let N : int, let M : int> matrix<T,N,M> sinh(matrix<T,N,M> x);

// Smooth step (Hermite interpolation)
__generic<T : __BuiltinFloatingPointType> T smoothstep(T min, T max, T x);
__generic<T : __BuiltinFloatingPointType, let N : int> vector<T,N> smoothstep(vector<T,N> min, vector<T,N> max, vector<T,N> x);
__generic<T : __BuiltinFloatingPointType, let N : int, let M : int> matrix<T,N,M> smoothstep(matrix<T,N,M> min, matrix<T,N,M> max, matrix<T,N,M> x);

// Square root
__generic<T : __BuiltinFloatingPointType> T sqrt(T x);
__generic<T : __BuiltinFloatingPointType, let N : int> vector<T,N> sqrt(vector<T,N> x);
__generic<T : __BuiltinFloatingPointType, let N : int, let M : int> matrix<T,N,M> sqrt(matrix<T,N,M> x);

// Step function
__generic<T : __BuiltinFloatingPointType> T step(T y, T x);
__generic<T : __BuiltinFloatingPointType, let N : int> vector<T,N> step(vector<T,N> y, vector<T,N> x);
__generic<T : __BuiltinFloatingPointType, let N : int, let M : int> matrix<T,N,M> step(matrix<T,N,M> y, matrix<T,N,M> x);

// Tangent
__generic<T : __BuiltinFloatingPointType> T tan(T x);
__generic<T : __BuiltinFloatingPointType, let N : int> vector<T,N> tan(vector<T,N> x);
__generic<T : __BuiltinFloatingPointType, let N : int, let M : int> matrix<T,N,M> tan(matrix<T,N,M> x);

// Hyperbolic tangent
__generic<T : __BuiltinFloatingPointType> T tanh(T x);
__generic<T : __BuiltinFloatingPointType, let N : int> vector<T,N> tanh(vector<T,N> x);
__generic<T : __BuiltinFloatingPointType, let N : int, let M : int> matrix<T,N,M> tanh(matrix<T,N,M> x);

// Legacy texture-fetch operations

/*
float4 tex1D(sampler1D s, float t);
float4 tex1D(sampler1D s, float t, float ddx, float ddy);
float4 tex1Dbias(sampler1D s, float4 t);
float4 tex1Dgrad(sampler1D s, float t, float ddx, float ddy);
float4 tex1Dlod(sampler1D s, float4 t);
float4 tex1Dproj(sampler1D s, float4 t);

float4 tex2D(sampler2D s, float2 t);
float4 tex2D(sampler2D s, float2 t, float2 ddx, float2 ddy);
float4 tex2Dbias(sampler2D s, float4 t);
float4 tex2Dgrad(sampler2D s, float2 t, float2 ddx, float2 ddy);
float4 tex2Dlod(sampler2D s, float4 t);
float4 tex2Dproj(sampler2D s, float4 t);

float4 tex3D(sampler3D s, float3 t);
float4 tex3D(sampler3D s, float3 t, float3 ddx, float3 ddy);
float4 tex3Dbias(sampler3D s, float4 t);
float4 tex3Dgrad(sampler3D s, float3 t, float3 ddx, float3 ddy);
float4 tex3Dlod(sampler3D s, float4 t);
float4 tex3Dproj(sampler3D s, float4 t);

float4 texCUBE(samplerCUBE s, float3 t);
float4 texCUBE(samplerCUBE s, float3 t, float3 ddx, float3 ddy);
float4 texCUBEbias(samplerCUBE s, float4 t);
float4 texCUBEgrad(samplerCUBE s, float3 t, float3 ddx, float3 ddy);
float4 texCUBElod(samplerCUBE s, float4 t);
float4 texCUBEproj(samplerCUBE s, float4 t);
*/

// Matrix transpose
__generic<T : __BuiltinType, let N : int, let M : int> matrix<T,M,N> transpose(matrix<T,N,M> x);

// Truncate to integer
__generic<T : __BuiltinFloatingPointType> T trunc(T x);
__generic<T : __BuiltinFloatingPointType, let N : int> vector<T,N> trunc(vector<T,N> x);
__generic<T : __BuiltinFloatingPointType, let N : int, let M : int> matrix<T,N,M> trunc(matrix<T,N,M> x);

// Shader model 6.0 stuff

uint GlobalOrderedCountIncrement(uint countToAppendForThisLane);

__generic<T : __BuiltinType> T QuadReadLaneAt(T sourceValue, int quadLaneID);
__generic<T : __BuiltinType, let N : int> vector<T,N> QuadReadLaneAt(vector<T,N> sourceValue, int quadLaneID);
__generic<T : __BuiltinType, let N : int, let M : int> matrix<T,N,M> QuadReadLaneAt(matrix<T,N,M> sourceValue, int quadLaneID);

__generic<T : __BuiltinType> T QuadSwapX(T localValue);
__generic<T : __BuiltinType, let N : int> vector<T,N> QuadSwapX(vector<T,N> localValue);
__generic<T : __BuiltinType, let N : int, let M : int> matrix<T,N,M> QuadSwapX(matrix<T,N,M> localValue);

__generic<T : __BuiltinType> T QuadSwapY(T localValue);
__generic<T : __BuiltinType, let N : int> vector<T,N> QuadSwapY(vector<T,N> localValue);
__generic<T : __BuiltinType, let N : int, let M : int> matrix<T,N,M> QuadSwapY(matrix<T,N,M> localValue);

__generic<T : __BuiltinIntegerType> T WaveAllBitAnd(T expr);
__generic<T : __BuiltinIntegerType, let N : int> vector<T,N> WaveAllBitAnd(vector<T,N> expr);
__generic<T : __BuiltinIntegerType, let N : int, let M : int> matrix<T,N,M> WaveAllBitAnd(matrix<T,N,M> expr);

__generic<T : __BuiltinIntegerType> T WaveAllBitOr(T expr);
__generic<T : __BuiltinIntegerType, let N : int> vector<T,N> WaveAllBitOr(vector<T,N> expr);
__generic<T : __BuiltinIntegerType, let N : int, let M : int> matrix<T,N,M> WaveAllBitOr(matrix<T,N,M> expr);

__generic<T : __BuiltinIntegerType> T WaveAllBitXor(T expr);
__generic<T : __BuiltinIntegerType, let N : int> vector<T,N> WaveAllBitXor(vector<T,N> expr);
__generic<T : __BuiltinIntegerType, let N : int, let M : int> matrix<T,N,M> WaveAllBitXor(matrix<T,N,M> expr);

__generic<T : __BuiltinArithmeticType> T WaveAllMax(T expr);
__generic<T : __BuiltinArithmeticType, let N : int> vector<T,N> WaveAllMax(vector<T,N> expr);
__generic<T : __BuiltinArithmeticType, let N : int, let M : int> matrix<T,N,M> WaveAllMax(matrix<T,N,M> expr);

__generic<T : __BuiltinArithmeticType> T WaveAllMin(T expr);
__generic<T : __BuiltinArithmeticType, let N : int> vector<T,N> WaveAllMin(vector<T,N> expr);
__generic<T : __BuiltinArithmeticType, let N : int, let M : int> matrix<T,N,M> WaveAllMin(matrix<T,N,M> expr);

__generic<T : __BuiltinArithmeticType> T WaveAllProduct(T expr);
__generic<T : __BuiltinArithmeticType, let N : int> vector<T,N> WaveAllProduct(vector<T,N> expr);
__generic<T : __BuiltinArithmeticType, let N : int, let M : int> matrix<T,N,M> WaveAllProduct(matrix<T,N,M> expr);

__generic<T : __BuiltinArithmeticType> T WaveAllSum(T expr);
__generic<T : __BuiltinArithmeticType, let N : int> vector<T,N> WaveAllSum(vector<T,N> expr);
__generic<T : __BuiltinArithmeticType, let N : int, let M : int> matrix<T,N,M> WaveAllSum(matrix<T,N,M> expr);

bool WaveAllEqual(bool expr);
bool WaveAllTrue(bool expr);
bool WaveAnyTrue(bool expr);

uint64_t WaveBallot(bool expr);

uint WaveGetLaneCount();
uint WaveGetLaneIndex();
uint WaveGetOrderedIndex();

bool WaveIsHelperLane();

bool WaveOnce();

__generic<T : __BuiltinArithmeticType> T WavePrefixProduct(T expr);
__generic<T : __BuiltinArithmeticType, let N : int> vector<T,N> WavePrefixProduct(vector<T,N> expr);
__generic<T : __BuiltinArithmeticType, let N : int, let M : int> matrix<T,N,M> WavePrefixProduct(matrix<T,N,M> expr);

__generic<T : __BuiltinArithmeticType> T WavePrefixSum(T expr);
__generic<T : __BuiltinArithmeticType, let N : int> vector<T,N> WavePrefixSum(vector<T,N> expr);
__generic<T : __BuiltinArithmeticType, let N : int, let M : int> matrix<T,N,M> WavePrefixSum(matrix<T,N,M> expr);

__generic<T : __BuiltinType> T WaveReadFirstLane(T expr);
__generic<T : __BuiltinType, let N : int> vector<T,N> WaveReadFirstLane(vector<T,N> expr);
__generic<T : __BuiltinType, let N : int, let M : int> matrix<T,N,M> WaveReadFirstLane(matrix<T,N,M> expr);

__generic<T : __BuiltinType> T WaveReadLaneAt(T expr, int laneIndex);
__generic<T : __BuiltinType, let N : int> vector<T,N> WaveReadLaneAt(vector<T,N> expr, int laneIndex);
__generic<T : __BuiltinType, let N : int, let M : int> matrix<T,N,M> WaveReadLaneAt(matrix<T,N,M> expr, int laneIndex);

// `typedef`s to help with the fact that HLSL has been sorta-kinda case insensitive at various points
typedef Texture2D texture2D;

${{{{
// Component-wise multiplication ops
for(auto op : binaryOps)
{
    switch (op.opCode)
    {
    default:
        continue;

    case kIROp_Mul:
    case kIRPseudoOp_MulAssign:
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

//

// Buffer types

static const struct {
    char const*         name;
    SlangResourceAccess access;
} kBaseBufferAccessLevels[] = {
    { "",                   SLANG_RESOURCE_ACCESS_READ },
    { "RW",                 SLANG_RESOURCE_ACCESS_READ_WRITE },
    { "RasterizerOrdered",  SLANG_RESOURCE_ACCESS_RASTER_ORDERED },
};
static const int kBaseBufferAccessLevelCount = sizeof(kBaseBufferAccessLevels) / sizeof(kBaseBufferAccessLevels[0]);

for (int aa = 0; aa < kBaseBufferAccessLevelCount; ++aa)
{
    auto flavor = TextureFlavor::create(TextureFlavor::Shape::ShapeBuffer, kBaseBufferAccessLevels[aa].access).flavor;
    sb << "__generic<T>\n";
    sb << "__magic_type(Texture," << int(flavor) << ")\n";
    sb << "__intrinsic_type(" << (kIROp_TextureType + (int(flavor) << kIROpMeta_OtherShift)) << ")\n";
    sb << "struct ";
    sb << kBaseBufferAccessLevels[aa].name;
    sb << "Buffer {\n";

    sb << "void GetDimensions(out uint dim);\n";

    sb << "__glsl_extension(GL_EXT_samplerless_texture_functions)";
    sb << "__target_intrinsic(glsl, \"texelFetch($0, $1)$z\")\n";
    sb << "T Load(int location);\n";

    sb << "T Load(int location, out uint status);\n";

    sb << "__subscript(uint index) -> T {\n";

    sb << "__glsl_extension(GL_EXT_samplerless_texture_functions)";
    sb << "__target_intrinsic(glsl, \"texelFetch($0, int($1))$z\") get;\n";

    if (kBaseBufferAccessLevels[aa].access != SLANG_RESOURCE_ACCESS_READ)
    {
        sb << "ref;\n";
    }

    sb << "}\n";

    sb << "};\n";
}
}}}}


// DirectX Raytracing (DXR) Support
//
// The following is based on the experimental DXR SDK v0.09.01.
//
// Numbering follows the sections in the "D3D12 Raytracing Functional Spec" v0.09 (2018-03-12)
//

// 10.1.1 - Ray Flags

typedef uint RAY_FLAG;

static const RAY_FLAG RAY_FLAG_NONE                             = 0x00;
static const RAY_FLAG RAY_FLAG_FORCE_OPAQUE                     = 0x01;
static const RAY_FLAG RAY_FLAG_FORCE_NON_OPAQUE                 = 0x02;
static const RAY_FLAG RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH  = 0x04;
static const RAY_FLAG RAY_FLAG_SKIP_CLOSEST_HIT_SHADER          = 0x08;
static const RAY_FLAG RAY_FLAG_CULL_BACK_FACING_TRIANGLES       = 0x10;
static const RAY_FLAG RAY_FLAG_CULL_FRONT_FACING_TRIANGLES      = 0x20;
static const RAY_FLAG RAY_FLAG_CULL_OPAQUE                      = 0x40;
static const RAY_FLAG RAY_FLAG_CULL_NON_OPAQUE                  = 0x80;

// 10.1.2 - Ray Description Structure

__target_intrinsic(hlsl, RayDesc)
struct RayDesc
{
    __target_intrinsic(hlsl, Origin)
    float3 Origin;

    __target_intrinsic(hlsl, TMin)
    float  TMin;

    __target_intrinsic(hlsl, Direction)
    float3 Direction;

    __target_intrinsic(hlsl, TMax)
    float  TMax;
};

// 10.1.3 - Ray Acceleration Structure

__builtin
__magic_type(RaytracingAccelerationStructureType)
__intrinsic_type($(kIROp_RaytracingAccelerationStructureType))
struct RaytracingAccelerationStructure {};

// 10.1.4 - Subobject Definitions

// TODO: We may decide to support these, but their reliance on C++ implicit
// constructor call syntax (`SomeType someVar(arg0, arg1);`) makes them
// annoying for the current Slang parsing strategy, and using global variables
// for this stuff comes across as a kludge rather than the best possible design.

// 10.1.5 - Intersection Attributes Structure

__target_intrinsic(hlsl, BuiltInTriangleIntersectionAttributes)
struct BuiltInTriangleIntersectionAttributes
{
    __target_intrinsic(hlsl, barycentrics)
    float2 barycentrics;
};

// 10.2 Shaders

// Right now new shader stages need to be added directly to the compiler
// implementation, rather than being something that can be declared in the stdlib.

// 10.3 - Intrinsics

// 10.3.1

void CallShader<Payload>(uint shaderIndex, inout Payload payload);

// `executeCallableNV` is the GLSL intrinsic that will be used to implement
// `CallShader()` for GLSL-based targets.
//
__target_intrinsic(glsl, "executeCallableNV")
void __executeCallableNV(uint shaderIndex, int payloadLocation);

// Next is the custom intrinsic that will compute the payload location
// for a type being used in a `CallShader()` call for GLSL-based targets.
//
__generic<Payload>
__target_intrinsic(glsl, "$XC")
[__readNone]
int __callablePayloadLocation(Payload payload);

// Now we provide a hard-coded definition of `CallShader()` for GLSL-based
// targets, which maps the generic HLSL operation into the non-generic
// GLSL equivalent.
//
__generic<Payload>
__specialized_for_target(glsl)
void CallShader(uint shaderIndex, inout Payload payload)
{
    [__vulkanRayPayload]
    static Payload p;

    p = payload;
    __executeCallableNV(shaderIndex, __callablePayloadLocation(p));
    payload = p;
}

// 10.3.2
void TraceRay<payload_t>(
    RaytracingAccelerationStructure AccelerationStructure,
    uint                            RayFlags,
    uint                            InstanceInclusionMask,
    uint                            RayContributionToHitGroupIndex,
    uint                            MultiplierForGeometryContributionToHitGroupIndex,
    uint                            MissShaderIndex,
    RayDesc                         Ray,
    inout payload_t                 Payload);

__target_intrinsic(glsl, "traceNV")
void __traceNV(
    RaytracingAccelerationStructure AccelerationStructure,
    uint                            RayFlags,
    uint                            InstanceInclusionMask,
    uint                            RayContributionToHitGroupIndex,
    uint                            MultiplierForGeometryContributionToHitGroupIndex,
    uint                            MissShaderIndex,
    float3                          Origin,
    float                           TMin,
    float3                          Direction,
    float                           TMax,
    int                             PayloadLocation);

// TODO: Slang's parsing logic currently puts modifiers on
// the `GenericDecl` rather than the inner decl when
// using our default syntax, which seems wrong. We need
// to fix this, but for now using the expanded `__generic`
// syntax works in a pinch.
//
__generic<Payload>
__target_intrinsic(glsl, "$XP")
[__readNone]
int __rayPayloadLocation(Payload payload);

__generic<payload_t>
__specialized_for_target(glsl)
void TraceRay(
    RaytracingAccelerationStructure AccelerationStructure,
    uint                            RayFlags,
    uint                            InstanceInclusionMask,
    uint                            RayContributionToHitGroupIndex,
    uint                            MultiplierForGeometryContributionToHitGroupIndex,
    uint                            MissShaderIndex,
    RayDesc                         Ray,
    inout payload_t                 Payload)
{
    [__vulkanRayPayload]
    static payload_t p;

    p = Payload;
    __traceNV(
        AccelerationStructure,
        RayFlags,
        InstanceInclusionMask,
        RayContributionToHitGroupIndex,
        MultiplierForGeometryContributionToHitGroupIndex,
        MissShaderIndex,
        Ray.Origin,
        Ray.TMin,
        Ray.Direction,
        Ray.TMax,
        __rayPayloadLocation(p));
    Payload = p;
}

// 10.3.3
bool ReportHit<A>(float tHit, uint hitKind, A attributes);

__target_intrinsic(glsl, "reportIntersectionNV")
bool __reportIntersectionNV(float tHit, uint hitKind);

__generic<A>
__specialized_for_target(glsl)
bool ReportHit(float tHit, uint hitKind, A attributes)
{
    [__vulkanHitAttributes]
    static A a;

    a = attributes;
    return __reportIntersectionNV(tHit, hitKind);
}

// 10.3.4
__target_intrinsic(glsl, ignoreIntersectionNV)
void IgnoreHit();

// 10.3.5
__target_intrinsic(glsl, terminateRayNV)
void AcceptHitAndEndSearch();

// 10.4 - System Values and Special Semantics

// TODO: Many of these functions need to be restricted so that
// they can only be accessed from specific stages.

// 10.4.1 - Ray Dispatch System Values

__target_intrinsic(glsl, "(gl_LaunchIDNV)")
uint3 DispatchRaysIndex();

__target_intrinsic(glsl, "(gl_LaunchSizeNV)")
uint3 DispatchRaysDimensions();

// 10.4.2 - Ray System Values

__target_intrinsic(glsl, "(gl_WorldRayOriginNV)")
float3 WorldRayOrigin();

__target_intrinsic(glsl, "(gl_WorldRayDirectionNV)")
float3 WorldRayDirection();

__target_intrinsic(glsl, "(gl_RayTminNV)")
float RayTMin();

// Note: The `RayTCurrent()` intrinsic should translate to
// either `gl_HitTNV` (for hit shaders) or `gl_RayTmaxNV`
// (for intersection shaders). Right now we are handling this
// during code emission, for simplicity.
//
// TODO: Once the compiler supports a more refined concept
// of profiles/capabilities and overloading based on them,
// we should simply provide two overloads here, specialized
// to the appropriate Vulkan stages.
//
__target_intrinsic(glsl, "$XT")
float RayTCurrent();

__target_intrinsic(glsl, "(gl_IncomingRayFlagsNV)")
uint RayFlags();

// 10.4.3 - Primitive/Object Space System Values

__target_intrinsic(glsl, "(gl_InstanceCustomIndexNV)")
uint InstanceIndex();

__target_intrinsic(glsl, "(gl_InstanceID)")
uint InstanceID();

__target_intrinsic(glsl, "(gl_PrimitiveID)")
uint PrimitiveIndex();

__target_intrinsic(glsl, "(gl_ObjectRayOriginNV)")
float3 ObjectRayOrigin();

__target_intrinsic(glsl, "(gl_ObjectRayDirectionNV)")
float3 ObjectRayDirection();

__target_intrinsic(glsl, "transpose(gl_ObjectToWorldNV)")
float3x4 ObjectToWorld3x4();

__target_intrinsic(glsl, "transpose(gl_WorldToObjectNV)")
float3x4 WorldToObject3x4();

__target_intrinsic(glsl, "(gl_ObjectToWorldNV)")
float4x3 ObjectToWorld4x3();

__target_intrinsic(glsl, "(gl_WorldToObjectNV)")
float4x3 WorldToObject4x3();

// Note: The provisional DXR spec included these unadorned
// `ObjectToWorld()` and `WorldToObject()` functions, so
// we will forward them to the new names as a convience
// for users who are porting their code.
//
// TODO: Should we provide a deprecation warning on these
// declarations, so that users can know they aren't coding
// against the final spec?
//
float3x4 ObjectToWorld() { return ObjectToWorld3x4(); }
float3x4 WorldToObject() { return WorldToObject3x4(); }

// 10.4.4 - Hit Specific System values
__target_intrinsic(glsl, "(gl_HitKindNV)")
uint HitKind();
