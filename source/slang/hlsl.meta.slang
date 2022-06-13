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

    T Load<T>(int location)
    {
        return __byteAddressBufferLoad<T>(this, location);
    }
};

// AtomicAdd

// Make the GLSL atomicAdd available.
// We have separate int/float implementations, as the float version requires some specific extensions
// https://www.khronos.org/registry/OpenGL/extensions/NV/NV_shader_atomic_float.txt

__target_intrinsic(glsl, "atomicAdd($0, $1)")
__glsl_version(430)
__glsl_extension(GL_EXT_shader_atomic_float)
float __atomicAdd(__ref float value, float amount);

// Helper for hlsl, using NVAPI
__target_intrinsic(hlsl, "NvInterlockedAddUint64($0, $1, $2)")
[__requiresNVAPI]
uint2 __atomicAdd(RWByteAddressBuffer buf, uint offset, uint2);

// atomic add for hlsl using SM6.6
__target_intrinsic(hlsl, "$0.InterlockedAdd64($1, $2, $3)")
void __atomicAdd(RWByteAddressBuffer buf, uint offset, int64_t value, out int64_t originalValue);
__target_intrinsic(hlsl, "$0.InterlockedAdd64($1, $2, $3)")
void __atomicAdd(RWByteAddressBuffer buf, uint offset, uint64_t value, out uint64_t originalValue);

// Int versions require glsl 4.30
// https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/atomicAdd.xhtml

__target_intrinsic(glsl, "atomicAdd($0, $1)")
__glsl_version(430)
int __atomicAdd(__ref int value, int amount);

__target_intrinsic(glsl, "atomicAdd($0, $1)")
__glsl_version(430)
uint __atomicAdd(__ref uint value, uint amount);

__target_intrinsic(glsl, "atomicAdd($0, $1)")
__glsl_version(430)
__glsl_extension(GL_EXT_shader_atomic_int64)
int64_t __atomicAdd(__ref int64_t value, int64_t amount);

__target_intrinsic(glsl, "atomicAdd($0, $1)")
__glsl_version(430)
__glsl_extension(GL_EXT_shader_atomic_int64)
uint64_t __atomicAdd(__ref uint64_t value, uint64_t amount);

// Cas - Compare and swap

// Helper for HLSL, using NVAPI

__target_intrinsic(hlsl, "NvInterlockedCompareExchangeUint64($0, $1, $2, $3)")
[__requiresNVAPI]
uint2 __cas(RWByteAddressBuffer buf, uint offset, uint2 compareValue, uint2 value);

// CAS using SM6.6
__target_intrinsic(hlsl, "$0.InterlockedCompareExchange64($1, $2, $3, $4)")
void __cas(RWByteAddressBuffer buf, uint offset, in int64_t compare_value, in int64_t value, out int64_t original_value);
__target_intrinsic(hlsl, "$0.InterlockedCompareExchange64($1, $2, $3, $4)")
void __cas(RWByteAddressBuffer buf, uint offset, in uint64_t compare_value, in uint64_t value, out uint64_t original_value);

__target_intrinsic(glsl, "atomicCompSwap($0, $1, $2)")
__glsl_version(430)
__glsl_extension(GL_EXT_shader_atomic_int64)
uint64_t __cas(__ref int64_t ioValue, int64_t compareValue, int64_t newValue);

__target_intrinsic(glsl, "atomicCompSwap($0, $1, $2)")
__glsl_version(430)
__glsl_extension(GL_EXT_shader_atomic_int64)
uint64_t __cas(__ref uint64_t ioValue, uint64_t compareValue, uint64_t newValue);

// Max

__target_intrinsic(hlsl, "NvInterlockedMaxUint64($0, $1, $2)")
[__requiresNVAPI]
uint2 __atomicMax(RWByteAddressBuffer buf, uint offset, uint2 value);

__target_intrinsic(glsl, "atomicMax($0, $1)")
__glsl_version(430)
__glsl_extension(GL_EXT_shader_atomic_int64)
uint64_t __atomicMax(__ref uint64_t ioValue, uint64_t value);

// Min

__target_intrinsic(hlsl, "NvInterlockedMinUint64($0, $1, $2)")
[__requiresNVAPI]
uint2 __atomicMin(RWByteAddressBuffer buf, uint offset, uint2 value);

__target_intrinsic(glsl, "atomicMin($0, $1)")
__glsl_version(430)
__glsl_extension(GL_EXT_shader_atomic_int64)
uint64_t __atomicMin(__ref uint64_t ioValue, uint64_t value);

// And

__target_intrinsic(hlsl, "NvInterlockedAndUint64($0, $1, $2)")
[__requiresNVAPI]
uint2 __atomicAnd(RWByteAddressBuffer buf, uint offset, uint2 value);

__target_intrinsic(glsl, "atomicAnd($0, $1)")
__glsl_version(430)
__glsl_extension(GL_EXT_shader_atomic_int64)
uint64_t __atomicAnd(__ref uint64_t ioValue, uint64_t value);

// Or

__target_intrinsic(hlsl, "NvInterlockedOrUint64($0, $1, $2)")
[__requiresNVAPI]
uint2 __atomicOr(RWByteAddressBuffer buf, uint offset, uint2 value);

__target_intrinsic(glsl, "atomicOr($0, $1)")
__glsl_version(430)
__glsl_extension(GL_EXT_shader_atomic_int64)
uint64_t __atomicOr(__ref uint64_t ioValue, uint64_t value);

// Xor

__target_intrinsic(hlsl, "NvInterlockedXorUint64($0, $1, $2)")
[__requiresNVAPI]
uint2 __atomicXor(RWByteAddressBuffer buf, uint offset, uint2 value);

__target_intrinsic(glsl, "atomicXor($0, $1)")
__glsl_version(430)
__glsl_extension(GL_EXT_shader_atomic_int64)
uint64_t __atomicXor(__ref uint64_t ioValue, uint64_t value);

// Exchange

__target_intrinsic(hlsl, "NvInterlockedExchangeUint64($0, $1, $2)")
[__requiresNVAPI]
uint2 __atomicExchange(RWByteAddressBuffer buf, uint offset, uint2 value);

__target_intrinsic(glsl, "atomicExchange($0, $1)")
__glsl_version(430)
__glsl_extension(GL_EXT_shader_atomic_int64)
uint64_t __atomicExchange(__ref uint64_t ioValue, uint64_t value);

// Conversion between uint64_t and uint2

uint2 __asuint2(uint64_t i)
{
    return uint2(uint(i), uint(uint64_t(i) >> 32));
}

uint64_t __asuint64(uint2 i)
{
    return (uint64_t(i.y) << 32) | i.x;
}

// 

__intrinsic_op($(kIROp_ByteAddressBufferLoad))
T __byteAddressBufferLoad<T>(ByteAddressBuffer buffer, int offset);

__intrinsic_op($(kIROp_ByteAddressBufferLoad))
T __byteAddressBufferLoad<T>(RWByteAddressBuffer buffer, int offset);

__intrinsic_op($(kIROp_ByteAddressBufferLoad))
T __byteAddressBufferLoad<T>(RasterizerOrderedByteAddressBuffer buffer, int offset);

__intrinsic_op($(kIROp_ByteAddressBufferStore))
void __byteAddressBufferStore<T>(RWByteAddressBuffer buffer, int offset, T value);

__intrinsic_op($(kIROp_ByteAddressBufferStore))
void __byteAddressBufferStore<T>(RasterizerOrderedByteAddressBuffer buffer, int offset, T value);

__generic<T>
__magic_type(HLSLStructuredBufferType)
__intrinsic_type($(kIROp_HLSLStructuredBufferType))
struct StructuredBuffer
{
    __target_intrinsic(glsl, "$1 = $0._data.length(); $2 = 0")
    void GetDimensions(
        out uint numStructs,
        out uint stride);

    __target_intrinsic(glsl, "$0._data[$1]")
    __target_intrinsic(spirv_direct, "%addr = 65 resultType*StorageBuffer resultId _0 const(int, 0) _1; 61 resultType resultId %addr;")
    T Load(int location);
    T Load(int location, out uint status);

    __subscript(uint index) -> T
    {
        __target_intrinsic(glsl, "$0._data[$1]")
        __target_intrinsic(spirv_direct, "%addr = 65 resultType*StorageBuffer resultId _0 const(int, 0) _1; 61 resultType resultId %addr;")
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

    T Load<T>(int location)
    {
        return __byteAddressBufferLoad<T>(this, location);
    }
${{{{
    if (item.op == kIROp_HLSLRWByteAddressBufferType)
    {  
}}}}

    // float32 and int64 atomic support. This is a Slang specific extension, it uses
    // GL_EXT_shader_atomic_float on Vulkan
    // NvAPI support on DX
    // NOTE! To use this feature on HLSL based targets the path to 'nvHLSLExtns.h' from the NvAPI SDK must
    // be set. That this include will be added to the *output* that is passed to a downstram compiler.
    // Also note that you *can* include NVAPI headers in your Slang source, and directly use NVAPI functions
    // Directly using NVAPI functions does *not* add the #include on the output
    // Finally note you can *mix* NVAPI direct calls, and use of NVAPI intrinsics below. This doesn't cause
    // any clashes, as Slang will emit any NVAPI function it parsed (say via a include in Slang source) with
    // unique functions.
    // 
    // https://www.khronos.org/registry/vulkan/specs/1.2-extensions/html/vkspec.html#VK_EXT_shader_atomic_float
    // https://htmlpreview.github.io/?https://github.com/KhronosGroup/SPIRV-Registry/blob/master/extensions/EXT/SPV_EXT_shader_atomic_float_add.html

    // F32 Add

    __target_intrinsic(hlsl, "($3 = NvInterlockedAddFp32($0, $1, $2))")
    __cuda_sm_version(2.0)
    __target_intrinsic(cuda, "(*$3 = atomicAdd($0._getPtrAt<float>($1), $2))")
    [__requiresNVAPI]
    void InterlockedAddF32(uint byteAddress, float valueToAdd, out float originalValue);

    __specialized_for_target(glsl)
    void InterlockedAddF32(uint byteAddress, float valueToAdd, out float originalValue)
    {
        RWStructuredBuffer<float> buf = __getEquivalentStructuredBuffer<float>(this);
        originalValue = __atomicAdd(buf[byteAddress / 4], valueToAdd);
    }

    // Without returning original value

    __target_intrinsic(hlsl, "(NvInterlockedAddFp32($0, $1, $2))")
    [__requiresNVAPI]
    __cuda_sm_version(2.0)
    __target_intrinsic(cuda, "atomicAdd($0._getPtrAt<float>($1), $2)")
    void InterlockedAddF32(uint byteAddress, float valueToAdd);

    __specialized_for_target(glsl)
    void InterlockedAddF32(uint byteAddress, float valueToAdd)
    {
        RWStructuredBuffer<float> buf = __getEquivalentStructuredBuffer<float>(this);
        __atomicAdd(buf[byteAddress / 4], valueToAdd);
    }

    // Int64 Add
    __cuda_sm_version(6.0)
    __target_intrinsic(cuda, "(*$3 = atomicAdd($0._getPtrAt<uint64_t>($1), $2))")
    void InterlockedAddI64(uint byteAddress, int64_t valueToAdd, out int64_t originalValue);

    __specialized_for_target(hlsl)
    void InterlockedAddI64(uint byteAddress, int64_t valueToAdd, out int64_t outOriginalValue)
    {
        outOriginalValue = __asuint64(__atomicAdd(this, byteAddress, __asuint2(valueToAdd)));
    }

    __specialized_for_target(glsl)
    void InterlockedAddI64(uint byteAddress, int64_t valueToAdd, out int64_t originalValue)
    {
        RWStructuredBuffer<int64_t> buf = __getEquivalentStructuredBuffer<int64_t>(this);
        originalValue = __atomicAdd(buf[byteAddress / 8], valueToAdd);
    }

    // Without returning original value
    __cuda_sm_version(6.0)
    __target_intrinsic(cuda, "atomicAdd($0._getPtrAt<uint64_t>($1), $2)")
    void InterlockedAddI64(uint byteAddress, int64_t valueToAdd);

    __specialized_for_target(hlsl)
    void InterlockedAddI64(uint byteAddress, int64_t valueToAdd)
    {
        __atomicAdd(this, byteAddress, __asuint2(valueToAdd));
    }

    __specialized_for_target(glsl)
    void InterlockedAddI64(uint byteAddress, int64_t valueToAdd)
    {
        RWStructuredBuffer<int64_t> buf = __getEquivalentStructuredBuffer<int64_t>(this);
        __atomicAdd(buf[byteAddress / 8], valueToAdd);
    }

    // Cas uint64_t

    __target_intrinsic(cuda, "(*$4 = atomicCAS($0._getPtrAt<uint64_t>($1), $2, $3))")
    void InterlockedCompareExchangeU64(uint byteAddress, uint64_t compareValue, uint64_t value, out uint64_t outOriginalValue);

    __specialized_for_target(hlsl)
    void InterlockedCompareExchangeU64(uint byteAddress, uint64_t compareValue, uint64_t value, out uint64_t outOriginalValue)
    {
        outOriginalValue = __asuint64(__cas(this, byteAddress, __asuint2(compareValue), __asuint2(value)));
    }

    __specialized_for_target(glsl)
    void InterlockedCompareExchangeU64(uint byteAddress, uint64_t compareValue, uint64_t value, out uint64_t outOriginalValue)
    {
        RWStructuredBuffer<uint64_t> buf = __getEquivalentStructuredBuffer<uint64_t>(this);
        outOriginalValue = __cas(buf[byteAddress / 8], compareValue, value);
    }

    // Max

    __cuda_sm_version(3.5)
    __target_intrinsic(cuda, "atomicMax($0._getPtrAt<uint64_t>($1), $2)")
    uint64_t InterlockedMaxU64(uint byteAddress, uint64_t value);

    __specialized_for_target(hlsl)
    uint64_t InterlockedMaxU64(uint byteAddress, uint64_t value) { return __asuint64(__atomicMax(this, byteAddress, __asuint2(value))); }

    __specialized_for_target(glsl)
    uint64_t InterlockedMaxU64(uint byteAddress, uint64_t value)
    {
        RWStructuredBuffer<uint64_t> buf = __getEquivalentStructuredBuffer<uint64_t>(this);
        return __atomicMax(buf[byteAddress / 8], value);
    }

    // Min
    
    __cuda_sm_version(3.5)
    __target_intrinsic(cuda, "atomicMin($0._getPtrAt<uint64_t>($1), $2)")
    uint64_t InterlockedMinU64(uint byteAddress, uint64_t value);

    __specialized_for_target(hlsl)
    uint64_t InterlockedMinU64(uint byteAddress, uint64_t value) { return __asuint64(__atomicMin(this, byteAddress, __asuint2(value))); }

    __specialized_for_target(glsl)
    uint64_t InterlockedMinU64(uint byteAddress, uint64_t value)
    {
        RWStructuredBuffer<uint64_t> buf = __getEquivalentStructuredBuffer<uint64_t>(this);
        return __atomicMin(buf[byteAddress / 8], value);
    }

    // And

    __target_intrinsic(cuda, "atomicAnd($0._getPtrAt<uint64_t>($1), $2)")
    uint64_t InterlockedAndU64(uint byteAddress, uint64_t value);

    __specialized_for_target(hlsl)
    uint64_t InterlockedAndU64(uint byteAddress, uint64_t value) { return __asuint64(__atomicAnd(this, byteAddress, __asuint2(value))); }

    __specialized_for_target(glsl)
    uint64_t InterlockedAndU64(uint byteAddress, uint64_t value)
    {
        RWStructuredBuffer<uint64_t> buf = __getEquivalentStructuredBuffer<uint64_t>(this);
        return __atomicAnd(buf[byteAddress / 8], value);
    }

    // Or

    __target_intrinsic(cuda, "atomicOr($0._getPtrAt<uint64_t>($1), $2)")
    uint64_t InterlockedOrU64(uint byteAddress, uint64_t value);

    __specialized_for_target(hlsl)
    uint64_t InterlockedOrU64(uint byteAddress, uint64_t value) { return __asuint64(__atomicOr(this, byteAddress, __asuint2(value))); }

    __specialized_for_target(glsl)
    uint64_t InterlockedOrU64(uint byteAddress, uint64_t value)
    {
        RWStructuredBuffer<uint64_t> buf = __getEquivalentStructuredBuffer<uint64_t>(this);
        return __atomicOr(buf[byteAddress / 8], value);
    }

    // Xor

    __target_intrinsic(cuda, "atomicXor($0._getPtrAt<uint64_t>($1), $2)")
    uint64_t InterlockedXorU64(uint byteAddress, uint64_t value);

    __specialized_for_target(hlsl)
    uint64_t InterlockedXorU64(uint byteAddress, uint64_t value) { return __asuint64(__atomicXor(this, byteAddress, __asuint2(value))); }

    __specialized_for_target(glsl)
    uint64_t InterlockedXorU64(uint byteAddress, uint64_t value)
    {
        RWStructuredBuffer<uint64_t> buf = __getEquivalentStructuredBuffer<uint64_t>(this);
        return __atomicXor(buf[byteAddress / 8], value);
    }

    // Exchange

    __target_intrinsic(cuda, "atomicExch($0._getPtrAt<uint64_t>($1), $2)")
    uint64_t InterlockedExchangeU64(uint byteAddress, uint64_t value);

    __specialized_for_target(hlsl)
    uint64_t InterlockedExchangeU64(uint byteAddress, uint64_t value) { return __asuint64(__atomicExchange(this, byteAddress, __asuint2(value))); }

    __specialized_for_target(glsl)
    uint64_t InterlockedExchangeU64(uint byteAddress, uint64_t value)
    {
        RWStructuredBuffer<uint64_t> buf = __getEquivalentStructuredBuffer<uint64_t>(this);
        return __atomicExchange(buf[byteAddress / 8], value);
    }

    // SM6.6 6 64bit atomics.
    __specialized_for_target(hlsl)
    void InterlockedAdd64(uint byteAddress, int64_t valueToAdd, out int64_t outOriginalValue)
    {
        __atomicAdd(this, byteAddress, valueToAdd, outOriginalValue);
    }
    __specialized_for_target(glsl)
    void InterlockedAdd64(uint byteAddress, int64_t valueToAdd, out int64_t originalValue)
    {
        RWStructuredBuffer<int64_t> buf = __getEquivalentStructuredBuffer<int64_t>(this);
        originalValue = __atomicAdd(buf[byteAddress / 8], valueToAdd);
    }
    __specialized_for_target(hlsl)
    void InterlockedAdd64(uint byteAddress, uint64_t valueToAdd, out uint64_t outOriginalValue)
    {
        __atomicAdd(this, byteAddress, valueToAdd, outOriginalValue);
    }
    __specialized_for_target(glsl)
    void InterlockedAdd64(uint byteAddress, uint64_t valueToAdd, out uint64_t originalValue)
    {
        RWStructuredBuffer<uint64_t> buf = __getEquivalentStructuredBuffer<uint64_t>(this);
        originalValue = __atomicAdd(buf[byteAddress / 8], valueToAdd);
    }
    __specialized_for_target(hlsl)
    void InterlockedCompareExchange64(uint byteAddress, int64_t compareValue, int64_t value, out int64_t outOriginalValue)
    {
        __cas(this, byteAddress, compareValue, value, outOriginalValue);
    }
    __specialized_for_target(glsl)
    void InterlockedCompareExchange64(uint byteAddress, int64_t compareValue, int64_t value, out int64_t outOriginalValue)
    {
        RWStructuredBuffer<int64_t> buf = __getEquivalentStructuredBuffer<int64_t>(this);
        outOriginalValue = __cas(buf[byteAddress / 8], compareValue, value);
    }
    __specialized_for_target(hlsl)
    void InterlockedCompareExchange64(uint byteAddress, uint64_t compareValue, uint64_t value, out uint64_t outOriginalValue)
    {
        __cas(this, byteAddress, compareValue, value, outOriginalValue);
    }
    __specialized_for_target(glsl)
    void InterlockedCompareExchange64(uint byteAddress, uint64_t compareValue, uint64_t value, out uint64_t outOriginalValue)
    {
        RWStructuredBuffer<uint64_t> buf = __getEquivalentStructuredBuffer<uint64_t>(this);
        outOriginalValue = __cas(buf[byteAddress / 8], compareValue, value);
    }
${{{{
    }
}}}}

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

    void Store<T>(int offset, T value)
    {
        __byteAddressBufferStore(this, offset, value);
    }
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

    __target_intrinsic(glsl, "$1 = $0._data.length(); $2 = 0")
    void GetDimensions(
        out uint numStructs,
        out uint stride);

    uint IncrementCounter();

    __target_intrinsic(glsl, "$0._data[$1]")
    __target_intrinsic(spirv_direct, "%addr = 65 resultType*StorageBuffer resultId _0 const(int, 0) _1; 61 resultType resultId %addr;")
    T Load(int location);
    T Load(int location, out uint status);

    __subscript(uint index) -> T
    {
        __target_intrinsic(glsl, "$0._data[$1]")
        __target_intrinsic(spirv_direct, "*StorageBuffer 65 resultType resultId _0 const(int, 0) _1")
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

#define VECTOR_MAP_UNARY(TYPE, COUNT, FUNC, VALUE) \
    vector<TYPE,COUNT> result; for(int i = 0; i < COUNT; ++i) { result[i] = FUNC(VALUE[i]); } return result
    
#define MATRIX_MAP_UNARY(TYPE, ROWS, COLS, FUNC, VALUE) \
    matrix<TYPE,ROWS,COLS> result; for(int i = 0; i < ROWS; ++i) { result[i] = FUNC(VALUE[i]); } return result

#define VECTOR_MAP_BINARY(TYPE, COUNT, FUNC, LEFT, RIGHT) \
    vector<TYPE,COUNT> result; for(int i = 0; i < COUNT; ++i) { result[i] = FUNC(LEFT[i], RIGHT[i]); } return result
    
#define MATRIX_MAP_BINARY(TYPE, ROWS, COLS, FUNC, LEFT, RIGHT) \
    matrix<TYPE,ROWS,COLS> result; for(int i = 0; i < ROWS; ++i) { result[i] = FUNC(LEFT[i], RIGHT[i]); } return result

#define VECTOR_MAP_TRINARY(TYPE, COUNT, FUNC, A, B, C) \
    vector<TYPE,COUNT> result; for(int i = 0; i < COUNT; ++i) { result[i] = FUNC(A[i], B[i], C[i]); } return result
    
#define MATRIX_MAP_TRINARY(TYPE, ROWS, COLS, FUNC, A, B, C) \
    matrix<TYPE,ROWS,COLS> result; for(int i = 0; i < ROWS; ++i) { result[i] = FUNC(A[i], B[i], C[i]); } return result

// Try to terminate the current draw or dispatch call (HLSL SM 4.0)
void abort();

// Absolute value (HLSL SM 1.0)

__generic<T : __BuiltinSignedArithmeticType>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(cuda, "$P_abs($0)")
__target_intrinsic(cpp, "$P_abs($0)")
__target_intrinsic(spirv_direct, "12 resultType resultId glsl450 fi(4,5) _0")
T abs(T x);
/*{
    // Note: this simple definition may not be appropriate for floating-point inputs
    return x < 0 ? -x : x;
}*/

__generic<T : __BuiltinSignedArithmeticType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv_direct, "12 resultType resultId glsl450 fi(4,5) _0")
vector<T, N> abs(vector<T, N> x)
{
    VECTOR_MAP_UNARY(T, N, abs, x);
}

__generic<T : __BuiltinSignedArithmeticType, let N : int, let M : int>
__target_intrinsic(hlsl)
matrix<T,N,M> abs(matrix<T,N,M> x)
{
    MATRIX_MAP_UNARY(T, N, M, abs, x);
}

// Inverse cosine (HLSL SM 1.0)

__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(cuda, "$P_acos($0)")
__target_intrinsic(cpp, "$P_acos($0)")
__target_intrinsic(spirv_direct, "12 resultType resultId glsl450 17 _0")
T acos(T x);

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv_direct, "12 resultType resultId glsl450 17 _0")
vector<T, N> acos(vector<T, N> x)
{
    VECTOR_MAP_UNARY(T, N, acos, x);
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(hlsl)
matrix<T, N, M> acos(matrix<T, N, M> x)
{
    MATRIX_MAP_UNARY(T, N, M, acos, x);
}

// Test if all components are non-zero (HLSL SM 1.0)
__generic<T : __BuiltinType>
__target_intrinsic(glsl, "bool($0)")
bool all(T x);

__generic<T : __BuiltinType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl, "all(bvec$N0($0))")
bool all(vector<T,N> x);
// TODO: implementation of `all()` in the stdlib is
// blocked on fixing implementation of `bool` vector
// `getAt` on the CUDA codegen path.
/*
{
    bool result = true;
    for(int i = 0; i < N; ++i)
        result = result && all(x[i]);
    return result;
}
*/

__generic<T : __BuiltinType, let N : int, let M : int>
__target_intrinsic(hlsl)
bool all(matrix<T,N,M> x);
/*
{
    bool result = true;
    for(int i = 0; i < N; ++i)
        result = result && all(x[i]);
    return result;
}
*/

// Barrier for writes to all memory spaces (HLSL SM 5.0)
__target_intrinsic(glsl, "memoryBarrier(), groupMemoryBarrier(), memoryBarrierImage(), memoryBarrierBuffer()")
__target_intrinsic(cuda, "__threadfence()")
void AllMemoryBarrier();

// Thread-group sync and barrier for writes to all memory spaces (HLSL SM 5.0)
__target_intrinsic(glsl, "memoryBarrier(), groupMemoryBarrier(), memoryBarrierImage(), memoryBarrierBuffer(), barrier()")
__target_intrinsic(cuda, "__syncthreads()")
void AllMemoryBarrierWithGroupSync();

// Test if any components is non-zero (HLSL SM 1.0)

__generic<T : __BuiltinType>
__target_intrinsic(glsl, "bool($0)")
bool any(T x);

__generic<T : __BuiltinType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl, "any(bvec$N0($0))")
bool any(vector<T, N> x);
// TODO: implementation of `any()` in the stdlib is
// blocked on fixing implementation of `bool` vector
// `getAt` on the CUDA codegen path.
/*
{
    bool result = false;
    for(int i = 0; i < N; ++i)
        result = result || any(x[i]);
    return result;
}
*/

__generic<T : __BuiltinType, let N : int, let M : int>
__target_intrinsic(hlsl)
bool any(matrix<T, N, M> x);
/*
{
    bool result = false;
    for(int i = 0; i < N; ++i)
        result = result || any(x[i]);
    return result;
}
*/


// Reinterpret bits as a double (HLSL SM 5.0)

__target_intrinsic(hlsl)
__target_intrinsic(glsl, "packDouble2x32(uvec2($0, $1))")
__target_intrinsic(spirv_direct, "%v = 80 _type(uint2) resultId _0 _1; 12 resultType resultId glsl450 59 %v")
__glsl_extension(GL_ARB_gpu_shader5)
double asdouble(uint lowbits, uint highbits);

// Reinterpret bits as a float (HLSL SM 4.0)

__target_intrinsic(hlsl)
__target_intrinsic(glsl, "intBitsToFloat")
__target_intrinsic(spirv_direct, "124 resultType resultId _0")
float asfloat(int x);

__target_intrinsic(hlsl)
__target_intrinsic(glsl, "uintBitsToFloat")
__target_intrinsic(spirv_direct, "124 resultType resultId _0")
float asfloat(uint x);

__generic<let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl, "intBitsToFloat")
__target_intrinsic(spirv_direct, "124 resultType resultId _0")
vector<float, N> asfloat(vector< int, N> x)
{
    VECTOR_MAP_UNARY(float, N, asfloat, x);
}

__generic<let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl, "uintBitsToFloat")
__target_intrinsic(spirv_direct, "124 resultType resultId _0")
vector<float,N> asfloat(vector<uint,N> x)
{
    VECTOR_MAP_UNARY(float, N, asfloat, x);
}

__generic<let N : int, let M : int>
__target_intrinsic(hlsl)
matrix<float,N,M> asfloat(matrix< int,N,M> x)
{
    MATRIX_MAP_UNARY(float, N, M, asfloat, x);
}

__generic<let N : int, let M : int>
__target_intrinsic(hlsl)
matrix<float,N,M> asfloat(matrix<uint,N,M> x)
{
    MATRIX_MAP_UNARY(float, N, M, asfloat, x);
}

// No op
[__unsafeForceInlineEarly]
float asfloat(float x)
{ return x; }

__generic<let N : int>
[__unsafeForceInlineEarly]
vector<float,N> asfloat(vector<float,N> x)
{ return x; }

__generic<let N : int, let M : int>
[__unsafeForceInlineEarly]
matrix<float,N,M> asfloat(matrix<float,N,M> x)
{ return x; }

// Inverse sine (HLSL SM 1.0)
__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(cuda, "$P_asin($0)")
__target_intrinsic(cpp, "$P_asin($0)")
__target_intrinsic(spirv_direct, "12 resultType resultId glsl450 16 _0")
T asin(T x);

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv_direct, "12 resultType resultId glsl450 16 _0")
vector<T, N> asin(vector<T, N> x)
{
    VECTOR_MAP_UNARY(T,N,asin,x);
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(hlsl)
matrix<T, N, M> asin(matrix<T, N, M> x)
{
    MATRIX_MAP_UNARY(T,N,M,asin,x);
}

// Reinterpret bits as an int (HLSL SM 4.0)

__target_intrinsic(hlsl)
__target_intrinsic(glsl, "floatBitsToInt")
__target_intrinsic(spirv_direct, "124 resultType resultId _0")
int asint(float x);

__target_intrinsic(hlsl)
__target_intrinsic(glsl, "int($0)")
__target_intrinsic(spirv_direct, "124 resultType resultId _0")
int asint(uint x);

__generic<let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl, "floatBitsToInt")
__target_intrinsic(spirv_direct, "124 resultType resultId _0")
vector<int, N> asint(vector<float, N> x)
{
    VECTOR_MAP_UNARY(int, N, asint, x);
}

__generic<let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl, "ivec$N0($0)")
__target_intrinsic(spirv_direct, "124 resultType resultId _0")
vector<int, N> asint(vector<uint, N> x)
{
    VECTOR_MAP_UNARY(int, N, asint, x);
}

__generic<let N : int, let M : int>
__target_intrinsic(hlsl)
matrix<int, N, M> asint(matrix<float, N, M> x)
{
    MATRIX_MAP_UNARY(int, N, M, asint, x);
}

__generic<let N : int, let M : int>
__target_intrinsic(hlsl)
matrix<int, N, M> asint(matrix<uint, N, M> x)
{
    MATRIX_MAP_UNARY(int, N, M, asint, x);
}

// No op
[__unsafeForceInlineEarly]
int asint(int x)
{ return x; }

__generic<let N : int>
[__unsafeForceInlineEarly]
vector<int,N> asint(vector<int,N> x)
{ return x; }

__generic<let N : int, let M : int>
[__unsafeForceInlineEarly]
matrix<int,N,M> asint(matrix<int,N,M> x)
{ return x; }

// Reinterpret bits of double as a uint (HLSL SM 5.0)

__target_intrinsic(hlsl)
__target_intrinsic(glsl, "{ uvec2 v = unpackDouble2x32($0); $1 = v.x; $2 = v.y; }")
__glsl_extension(GL_ARB_gpu_shader5)
void asuint(double value, out uint lowbits, out uint highbits);

// Reinterpret bits as a uint (HLSL SM 4.0)

__target_intrinsic(hlsl)
__target_intrinsic(glsl, "floatBitsToUint")
__target_intrinsic(spirv_direct, "124 resultType resultId _0")
uint asuint(float x);

__target_intrinsic(hlsl)
__target_intrinsic(glsl, "uint($0)")
__target_intrinsic(spirv_direct, "124 resultType resultId _0")
uint asuint(int x);

__generic<let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl, "floatBitsToUint")
__target_intrinsic(spirv_direct, "124 resultType resultId _0")
vector<uint,N> asuint(vector<float,N> x)
{
    VECTOR_MAP_UNARY(uint, N, asuint, x);
}

__generic<let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl, "uvec$N0($0)")
__target_intrinsic(spirv_direct, "124 resultType resultId _0")
vector<uint, N> asuint(vector<int, N> x)
{
    VECTOR_MAP_UNARY(uint, N, asuint, x);
}

__generic<let N : int, let M : int>
__target_intrinsic(hlsl)
matrix<uint,N,M> asuint(matrix<float,N,M> x)
{
    MATRIX_MAP_UNARY(uint, N, M, asuint, x);
}

__generic<let N : int, let M : int>
__target_intrinsic(hlsl)
matrix<uint, N, M> asuint(matrix<int, N, M> x)
{
    MATRIX_MAP_UNARY(uint, N, M, asuint, x);
}

[__unsafeForceInlineEarly]
uint asuint(uint x)
{ return x; }

__generic<let N : int>
[__unsafeForceInlineEarly]
vector<uint,N> asuint(vector<uint,N> x)
{ return x; }

__generic<let N : int, let M : int>
[__unsafeForceInlineEarly]
matrix<uint,N,M> asuint(matrix<uint,N,M> x)
{ return x; }


// 16-bit bitcast ops (HLSL SM 6.2)
//
// TODO: We need to map these to GLSL/SPIR-V
// operations that don't require an intermediate
// conversion to fp32.

// Identity cases:

[__unsafeForceInlineEarly] float16_t asfloat16(float16_t value) { return value; }
[__unsafeForceInlineEarly] vector<float16_t,N> asfloat16<let N : int>(vector<float16_t,N> value) { return value; }
[__unsafeForceInlineEarly] matrix<float16_t,R,C> asfloat16<let R : int, let C : int>(matrix<float16_t,R,C> value) { return value; }

[__unsafeForceInlineEarly] int16_t asint16(int16_t value) { return value; }
[__unsafeForceInlineEarly] vector<int16_t,N> asint16<let N : int>(vector<int16_t,N> value) { return value; }
[__unsafeForceInlineEarly] matrix<int16_t,R,C> asint16<let R : int, let C : int>(matrix<int16_t,R,C> value) { return value; }

[__unsafeForceInlineEarly] uint16_t asuint16(uint16_t value) { return value; }
[__unsafeForceInlineEarly] vector<uint16_t,N> asuint16<let N : int>(vector<uint16_t,N> value) { return value; }
[__unsafeForceInlineEarly] matrix<uint16_t,R,C> asuint16<let R : int, let C : int>(matrix<uint16_t,R,C> value) { return value; }

// Signed<->unsigned cases:

[__unsafeForceInlineEarly] int16_t asint16(uint16_t value) { return value; }
[__unsafeForceInlineEarly] vector<int16_t,N> asint16<let N : int>(vector<uint16_t,N> value) { return value; }
[__unsafeForceInlineEarly] matrix<int16_t,R,C> asint16<let R : int, let C : int>(matrix<uint16_t,R,C> value) { return value; }

[__unsafeForceInlineEarly] uint16_t asuint16(int16_t value) { return value; }
[__unsafeForceInlineEarly] vector<uint16_t,N> asuint16<let N : int>(vector<int16_t,N> value) { return value; }
[__unsafeForceInlineEarly] matrix<uint16_t,R,C> asuint16<let R : int, let C : int>(matrix<int16_t,R,C> value) { return value; }

// Float->unsigned cases:

__target_intrinsic(hlsl)
__target_intrinsic(glsl, "uint16_t(packHalf2x16(vec2($0, 0.0)))")
__target_intrinsic(cuda, "__half_as_ushort")
uint16_t asuint16(float16_t value);

vector<uint16_t,N> asuint16<let N : int>(vector<float16_t,N> value)
{ VECTOR_MAP_UNARY(uint16_t, N, asuint16, value); }

matrix<uint16_t,R,C> asuint16<let R : int, let C : int>(matrix<float16_t,R,C> value)
{ MATRIX_MAP_UNARY(uint16_t, R, C, asuint16, value); }

// Unsigned->float cases:

__target_intrinsic(hlsl)
__target_intrinsic(glsl, "float16_t(unpackHalf2x16($0).x)")
__target_intrinsic(cuda, "__ushort_as_half")
float16_t asfloat16(uint16_t value);

vector<float16_t,N> asfloat16<let N : int>(vector<uint16_t,N> value)
{ VECTOR_MAP_UNARY(float16_t, N, asfloat16, value); }

matrix<float16_t,R,C> asfloat16<let R : int, let C : int>(matrix<uint16_t,R,C> value)
{ MATRIX_MAP_UNARY(float16_t, R, C, asfloat16, value); }

// Float<->signed cases:

__target_intrinsic(hlsl)
__target_intrinsic(cuda, "__half_as_short")
[__unsafeForceInlineEarly] int16_t asint16(float16_t value) { return asuint16(value); }
__target_intrinsic(hlsl) [__unsafeForceInlineEarly] vector<int16_t,N> asint16<let N : int>(vector<float16_t,N> value) { return asuint16(value); }
__target_intrinsic(hlsl) [__unsafeForceInlineEarly] matrix<int16_t,R,C> asint16<let R : int, let C : int>(matrix<float16_t,R,C> value) { return asuint16(value); }

__target_intrinsic(hlsl)
__target_intrinsic(cuda, "__short_as_half")
[__unsafeForceInlineEarly] float16_t asfloat16(int16_t value) { return asfloat16(asuint16(value)); }

__target_intrinsic(hlsl) [__unsafeForceInlineEarly] vector<float16_t,N> asfloat16<let N : int>(vector<int16_t,N> value) { return asfloat16(asuint16(value)); }
__target_intrinsic(hlsl) [__unsafeForceInlineEarly] matrix<float16_t,R,C> asfloat16<let R : int, let C : int>(matrix<int16_t,R,C> value) { return asfloat16(asuint16(value)); }

// Inverse tangent (HLSL SM 1.0)
__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(cuda, "$P_atan($0)")
__target_intrinsic(cpp, "$P_atan($0)")
__target_intrinsic(spirv_direct, "12 resultType resultId glsl450 18 _0")
T atan(T x);

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv_direct, "12 resultType resultId glsl450 18 _0")
vector<T, N> atan(vector<T, N> x)
{
    VECTOR_MAP_UNARY(T, N, atan, x);
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(hlsl)
matrix<T, N, M> atan(matrix<T, N, M> x)
{
    MATRIX_MAP_UNARY(T, N, M, atan, x);
}

__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(hlsl)
__target_intrinsic(glsl,"atan($0,$1)")
__target_intrinsic(cuda, "$P_atan2($0, $1)")
__target_intrinsic(cpp, "$P_atan2($0, $1)")
__target_intrinsic(spirv_direct, "12 resultType resultId glsl450 25 _0 _1")
T atan2(T y, T x);

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl,"atan($0,$1)")
__target_intrinsic(spirv_direct, "12 resultType resultId glsl450 25 _0 _1")
vector<T, N> atan2(vector<T, N> y, vector<T, N> x)
{
    VECTOR_MAP_BINARY(T, N, atan2, y, x);
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(hlsl)
matrix<T,N,M> atan2(matrix<T,N,M> y, matrix<T,N,M> x)
{
    MATRIX_MAP_BINARY(T, N, M, atan2, y, x);
}

// Ceiling (HLSL SM 1.0)
__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(cuda, "$P_ceil($0)")
__target_intrinsic(cpp, "$P_ceil($0)")
__target_intrinsic(spirv_direct, "12 resultType resultId glsl450 9 _0")
T ceil(T x);

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv_direct, "12 resultType resultId glsl450 9 _0")
vector<T, N> ceil(vector<T, N> x)
{
    VECTOR_MAP_UNARY(T, N, ceil, x);
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(hlsl)
matrix<T, N, M> ceil(matrix<T, N, M> x)
{
    MATRIX_MAP_UNARY(T, N, M, ceil, x);
}


// Check access status to tiled resource
bool CheckAccessFullyMapped(uint status);

// Clamp (HLSL SM 1.0)
__generic<T : __BuiltinArithmeticType>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv_direct, "12 resultType resultId glsl450 fus(43,44,45) _0 _1 _2")
T clamp(T x, T minBound, T maxBound)
{
    return min(max(x, minBound), maxBound);
}

__generic<T : __BuiltinArithmeticType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv_direct, "12 resultType resultId glsl450 fus(43,44,45) _0 _1 _2")
vector<T, N> clamp(vector<T, N> x, vector<T, N> minBound, vector<T, N> maxBound)
{
    return min(max(x, minBound), maxBound);
}

__generic<T : __BuiltinArithmeticType, let N : int, let M : int>
__target_intrinsic(hlsl)
matrix<T,N,M> clamp(matrix<T,N,M> x, matrix<T,N,M> minBound, matrix<T,N,M> maxBound)
{
    return min(max(x, minBound), maxBound);
}

// Clip (discard) fragment conditionally
__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(hlsl)
void clip(T x)
{
    if(x < T(0)) discard;
}

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
void clip(vector<T,N> x)
{
    if(any(x < T(0))) discard;
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(hlsl)
void clip(matrix<T,N,M> x)
{
    if(any(x < T(0))) discard;
}

// Cosine
__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(cuda, "$P_cos($0)")
__target_intrinsic(cpp, "$P_cos($0)")
__target_intrinsic(spirv_direct, "12 resultType resultId glsl450 14 _0")
T cos(T x);

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv_direct, "12 resultType resultId glsl450 14 _0")
vector<T, N> cos(vector<T, N> x)
{
    VECTOR_MAP_UNARY(T,N, cos, x);
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(hlsl)
matrix<T, N, M> cos(matrix<T, N, M> x)
{
    MATRIX_MAP_UNARY(T, N, M, cos, x);
}

// Hyperbolic cosine
__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(cuda, "$P_cosh($0)")
__target_intrinsic(cpp, "$P_cosh($0)")
__target_intrinsic(spirv_direct, "12 resultType resultId glsl450 20 _0")
T cosh(T x);

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv_direct, "12 resultType resultId glsl450 20 _0")
vector<T,N> cosh(vector<T,N> x)
{
    VECTOR_MAP_UNARY(T,N, cosh, x);
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(hlsl)
matrix<T, N, M> cosh(matrix<T, N, M> x)
{
    MATRIX_MAP_UNARY(T, N, M, cosh, x);
}

// Population count
__target_intrinsic(hlsl)
__target_intrinsic(glsl, "bitCount")
__target_intrinsic(cuda, "$P_countbits($0)")
__target_intrinsic(cpp, "$P_countbits($0)")
uint countbits(uint value);

// Cross product
// TODO: SPIRV does not support integer vectors.
__generic<T : __BuiltinArithmeticType>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv_direct, "12 resultType resultId glsl450 68 _0 _1")
vector<T,3> cross(vector<T,3> left, vector<T,3> right)
{
    return vector<T,3>(
        left.y * right.z - left.z * right.y,
        left.z * right.x - left.x * right.z,
        left.x * right.y - left.y * right.x);
}

// Convert encoded color
__target_intrinsic(hlsl)
int4 D3DCOLORtoUBYTE4(float4 color)
{
    let scaled = color.zyxw * 255.001999f;
    return int4(scaled);
}

// Partial-difference derivatives
__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(glsl, dFdx)
T ddx(T x);

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl, dFdx)
vector<T, N> ddx(vector<T, N> x)
{
    VECTOR_MAP_UNARY(T, N, ddx, x);
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(hlsl)
matrix<T, N, M> ddx(matrix<T, N, M> x)
{
    MATRIX_MAP_UNARY(T, N, M, ddx, x);
}

__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(hlsl)
__glsl_extension(GL_ARB_derivative_control)
__target_intrinsic(glsl, dFdxCoarse)
T ddx_coarse(T x);

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__glsl_extension(GL_ARB_derivative_control)
__target_intrinsic(glsl, dFdxCoarse)
vector<T, N> ddx_coarse(vector<T, N> x)
{
    VECTOR_MAP_UNARY(T, N, ddx_coarse, x);
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(hlsl)
matrix<T, N, M> ddx_coarse(matrix<T, N, M> x)
{
    MATRIX_MAP_UNARY(T, N, M, ddx_coarse, x);
}

__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(hlsl)
__glsl_extension(GL_ARB_derivative_control)
__target_intrinsic(glsl, dFdxFine)
T ddx_fine(T x);

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__glsl_extension(GL_ARB_derivative_control)
__target_intrinsic(glsl, dFdxFine)
vector<T, N> ddx_fine(vector<T, N> x)
{
    VECTOR_MAP_UNARY(T, N, ddx_fine, x);
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(hlsl)
matrix<T, N, M> ddx_fine(matrix<T, N, M> x)
{
    MATRIX_MAP_UNARY(T, N, M, ddx_fine, x);
}

__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(hlsl)
__target_intrinsic(glsl, dFdy)
T ddy(T x);

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl, dFdy)
vector<T, N> ddy(vector<T, N> x)
{
    VECTOR_MAP_UNARY(T, N, ddy, x);
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(hlsl)
matrix<T, N, M> ddy(matrix<T, N, M> x)
{
    MATRIX_MAP_UNARY(T, N, M, ddy, x);
}

__generic<T : __BuiltinFloatingPointType>
__glsl_extension(GL_ARB_derivative_control)
__target_intrinsic(glsl, dFdyCoarse)
T ddy_coarse(T x);

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__glsl_extension(GL_ARB_derivative_control)
__target_intrinsic(glsl, dFdyCoarse)
vector<T, N> ddy_coarse(vector<T, N> x)
{
    VECTOR_MAP_UNARY(T, N, ddy_coarse, x);
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(hlsl)
matrix<T, N, M> ddy_coarse(matrix<T, N, M> x)
{
    MATRIX_MAP_UNARY(T, N, M, ddy_coarse, x);
}

__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(hlsl)
__glsl_extension(GL_ARB_derivative_control)
__target_intrinsic(glsl, dFdyFine)
T ddy_fine(T x);

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__glsl_extension(GL_ARB_derivative_control)
__target_intrinsic(glsl, dFdyFine)
vector<T, N> ddy_fine(vector<T, N> x)
{
    VECTOR_MAP_UNARY(T, N, ddy_fine, x);
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(hlsl)
matrix<T, N, M> ddy_fine(matrix<T, N, M> x)
{
    MATRIX_MAP_UNARY(T, N, M, ddy_fine, x);
}


// Radians to degrees

__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv_direct, "12 resultType resultId glsl450 12 _0")
T degrees(T x)
{
    return x * (T(180) / T.getPi());
}

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv_direct, "12 resultType resultId glsl450 12 _0")
vector<T, N> degrees(vector<T, N> x)
{
    VECTOR_MAP_UNARY(T, N, degrees, x);
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(hlsl)
matrix<T, N, M> degrees(matrix<T, N, M> x)
{
    MATRIX_MAP_UNARY(T, N, M, degrees, x);
}

// Matrix determinant

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv_direct, "12 resultType resultId glsl450 33 _0")
T determinant(matrix<T,N,N> m);

// Barrier for device memory
__target_intrinsic(glsl, "memoryBarrier(), memoryBarrierImage(), memoryBarrierBuffer()")
__target_intrinsic(cuda, "__threadfence()")
void DeviceMemoryBarrier();

__target_intrinsic(glsl, "memoryBarrier(), memoryBarrierImage(), memoryBarrierBuffer(), barrier()")
__target_intrinsic(glsl, "__syncthreads()")
void DeviceMemoryBarrierWithGroupSync();

// Vector distance

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv_direct, "12 resultType resultId glsl450 67 _0 _1")
T distance(vector<T, N> x, vector<T, N> y)
{
    return length(x - y);
}

// Vector dot product

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
T dot(vector<T, N> x, vector<T, N> y)
{
    T result = T(0);
    for(int i = 0; i < N; ++i)
        result += x[i] * y[i];
    return result;
}

__generic<T : __BuiltinIntegerType, let N : int>
__target_intrinsic(hlsl)
T dot(vector<T, N> x, vector<T, N> y)
{
    T result = T(0);
    for(int i = 0; i < N; ++i)
        result += x[i] * y[i];
    return result;
}


// Helper for computing distance terms for lighting (obsolete)

__generic<T : __BuiltinFloatingPointType> vector<T,4> dst(vector<T,4> x, vector<T,4> y);

// Given a RWByteAddressBuffer allow it to be interpretted as a RWStructuredBuffer
__intrinsic_op($(kIROp_GetEquivalentStructuredBuffer))
RWStructuredBuffer<T> __getEquivalentStructuredBuffer<T>(RWByteAddressBuffer b);

// Error message

// void errorf( string format, ... );

// Attribute evaluation

// TODO: The matrix cases of these functions won't actuall work
// when compiled to GLSL, since they only support scalar/vector

// TODO: Should these be constrains to `__BuiltinFloatingPointType`?
// TODO: SPIRV-direct does not support non-floating-point types.

__generic<T : __BuiltinArithmeticType>
__target_intrinsic(glsl, interpolateAtCentroid)
__target_intrinsic(spirv_direct, "12 resultType resultId glsl450 76 _0")
T EvaluateAttributeAtCentroid(T x);

__generic<T : __BuiltinArithmeticType, let N : int>
__target_intrinsic(glsl, interpolateAtCentroid)
__target_intrinsic(spirv_direct, "12 resultType resultId glsl450 76 _0")
vector<T,N> EvaluateAttributeAtCentroid(vector<T,N> x);

__generic<T : __BuiltinArithmeticType, let N : int, let M : int>
__target_intrinsic(glsl, interpolateAtCentroid)
matrix<T,N,M> EvaluateAttributeAtCentroid(matrix<T,N,M> x)
{
    MATRIX_MAP_UNARY(T, N, M, EvaluateAttributeAtCentroid, x);
}

__generic<T : __BuiltinArithmeticType>
__target_intrinsic(glsl, "interpolateAtSample($0, int($1))")
__target_intrinsic(spirv_direct, "12 resultType resultId glsl450 77 _0 _1")
T EvaluateAttributeAtSample(T x, uint sampleindex);

__generic<T : __BuiltinArithmeticType, let N : int>
__target_intrinsic(glsl, "interpolateAtSample($0, int($1))")
__target_intrinsic(spirv_direct, "12 resultType resultId glsl450 77 _0 _1")
vector<T,N> EvaluateAttributeAtSample(vector<T,N> x, uint sampleindex);

__generic<T : __BuiltinArithmeticType, let N : int, let M : int>
__target_intrinsic(glsl, "interpolateAtSample($0, int($1))")
matrix<T,N,M> EvaluateAttributeAtSample(matrix<T,N,M> x, uint sampleindex)
{
    matrix<T,N,M> result;
    for(int i = 0; i < N; ++i)
    {
        result[i] = EvaluateAttributeAtSample(x[i], sampleindex);
    }
    return result;
}

__generic<T : __BuiltinArithmeticType>
__target_intrinsic(glsl, "interpolateAtOffset($0, vec2($1) / 16.0f)")
__target_intrinsic(spirv_direct, "%foffset = 111 _type(float2) resultId _1; %offsetdiv16 = 136 _type(float2) resultId %foffset const(float2, 16.0, 16.0); 12 resultType resultId glsl450 78 _0 %offsetdiv16")
T EvaluateAttributeSnapped(T x, int2 offset);

__generic<T : __BuiltinArithmeticType, let N : int>
__target_intrinsic(glsl, "interpolateAtOffset($0, vec2($1) / 16.0f)")
__target_intrinsic(spirv_direct, "%foffset = 111 _type(float2) resultId _1; %offsetdiv16 = 136 _type(float2) resultId %foffset const(float2, 16.0, 16.0); 12 resultType resultId glsl450 78 _0 %offsetdiv16")
vector<T,N> EvaluateAttributeSnapped(vector<T,N> x, int2 offset);

__generic<T : __BuiltinArithmeticType, let N : int, let M : int>
__target_intrinsic(glsl, "interpolateAtOffset($0, vec2($1) / 16.0f)")
matrix<T,N,M> EvaluateAttributeSnapped(matrix<T,N,M> x, int2 offset)
{
    matrix<T,N,M> result;
    for(int i = 0; i < N; ++i)
    {
        result[i] = EvaluateAttributeSnapped(x[i], offset);
    }
    return result;
}

// Base-e exponent

__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(cuda, "$P_exp($0)")
__target_intrinsic(cpp, "$P_exp($0)")
__target_intrinsic(spirv_direct, "12 resultType resultId glsl450 27 _0")
T exp(T x);

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv_direct, "12 resultType resultId glsl450 27 _0")
vector<T, N> exp(vector<T, N> x)
{
    VECTOR_MAP_UNARY(T, N, exp, x);
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(hlsl)
matrix<T, N, M> exp(matrix<T, N, M> x)
{
    MATRIX_MAP_UNARY(T, N, M, exp, x);
}

// Base-2 exponent

__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(cuda, "$P_exp2($0)")
__target_intrinsic(cpp, "$P_exp2($0)")
__target_intrinsic(spirv_direct, "12 resultType resultId glsl450 29 _0")
T exp2(T x);

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv_direct, "12 resultType resultId glsl450 29 _0")
vector<T,N> exp2(vector<T,N> x)
{
    VECTOR_MAP_UNARY(T, N, exp2, x);
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(hlsl)
matrix<T,N,M> exp2(matrix<T,N,M> x)
{
    MATRIX_MAP_UNARY(T, N, M, exp2, x);
}


// Convert 16-bit float stored in low bits of integer
__target_intrinsic(glsl, "unpackHalf2x16($0).x")
__glsl_version(420)
__target_intrinsic(hlsl)
float f16tof32(uint value);

__generic<let N : int>
__target_intrinsic(hlsl)
vector<float, N> f16tof32(vector<uint, N> value)
{
    VECTOR_MAP_UNARY(float, N, f16tof32, value);
}



// Convert to 16-bit float stored in low bits of integer
__target_intrinsic(glsl, "packHalf2x16(vec2($0,0.0))")
__glsl_version(420)
__target_intrinsic(hlsl)
uint f32tof16(float value);

__generic<let N : int>
__target_intrinsic(hlsl)
vector<uint, N> f32tof16(vector<float, N> value)
{
    VECTOR_MAP_UNARY(uint, N, f32tof16, value);
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// The following is Slang specific and NOT part of standard HLSL
// It's not clear what happens with float16 time in HLSL -> can the float16 coerce to uint for example? If so that would
// give the wrong result

__target_intrinsic(glsl, "unpackHalf2x16($0).x")
__target_intrinsic(cuda, "__half2float")
__glsl_version(420)
float f16tof32(float16_t value);

__generic<let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(cuda, "__half2float")
vector<float, N> f16tof32(vector<float16_t, N> value)
{
    VECTOR_MAP_UNARY(float, N, f16tof32, value);
}

// Convert to float16_t
__target_intrinsic(glsl, "packHalf2x16(vec2($0,0.0))")
__glsl_version(420)
__target_intrinsic(cuda, "__float2half")
float16_t f32tof16_(float value);

__generic<let N : int>
__target_intrinsic(cuda, "__float2half")
vector<float16_t, N> f32tof16_(vector<float, N> value)
{
    VECTOR_MAP_UNARY(uint, N, f32tof16, value);
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

// Flip surface normal to face forward, if needed
__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
vector<T,N> faceforward(vector<T,N> n, vector<T,N> i, vector<T,N> ng)
{
    return dot(ng, i) < T(0.0f) ? n : -n;
}

// Find first set bit starting at high bit and working down
__target_intrinsic(hlsl)
__target_intrinsic(glsl,"findMSB")
__target_intrinsic(cuda, "$P_firstbithigh($0)")
__target_intrinsic(cpp, "$P_firstbithigh($0)")
__target_intrinsic(spirv_direct, "12 resultType resultId glsl450 74 _0")
int firstbithigh(int value);

__target_intrinsic(hlsl)
__target_intrinsic(glsl,"findMSB")
__target_intrinsic(spirv_direct, "12 resultType resultId glsl450 74 _0")
__generic<let N : int>
vector<int, N> firstbithigh(vector<int, N> value)
{
    VECTOR_MAP_UNARY(int, N, firstbithigh, value);
}

__target_intrinsic(hlsl)
__target_intrinsic(glsl,"findMSB")
__target_intrinsic(cuda, "$P_firstbithigh($0)")
__target_intrinsic(cpp, "$P_firstbithigh($0)")
__target_intrinsic(spirv_direct, "12 resultType resultId glsl450 75 _0")
uint firstbithigh(uint value);

__target_intrinsic(hlsl)
__target_intrinsic(glsl,"findMSB")
__target_intrinsic(spirv_direct, "12 resultType resultId glsl450 75 _0")
__generic<let N : int>
vector<uint,N> firstbithigh(vector<uint,N> value)
{
    VECTOR_MAP_UNARY(uint, N, firstbithigh, value);
}

// Find first set bit starting at low bit and working up
__target_intrinsic(hlsl)
__target_intrinsic(glsl,"findLSB")
__target_intrinsic(cuda, "$P_firstbitlow($0)")
__target_intrinsic(cpp, "$P_firstbitlow($0)")
__target_intrinsic(spirv_direct, "12 resultType resultId glsl450 73 _0")
int firstbitlow(int value);

__target_intrinsic(hlsl)
__target_intrinsic(glsl,"findLSB")
__target_intrinsic(spirv_direct, "12 resultType resultId glsl450 73 _0")
__generic<let N : int>
vector<int,N> firstbitlow(vector<int,N> value)
{
    VECTOR_MAP_UNARY(int, N, firstbitlow, value);
}

__target_intrinsic(hlsl)
__target_intrinsic(glsl,"findLSB")
__target_intrinsic(cuda, "$P_firstbitlow($0)")
__target_intrinsic(cpp, "$P_firstbitlow($0)")
__target_intrinsic(spirv_direct, "12 resultType resultId glsl450 73 _0")
uint firstbitlow(uint value);

__target_intrinsic(hlsl)
__target_intrinsic(glsl,"findLSB")
__generic<let N : int>
__target_intrinsic(spirv_direct, "12 resultType resultId glsl450 73 _0")
vector<uint,N> firstbitlow(vector<uint,N> value)
{
    VECTOR_MAP_UNARY(uint, N, firstbitlow, value);
}

// Floor (HLSL SM 1.0)

__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(cuda, "$P_floor($0)")
__target_intrinsic(cpp, "$P_floor($0)")
__target_intrinsic(spirv_direct, "12 resultType resultId glsl450 8 _0")
T floor(T x);

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv_direct, "12 resultType resultId glsl450 8 _0")
vector<T, N> floor(vector<T, N> x)
{
    VECTOR_MAP_UNARY(T, N, floor, x);
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(hlsl)
matrix<T, N, M> floor(matrix<T, N, M> x)
{
    MATRIX_MAP_UNARY(T, N, M, floor, x);
}

// Fused multiply-add for doubles
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(cuda, "$P_fma($0, $1, $2)")
__target_intrinsic(cpp, "$P_fma($0, $1, $2)")
__target_intrinsic(spirv_direct, "12 resultType resultId glsl450 50 _0 _1 _2")
double fma(double a, double b, double c);

__generic<let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv_direct, "12 resultType resultId glsl450 50 _0 _1 _2")
vector<double, N> fma(vector<double, N> a, vector<double, N> b, vector<double, N> c)
{
    VECTOR_MAP_TRINARY(double, N, fma, a, b, c);
}

__generic<let N : int, let M : int>
__target_intrinsic(hlsl)
matrix<double, N, M> fma(matrix<double, N, M> a, matrix<double, N, M> b, matrix<double, N, M> c)
{
    MATRIX_MAP_TRINARY(double, N, M, fma, a, b, c);
}

// Floating point remainder of x/y
__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(hlsl)
__target_intrinsic(cuda, "$P_fmod($0, $1)")
__target_intrinsic(cpp, "$P_fmod($0, $1)")
T fmod(T x, T y)
{
    return x - y * trunc(x/y);
}

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
vector<T, N> fmod(vector<T, N> x, vector<T, N> y)
{
    VECTOR_MAP_BINARY(T, N, fmod, x, y);
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(hlsl)
matrix<T, N, M> fmod(matrix<T, N, M> x, matrix<T, N, M> y)
{
    MATRIX_MAP_BINARY(T, N, M, fmod, x, y);
}

// Fractional part
__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(hlsl)
__target_intrinsic(glsl, fract)
__target_intrinsic(cuda, "$P_frac($0)")
__target_intrinsic(cpp, "$P_frac($0)")
__target_intrinsic(spirv_direct, "12 resultType resultId glsl450 10 _0")
T frac(T x);

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl, fract)
__target_intrinsic(spirv_direct, "12 resultType resultId glsl450 10 _0")
vector<T, N> frac(vector<T, N> x)
{
    VECTOR_MAP_UNARY(T, N, frac, x);
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
matrix<T, N, M> frac(matrix<T, N, M> x)
{
    MATRIX_MAP_UNARY(T, N, M, frac, x);
}

// Split float into mantissa and exponent
__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv_direct, "12 resultType resultId glsl450 51 _0 _1")
T frexp(T x, out T exp);

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv_direct, "12 resultType resultId glsl450 51 _0 _1")
vector<T, N> frexp(vector<T, N> x, out vector<T, N> exp)
{
    VECTOR_MAP_BINARY(T, N, frexp, x, exp);
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(hlsl)
matrix<T, N, M> frexp(matrix<T, N, M> x, out matrix<T, N, M> exp)
{
    MATRIX_MAP_BINARY(T, N, M, frexp, x, exp);
}

// Texture filter width
__generic<T : __BuiltinFloatingPointType>
T fwidth(T x);

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
vector<T, N> fwidth(vector<T, N> x)
{
    VECTOR_MAP_UNARY(T, N, fwidth, x);
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(hlsl)
matrix<T, N, M> fwidth(matrix<T, N, M> x)
{
    MATRIX_MAP_UNARY(T, N, M, fwidth, x);
}

/// Get the value of a vertex attribute at a specific vertex.
///
/// The `GetAttributeAtVertex()` function can be used in a fragment shader
/// to get the value of the given `attribute` at the vertex of the primitive
/// that corresponds to the given `vertexIndex`.
///
/// Note that the `attribute` must have been a declared varying input to
/// the fragment shader with the `nointerpolation` modifier.
///
/// This function can be applied to scalars, vectors, and matrices of
/// built-in scalar types.
///
/// Note: these functions are not curently implemented for Vulkan/SPIR-V output.
///
__generic<T : __BuiltinType>
[__readNone]
__target_intrinsic(hlsl)
__target_intrinsic(glsl, "$0[$1]")
__glsl_version(450)
__glsl_extension(GL_NV_fragment_shader_barycentric)
T GetAttributeAtVertex(T attribute, uint vertexIndex);

/// Get the value of a vertex attribute at a specific vertex.
///
/// The `GetAttributeAtVertex()` function can be used in a fragment shader
/// to get the value of the given `attribute` at the vertex of the primitive
/// that corresponds to the given `vertexIndex`.
///
/// Note that the `attribute` must have been a declared varying input to
/// the fragment shader with the `nointerpolation` modifier.
///
/// This function can be applied to scalars, vectors, and matrices of
/// built-in scalar types.
///
/// Note: these functions are not curently implemented for Vulkan/SPIR-V output.
///
__generic<T : __BuiltinType, let N : int>
[__readNone]
__target_intrinsic(hlsl)
__target_intrinsic(glsl, "$0[$1]")
__glsl_version(450)
__glsl_extension(GL_NV_fragment_shader_barycentric)
vector<T,N> GetAttributeAtVertex(vector<T,N> attribute, uint vertexIndex);

/// Get the value of a vertex attribute at a specific vertex.
///
/// The `GetAttributeAtVertex()` function can be used in a fragment shader
/// to get the value of the given `attribute` at the vertex of the primitive
/// that corresponds to the given `vertexIndex`.
///
/// Note that the `attribute` must have been a declared varying input to
/// the fragment shader with the `nointerpolation` modifier.
///
/// This function can be applied to scalars, vectors, and matrices of
/// built-in scalar types.
///
/// Note: these functions are not curently implemented for Vulkan/SPIR-V output.
///
__generic<T : __BuiltinType, let N : int, let M : int>
[__readNone]
__target_intrinsic(hlsl)
__target_intrinsic(glsl, "$0[$1]")
__glsl_version(450)
__glsl_extension(GL_NV_fragment_shader_barycentric)
matrix<T,N,M> GetAttributeAtVertex(matrix<T,N,M> attribute, uint vertexIndex);


// Get number of samples in render target
uint GetRenderTargetSampleCount();

// Get position of given sample
float2 GetRenderTargetSamplePosition(int Index);

// Group memory barrier
__target_intrinsic(glsl, "groupMemoryBarrier")
__target_intrinsic(cuda, "__threadfence_block")
void GroupMemoryBarrier();


__target_intrinsic(glsl, "groupMemoryBarrier(), barrier()")
__target_intrinsic(cuda, "__syncthreads()")
void GroupMemoryBarrierWithGroupSync();

// Atomics

__target_intrinsic(glsl, "$atomicAdd($A, $1)")
__target_intrinsic(cuda, "atomicAdd($0, $1)")
void InterlockedAdd(__ref  int dest,  int value);

__target_intrinsic(glsl, "$atomicAdd($A, $1)")
__target_intrinsic(cuda, "atomicAdd((uint*)$0, $1)")
void InterlockedAdd(__ref uint dest, uint value);

__target_intrinsic(glsl, "($2 = $atomicAdd($A, $1))")
__target_intrinsic(cuda, "(*$2 = atomicAdd($0, $1))")
void InterlockedAdd(__ref  int dest,  int value, out  int original_value);

__target_intrinsic(glsl, "($2 = $atomicAdd($A, $1))")
__target_intrinsic(cuda, "(*$2 = (uint)atomicAdd((uint*)$0, $1))")
void InterlockedAdd(__ref uint dest, uint value, out uint original_value);

__target_intrinsic(glsl, "$atomicAnd($A, $1)")
__target_intrinsic(cuda, "atomicAnd($0, $1)")
void InterlockedAnd(__ref  int dest,  int value);

__target_intrinsic(glsl, "$atomicAnd($A, $1)")
__target_intrinsic(cuda, "atomicAnd((int*)$0, $1)")
void InterlockedAnd(__ref uint dest, uint value);

__target_intrinsic(glsl, "($2 = $atomicAnd($A, $1))")
__target_intrinsic(cuda, "(*$2 = atomicAnd($0, $1))")
void InterlockedAnd(__ref  int dest,  int value, out  int original_value);

__target_intrinsic(glsl, "($2 = $atomicAnd($A, $1))")
__target_intrinsic(cuda, "(*$2 = atomicAnd((int*)$0, $1))")
void InterlockedAnd(__ref uint dest, uint value, out uint original_value);

__target_intrinsic(glsl, "($3 = $atomicCompSwap($A, $1, $2))")
__target_intrinsic(cuda, "(*$3 = atomicCAS($0, $1, $2))")
void InterlockedCompareExchange(__ref  int dest,  int compare_value,  int value, out  int original_value);

__target_intrinsic(glsl, "($3 = $atomicCompSwap($A, $1, $2))")
__target_intrinsic(cuda, "(*$3 = (uint)atomicCAS((int*)$0, $1, $2))")
void InterlockedCompareExchange(__ref uint dest, uint compare_value, uint value, out uint original_value);

__target_intrinsic(glsl, "$atomicCompSwap($A, $1, $2)")
__target_intrinsic(cuda, "atomicCAS($0, $1, $2)")
void InterlockedCompareStore(__ref int dest,  int compare_value,  int value);

__target_intrinsic(glsl, "$atomicCompSwap($A, $1, $2)")
__target_intrinsic(cuda, "atomicCAS((int*)$0, $1, $2)")
void InterlockedCompareStore(__ref uint dest, uint compare_value, uint value);

__target_intrinsic(glsl, "($2 = $atomicExchange($A, $1))")
__target_intrinsic(cuda, "(*$2 = atomicExch($0, $1))")
void InterlockedExchange(__ref  int dest,  int value, out  int original_value);

__target_intrinsic(glsl, "($2 = $atomicExchange($A, $1))")
__target_intrinsic(cuda, "(*$2 = (uint)atomicExch((int*)$0, $1))")
void InterlockedExchange(__ref uint dest, uint value, out uint original_value);

__target_intrinsic(glsl, "$atomicMax($A, $1)")
__target_intrinsic(cuda, "atomicMax($0, $1)")
void InterlockedMax(__ref  int dest,  int value);

__target_intrinsic(glsl, "$atomicMax($A, $1)")
__target_intrinsic(cuda, "atomicMax((int*)$0, $1)")
void InterlockedMax(__ref uint dest, uint value);

__target_intrinsic(glsl, "($2 = $atomicMax($A, $1))")
__target_intrinsic(cuda, "(*$2 = atomicMax($0, $1))")
void InterlockedMax(__ref  int dest,  int value, out  int original_value);

__target_intrinsic(glsl, "($2 = $atomicMax($A, $1))")
__target_intrinsic(cuda, "(*$2 = (uint)atomicMax((int*)$0, $1))")
void InterlockedMax(__ref uint dest, uint value, out uint original_value);

__target_intrinsic(glsl, "$atomicMin($A, $1)")
__target_intrinsic(cuda, "atomicMin($0, $1)")
void InterlockedMin(__ref  int dest,  int value);

__target_intrinsic(glsl, "$atomicMin($A, $1)")
__target_intrinsic(cuda, "atomicMin((int*)$0, $1)")
void InterlockedMin(__ref uint dest, uint value);

__target_intrinsic(glsl, "($2 = $atomicMin($A, $1))")
__target_intrinsic(cuda, "(*$2 = atomicMin($0, $1))")
void InterlockedMin(__ref  int dest,  int value, out  int original_value);

__target_intrinsic(glsl, "($2 = $atomicMin($A, $1))")
__target_intrinsic(cuda, "(*$2 = (uint)atomicMin((int*)$0, $1))")
void InterlockedMin(__ref uint dest, uint value, out uint original_value);

__target_intrinsic(glsl, "$atomicOr($A, $1)")
__target_intrinsic(cuda, "atomicOr($0, $1)")
void InterlockedOr(__ref  int dest,  int value);

__target_intrinsic(glsl, "$atomicOr($A, $1)")
__target_intrinsic(cuda, "atomicOr((int*)$0, $1)")
void InterlockedOr(__ref uint dest, uint value);

__target_intrinsic(glsl, "($2 = $atomicOr($A, $1))")
__target_intrinsic(cuda, "(*$2 = atomicOr($0, $1))")
void InterlockedOr(__ref  int dest,  int value, out  int original_value);

__target_intrinsic(glsl, "($2 = $atomicOr($A, $1))")
__target_intrinsic(cuda, "(*$2 = (uint)atomicOr((int*)$0, $1))")
void InterlockedOr(__ref uint dest, uint value, out uint original_value);

__target_intrinsic(glsl, "$atomicXor($A, $1)")
__target_intrinsic(cuda, "atomicXor($0, $1)")
void InterlockedXor(__ref  int dest,  int value);

__target_intrinsic(glsl, "$atomicXor($A, $1)")
__target_intrinsic(cuda, "atomicXor((int*)$0, $1)")
void InterlockedXor(__ref uint dest, uint value);

__target_intrinsic(glsl, "($2 = $atomicXor($A, $1))")
__target_intrinsic(cuda, "(*$2 = atomicXor($0, $1))")
void InterlockedXor(__ref  int dest,  int value, out  int original_value);

__target_intrinsic(glsl, "($2 = $atomicXor($A, $1))")
__target_intrinsic(cuda, "(*$2 = (uint)atomicXor((int*)$0, $1))")
void InterlockedXor(__ref uint dest, uint value, out uint original_value);

// Is floating-point value finite?

__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(hlsl)
__target_intrinsic(cuda, "$P_isfinite($0)")
__target_intrinsic(cpp, "$P_isfinite($0)")
bool isfinite(T x)
{
    return !(isinf(x) || isnan(x));
}

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
vector<bool, N> isfinite(vector<T, N> x)
{
    VECTOR_MAP_UNARY(bool, N, isfinite, x);
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(hlsl)
matrix<bool, N, M> isfinite(matrix<T, N, M> x)
{
    MATRIX_MAP_UNARY(bool, N, M, isfinite, x);
}

// Is floating-point value infinite?
__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(cuda, "$P_isinf($0)")
__target_intrinsic(cpp, "$P_isinf($0)")
bool isinf(T x);

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
vector<bool, N> isinf(vector<T, N> x)
{
    VECTOR_MAP_UNARY(bool, N, isinf, x);
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(hlsl)
matrix<bool, N, M> isinf(matrix<T, N, M> x)
{
    MATRIX_MAP_UNARY(bool, N, M, isinf, x);
}

// Is floating-point value not-a-number?
__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(cuda, "$P_isnan($0)")
__target_intrinsic(cpp, "$P_isnan($0)")
bool isnan(T x);

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
vector<bool, N> isnan(vector<T, N> x)
{
    VECTOR_MAP_UNARY(bool, N, isnan, x);
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(hlsl)
matrix<bool, N, M> isnan(matrix<T, N, M> x)
{
    MATRIX_MAP_UNARY(bool, N, M, isnan, x);
}

// Construct float from mantissa and exponent

__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(hlsl)
__target_intrinsic(spirv_direct, "12 resultType resultId glsl450 53 _0 _1")
T ldexp(T x, T exp)
{
    return x * exp2(exp);
}

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(spirv_direct, "12 resultType resultId glsl450 53 _0 _1")
vector<T, N> ldexp(vector<T, N> x, vector<T, N> exp)
{
    return x * exp2(exp);
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(hlsl)
matrix<T, N, M> ldexp(matrix<T, N, M> x, matrix<T, N, M> exp)
{
    MATRIX_MAP_BINARY(T, N, M, ldexp, x, exp);
}

// Vector length
__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv_direct, "12 resultType resultId glsl450 66 _0")
T length(vector<T, N> x)
{
    return sqrt(dot(x, x));
}

// Linear interpolation
__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(hlsl)
__target_intrinsic(glsl, mix)
__target_intrinsic(spirv_direct, "12 resultType resultId glsl450 46 _0 _1 _2")
T lerp(T x, T y, T s)
{
    return x * (T(1.0f) - s) + y * s;
}

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl, mix)
__target_intrinsic(spirv_direct, "12 resultType resultId glsl450 46 _0 _1 _2")
vector<T, N> lerp(vector<T, N> x, vector<T, N> y, vector<T, N> s)
{
    return x * (T(1.0f) - s) + y * s;
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(hlsl)
matrix<T,N,M> lerp(matrix<T,N,M> x, matrix<T,N,M> y, matrix<T,N,M> s)
{
    MATRIX_MAP_TRINARY(T, N, M, lerp, x, y, s);
}

// Legacy lighting function (obsolete)
__target_intrinsic(hlsl)
float4 lit(float n_dot_l, float n_dot_h, float m)
{
    let ambient = 1.0f;
    let diffuse = max(n_dot_l, 0.0f);
    let specular = step(0.0f, n_dot_l) * max(n_dot_h * m, 0.0f);
    return float4(ambient, diffuse, specular, 1.0f);
}

// Base-e logarithm
__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(cuda, "$P_log($0)")
__target_intrinsic(cpp, "$P_log($0)")
__target_intrinsic(spirv_direct, "12 resultType resultId glsl450 28 _0")
T log(T x);

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv_direct, "12 resultType resultId glsl450 28 _0")
vector<T, N> log(vector<T, N> x)
{
    VECTOR_MAP_UNARY(T, N, log, x);
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(hlsl)
matrix<T, N, M> log(matrix<T, N, M> x)
{
    MATRIX_MAP_UNARY(T, N, M, log, x);
}

// Base-10 logarithm
__generic<T : __BuiltinFloatingPointType> 
__target_intrinsic(hlsl)
__target_intrinsic(glsl, "(log( $0 ) * $S0( 0.43429448190325182765112891891661) )" )
__target_intrinsic(cuda, "$P_log10($0)")
__target_intrinsic(cpp, "$P_log10($0)")
__target_intrinsic(spirv_direct, "%baseElog = 12 resultType resultId glsl450 28 _0; 133 resultType resultId _0 %baseElog const(_p,0.43429448190325182765112891891661)")
T log10(T x);

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl, "(log( $0 ) * $S0(0.43429448190325182765112891891661) )" )
__target_intrinsic(spirv_direct, "%baseElog = 12 resultType resultId glsl450 28 _0; 142 resultType resultId _0 %baseElog const(_p,0.43429448190325182765112891891661)")
vector<T,N> log10(vector<T,N> x)
{
    VECTOR_MAP_UNARY(T, N, log10, x);
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int> 
__target_intrinsic(hlsl)
matrix<T,N,M> log10(matrix<T,N,M> x)
{
    MATRIX_MAP_UNARY(T, N, M, log10, x);
}

// Base-2 logarithm
__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(cuda, "$P_log2($0)")
__target_intrinsic(cpp, "$P_log2($0)")
__target_intrinsic(spirv_direct, "12 resultType resultId glsl450 30 _0")
T log2(T x);

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv_direct, "12 resultType resultId glsl450 30 _0")
vector<T,N> log2(vector<T,N> x)
{
    VECTOR_MAP_UNARY(T, N, log2, x);
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(hlsl)
matrix<T,N,M> log2(matrix<T,N,M> x)
{
    MATRIX_MAP_UNARY(T, N, M, log2, x);
}

// multiply-add

__generic<T : __BuiltinArithmeticType>
__target_intrinsic(hlsl)
__target_intrinsic(glsl, fma)
__target_intrinsic(cuda, "$P_fma($0, $1, $2)")
__target_intrinsic(cpp, "$P_fma($0, $1, $2)")
__target_intrinsic(spirv_direct, "12 resultType resultId glsl450 50 _0 _1 _2")
T mad(T mvalue, T avalue, T bvalue);

__generic<T : __BuiltinArithmeticType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl, fma)
__target_intrinsic(spirv_direct, "12 resultType resultId glsl450 50 _0 _1 _2")
vector<T, N> mad(vector<T, N> mvalue, vector<T, N> avalue, vector<T, N> bvalue)
{
    VECTOR_MAP_TRINARY(T, N, mad, mvalue, avalue, bvalue);
}

__generic<T : __BuiltinArithmeticType, let N : int, let M : int>
__target_intrinsic(hlsl)
matrix<T, N, M> mad(matrix<T, N, M> mvalue, matrix<T, N, M> avalue, matrix<T, N, M> bvalue)
{
    MATRIX_MAP_TRINARY(T, N, M, mad, mvalue, avalue, bvalue);
}

// maximum
__generic<T : __BuiltinArithmeticType>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(cuda, "$P_max($0, $1)")
__target_intrinsic(cpp, "$P_max($0, $1)")
__target_intrinsic(spirv_direct, "12 resultType resultId glsl450 fus(40,41,42) _0")
T max(T x, T y);
// Note: a stdlib implementation of `max` (or `min`) will require splitting
// floating-point and integer cases apart, because the floating-point
// version needs to correctly handle the case where one of the inputs
// is not-a-number.

__generic<T : __BuiltinArithmeticType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv_direct, "12 resultType resultId glsl450 fus(40,41,42) _0")
vector<T, N> max(vector<T, N> x, vector<T, N> y)
{
    VECTOR_MAP_BINARY(T, N, max, x, y);
}

__generic<T : __BuiltinArithmeticType, let N : int, let M : int>
__target_intrinsic(hlsl)
matrix<T, N, M> max(matrix<T, N, M> x, matrix<T, N, M> y)
{
    MATRIX_MAP_BINARY(T, N, M, max, x, y);
}

// minimum
__generic<T : __BuiltinArithmeticType>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(cuda, "$P_min($0, $1)")
__target_intrinsic(cpp, "$P_min($0, $1)")
__target_intrinsic(spirv_direct, "12 resultType resultId glsl450 fus(37,38,39) _0")
T min(T x, T y);

__generic<T : __BuiltinArithmeticType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv_direct, "12 resultType resultId glsl450 fus(37,38,39) _0")
vector<T,N> min(vector<T,N> x, vector<T,N> y)
{
    VECTOR_MAP_BINARY(T, N, min, x, y);
}

__generic<T : __BuiltinArithmeticType, let N : int, let M : int>
__target_intrinsic(hlsl)
matrix<T,N,M> min(matrix<T,N,M> x, matrix<T,N,M> y)
{
    MATRIX_MAP_BINARY(T, N, M, min, x, y);
}

// split into integer and fractional parts (both with same sign)
__generic<T : __BuiltinFloatingPointType>
T modf(T x, out T ip);

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
vector<T,N> modf(vector<T,N> x, out vector<T,N> ip)
{
    VECTOR_MAP_BINARY(T, N, modf, x, ip);
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(hlsl)
matrix<T,N,M> modf(matrix<T,N,M> x, out matrix<T,N,M> ip)
{
    MATRIX_MAP_BINARY(T, N, M, modf, x, ip);
}

// msad4 (whatever that is)
__target_intrinsic(hlsl)
uint4 msad4(uint reference, uint2 source, uint4 accum)
{
    int4 bytesRef = (reference >> uint4(24, 16, 8, 0)) & 0xFF;
    int4 bytesX   = (source.x  >> uint4(24, 16, 8, 0)) & 0xFF;
    int4 bytesY   = (source.y  >> uint4(24, 16, 8, 0)) & 0xFF;

    uint4 mask = bytesRef == 0 ? 0 : 0xFFFFFFFFu;

    uint4 result = accum;
    result += mask.x & abs(bytesRef - int4(bytesX.x,           bytesY.y, bytesY.z, bytesY.w));
    result += mask.y & abs(bytesRef - int4(bytesX.x, bytesX.y,           bytesY.z, bytesY.w));
    result += mask.z & abs(bytesRef - int4(bytesX.x, bytesX.y, bytesX.z,           bytesY.w));
    result += mask.w & abs(bytesRef - int4(bytesX.x, bytesX.y, bytesX.z, bytesX.w));
    return result;
}

// General inner products

// scalar-scalar
__generic<T : __BuiltinArithmeticType>
__intrinsic_op($(kIROp_Mul))
T mul(T x, T y);

// scalar-vector and vector-scalar
__generic<T : __BuiltinArithmeticType, let N : int>
__intrinsic_op($(kIROp_Mul))
vector<T, N> mul(vector<T, N> x, T y);

__generic<T : __BuiltinArithmeticType, let N : int>
__intrinsic_op($(kIROp_Mul))
vector<T, N> mul(T x, vector<T, N> y);

// scalar-matrix and matrix-scalar
__generic<T : __BuiltinArithmeticType, let N : int, let M :int>
__intrinsic_op($(kIROp_Mul))
matrix<T, N, M> mul(matrix<T, N, M> x, T y);

__generic<T : __BuiltinArithmeticType, let N : int, let M :int>
__intrinsic_op($(kIROp_Mul))
matrix<T, N, M> mul(T x, matrix<T, N, M> y);

// vector-vector (dot product)
__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl, "dot")
T mul(vector<T, N> x, vector<T, N> y)
{
    return dot(x, y);
}
__generic<T : __BuiltinIntegerType, let N : int>
__target_intrinsic(hlsl)
T mul(vector<T, N> x, vector<T, N> y)
{
    return dot(x, y);
}

// vector-matrix
__generic<T : __BuiltinArithmeticType, let N : int, let M : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl, "($1 * $0)")
vector<T, M> mul(vector<T, N> left, matrix<T, N, M> right)
{
    vector<T,M> result;
    for( int j = 0; j < M; ++j )
    {
        T sum = T(0);
        for( int i = 0; i < N; ++i )
        {
            sum += left[i] * right[i][j];
        }
        result[j] = sum;
    }
    return result;
}

// matrix-vector
__generic<T : __BuiltinArithmeticType, let N : int, let M : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl, "($1 * $0)")
vector<T,N> mul(matrix<T,N,M> left, vector<T,M> right)
{
    vector<T,N> result;
    for( int i = 0; i < N; ++i )
    {
        T sum = T(0);
        for( int j = 0; j < M; ++j )
        {
            sum += left[i][j] * right[j];
        }
        result[i] = sum;
    }
    return result;
}


// matrix-matrix
__generic<T : __BuiltinArithmeticType, let R : int, let N : int, let C : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl, "($1 * $0)")
matrix<T,R,C> mul(matrix<T,R,N> right, matrix<T,N,C> left)
{
    matrix<T,R,C> result;
    for( int r = 0; r < R; ++r)
    for( int c = 0; c < C; ++c)
    {
        T sum = T(0);
        for( int i = 0; i < N; ++i )
        {
            sum += left[r][i] * right[i][c];
        }
        result[r][c] = sum;
    }
    return result;
}

// noise (deprecated)

float noise(float x)
{
    return 0;
}

__generic<let N : int> float noise(vector<float, N> x)
{
    return 0;
}

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
__target_intrinsic(hlsl)
__target_intrinsic(glsl, nonuniformEXT)
__glsl_extension(GL_EXT_nonuniform_qualifier)
[__readNone]
uint NonUniformResourceIndex(uint index)
{
    return index;
}

__target_intrinsic(hlsl)
__target_intrinsic(glsl, nonuniformEXT)
__glsl_extension(GL_EXT_nonuniform_qualifier)
[__readNone]
int NonUniformResourceIndex(int index)
{
    return index;
}

// Normalize a vector
__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv_direct, "12 resultType resultId glsl450 69 _0")
vector<T,N> normalize(vector<T,N> x)
{
    return x / length(x);
}

// Raise to a power
__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(cuda, "$P_pow($0, $1)")
__target_intrinsic(cpp, "$P_pow($0, $1)")
__target_intrinsic(spirv_direct, "12 resultType resultId glsl450 26 _0 _1")
T pow(T x, T y);

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv_direct, "12 resultType resultId glsl450 26 _0 _1")
vector<T, N> pow(vector<T, N> x, vector<T, N> y)
{
    VECTOR_MAP_BINARY(T, N, pow, x, y);
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(hlsl)
matrix<T,N,M> pow(matrix<T,N,M> x, matrix<T,N,M> y)
{
    MATRIX_MAP_BINARY(T, N, M, pow, x, y);
}

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
__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv_direct, "12 resultType resultId glsl450 11 _0")
T radians(T x)
{
    return x * (T.getPi() / T(180.0f));
}

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv_direct, "12 resultType resultId glsl450 11 _0")
vector<T, N> radians(vector<T, N> x)
{
    return x * (T.getPi() / T(180.0f));
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(hlsl)
matrix<T, N, M> radians(matrix<T, N, M> x)
{
    return x * (T.getPi() / T(180.0f));
}

// Approximate reciprocal
__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(hlsl)
T rcp(T x)
{
    return T(1.0) / x;
}

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
vector<T, N> rcp(vector<T, N> x)
{
    VECTOR_MAP_UNARY(T, N, rcp, x);
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(hlsl)
matrix<T, N, M> rcp(matrix<T, N, M> x)
{
    MATRIX_MAP_UNARY(T, N, M, rcp, x);
}

// Reflect incident vector across plane with given normal
__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv_direct, "12 resultType resultId glsl450 71 _0 _1")
vector<T,N> reflect(vector<T,N> i, vector<T,N> n)
{
    return i - T(2) * dot(n,i) * n;
}

// Refract incident vector given surface normal and index of refraction
__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv_direct, "12 resultType resultId glsl450 72 _0 _1 _2")
vector<T,N> refract(vector<T,N> i, vector<T,N> n, T eta)
{
    let dotNI = dot(n,i);
    let k = T(1) - eta*eta*(T(1) - dotNI * dotNI);
    if(k < T(0)) return vector<T,N>(T(0));
    return eta * i - (eta * dotNI + sqrt(k)) * n;
}

// Reverse order of bits
__target_intrinsic(hlsl)
__target_intrinsic(glsl, "bitfieldReverse")
__target_intrinsic(cuda, "$P_reversebits($0)")
__target_intrinsic(cpp, "$P_reversebits($0)")
uint reversebits(uint value);

__target_intrinsic(glsl, "bitfieldReverse")
__generic<let N : int>
vector<uint, N> reversebits(vector<uint, N> value)
{
    VECTOR_MAP_UNARY(uint, N, reversebits, value);
}

// Round-to-nearest
__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(cuda, "$P_round($0)")
__target_intrinsic(cpp, "$P_round($0)")
__target_intrinsic(spirv_direct, "12 resultType resultId glsl450 1 _0")
T round(T x);

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv_direct, "12 resultType resultId glsl450 1 _0")
vector<T, N> round(vector<T, N> x)
{
    VECTOR_MAP_UNARY(T, N, round, x);
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(hlsl)
matrix<T,N,M> round(matrix<T,N,M> x)
{
    MATRIX_MAP_UNARY(T, N, M, round, x);
}

// Reciprocal of square root
__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(hlsl)
__target_intrinsic(glsl, "inversesqrt($0)")
__target_intrinsic(cuda, "$P_rsqrt($0)")
__target_intrinsic(cpp, "$P_rsqrt($0)")
__target_intrinsic(spirv_direct, "12 resultType resultId glsl450 32 _0")
T rsqrt(T x)
{
    return T(1.0) / sqrt(x);
}

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl, "inversesqrt($0)")
__target_intrinsic(spirv_direct, "12 resultType resultId glsl450 32 _0")
vector<T, N> rsqrt(vector<T, N> x)
{
    VECTOR_MAP_UNARY(T, N, rsqrt, x);
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(hlsl)
matrix<T, N, M> rsqrt(matrix<T, N, M> x)
{
    MATRIX_MAP_UNARY(T, N, M, rsqrt, x);
}

// Clamp value to [0,1] range

__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(hlsl)
T saturate(T x)
{
    return clamp<T>(x, T(0), T(1));
}

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
vector<T,N> saturate(vector<T,N> x)
{
    return clamp<T,N>(x,
        vector<T,N>(T(0)),
        vector<T,N>(T(1)));
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(hlsl)
matrix<T,N,M> saturate(matrix<T,N,M> x)
{
    MATRIX_MAP_UNARY(T, N, M, saturate, x);
}

// Extract sign of value
__generic<T : __BuiltinSignedArithmeticType>
__target_intrinsic(hlsl)
__target_intrinsic(glsl, "int(sign($0))")
__target_intrinsic(cuda, "$P_sign($0)")
__target_intrinsic(cpp, "$P_sign($0)")
__target_intrinsic(spirv_direct, "12 resultType resultId glsl450 fi(6,7) _0")
int sign(T x);

__generic<T : __BuiltinSignedArithmeticType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl, "ivec$N0(sign($0))")
__target_intrinsic(spirv_direct, "12 resultType resultId glsl450 fi(6,7) _0")
vector<int, N> sign(vector<T, N> x)
{
    VECTOR_MAP_UNARY(int, N, sign, x);
}

__generic<T : __BuiltinSignedArithmeticType, let N : int, let M : int>
__target_intrinsic(hlsl)
matrix<int, N, M> sign(matrix<T, N, M> x)
{
    MATRIX_MAP_UNARY(int, N, M, sign, x);
}


// Sine

__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(cuda, "$P_sin($0)")
__target_intrinsic(cpp, "$P_sin($0)")
__target_intrinsic(spirv_direct, "12 resultType resultId glsl450 13 _0")
T sin(T x);

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv_direct, "12 resultType resultId glsl450 13 _0")
vector<T, N> sin(vector<T, N> x)
{
    VECTOR_MAP_UNARY(T, N, sin, x);
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(hlsl)
matrix<T, N, M> sin(matrix<T, N, M> x)
{
    MATRIX_MAP_UNARY(T, N, M, sin, x);
}

// Sine and cosine
__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(hlsl)
__target_intrinsic(cuda, "$P_sincos($0, $1, $2)")
void sincos(T x, out T s, out T c)
{
    s = sin(x);
    c = cos(x);
}

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
void sincos(vector<T,N> x, out vector<T,N> s, out vector<T,N> c)
{
    s = sin(x);
    c = cos(x);
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(hlsl)
void sincos(matrix<T,N,M> x, out matrix<T,N,M> s, out matrix<T,N,M> c)
{
    s = sin(x);
    c = cos(x);
}

// Hyperbolic Sine
__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(cuda, "$P_sinh($0)")
__target_intrinsic(cpp, "$P_sinh($0)")
__target_intrinsic(spirv_direct, "12 resultType resultId glsl450 19 _0")
T sinh(T x);

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv_direct, "12 resultType resultId glsl450 19 _0")
vector<T, N> sinh(vector<T, N> x)
{
    VECTOR_MAP_UNARY(T, N, sinh, x);
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(hlsl)
matrix<T, N, M> sinh(matrix<T, N, M> x)
{
    MATRIX_MAP_UNARY(T, N, M, sinh, x);
}

// Smooth step (Hermite interpolation)
__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv_direct, "12 resultType resultId glsl450 49 _0 _1 _2")
T smoothstep(T min, T max, T x)
{
    let t = saturate((x - min) / (max - min));
    return t * t * (T(3.0f) - (t + t));
}

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv_direct, "12 resultType resultId glsl450 49 _0 _1 _2")
vector<T, N> smoothstep(vector<T, N> min, vector<T, N> max, vector<T, N> x)
{
    VECTOR_MAP_TRINARY(T, N, smoothstep, min, max, x);
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(hlsl)
matrix<T, N, M> smoothstep(matrix<T, N, M> min, matrix<T, N, M> max, matrix<T, N, M> x)
{
    MATRIX_MAP_TRINARY(T, N, M, smoothstep, min, max, x);
}

// Square root
__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(cuda, "$P_sqrt($0)")
__target_intrinsic(cpp, "$P_sqrt($0)")
__target_intrinsic(spirv_direct, "12 resultType resultId glsl450 31 _0")
T sqrt(T x);

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv_direct, "12 resultType resultId glsl450 31 _0")
vector<T, N> sqrt(vector<T, N> x)
{
    VECTOR_MAP_UNARY(T, N, sqrt, x);
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(hlsl)
matrix<T, N, M> sqrt(matrix<T, N, M> x)
{
    MATRIX_MAP_UNARY(T, N, M, sqrt, x);
}

// Step function
__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv_direct, "12 resultType resultId glsl450 48 _0 _1")
T step(T y, T x)
{
    return x < y ? T(0.0f) : T(1.0f);
}

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv_direct, "12 resultType resultId glsl450 48 _0 _1")
vector<T,N> step(vector<T,N> y, vector<T,N> x)
{
    VECTOR_MAP_BINARY(T, N, step, y, x);
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(hlsl)
matrix<T, N, M> step(matrix<T, N, M> y, matrix<T, N, M> x)
{
    MATRIX_MAP_BINARY(T, N, M, step, y, x);
}

// Tangent
__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(cuda, "$P_tan($0)")
__target_intrinsic(cpp, "$P_tan($0)")
__target_intrinsic(spirv_direct, "12 resultType resultId glsl450 15 _0")
T tan(T x);

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv_direct, "12 resultType resultId glsl450 15 _0")
vector<T, N> tan(vector<T, N> x)
{
    VECTOR_MAP_UNARY(T, N, tan, x);
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(hlsl)
matrix<T, N, M> tan(matrix<T, N, M> x)
{
    MATRIX_MAP_UNARY(T, N, M, tan, x);
}

// Hyperbolic tangent
__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(cuda, "$P_tanh($0)")
__target_intrinsic(cpp, "$P_tanh($0)")
__target_intrinsic(spirv_direct, "12 resultType resultId glsl450 21 _0")
T tanh(T x);

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv_direct, "12 resultType resultId glsl450 21 _0")
vector<T,N> tanh(vector<T,N> x)
{
    VECTOR_MAP_UNARY(T, N, tanh, x);
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(hlsl)
matrix<T,N,M> tanh(matrix<T,N,M> x)
{
    MATRIX_MAP_UNARY(T, N, M, tanh, x);
}

// Matrix transpose
__generic<T : __BuiltinType, let N : int, let M : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
matrix<T, M, N> transpose(matrix<T, N, M> x)
{
    matrix<T,M,N> result;
    for(int r = 0; r < M; ++r)
        for(int c = 0; c < N; ++c)
            result[r][c] = x[c][r];
    return result;
}

// Truncate to integer
__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(cuda, "$P_trunc($0)")
__target_intrinsic(cpp, "$P_trunc($0)")
__target_intrinsic(spirv_direct, "12 resultType resultId glsl450 3 _0")
T trunc(T x);

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(glsl)
__target_intrinsic(spirv_direct, "12 resultType resultId glsl450 3 _0")
vector<T, N> trunc(vector<T, N> x)
{
    VECTOR_MAP_UNARY(T, N, trunc, x);
}

__generic<T : __BuiltinFloatingPointType, let N : int, let M : int>
__target_intrinsic(hlsl)
matrix<T, N, M> trunc(matrix<T, N, M> x)
{
    MATRIX_MAP_UNARY(T, N, M, trunc, x);
}

// Slang Specific 'Mask' Wave Intrinsics 

typedef uint WaveMask;

__glsl_extension(GL_KHR_shader_subgroup_ballot)
__spirv_version(1.3)
__target_intrinsic(glsl, "subgroupBallot(true).x")
__target_intrinsic(cuda, "__activemask()")
__target_intrinsic(hlsl, "WaveActiveBallot(true).x")
WaveMask WaveGetConvergedMask();

__intrinsic_op($(kIROp_WaveGetActiveMask))
WaveMask __WaveGetActiveMask();

__glsl_extension(GL_KHR_shader_subgroup_ballot)
__spirv_version(1.3)
__target_intrinsic(glsl, "subgroupBallot(true).x")
__target_intrinsic(hlsl, "WaveActiveBallot(true).x")
WaveMask WaveGetActiveMask()
{
    return __WaveGetActiveMask();
}

__glsl_extension(GL_KHR_shader_subgroup_basic)
__spirv_version(1.3)
__target_intrinsic(glsl, "subgroupElect()")
__target_intrinsic(cuda, "(($0 & -$0) == (WarpMask(1) << _getLaneId()))")
__target_intrinsic(hlsl, "WaveIsFirstLane()")
bool WaveMaskIsFirstLane(WaveMask mask);

__glsl_extension(GL_KHR_shader_subgroup_vote)
__spirv_version(1.3)
__target_intrinsic(glsl, "subgroupAll($1)") 
__target_intrinsic(cuda, "(__all_sync($0, $1) != 0)")
__target_intrinsic(hlsl, "WaveActiveAllTrue($1)")
bool WaveMaskAllTrue(WaveMask mask, bool condition);

__glsl_extension(GL_KHR_shader_subgroup_vote)
__spirv_version(1.3)
__target_intrinsic(glsl, "subgroupAny($1)") 
__target_intrinsic(cuda, "(__any_sync($0, $1) != 0)")
__target_intrinsic(hlsl, "WaveActiveAnyTrue($1)")
bool WaveMaskAnyTrue(WaveMask mask, bool condition);

__glsl_extension(GL_KHR_shader_subgroup_ballot)
__spirv_version(1.3)
__target_intrinsic(glsl, "subgroupBallot($1).x")
__target_intrinsic(cuda, "__ballot_sync($0, $1)")
__target_intrinsic(hlsl, "WaveActiveBallot($1)")
WaveMask WaveMaskBallot(WaveMask mask, bool condition);

__glsl_extension(GL_KHR_shader_subgroup_ballot)
__target_intrinsic(cuda, "__popc(__ballot_sync($0, $1))")
__target_intrinsic(hlsl, "WaveActiveCountBits($1)")
uint WaveMaskCountBits(WaveMask mask, bool value)
{
    return _WaveCountBits(WaveActiveBallot(value));
}

// Waits until all warp lanes named in mask have executed a WaveMaskSharedSync (with the same mask)
// before resuming execution. Guarantees memory ordering in shared memory among threads participating
// in the barrier.
//
// The CUDA intrinsic says it orders *all* memory accesses, which appears to match most closely subgroupBarrier.
//
// TODO(JS):
// For HLSL it's not clear what to do. There is no explicit mechanism to 'reconverge' threads. In the docs it describes
// behavior as
// "These intrinsics are dependent on active lanes and therefore flow control. In the model of this document, implementations
// must enforce that the number of active lanes exactly corresponds to the programmer’s view of flow control."
// 
// It seems this can only mean the active threads are the "threads the program flow would lead to". This implies a lockstep
// "straight SIMD" style interpretation. That being the case this op on HLSL is just a memory barrier without any Sync.

__target_intrinsic(cuda, "__syncwarp($0)")
__glsl_extension(GL_KHR_shader_subgroup_basic)
__spirv_version(1.3)
__target_intrinsic(glsl, "subgroupBarrier()")
__target_intrinsic(hlsl, "AllMemoryBarrier()")
void AllMemoryBarrierWithWaveMaskSync(WaveMask mask);

// On GLSL, it appears we can't use subgroupMemoryBarrierShared, because it only implies a memory ordering, it does not
// imply convergence. For subgroupBarrier we have from the docs..
// "The function subgroupBarrier() enforces that all active invocations within a subgroup must execute this function before any
// are allowed to continue their execution"
// TODO(JS):
// It's not entirely clear what to do here on HLSL. 
// Reading the dxc wiki (https://github.com/Microsoft/DirectXShaderCompiler/wiki/Wave-Intrinsics), we have statements like:
//    ... these intrinsics enable the elimination of barrier constructs when the scope of synchronization is within the width of the SIMD processor.
//    Wave: A set of lanes executed simultaneously in the processor. No explicit barriers are required to guarantee that they execute in parallel.
// Which seems to imply at least some memory barriers like Shared might not be needed.
//
// The barrier is left here though, because not only is the barrier make writes before the barrier across the wave appear to others afterwards, it's
// also there to inform the compiler on what order reads and writes can take place. This might seem to be silly because of the 'Active' lanes
// aspect of HLSL seems to make everything in lock step - but that's not quite so, it only has to apparently be that way as far as the programmers
// model appears - divergence could perhaps potentially still happen. 
__target_intrinsic(cuda, "__syncwarp($0)")
__glsl_extension(GL_KHR_shader_subgroup_basic)
__spirv_version(1.3)
__target_intrinsic(glsl, "subgroupBarrier()")
__target_intrinsic(hlsl, "GroupMemoryBarrier()")
void GroupMemoryBarrierWithWaveMaskSync(WaveMask mask);

__glsl_extension(GL_KHR_shader_subgroup_basic)
__spirv_version(1.3)
__target_intrinsic(glsl, "subgroupBarrier()")
__target_intrinsic(hlsl, "AllMemoryBarrier()")
void AllMemoryBarrierWithWaveSync();

__glsl_extension(GL_KHR_shader_subgroup_basic)
__spirv_version(1.3)
__target_intrinsic(glsl, "subgroupBarrier()")
__target_intrinsic(hlsl, "GroupMemoryBarrier()")
__target_intrinsic(cuda, "__syncwarp()")
void GroupMemoryBarrierWithWaveSync();

// NOTE! WaveMaskBroadcastLaneAt is *NOT* standard HLSL
// It is provided as access to subgroupBroadcast which can only take a
// constexpr laneId.
// https://github.com/KhronosGroup/GLSL/blob/master/extensions/khr/GL_KHR_shader_subgroup.txt
// Versions SPIR-V greater than 1.4 loosen this restriction, and allow 'dynamic uniform' index
// If that's the behavior required then client code should use WaveReadLaneAt which works this way.

__generic<T : __BuiltinType>
__glsl_extension(GL_KHR_shader_subgroup_ballot)
__spirv_version(1.3)
__target_intrinsic(glsl, "subgroupBroadcast($1, $2)")
__target_intrinsic(cuda, "__shfl_sync($0, $1, $2)")
__target_intrinsic(hlsl, "WaveReadLaneAt($1, $2)")
T WaveMaskBroadcastLaneAt(WaveMask mask, T value, constexpr int lane);
__generic<T : __BuiltinType, let N : int>
__glsl_extension(GL_KHR_shader_subgroup_ballot)
__spirv_version(1.3)
__target_intrinsic(glsl, "subgroupBroadcast($1, $2)")
__target_intrinsic(cuda, "_waveShuffleMultiple($0, $1, $2)")
__target_intrinsic(hlsl, "WaveReadLaneAt($1, $2)")
vector<T,N> WaveMaskBroadcastLaneAt(WaveMask mask, vector<T,N> value, constexpr int lane);
__generic<T : __BuiltinType, let N : int, let M : int>
__target_intrinsic(cuda, "_waveShuffleMultiple($0, $1, $2)")
__target_intrinsic(hlsl, "WaveReadLaneAt($1, $2)")
matrix<T,N,M> WaveMaskBroadcastLaneAt(WaveMask mask, matrix<T,N,M> value, constexpr int lane);

// TODO(JS): If it can be determines that the `laneId` is constExpr, then subgroupBroadcast
// could be used on GLSL. For now we just use subgroupShuffle
__generic<T : __BuiltinType>
__glsl_extension(GL_KHR_shader_subgroup_shuffle)
__spirv_version(1.3)
__target_intrinsic(glsl, "subgroupShuffle($1, $2)")
__target_intrinsic(cuda, "__shfl_sync($0, $1, $2)")
__target_intrinsic(hlsl, "WaveReadLaneAt($1, $2)")
T WaveMaskReadLaneAt(WaveMask mask, T value, int lane);
__generic<T : __BuiltinType, let N : int>
__spirv_version(1.3)
__glsl_extension(GL_KHR_shader_subgroup_shuffle)
__target_intrinsic(glsl, "subgroupShuffle($1, $2)")
__target_intrinsic(cuda, "_waveShuffleMultiple($0, $1, $2)")
__target_intrinsic(hlsl, "WaveReadLaneAt($1, $2)")
vector<T,N> WaveMaskReadLaneAt(WaveMask mask, vector<T,N> value, int lane);
__generic<T : __BuiltinType, let N : int, let M : int>
__target_intrinsic(cuda, "_waveShuffleMultiple($0, $1, $2)")
__target_intrinsic(hlsl, "WaveReadLaneAt($1, $2)")
matrix<T,N,M> WaveMaskReadLaneAt(WaveMask mask, matrix<T,N,M> value, int lane);

// NOTE! WaveMaskShuffle is a NON STANDARD HLSL intrinsic! It will map to WaveReadLaneAt on HLSL
// which means it will only work on hardware which allows arbitrary laneIds which is not true
// in general because it breaks the HLSL standard, which requires it's 'dynamically uniform' across the Wave.
__generic<T : __BuiltinType>
__glsl_extension(GL_KHR_shader_subgroup_shuffle)
__spirv_version(1.3)
__target_intrinsic(glsl, "subgroupShuffle($1, $2)")
__target_intrinsic(cuda, "__shfl_sync($0, $1, $2)")
__target_intrinsic(hlsl, "WaveReadLaneAt($1, $2)")
T WaveMaskShuffle(WaveMask mask, T value, int lane);
__generic<T : __BuiltinType, let N : int>
__glsl_extension(GL_KHR_shader_subgroup_shuffle)
__spirv_version(1.3)
__target_intrinsic(glsl, "subgroupShuffle($1, $2)")
__target_intrinsic(cuda, "_waveShuffleMultiple($0, $1, $2)")
__target_intrinsic(hlsl, "WaveReadLaneAt($1, $2)")
vector<T,N> WaveMaskShuffle(WaveMask mask, vector<T,N> value, int lane);
__generic<T : __BuiltinType, let N : int, let M : int>
__target_intrinsic(cuda, "_waveShuffleMultiple($0, $1, $2)")
__target_intrinsic(hlsl, "WaveReadLaneAt($1, $2)")
matrix<T,N,M> WaveMaskShuffle(WaveMask mask, matrix<T,N,M> value, int lane);

__glsl_extension(GL_KHR_shader_subgroup_ballot)
__spirv_version(1.3)
__target_intrinsic(glsl, "subgroupBallotExclusiveBitCount(subgroupBallot($1))")
__target_intrinsic(cuda, "__popc(__ballot_sync($0, $1)  & _getLaneLtMask())")
__target_intrinsic(hlsl, "WavePrefixCountBits($1)")
uint WaveMaskPrefixCountBits(WaveMask mask, bool value);

// Across lane ops

__generic<T : __BuiltinIntegerType>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
__target_intrinsic(glsl, "subgroupAnd($1)")
__target_intrinsic(cuda, "_waveAnd($0, $1)")
__target_intrinsic(hlsl, "WaveActiveBitAnd($1)")
T WaveMaskBitAnd(WaveMask mask, T expr);
__generic<T : __BuiltinIntegerType, let N : int>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
__target_intrinsic(glsl, "subgroupAnd($1)")
__target_intrinsic(cuda, "_waveAndMultiple($0, $1)")
__target_intrinsic(hlsl, "WaveActiveBitAnd($1)")
vector<T,N> WaveMaskBitAnd(WaveMask mask, vector<T,N> expr);
__generic<T : __BuiltinIntegerType, let N : int, let M : int>
__target_intrinsic(cuda, "_waveAndMultiple($0, $1)")
__target_intrinsic(hlsl, "WaveActiveBitAnd($1)")
matrix<T,N,M> WaveMaskBitAnd(WaveMask mask, matrix<T,N,M> expr);

__generic<T : __BuiltinIntegerType>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
__target_intrinsic(glsl, "subgroupOr($1)")
__target_intrinsic(cuda, "_waveOr($0, $1)")
__target_intrinsic(hlsl, "WaveActiveBitOr($1)")
T WaveMaskBitOr(WaveMask mask, T expr);
__generic<T : __BuiltinIntegerType, let N : int>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
__target_intrinsic(glsl, "subgroupOr($1)")
__target_intrinsic(cuda, "_waveOrMultiple($0, $1)")
__target_intrinsic(hlsl, "WaveActiveBitOr($1)")
vector<T,N> WaveMaskBitOr(WaveMask mask, vector<T,N> expr);
__generic<T : __BuiltinIntegerType, let N : int, let M : int>
__target_intrinsic(cuda, "_waveOrMultiple($0, $1)")
__target_intrinsic(hlsl, "WaveActiveBitOr($1)")
matrix<T,N,M> WaveMaskBitOr(WaveMask mask, matrix<T,N,M> expr);

__generic<T : __BuiltinIntegerType>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
__target_intrinsic(glsl, "subgroupXor($1)")
__target_intrinsic(cuda, "_waveXor($0, $1)")
__target_intrinsic(hlsl, "WaveActiveBitXor($1)")
T WaveMaskBitXor(WaveMask mask, T expr);
__generic<T : __BuiltinIntegerType, let N : int> 
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
__target_intrinsic(glsl, "subgroupXor($1)")
__target_intrinsic(cuda, "_waveXorMultiple($0, $1)")
__target_intrinsic(hlsl, "WaveActiveBitXor($1)")
vector<T,N> WaveMaskBitXor(WaveMask mask, vector<T,N> expr);
__generic<T : __BuiltinIntegerType, let N : int, let M : int>
__target_intrinsic(cuda, "_waveXorMultiple($0, $1)")
__target_intrinsic(hlsl, "WaveActiveBitXor($1)")
matrix<T,N,M> WaveMaskBitXor(WaveMask mask, matrix<T,N,M> expr);

__generic<T : __BuiltinArithmeticType>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
__target_intrinsic(glsl, "subgroupMax($1)")
__target_intrinsic(cuda, "_waveMax($0, $1)")
__target_intrinsic(hlsl, "WaveActiveMax($1)")
T WaveMaskMax(WaveMask mask, T expr);
__generic<T : __BuiltinArithmeticType, let N : int>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
__target_intrinsic(glsl, "subgroupMax($1)")
__target_intrinsic(cuda, "_waveMaxMultiple($0, $1)")
__target_intrinsic(hlsl, "WaveActiveMax($1)")
vector<T,N> WaveMaskMax(WaveMask mask, vector<T,N> expr);
__generic<T : __BuiltinArithmeticType, let N : int, let M : int>
__target_intrinsic(cuda, "_waveMaxMultiple($0, $1)")
__target_intrinsic(hlsl, "WaveActiveMax($1)")
matrix<T,N,M> WaveMaskMax(WaveMask mask, matrix<T,N,M> expr);

__generic<T : __BuiltinArithmeticType>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
__target_intrinsic(glsl, "subgroupMin($1)")
__target_intrinsic(cuda, "_waveMin($0, $1)")
__target_intrinsic(hlsl, "WaveActiveMin($1)")
T WaveMaskMin(WaveMask mask, T expr);
__generic<T : __BuiltinArithmeticType, let N : int>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
__target_intrinsic(glsl, "subgroupMin($1)")
__target_intrinsic(cuda, "_waveMinMultiple($0, $1)")
__target_intrinsic(hlsl, "WaveActiveMin($1)")
vector<T,N> WaveMaskMin(WaveMask mask, vector<T,N> expr);
__generic<T : __BuiltinArithmeticType, let N : int, let M : int>
__target_intrinsic(cuda, "_waveMinMultiple($0, $1)")
__target_intrinsic(hlsl, "WaveActiveMin($1)")
matrix<T,N,M> WaveMaskMin(WaveMask mask, matrix<T,N,M> expr);

__generic<T : __BuiltinArithmeticType>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
__target_intrinsic(glsl, "subgroupMul($1)")
__target_intrinsic(cuda, "_waveProduct($0, $1)")
__target_intrinsic(hlsl, "WaveActiveProduct($1)")
T WaveMaskProduct(WaveMask mask, T expr);
__generic<T : __BuiltinArithmeticType, let N : int>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
__target_intrinsic(glsl, "subgroupMul($1)")
__target_intrinsic(cuda, "_waveProductMultiple($0, $1)")
__target_intrinsic(hlsl, "WaveActiveProduct($1)")
vector<T,N> WaveMaskProduct(WaveMask mask, vector<T,N> expr);
__generic<T : __BuiltinArithmeticType, let N : int, let M : int>
__target_intrinsic(cuda, "_waveProductMultiple($0, $1)")
__target_intrinsic(hlsl, "WaveActiveProduct($1)")
matrix<T,N,M> WaveMaskProduct(WaveMask mask, matrix<T,N,M> expr);

__generic<T : __BuiltinArithmeticType>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
__target_intrinsic(glsl, "subgroupAdd($1)")
__target_intrinsic(cuda, "_waveSum($0, $1)")
__target_intrinsic(hlsl, "WaveActiveSum($1)")
T WaveMaskSum(WaveMask mask, T expr);
__generic<T : __BuiltinArithmeticType, let N : int>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
__target_intrinsic(glsl, "subgroupAdd($1)")
__target_intrinsic(cuda, "_waveSumMultiple($0, $1)")
__target_intrinsic(hlsl, "WaveActiveSum($1)")
vector<T,N> WaveMaskSum(WaveMask mask, vector<T,N> expr);
__generic<T : __BuiltinArithmeticType, let N : int, let M : int>
__target_intrinsic(cuda, "_waveSumMultiple($0, $1)")
__target_intrinsic(hlsl, "WaveActiveSum($1)")
matrix<T,N,M> WaveMaskSum(WaveMask mask, matrix<T,N,M> expr);

__generic<T : __BuiltinType>
__glsl_extension(GL_KHR_shader_subgroup_vote)
__spirv_version(1.3)
__target_intrinsic(glsl, "subgroupAllEqual($1)")
__cuda_sm_version(7.0)
__target_intrinsic(cuda, "_waveAllEqual($0, $1)")
__target_intrinsic(hlsl, "WaveActiveAllEqual($1)")
bool WaveMaskAllEqual(WaveMask mask, T value);
__generic<T : __BuiltinType, let N : int> 
__glsl_extension(GL_KHR_shader_subgroup_vote)
__spirv_version(1.3)
__target_intrinsic(glsl, "subgroupAllEqual($1)")
__cuda_sm_version(7.0)
__target_intrinsic(cuda, "_waveAllEqualMultiple($0, $1)")
__target_intrinsic(hlsl, "WaveActiveAllEqual($1)")
bool WaveMaskAllEqual(WaveMask mask, vector<T,N> value);
__generic<T : __BuiltinType, let N : int, let M : int>
__cuda_sm_version(7.0)
__target_intrinsic(cuda, "_waveAllEqualMultiple($0, $1)")
__target_intrinsic(hlsl, "WaveActiveAllEqual($1)")
bool WaveMaskAllEqual(WaveMask mask, matrix<T,N,M> value);

// Prefix

__generic<T : __BuiltinArithmeticType>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
__target_intrinsic(glsl, "subgroupExclusiveMul($1)")
__target_intrinsic(cuda, "_wavePrefixProduct($0, $1)")
__target_intrinsic(hlsl, "WavePrefixProduct($1)")
T WaveMaskPrefixProduct(WaveMask mask, T expr);
__generic<T : __BuiltinArithmeticType, let N : int>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
__target_intrinsic(glsl, "subgroupExclusiveMul($1)")
__target_intrinsic(cuda, "_wavePrefixProductMultiple($0, $1)")
__target_intrinsic(hlsl, "WavePrefixProduct($1)")
vector<T,N> WaveMaskPrefixProduct(WaveMask mask, vector<T,N> expr);
__generic<T : __BuiltinArithmeticType, let N : int, let M : int>
__target_intrinsic(cuda, "_wavePrefixProductMultiple($0, $1)")
__target_intrinsic(hlsl, "WavePrefixProduct($1)")
matrix<T,N,M> WaveMaskPrefixProduct(WaveMask mask, matrix<T,N,M> expr);

__generic<T : __BuiltinArithmeticType>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
__target_intrinsic(glsl, "subgroupExclusiveAdd($1)")
__target_intrinsic(cuda, "_wavePrefixSum($0, $1)")
__target_intrinsic(hlsl, "WavePrefixSum($1)")
T WaveMaskPrefixSum(WaveMask mask, T expr);
__generic<T : __BuiltinArithmeticType, let N : int>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
__target_intrinsic(glsl, "subgroupExclusiveAdd($1)")
__target_intrinsic(cuda, "_wavePrefixSumMultiple($0, $1)")
__target_intrinsic(hlsl, "WavePrefixSum($1)")
vector<T,N> WaveMaskPrefixSum(WaveMask mask, vector<T,N> expr);
__generic<T : __BuiltinArithmeticType, let N : int, let M : int>
__target_intrinsic(cuda, "_wavePrefixSumMultiple($0, $1)")
__target_intrinsic(hlsl, "WavePrefixSum($1)")
matrix<T,N,M> WaveMaskPrefixSum(WaveMask mask, matrix<T,N,M> expr);

__generic<T : __BuiltinType>
__glsl_extension(GL_KHR_shader_subgroup_ballot)
__spirv_version(1.3)
__target_intrinsic(glsl, "subgroupBroadcastFirst($1)")
__target_intrinsic(cuda, "_waveReadFirst($0, $1)")
T WaveMaskReadLaneFirst(WaveMask mask, T expr);
__generic<T : __BuiltinType, let N : int>
__glsl_extension(GL_KHR_shader_subgroup_ballot)
__spirv_version(1.3)
__target_intrinsic(glsl, "subgroupBroadcastFirst($1)")
__target_intrinsic(cuda, "_waveReadFirstMultiple($0, $1)")
vector<T,N> WaveMaskReadLaneFirst(WaveMask mask, vector<T,N> expr);
__generic<T : __BuiltinType, let N : int, let M : int>
__target_intrinsic(cuda, "_waveReadFirstMultiple($0, $1)")
matrix<T,N,M> WaveMaskReadLaneFirst(WaveMask mask, matrix<T,N,M> expr);

// WaveMask SM6.5 like intrinsics

// TODO(JS): On HLSL it only works for 32 bits or less

__generic<T : __BuiltinType>
__target_intrinsic(hlsl, "WaveMatch($1).x")
__cuda_sm_version(7.0)
__target_intrinsic(cuda, "_waveMatchScalar($0, $1).x")
WaveMask WaveMaskMatch(WaveMask mask, T value);
__generic<T : __BuiltinType, let N : int>
__target_intrinsic(hlsl, "WaveMatch($1).x")
__cuda_sm_version(7.0)
__target_intrinsic(cuda, "_waveMatchMultiple($0, $1)")
WaveMask WaveMaskMatch(WaveMask mask, vector<T,N> value);
__generic<T : __BuiltinType, let N : int, let M : int>
__target_intrinsic(hlsl, "WaveMatch($1).x")
__cuda_sm_version(7.0)
__target_intrinsic(cuda, "_waveMatchMultiple($0, $1)")
WaveMask WaveMaskMatch(WaveMask mask, matrix<T,N,M> value);

__generic<T : __BuiltinArithmeticType>
__target_intrinsic(hlsl, "WaveMultiPrefixBitAnd($1, uint4($0, 0, 0, 0))")
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
//__target_intrinsic(glsl, "subgroupExclusiveAnd($1)")
__target_intrinsic(cuda, "_wavePrefixAnd($0, $1)")
T WaveMaskPrefixBitAnd(WaveMask mask, T expr);
__target_intrinsic(hlsl, "WaveMultiPrefixBitAnd($1, uint4($0, 0, 0, 0))")
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
__target_intrinsic(glsl, "subgroupExclusiveAnd($1)")
__target_intrinsic(cuda, "_wavePrefixAndMultiple($0, $1)")
__generic<T : __BuiltinArithmeticType, let N : int>
vector<T,N> WaveMaskPrefixBitAnd(WaveMask mask, vector<T,N> expr);
__generic<T : __BuiltinArithmeticType, let N : int, let M : int>
__target_intrinsic(hlsl, "WaveMultiPrefixBitAnd($1, uint4($0, 0, 0, 0))")
__target_intrinsic(cuda, "_wavePrefixAndMultiple(_getMultiPrefixMask($0, $1)")
matrix<T,N,M> WaveMaskPrefixBitAnd(WaveMask mask, matrix<T,N,M> expr);

__generic<T : __BuiltinArithmeticType>
__target_intrinsic(hlsl, "WaveMultiPrefixBitOr($1, uint4($0, 0, 0, 0))")
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
//__target_intrinsic(glsl, "subgroupExclusiveOr($1)")
__target_intrinsic(cuda, "_wavePrefixOr($0, $1)")
T WaveMaskPrefixBitOr(WaveMask mask, T expr);
__generic<T : __BuiltinArithmeticType, let N : int>
__target_intrinsic(hlsl, "WaveMultiPrefixBitOr($1, uint4($0, 0, 0, 0))")
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
//__target_intrinsic(glsl, "subgroupExclusiveOr($1)")
__target_intrinsic(cuda, "_wavePrefixOrMultiple($0, $1)")
vector<T,N> WaveMaskPrefixBitOr(WaveMask mask, vector<T,N> expr);
__generic<T : __BuiltinArithmeticType, let N : int, let M : int>
__target_intrinsic(hlsl, "WaveMultiPrefixBitOr($1, uint4($0, 0, 0, 0))")
__target_intrinsic(cuda, "_wavePrefixOrMultiple($0, $1)")
matrix<T,N,M> WaveMaskPrefixBitOr(WaveMask mask, matrix<T,N,M> expr);

__generic<T : __BuiltinArithmeticType>
__target_intrinsic(hlsl, "WaveMultiPrefixBitXor($1, uint4($0, 0, 0, 0))")
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
__target_intrinsic(glsl, "subgroupExclusiveXor($1)")
__target_intrinsic(cuda, "_wavePrefixXor($0, $1)")
T WaveMaskPrefixBitXor(WaveMask mask, T expr);
__generic<T : __BuiltinArithmeticType, let N : int>
__target_intrinsic(hlsl, "WaveMultiPrefixBitXor($1, uint4($0, 0, 0, 0))")
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
__target_intrinsic(glsl, "subgroupExclusiveXor($1)")
__target_intrinsic(cuda, "_wavePrefixXorMultiple($0, $1)")
vector<T,N> WaveMaskPrefixBitXor(WaveMask mask, vector<T,N> expr);
__generic<T : __BuiltinArithmeticType, let N : int, let M : int>
__target_intrinsic(hlsl, "WaveMultiPrefixBitXor($1, uint4($0, 0, 0, 0))")
__target_intrinsic(cuda, "_wavePrefixXorMultiple($0, $1)")
matrix<T,N,M> WaveMaskPrefixBitXor(WaveMask mask, matrix<T,N,M> expr);

// Shader model 6.0 stuff

// Information for GLSL wave/subgroup support
// https://github.com/KhronosGroup/GLSL/blob/master/extensions/khr/GL_KHR_shader_subgroup.txt

__generic<T : __BuiltinType> T QuadReadLaneAt(T sourceValue, uint quadLaneID);
__generic<T : __BuiltinType, let N : int> vector<T,N> QuadReadLaneAt(vector<T,N> sourceValue, uint quadLaneID);
__generic<T : __BuiltinType, let N : int, let M : int> matrix<T,N,M> QuadReadLaneAt(matrix<T,N,M> sourceValue, uint quadLaneID);

__generic<T : __BuiltinType> T QuadReadAcrossX(T localValue);
__generic<T : __BuiltinType, let N : int> vector<T,N> QuadReadAcrossX(vector<T,N> localValue);
__generic<T : __BuiltinType, let N : int, let M : int> matrix<T,N,M> QuadReadAcrossX(matrix<T,N,M> localValue);

__generic<T : __BuiltinType> T QuadReadAcrossY(T localValue);
__generic<T : __BuiltinType, let N : int> vector<T,N> QuadReadAcrossY(vector<T,N> localValue);
__generic<T : __BuiltinType, let N : int, let M : int> matrix<T,N,M> QuadReadAcrossY(matrix<T,N,M> localValue);

__generic<T : __BuiltinType> T QuadReadAcrossDiagonal(T localValue);
__generic<T : __BuiltinType, let N : int> vector<T,N> QuadReadAcrossDiagonal(vector<T,N> localValue);
__generic<T : __BuiltinType, let N : int, let M : int> matrix<T,N,M> QuadReadAcrossDiagonal(matrix<T,N,M> localValue);


__generic<T : __BuiltinIntegerType>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
__target_intrinsic(glsl, "subgroupAnd($0)")
__target_intrinsic(hlsl)
T WaveActiveBitAnd(T expr)
{
    return WaveMaskBitAnd(WaveGetActiveMask(), expr);
}

__generic<T : __BuiltinIntegerType, let N : int>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
__target_intrinsic(glsl, "subgroupAnd($0)")
__target_intrinsic(hlsl)
vector<T, N> WaveActiveBitAnd(vector<T, N> expr)
{
    return WaveMaskBitAnd(WaveGetActiveMask(), expr);
}

__generic<T : __BuiltinIntegerType, let N : int, let M : int>
__target_intrinsic(hlsl)
matrix<T, N, M> WaveActiveBitAnd(matrix<T, N, M> expr)
{
    return WaveMaskBitAnd(WaveGetActiveMask(), expr);
}

__generic<T : __BuiltinIntegerType>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
__target_intrinsic(glsl, "subgroupOr($0)")
__target_intrinsic(hlsl)
T WaveActiveBitOr(T expr)
{
    return WaveMaskBitOr(WaveGetActiveMask(), expr);
}

__generic<T : __BuiltinIntegerType, let N : int>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
__target_intrinsic(glsl, "subgroupOr($0)")
__target_intrinsic(hlsl)
vector<T,N> WaveActiveBitOr(vector<T,N> expr)
{
    return WaveMaskBitOr(WaveGetActiveMask(), expr);
}

__generic<T : __BuiltinIntegerType, let N : int, let M : int>
__target_intrinsic(hlsl)
matrix<T, N, M> WaveActiveBitOr(matrix<T, N, M> expr)
{
    return WaveMaskBitOr(WaveGetActiveMask(), expr);
}

__generic<T : __BuiltinIntegerType>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
__target_intrinsic(glsl, "subgroupXor($0)")
__target_intrinsic(hlsl)
T WaveActiveBitXor(T expr)
{
    return WaveMaskBitXor(WaveGetActiveMask(), expr);
}

__generic<T : __BuiltinIntegerType, let N : int> 
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
__target_intrinsic(glsl, "subgroupXor($0)")
__target_intrinsic(hlsl)
vector<T,N> WaveActiveBitXor(vector<T,N> expr)
{
    return WaveMaskBitXor(WaveGetActiveMask(), expr);
}

__generic<T : __BuiltinIntegerType, let N : int, let M : int>
__target_intrinsic(hlsl)
matrix<T, N, M> WaveActiveBitXor(matrix<T, N, M> expr)
{
    return WaveMaskBitXor(WaveGetActiveMask(), expr);
}

__generic<T : __BuiltinArithmeticType>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
__target_intrinsic(glsl, "subgroupMax($0)")
__target_intrinsic(hlsl)
T WaveActiveMax(T expr)
{
    return WaveMaskMax(WaveGetActiveMask(), expr);
}

__generic<T : __BuiltinArithmeticType, let N : int>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
__target_intrinsic(glsl, "subgroupMax($0)")
__target_intrinsic(hlsl)
vector<T, N> WaveActiveMax(vector<T, N> expr)
{
    return WaveMaskMax(WaveGetActiveMask(), expr);
}

__generic<T : __BuiltinArithmeticType, let N : int, let M : int>
__target_intrinsic(hlsl)
matrix<T, N, M> WaveActiveMax(matrix<T, N, M> expr)
{
    return WaveMaskMax(WaveGetActiveMask(), expr);
}

__generic<T : __BuiltinArithmeticType>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
__target_intrinsic(glsl, "subgroupMin($0)")
__target_intrinsic(hlsl)
T WaveActiveMin(T expr)
{
    return WaveMaskMin(WaveGetActiveMask(), expr);
}

__generic<T : __BuiltinArithmeticType, let N : int>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
__target_intrinsic(glsl, "subgroupMin($0)")
__target_intrinsic(hlsl)
vector<T, N> WaveActiveMin(vector<T, N> expr)
{
    return WaveMaskMin(WaveGetActiveMask(), expr);
}

__generic<T : __BuiltinArithmeticType, let N : int, let M : int>
__target_intrinsic(hlsl)
matrix<T, N, M> WaveActiveMin(matrix<T, N, M> expr)
{
    return WaveMaskMin(WaveGetActiveMask(), expr);
}

__generic<T : __BuiltinArithmeticType>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
__target_intrinsic(glsl, "subgroupMul($0)")
__target_intrinsic(hlsl)
T WaveActiveProduct(T expr)
{
    return WaveMaskProduct(WaveGetActiveMask(), expr);
}

__generic<T : __BuiltinArithmeticType, let N : int>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
__target_intrinsic(glsl, "subgroupMul($0)")
__target_intrinsic(hlsl)
vector<T,N> WaveActiveProduct(vector<T,N> expr)
{
    return WaveMaskProduct(WaveGetActiveMask(), expr);
}

__generic<T : __BuiltinArithmeticType, let N : int, let M : int>
__target_intrinsic(hlsl)
matrix<T, N, M> WaveActiveProduct(matrix<T, N, M> expr)
{
    return WaveMaskProduct(WaveGetActiveMask(), expr);
}

__generic<T : __BuiltinArithmeticType>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
__target_intrinsic(glsl, "subgroupAdd($0)")
__target_intrinsic(hlsl)
T WaveActiveSum(T expr)
{
    return WaveMaskSum(WaveGetActiveMask(), expr);
}

__generic<T : __BuiltinArithmeticType, let N : int>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
__target_intrinsic(glsl, "subgroupAdd($0)")
__target_intrinsic(hlsl)
vector<T,N> WaveActiveSum(vector<T,N> expr)
{
    return WaveMaskSum(WaveGetActiveMask(), expr);
}

__generic<T : __BuiltinArithmeticType, let N : int, let M : int>
__target_intrinsic(hlsl)
matrix<T,N,M> WaveActiveSum(matrix<T,N,M> expr)
{
    return WaveMaskSum(WaveGetActiveMask(), expr);
}

__generic<T : __BuiltinType>
__glsl_extension(GL_KHR_shader_subgroup_vote)
__spirv_version(1.3)
__target_intrinsic(glsl, "subgroupAllEqual($0)")
__target_intrinsic(hlsl)
bool WaveActiveAllEqual(T value)
{
    return WaveMaskAllEqual(WaveGetActiveMask(), value);
}

__generic<T : __BuiltinType, let N : int> 
__glsl_extension(GL_KHR_shader_subgroup_vote)
__spirv_version(1.3)
__target_intrinsic(glsl, "subgroupAllEqual($0)")
__target_intrinsic(hlsl)
bool WaveActiveAllEqual(vector<T,N> value)
{
    return WaveMaskAllEqual(WaveGetActiveMask(), value);
}

__generic<T : __BuiltinType, let N : int, let M : int>
__target_intrinsic(hlsl)
bool WaveActiveAllEqual(matrix<T, N, M> value)
{
    return WaveMaskAllEqual(WaveGetActiveMask(), value);
}

__glsl_extension(GL_KHR_shader_subgroup_vote)
__spirv_version(1.3)
__target_intrinsic(glsl, "subgroupAll($0)") 
__target_intrinsic(hlsl)
bool WaveActiveAllTrue(bool condition)
{
    return WaveMaskAllTrue(WaveGetActiveMask(), condition);
}

__glsl_extension(GL_KHR_shader_subgroup_vote)
__spirv_version(1.3)
__target_intrinsic(glsl, "subgroupAny($0)") 
__target_intrinsic(hlsl)
bool WaveActiveAnyTrue(bool condition)
{
    return WaveMaskAnyTrue(WaveGetActiveMask(), condition);
}

__glsl_extension(GL_KHR_shader_subgroup_ballot)
__spirv_version(1.3)
__target_intrinsic(glsl, "subgroupBallot($0)")
__target_intrinsic(hlsl)
uint4 WaveActiveBallot(bool condition)
{
    return WaveMaskBallot(WaveGetActiveMask(), condition);
}

__target_intrinsic(hlsl)
uint WaveActiveCountBits(bool value)
{
    return WaveMaskCountBits(WaveGetActiveMask(), value);
}

__glsl_extension(GL_KHR_shader_subgroup_basic)
__spirv_version(1.3)
__target_intrinsic(glsl, "(gl_SubgroupSize)")
__target_intrinsic(cuda, "(warpSize)")
uint WaveGetLaneCount();

__glsl_extension(GL_KHR_shader_subgroup_basic)
__spirv_version(1.3)
__target_intrinsic(glsl, "(gl_SubgroupInvocationID)")
__target_intrinsic(cuda, "_getLaneId()")
uint WaveGetLaneIndex();

__glsl_extension(GL_KHR_shader_subgroup_basic)
__spirv_version(1.3)
__target_intrinsic(glsl, "subgroupElect()")
__target_intrinsic(hlsl)
bool WaveIsFirstLane()
{
    return WaveMaskIsFirstLane(WaveGetActiveMask());
}

// It's useful to have a wave uint4 version of countbits, because some wave functions return uint4.
// This implementation tries to limit the amount of work required by the actual lane count.
uint _WaveCountBits(uint4 value)
{
    // Assume since WaveGetLaneCount should be known at compile time, the branches will hopefully boil away
    const uint waveLaneCount = WaveGetLaneCount();
    switch ((waveLaneCount - 1) / 32)
    {
        default:
        case 0: return countbits(value.x); 
        case 1: return countbits(value.x) + countbits(value.y); 
        case 2: return countbits(value.x) + countbits(value.y) + countbits(value.z); 
        case 3: return countbits(value.x) + countbits(value.y) + countbits(value.z) + countbits(value.w);
    }
}


// Prefix

__generic<T : __BuiltinArithmeticType>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
__target_intrinsic(glsl, "subgroupExclusiveMul($0)")
__target_intrinsic(hlsl)
T WavePrefixProduct(T expr)
{
    return WaveMaskPrefixProduct(WaveGetActiveMask(), expr);
}


__generic<T : __BuiltinArithmeticType, let N : int>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
__target_intrinsic(glsl, "subgroupExclusiveMul($0)")
__target_intrinsic(hlsl)
vector<T,N> WavePrefixProduct(vector<T,N> expr)
{
    return WaveMaskPrefixProduct(WaveGetActiveMask(), expr);
}

__generic<T : __BuiltinArithmeticType, let N : int, let M : int>
__target_intrinsic(hlsl)
matrix<T, N, M> WavePrefixProduct(matrix<T, N, M> expr)
{
    return WaveMaskPrefixProduct(WaveGetActiveMask(), expr);
}

__generic<T : __BuiltinArithmeticType>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
__target_intrinsic(glsl, "subgroupExclusiveAdd($0)")
__target_intrinsic(hlsl)
T WavePrefixSum(T expr)
{
    return WaveMaskPrefixSum(WaveGetActiveMask(), expr);
}

__generic<T : __BuiltinArithmeticType, let N : int>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
__target_intrinsic(glsl, "subgroupExclusiveAdd($0)")
__target_intrinsic(hlsl)
vector<T,N> WavePrefixSum(vector<T,N> expr)
{
    return WaveMaskPrefixSum(WaveGetActiveMask(), expr);
}

__generic<T : __BuiltinArithmeticType, let N : int, let M : int>
__target_intrinsic(hlsl)
matrix<T,N,M> WavePrefixSum(matrix<T,N,M> expr)
{
    return WaveMaskPrefixSum(WaveGetActiveMask(), expr);
}

__generic<T : __BuiltinType>
__glsl_extension(GL_KHR_shader_subgroup_ballot)
__spirv_version(1.3)
__target_intrinsic(glsl, "subgroupBroadcastFirst($0)")
__target_intrinsic(hlsl)
T WaveReadLaneFirst(T expr)
{
    return WaveMaskReadLaneFirst(WaveGetActiveMask(), expr);
}

__generic<T : __BuiltinType, let N : int>
__glsl_extension(GL_KHR_shader_subgroup_ballot)
__spirv_version(1.3)
__target_intrinsic(glsl, "subgroupBroadcastFirst($0)")
__target_intrinsic(hlsl)
vector<T,N> WaveReadLaneFirst(vector<T,N> expr)
{
    return WaveMaskReadLaneFirst(WaveGetActiveMask(), expr);
}

__generic<T : __BuiltinType, let N : int, let M : int>
__target_intrinsic(hlsl)
matrix<T,N,M> WaveReadLaneFirst(matrix<T,N,M> expr)
{
    return WaveMaskReadLaneFirst(WaveGetActiveMask(), expr);
}

// NOTE! WaveBroadcastLaneAt is *NOT* standard HLSL
// It is provided as access to subgroupBroadcast which can only take a
// constexpr laneId.
// https://github.com/KhronosGroup/GLSL/blob/master/extensions/khr/GL_KHR_shader_subgroup.txt
// Versions SPIR-V greater than 1.4 loosen this restriction, and allow 'dynamic uniform' index
// If that's the behavior required then client code should use WaveReadLaneAt which works this way.
__generic<T : __BuiltinType>
__glsl_extension(GL_KHR_shader_subgroup_ballot)
__spirv_version(1.3)
__target_intrinsic(glsl, "subgroupBroadcast($0, $1)")
__target_intrinsic(hlsl, "WaveReadLaneAt")
T WaveBroadcastLaneAt(T value, constexpr int lane)
{
    return WaveMaskBroadcastLaneAt(WaveGetActiveMask(), value, lane);
}

__generic<T : __BuiltinType, let N : int>
__glsl_extension(GL_KHR_shader_subgroup_ballot)
__spirv_version(1.3)
__target_intrinsic(glsl, "subgroupBroadcast($0, $1)")
__target_intrinsic(hlsl, "WaveReadLaneAt")
vector<T,N> WaveBroadcastLaneAt(vector<T,N> value, constexpr int lane)
{
    return WaveMaskBroadcastLaneAt(WaveGetActiveMask(), value, lane);
}

__generic<T : __BuiltinType, let N : int, let M : int>
__target_intrinsic(cuda, "_waveShuffleMultiple(_getActiveMask(), $0, $1)")
__target_intrinsic(hlsl, "WaveReadLaneAt")
matrix<T, N, M> WaveBroadcastLaneAt(matrix<T, N, M> value, constexpr int lane)
{
    return WaveMaskBroadcastLaneAt(WaveGetActiveMask(), value, lane);
}

// TODO(JS): If it can be determines that the `laneId` is constExpr, then subgroupBroadcast
// could be used on GLSL. For now we just use subgroupShuffle
__generic<T : __BuiltinType>
__glsl_extension(GL_KHR_shader_subgroup_shuffle)
__spirv_version(1.3)
__target_intrinsic(glsl, "subgroupShuffle($0, $1)")
__target_intrinsic(hlsl)
T WaveReadLaneAt(T value, int lane)
{
    return WaveMaskReadLaneAt(WaveGetActiveMask(), value, lane);
}

__generic<T : __BuiltinType, let N : int>
__spirv_version(1.3)
__glsl_extension(GL_KHR_shader_subgroup_shuffle)
__target_intrinsic(glsl, "subgroupShuffle($0, $1)")
__target_intrinsic(hlsl)
vector<T,N> WaveReadLaneAt(vector<T,N> value, int lane)
{
    return WaveMaskReadLaneAt(WaveGetActiveMask(), value, lane);
}

__generic<T : __BuiltinType, let N : int, let M : int>
__target_intrinsic(cuda, "_waveShuffleMultiple(_getActiveMask(), $0, $1)")
__target_intrinsic(hlsl)
matrix<T, N, M> WaveReadLaneAt(matrix<T, N, M> value, int lane)
{
    return WaveMaskReadLaneAt(WaveGetActiveMask(), value, lane);
}

// NOTE! WaveShuffle is a NON STANDARD HLSL intrinsic! It will map to WaveReadLaneAt on HLSL
// which means it will only work on hardware which allows arbitrary laneIds which is not true
// in general because it breaks the HLSL standard, which requires it's 'dynamically uniform' across the Wave.
__generic<T : __BuiltinType>
__glsl_extension(GL_KHR_shader_subgroup_shuffle)
__spirv_version(1.3)
__target_intrinsic(glsl, "subgroupShuffle($0, $1)")
__target_intrinsic(hlsl, "WaveReadLaneAt")
T WaveShuffle(T value, int lane)
{
    return WaveMaskShuffle(WaveGetActiveMask(), value, lane);
}

__generic<T : __BuiltinType, let N : int>
__glsl_extension(GL_KHR_shader_subgroup_shuffle)
__spirv_version(1.3)
__target_intrinsic(glsl, "subgroupShuffle($0, $1)")
__target_intrinsic(hlsl, "WaveReadLaneAt")
vector<T,N> WaveShuffle(vector<T,N> value, int lane)
{
    return WaveMaskShuffle(WaveGetActiveMask(), value, lane);
}

__generic<T : __BuiltinType, let N : int, let M : int>
__target_intrinsic(hlsl, "WaveReadLaneAt")
matrix<T, N, M> WaveShuffle(matrix<T, N, M> value, int lane)
{
    return WaveMaskShuffle(WaveGetActiveMask(), value, lane);
}

__glsl_extension(GL_KHR_shader_subgroup_ballot)
__spirv_version(1.3)
__target_intrinsic(glsl, "subgroupBallotExclusiveBitCount(subgroupBallot($0))")
__target_intrinsic(hlsl)
uint WavePrefixCountBits(bool value)
{
    return WaveMaskPrefixCountBits(WaveGetActiveMask(), value);
}

__glsl_extension(GL_KHR_shader_subgroup_ballot)
__spirv_version(1.3)
__target_intrinsic(glsl, "subgroupBallot(true)")
__target_intrinsic(cuda, "make_uint4(__activemask(), 0, 0, 0)")
__target_intrinsic(hlsl, "WaveActiveBallot(true)")
uint4 WaveGetConvergedMulti();

__glsl_extension(GL_KHR_shader_subgroup_ballot)
__spirv_version(1.3)
__target_intrinsic(glsl, "subgroupBallot(true)")
__target_intrinsic(hlsl, "WaveActiveBallot(true)")
uint4 WaveGetActiveMulti();

// Shader model 6.5 stuff
// https://github.com/microsoft/DirectX-Specs/blob/master/d3d/HLSL_ShaderModel6_5.md

__generic<T : __BuiltinType>
__target_intrinsic(hlsl)
uint4 WaveMatch(T value)
{
    return WaveMaskMatch(WaveGetActiveMask(), value);
}

__generic<T : __BuiltinType, let N : int>
__target_intrinsic(hlsl)
uint4 WaveMatch(vector<T,N> value)
{
    return WaveMaskMatch(WaveGetActiveMask(), value);
}

__generic<T : __BuiltinType, let N : int, let M : int>
__target_intrinsic(hlsl)
uint4 WaveMatch(matrix<T,N,M> value)
{
    return WaveMaskMatch(WaveGetActiveMask(), value);
}

__target_intrinsic(hlsl)
__target_intrinsic(cuda, "_popc(__ballot_sync(($1).x, $0) & _getLaneLtMask())")
uint WaveMultiPrefixCountBits(bool value, uint4 mask);

__generic<T : __BuiltinArithmeticType>
__target_intrinsic(hlsl)
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
__target_intrinsic(glsl, "subgroupExclusiveAnd($0)")
__target_intrinsic(cuda, "_wavePrefixAnd(_getMultiPrefixMask(($1).x), $0)")
T WaveMultiPrefixBitAnd(T expr, uint4 mask);

__target_intrinsic(hlsl)
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
__target_intrinsic(glsl, "subgroupExclusiveAnd($0)")
__target_intrinsic(cuda, "_wavePrefixAndMultiple(_getMultiPrefixMask(($1).x), $0)")
__generic<T : __BuiltinArithmeticType, let N : int>
vector<T,N> WaveMultiPrefixBitAnd(vector<T,N> expr, uint4 mask);

__generic<T : __BuiltinArithmeticType, let N : int, let M : int>
__target_intrinsic(hlsl)
__target_intrinsic(cuda, "_wavePrefixAndMultiple(_getMultiPrefixMask(($1).x), $0)")
matrix<T,N,M> WaveMultiPrefixBitAnd(matrix<T,N,M> expr, uint4 mask);

__generic<T : __BuiltinArithmeticType>
__target_intrinsic(hlsl)
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
//__target_intrinsic(glsl, "subgroupExclusiveOr($0)")
__target_intrinsic(cuda, "_wavePrefixOr(, _getMultiPrefixMask(($1).x), $0)")
T WaveMultiPrefixBitOr(T expr, uint4 mask);

__generic<T : __BuiltinArithmeticType, let N : int>
__target_intrinsic(hlsl)
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
//__target_intrinsic(glsl, "subgroupExclusiveOr($0)")
__target_intrinsic(cuda, "_wavePrefixOrMultiple(_getMultiPrefixMask(($1).x), $0)")
vector<T,N> WaveMultiPrefixBitOr(vector<T,N> expr, uint4 mask);

__generic<T : __BuiltinArithmeticType, let N : int, let M : int>
__target_intrinsic(hlsl)
__target_intrinsic(cuda, "_wavePrefixOrMultiple(_getMultiPrefixMask(($1).x), $0)")
matrix<T,N,M> WaveMultiPrefixBitOr(matrix<T,N,M> expr, uint4 mask);

__generic<T : __BuiltinArithmeticType>
__target_intrinsic(hlsl)
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
__target_intrinsic(glsl, "subgroupExclusiveXor($0)")
__target_intrinsic(cuda, "_wavePrefixXor(_getMultiPrefixMask(($1).x), $0)")
T WaveMultiPrefixBitXor(T expr, uint4 mask);

__generic<T : __BuiltinArithmeticType, let N : int>
__target_intrinsic(hlsl)
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
__target_intrinsic(glsl, "subgroupExclusiveXor($0)")
__target_intrinsic(cuda, "_wavePrefixXorMultiple(_getMultiPrefixMask(($1).x), $0)")
vector<T,N> WaveMultiPrefixBitXor(vector<T,N> expr, uint4 mask);

__generic<T : __BuiltinArithmeticType, let N : int, let M : int>
__target_intrinsic(hlsl)
__target_intrinsic(cuda, "_wavePrefixXorMultiple(_getMultiPrefixMask(($1).x), $0)")
matrix<T,N,M> WaveMultiPrefixBitXor(matrix<T,N,M> expr, uint4 mask);

__generic<T : __BuiltinArithmeticType>
__target_intrinsic(hlsl)
__target_intrinsic(cuda, "_wavePrefixProduct(_getMultiPrefixMask(($1).x), $0)")
T WaveMultiPrefixProduct(T value, uint4 mask);

__generic<T : __BuiltinArithmeticType, let N : int> 
__target_intrinsic(hlsl)
__target_intrinsic(cuda, "_wavePrefixProductMultiple(_getMultiPrefixMask(($1).x), $0)")
vector<T,N> WaveMultiPrefixProduct(vector<T,N> value, uint4 mask);

__generic<T : __BuiltinArithmeticType, let N : int, let M : int>
__target_intrinsic(hlsl)
__target_intrinsic(cuda, "_wavePrefixProductMultiple(_getMultiPrefixMask(($1).x), $0)")
matrix<T,N,M> WaveMultiPrefixProduct(matrix<T,N,M> value, uint4 mask);

__generic<T : __BuiltinArithmeticType>
__target_intrinsic(hlsl)
__target_intrinsic(cuda, "_wavePrefixSum(_getMultiPrefixMask(($1).x), $0)")
T WaveMultiPrefixSum(T value, uint4 mask);

__generic<T : __BuiltinArithmeticType, let N : int>
__target_intrinsic(hlsl)
__target_intrinsic(cuda, "_wavePrefixSumMultiple(_getMultiPrefixMask(($1).x), $0 )")
vector<T,N> WaveMultiPrefixSum(vector<T,N> value, uint4 mask);

__generic<T : __BuiltinArithmeticType, let N : int, let M : int>
__target_intrinsic(hlsl)
__target_intrinsic(cuda, "_wavePrefixSumMultiple(_getMultiPrefixMask(($1).x), $0)")
matrix<T,N,M> WaveMultiPrefixSum(matrix<T,N,M> value, uint4 mask);

// `typedef`s to help with the fact that HLSL has been sorta-kinda case insensitive at various points
typedef Texture2D texture2D;



${{{{

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
    auto access = kBaseBufferAccessLevels[aa].access;
    auto flavor = TextureFlavor::create(TextureFlavor::Shape::ShapeBuffer, access).flavor;
    sb << "__generic<T>\n";
    sb << "__magic_type(Texture," << int(flavor) << ")\n";
    sb << "__intrinsic_type(" << (kIROp_TextureType + (int(flavor) << kIROpMeta_OtherShift)) << ")\n";
    sb << "struct ";
    sb << kBaseBufferAccessLevels[aa].name;
    sb << "Buffer {\n";

    sb << "void GetDimensions(out uint dim);\n";

    char const* glslLoadFuncName = (access == SLANG_RESOURCE_ACCESS_READ) ? "texelFetch" : "imageLoad";

    sb << "__glsl_extension(GL_EXT_samplerless_texture_functions)";
    sb << "__target_intrinsic(glsl, \"" << glslLoadFuncName << "($0, $1)$z\")\n";
    sb << "T Load(int location);\n";

    sb << "T Load(int location, out uint status);\n";

    sb << "__subscript(uint index) -> T {\n";

    sb << "__glsl_extension(GL_EXT_samplerless_texture_functions)";
    sb << "__target_intrinsic(glsl, \"" << glslLoadFuncName << "($0, int($1))$z\") get;\n";

    if (access != SLANG_RESOURCE_ACCESS_READ)
    {
        sb << "__target_intrinsic(glsl, \"imageStore($0, int($1), $V2)\") [nonmutating] set;\n";

        sb << "__intrinsic_op(" << int(kIROp_ImageSubscript) << ") ref;\n";
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
static const RAY_FLAG RAY_FLAG_SKIP_TRIANGLES                   = 0x100;
static const RAY_FLAG RAY_FLAG_SKIP_PROCEDURAL_PRIMITIVES       = 0x200;

// 10.1.2 - Ray Description Structure

__target_intrinsic(hlsl, RayDesc)
__target_intrinsic(cuda, RayDesc)
struct RayDesc
{
    __target_intrinsic(hlsl, Origin)
    __target_intrinsic(cuda, Origin)
    float3 Origin;

    __target_intrinsic(hlsl, TMin)
    __target_intrinsic(cuda, TMin)
    float  TMin;

    __target_intrinsic(hlsl, Direction)
    __target_intrinsic(cuda, Direction)
    float3 Direction;

    __target_intrinsic(hlsl, TMax)
    __target_intrinsic(cuda, TMax)
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

__target_intrinsic(hlsl)
void CallShader<Payload>(uint shaderIndex, inout Payload payload);

// `executeCallableNV` is the GLSL intrinsic that will be used to implement
// `CallShader()` for GLSL-based targets.
//
__target_intrinsic(GL_NV_ray_tracing, "executeCallableNV")
__target_intrinsic(GL_EXT_ray_tracing, "executeCallableEXT")
void __executeCallable(uint shaderIndex, int payloadLocation);

// Next is the custom intrinsic that will compute the payload location
// for a type being used in a `CallShader()` call for GLSL-based targets.
//
__generic<Payload>
__target_intrinsic(__glslRayTracing, "$XC")
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
    [__vulkanCallablePayload]
    static Payload p;

    p = payload;
    __executeCallable(shaderIndex, __callablePayloadLocation(p));
    payload = p;
}

// 10.3.2
__target_intrinsic(hlsl)
__target_intrinsic(cuda, "traceOptiXRay")
void TraceRay<payload_t>(
    RaytracingAccelerationStructure AccelerationStructure,
    uint                            RayFlags,
    uint                            InstanceInclusionMask,
    uint                            RayContributionToHitGroupIndex,
    uint                            MultiplierForGeometryContributionToHitGroupIndex,
    uint                            MissShaderIndex,
    RayDesc                         Ray,
    inout payload_t                 Payload);

__target_intrinsic(GL_NV_ray_tracing, "traceNV")
__target_intrinsic(GL_EXT_ray_tracing, "traceRayEXT")
void __traceRay(
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
__target_intrinsic(__glslRayTracing, "$XP")
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
    __traceRay(
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


// NOTE!
// The name of the following functions may change when DXR supports
// a feature similar to the `GL_NV_ray_tracing_motion_blur` extension
//
// https://github.com/KhronosGroup/GLSL/blob/master/extensions/nv/GLSL_NV_ray_tracing_motion_blur.txt

void TraceMotionRay<payload_t>(
    RaytracingAccelerationStructure AccelerationStructure,
    uint                            RayFlags,
    uint                            InstanceInclusionMask,
    uint                            RayContributionToHitGroupIndex,
    uint                            MultiplierForGeometryContributionToHitGroupIndex,
    uint                            MissShaderIndex,
    RayDesc                         Ray,
    float                           CurrentTime,
    inout payload_t                 Payload);

__target_intrinsic(glsl, "traceRayMotionNV")
__glsl_version(460)
__glsl_extension(GL_NV_ray_tracing_motion_blur)
__glsl_extension(GL_EXT_ray_tracing)
void __traceMotionRay(
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
    float                           CurrentTime,               
    int                             PayloadLocation);

__generic<payload_t>
__specialized_for_target(glsl)
void TraceMotionRay(
    RaytracingAccelerationStructure AccelerationStructure,
    uint                            RayFlags,
    uint                            InstanceInclusionMask,
    uint                            RayContributionToHitGroupIndex,
    uint                            MultiplierForGeometryContributionToHitGroupIndex,
    uint                            MissShaderIndex,
    RayDesc                         Ray,
    float                           CurrentTime,
    inout payload_t                 Payload)
{
    [__vulkanRayPayload]
    static payload_t p;

    p = Payload;
    __traceMotionRay(
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
        CurrentTime,
        __rayPayloadLocation(p));
    Payload = p;
}

// 10.3.3
__target_intrinsic(hlsl)
bool ReportHit<A>(float tHit, uint hitKind, A attributes);

__target_intrinsic(GL_NV_ray_tracing, "reportIntersectionNV")
__target_intrinsic(GL_EXT_ray_tracing, "reportIntersectionEXT")
bool __reportIntersection(float tHit, uint hitKind);

__generic<A>
__specialized_for_target(glsl)
bool ReportHit(float tHit, uint hitKind, A attributes)
{
    [__vulkanHitAttributes]
    static A a;

    a = attributes;
    return __reportIntersection(tHit, hitKind);
}

// 10.3.4
__target_intrinsic(hlsl)
__target_intrinsic(GL_NV_ray_tracing, ignoreIntersectionNV)
__target_intrinsic(GL_EXT_ray_tracing, "ignoreIntersectionEXT;")
__target_intrinsic(cuda, "optixIgnoreIntersection")
void IgnoreHit();

// 10.3.5
__target_intrinsic(hlsl)
__target_intrinsic(GL_NV_ray_tracing, terminateRayNV)
__target_intrinsic(GL_EXT_ray_tracing, "terminateRayEXT;")
__target_intrinsic(cuda, "optixTerminateRay")
void AcceptHitAndEndSearch();

// 10.4 - System Values and Special Semantics

// TODO: Many of these functions need to be restricted so that
// they can only be accessed from specific stages.

// 10.4.1 - Ray Dispatch System Values

__target_intrinsic(GL_NV_ray_tracing, "(gl_LaunchIDNV)")
__target_intrinsic(GL_EXT_ray_tracing, "(gl_LaunchIDEXT)")
__target_intrinsic(cuda, "optixGetLaunchIndex")
uint3 DispatchRaysIndex();

__target_intrinsic(GL_NV_ray_tracing, "(gl_LaunchSizeNV)")
__target_intrinsic(GL_EXT_ray_tracing, "(gl_LaunchSizeEXT)")
__target_intrinsic(cuda, "optixGetLaunchDimensions")
uint3 DispatchRaysDimensions();

// 10.4.2 - Ray System Values

__target_intrinsic(GL_NV_ray_tracing, "(gl_WorldRayOriginNV)")
__target_intrinsic(GL_EXT_ray_tracing, "(gl_WorldRayOriginEXT)")
__target_intrinsic(cuda, "optixGetWorldRayOrigin")
float3 WorldRayOrigin();

__target_intrinsic(GL_NV_ray_tracing, "(gl_WorldRayDirectionNV)")
__target_intrinsic(GL_EXT_ray_tracing, "(gl_WorldRayDirectionEXT)")
__target_intrinsic(cuda, "optixGetWorldRayDirection")
float3 WorldRayDirection();

__target_intrinsic(GL_NV_ray_tracing, "(gl_RayTminNV)")
__target_intrinsic(GL_EXT_ray_tracing, "(gl_RayTminEXT)")
__target_intrinsic(cuda, "optixGetRayTmin")
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
__target_intrinsic(GL_NV_ray_tracing, "(gl_RayTmaxNV)")
__target_intrinsic(GL_EXT_ray_tracing, "(gl_RayTmaxEXT)")
__target_intrinsic(cuda, "optixGetRayTmax")
float RayTCurrent();

__target_intrinsic(GL_NV_ray_tracing, "(gl_IncomingRayFlagsNV)")
__target_intrinsic(GL_EXT_ray_tracing, "(gl_IncomingRayFlagsEXT)")
__target_intrinsic(cuda, "optixGetRayFlags")
uint RayFlags();

// 10.4.3 - Primitive/Object Space System Values

__target_intrinsic(__glslRayTracing, "(gl_InstanceID)")
__target_intrinsic(cuda, "optixGetInstanceIndex")
uint InstanceIndex();

__target_intrinsic(GL_NV_ray_tracing, "(gl_InstanceCustomIndexNV)")
__target_intrinsic(GL_EXT_ray_tracing, "(gl_InstanceCustomIndexEXT)")
__target_intrinsic(cuda, "optixGetInstanceId")
uint InstanceID();

__target_intrinsic(__glslRayTracing, "(gl_PrimitiveID)")
__target_intrinsic(cuda, "optixGetPrimitiveIndex")
uint PrimitiveIndex();

__target_intrinsic(GL_NV_ray_tracing, "(gl_ObjectRayOriginNV)")
__target_intrinsic(GL_EXT_ray_tracing, "(gl_ObjectRayOriginEXT)")
__target_intrinsic(cuda, "optixGetObjectRayOrigin")
float3 ObjectRayOrigin();

__target_intrinsic(GL_NV_ray_tracing, "(gl_ObjectRayDirectionNV)")
__target_intrinsic(GL_EXT_ray_tracing, "(gl_ObjectRayDirectionEXT)")
__target_intrinsic(cuda, "optixGetObjectRayDirection")
float3 ObjectRayDirection();

// TODO: optix has an optixGetObjectToWorldTransformMatrix function that returns 12
// floats by reference. 
__target_intrinsic(GL_NV_ray_tracing, "transpose(gl_ObjectToWorldNV)")
__target_intrinsic(GL_EXT_ray_tracing, "transpose(gl_ObjectToWorldEXT)")
float3x4 ObjectToWorld3x4();

__target_intrinsic(GL_NV_ray_tracing, "transpose(gl_WorldToObjectNV)")
__target_intrinsic(GL_EXT_ray_tracing, "transpose(gl_WorldToObjectEXT)")
float3x4 WorldToObject3x4();

__target_intrinsic(GL_NV_ray_tracing, "(gl_ObjectToWorldNV)")
__target_intrinsic(GL_EXT_ray_tracing, "(gl_ObjectToWorld3x4EXT)")
float4x3 ObjectToWorld4x3();

__target_intrinsic(GL_NV_ray_tracing, "(gl_WorldToObjectNV)")
__target_intrinsic(GL_EXT_ray_tracing, "(gl_WorldToObject3x4EXT)")
float4x3 WorldToObject4x3();

// NOTE!
// The name of the following functions may change when DXR supports
// a feature similar to the `GL_NV_ray_tracing_motion_blur` extension

__target_intrinsic(glsl, "(gl_CurrentRayTimeNV)")
__glsl_version(460)
__glsl_extension(GL_NV_ray_tracing_motion_blur)
__glsl_extension(GL_EXT_ray_tracing)
float RayCurrentTime();

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
__target_intrinsic(GL_NV_ray_tracing, "(gl_HitKindNV)")
__target_intrinsic(GL_EXT_ray_tracing, "(gl_HitKindEXT)")
__target_intrinsic(cuda, "optixGetHitKind")
uint HitKind();

// Pre-defined hit kinds (not documented explicitly)
static const uint HIT_KIND_TRIANGLE_FRONT_FACE  = 254;
static const uint HIT_KIND_TRIANGLE_BACK_FACE   = 255;

//
// Shader Model 6.4
//

// Treats `left` and `right` as 4-component vectors of `UInt8` and computes `dot(left, right) + acc`
uint dot4add_u8packed(uint left, uint right, uint acc);

// Treats `left` and `right` as 4-component vectors of `Int8` and computes `dot(left, right) + acc`
int dot4add_i8packed(uint left, uint right, int acc);

// Computes `dot(left, right) + acc`.
//
// May not produce infinities or NaNs for intermediate results that overflow the range of `half`
float dot2add(float2 left, float2 right, float acc);

//
// Shader Model 6.5
//

//
// Mesh Shaders
//

// Set the number of output vertices and primitives for a mesh shader invocation.
void SetMeshOutputCounts(uint vertexCount, uint primitiveCount);

// Specify the number of downstream mesh shader thread groups to invoke from an amplification shader,
// and provide the values for per-mesh payload parameters.
//
void DispatchMesh<P>(uint threadGroupCountX, uint threadGroupCountY, uint threadGroupCountZ, P meshPayload);

//
// "Sampler feedback" types `FeedbackTexture2D` and `FeedbackTexture2DArray`.
//

// https://microsoft.github.io/DirectX-Specs/d3d/SamplerFeedback.html

// The docs describe these as 'types' but their syntax makes them seem enum like, and enum is a simpler way to implement them
// But slang enums are always 'enum class like', so I use an empty struct type here

[sealed]
[builtin]
interface __BuiltinSamplerFeedbackType {};

[sealed]
__magic_type(FeedbackType, $(int(FeedbackType::Kind::MinMip)))
__target_intrinsic(hlsl, SAMPLER_FEEDBACK_MIN_MIP)
struct SAMPLER_FEEDBACK_MIN_MIP : __BuiltinSamplerFeedbackType {};

[sealed]
__magic_type(FeedbackType, $(int(FeedbackType::Kind::MipRegionUsed)))
__target_intrinsic(hlsl, SAMPLER_FEEDBACK_MIP_REGION_USED)
struct SAMPLER_FEEDBACK_MIP_REGION_USED : __BuiltinSamplerFeedbackType {};

// All of these objects are write-only resources that point to a special kind of unordered access view meant for sampler feedback.

// Calculate the flavor constants
${{{{
static const int feedbackTexture2DFlavor = int(TextureFlavor::create(TextureFlavor::Shape::Shape2D, SLANG_RESOURCE_ACCESS_WRITE, SLANG_TEXTURE_FEEDBACK_FLAG).flavor);
static const int feedbackTexture2DArrayFlavor = int(TextureFlavor::create(TextureFlavor::Shape::Shape2D, SLANG_RESOURCE_ACCESS_WRITE, SLANG_TEXTURE_FEEDBACK_FLAG | SLANG_TEXTURE_ARRAY_FLAG).flavor);
}}}}

__magic_type(Texture, $(feedbackTexture2DFlavor))
__intrinsic_type($(kIROp_TextureType + (feedbackTexture2DFlavor << kIROpMeta_OtherShift)))
struct FeedbackTexture2D<T : __BuiltinSamplerFeedbackType>
{
    __target_intrinsic
    void GetDimensions(out uint width, out uint height);

    __target_intrinsic
    void GetDimensions(uint mipLevel, out uint width, out uint height, out uint numberOfLevels);

    __target_intrinsic
    void GetDimensions(out float width,out float height);

    __target_intrinsic
    void GetDimensions(uint mipLevel, out float width,out float height, out float numberOfLevels);

    // With Clamp

    __target_intrinsic(hlsl, "($0).WriteSamplerFeedback($1, $2, $3, $4)")
    __target_intrinsic(cpp, "($0).WriteSamplerFeedback($1, $2, $3, $4)")
    void WriteSamplerFeedback<S>(Texture2D<S> tex, SamplerState samp, float2 location, float clamp);

    __target_intrinsic(hlsl, "($0).WriteSamplerFeedbackBias($1, $2, $3, $4, $5)")
    __target_intrinsic(cpp, "($0).WriteSamplerFeedbackBias($1, $2, $3, $4, $5)")
    void WriteSamplerFeedbackBias<S>(Texture2D<S> tex, SamplerState samp, float2 location, float bias, float clamp);

    __target_intrinsic(hlsl, "($0).WriteSamplerFeedbackGrad($1, $2, $3, $4, $5, $6)")
    __target_intrinsic(cpp, "($0).WriteSamplerFeedbackGrad($1, $2, $3, $4, $5, $6)")
    void WriteSamplerFeedbackGrad<S>(Texture2D<S> tex, SamplerState samp, float2 location, float2 ddx, float2 ddy, float clamp);

    // Level

    __target_intrinsic(hlsl, "($0).WriteSamplerFeedbackLevel($1, $2, $3, $4)")
    __target_intrinsic(cpp, "($0).WriteSamplerFeedbackLevel($1, $2, $3, $4)")
    void WriteSamplerFeedbackLevel<S>(Texture2D<S> tex, SamplerState samp, float2 location, float lod);
    
    // Without Clamp

    __target_intrinsic(hlsl, "($0).WriteSamplerFeedback($1, $2, $3)")
    __target_intrinsic(cpp, "($0).WriteSamplerFeedback($1, $2, $3)")
    void WriteSamplerFeedback<S>(Texture2D<S> tex, SamplerState samp, float2 location);

    __target_intrinsic(hlsl, "($0).WriteSamplerFeedbackBias($1, $2, $3, $4)")
    __target_intrinsic(cpp, "($0).WriteSamplerFeedbackBias($1, $2, $3, $4)")
    void WriteSamplerFeedbackBias<S>(Texture2D<S> tex, SamplerState samp, float2 location, float bias);

    __target_intrinsic(hlsl, "($0).WriteSamplerFeedbackGrad($1, $2, $3, $4, $5)")
    __target_intrinsic(cpp, "($0).WriteSamplerFeedbackGrad($1, $2, $3, $4, $5)")
    void WriteSamplerFeedbackGrad<S>(Texture2D<S> tex, SamplerState samp, float2 location, float2 ddx, float2 ddy);
};



__magic_type(Texture, $(feedbackTexture2DArrayFlavor))
__intrinsic_type($(kIROp_TextureType + (feedbackTexture2DArrayFlavor << kIROpMeta_OtherShift)))
struct FeedbackTexture2DArray<T : __BuiltinSamplerFeedbackType>
{
    __target_intrinsic
    void GetDimensions(out uint width,out uint height, out uint elements);

    __target_intrinsic
    void GetDimensions(uint mipLevel, out uint width,out uint height, out uint elements, out uint numberOfLevels);

    __target_intrinsic
    void GetDimensions(out float width,out float height, out float elements);

    __target_intrinsic
    void GetDimensions(uint mipLevel, out float width,out float height, out float elements, out float numberOfLevels);

    // With Clamp

    __target_intrinsic(hlsl, "($0).WriteSamplerFeedback($1, $2, $3, $4)")
    __target_intrinsic(cpp, "($0).WriteSamplerFeedback($1, $2, $3, $4)")
    void WriteSamplerFeedback<S>(Texture2DArray<S> texArray, SamplerState samp, float3 location, float clamp);

    __target_intrinsic(hlsl, "($0).WriteSamplerFeedbackBias($1, $2, $3, $4, $5)")
    __target_intrinsic(cpp, "($0).WriteSamplerFeedbackBias($1, $2, $3, $4, $5)")
    void WriteSamplerFeedbackBias<S>(Texture2DArray<S> texArray, SamplerState samp, float3 location, float bias, float clamp);

    __target_intrinsic(hlsl, "($0).WriteSamplerFeedbackGrad($1, $2, $3, $4, $5, $6)")
    __target_intrinsic(cpp, "($0).WriteSamplerFeedbackGrad($1, $2, $3, $4, $5, $6)")
    void WriteSamplerFeedbackGrad<S>(Texture2DArray<S> texArray, SamplerState samp, float3 location, float3 ddx, float3 ddy, float clamp);

    // Level

    __target_intrinsic(hlsl, "($0).WriteSamplerFeedbackLevel($1, $2, $3, $4)")
    __target_intrinsic(cpp, "($0).WriteSamplerFeedbackLevel($1, $2, $3, $4)")
    void WriteSamplerFeedbackLevel<S>(Texture2DArray<S> texArray, SamplerState samp, float3 location, float lod);

    // Without Clamp

    __target_intrinsic(hlsl, "($0).WriteSamplerFeedback($1, $2, $3)")
    __target_intrinsic(cpp, "($0).WriteSamplerFeedback($1, $2, $3)")
    void WriteSamplerFeedback<S>(Texture2DArray<S> texArray, SamplerState samp, float3 location);

    __target_intrinsic(hlsl, "($0).WriteSamplerFeedbackBias($1, $2, $3, $4)")
    __target_intrinsic(cpp, "($0).WriteSamplerFeedbackBias($1, $2, $3, $4)")
    void WriteSamplerFeedbackBias<S>(Texture2DArray<S> texArray, SamplerState samp, float3 location, float bias);

    __target_intrinsic(hlsl, "($0).WriteSamplerFeedbackGrad($1, $2, $3, $4, $5)")
    __target_intrinsic(cpp, "($0).WriteSamplerFeedbackGrad($1, $2, $3, $4, $5)")
    void WriteSamplerFeedbackGrad<S>(Texture2DArray<S> texArray, SamplerState samp, float3 location, float3 ddx, float3 ddy);
};

//
// DXR 1.1 and `TraceRayInline` support
//

// Get the index of the geometry that was hit in an intersection, any-hit, or closest-hit shader
__target_intrinsic(GL_EXT_ray_tracing, "(gl_GeometryIndexEXT)")
uint GeometryIndex();

// Status of whether a (closest) hit has been committed in a `RayQuery`.
typedef uint COMMITTED_STATUS;

// No hit committed.
static const COMMITTED_STATUS COMMITTED_NOTHING = 0;

// Closest hit is a triangle.
//
// This could be an opaque triangle hit found by the fixed-function
// traversal and intersection implementation, or a non-opaque
// triangle hit committed by user code with `RayQuery.CommitNonOpaqueTriangleHit`
//
static const COMMITTED_STATUS COMMITTED_TRIANGLE_HIT = 1;

// Closest hit is a procedural primitive.
//
// A procedural hit primitive is committed using `RayQuery.CommitProceduralPrimitiveHit`.
static const COMMITTED_STATUS COMMITTED_PROCEDURAL_PRIMITIVE_HIT = 2;

// Type of candidate hit that a `RayQuery` is pausing at.
//
// A `RayQuery` can automatically commit hits with opaque triangles,
// but yields to user code for other hits to allow them to be
// dismissed or committed.
//
typedef uint CANDIDATE_TYPE;

// Candidate hit is a non-opaque triangle.
static const CANDIDATE_TYPE CANDIDATE_NON_OPAQUE_TRIANGLE = 0;

// Candidate hit is a procedural primitive.
static const CANDIDATE_TYPE CANDIDATE_PROCEDURAL_PRIMITIVE = 1;

// Handle to state of an in-progress ray-tracing query.
//
// The ray query is effectively a coroutine that user shader
// code can resume to continue tracing the ray, and which yields
// back to the user code at interesting events along the ray.
//
__target_intrinsic(hlsl, RayQuery)
__target_intrinsic(glsl, rayQueryEXT)
__glsl_extension(GL_EXT_ray_query)
__glsl_version(460)
struct RayQuery <let rayFlagsGeneric : RAY_FLAG = RAY_FLAG_NONE>
{
    // Initialize the query object in a "fresh" state.
    //
    __intrinsic_op($(kIROp_DefaultConstruct))
    __init();

    // Initialize a ray-tracing query.
    //
    // This method may be called on a "fresh" ray query, or
    // on one that is already tracing a ray. In the latter
    // case any state related to the ray previously being
    // traced is overwritten.
    //
    // The `rayFlags` here will be bitwise ORed with
    // the `rayFlags` passed as a generic argument to
    // `RayQuery` to get the effective ray flags, which
    // must obey any API-imposed restrictions.
    //
    __target_intrinsic(hlsl)
    void TraceRayInline(
        RaytracingAccelerationStructure accelerationStructure,
        RAY_FLAG                        rayFlags,
        uint                            instanceInclusionMask,
        RayDesc                         ray);

    __target_intrinsic(glsl, "rayQueryInitializeEXT($0, $1, $2, $3, $4, $5, $6, $7)")
    __glsl_extension(GL_EXT_ray_query)
    __glsl_version(460)
    void __rayQueryInitializeEXT(
        RaytracingAccelerationStructure accelerationStructure,
        RAY_FLAG                        rayFlags,
        uint                            instanceInclusionMask,
        float3                          origin,
        float                           tMin,
        float3                          direction,
        float                           tMax);

    [__unsafeForceInlineEarly]
    __specialized_for_target(glsl)
    void TraceRayInline(
        RaytracingAccelerationStructure accelerationStructure,
        RAY_FLAG                        rayFlags,
        uint                            instanceInclusionMask,
        RayDesc                         ray)
    {
        __rayQueryInitializeEXT(
            accelerationStructure,
            rayFlags | rayFlagsGeneric,
            instanceInclusionMask,
            ray.Origin,
            ray.TMin,
            ray.Direction,
            ray.TMax);
    }

    // Resume the ray query coroutine.
    //
    // If the coroutine suspends because of encountering
    // a candidate hit that cannot be resolved with fixed-funciton
    // logic, this function returns `true`, and the `Candidate*()`
    // functions should be used by application code to resolve
    // the candidate hit (by either committing or ignoring it).
    //
    // If the coroutine terminates because traversal is
    // complete (or has been aborted), this function returns
    // `false`, and application code should use the `Committed*()`
    // functions to appropriately handle the closest hit (it any)
    // that was found.
    //
    __target_intrinsic(glsl, rayQueryProceedEXT)
    __glsl_extension(GL_EXT_ray_query)
    __glsl_version(460)
    bool Proceed();

    // Causes the ray query to terminate.
    //
    // This function cases the ray query to act as if
    // traversal has terminated, so that subsequent
    // `Proceed()` calls will return `false`.
    //
    __target_intrinsic(glsl, rayQueryTerminateEXT)
    __glsl_extension(GL_EXT_ray_query)
    __glsl_version(460)
    void Abort();

    // Get the type of candidate hit being considered.
    //
    // The ray query coroutine will suspend when it encounters
    // a hit that cannot be resolved with fixed-function logic
    // (either a non-opaque triangle or a procedural primitive).
    // In either of those cases, `CandidateType()` will return
    // the kind of candidate hit that must be resolved by
    // user code.
    //
    __target_intrinsic(glsl, "rayQueryGetIntersectionTypeEXT($0, false)")
    __glsl_extension(GL_EXT_ray_query)
    __glsl_version(460)
    CANDIDATE_TYPE CandidateType();

    // Access properties of a candidate hit.

    __target_intrinsic(glsl, "transpose(rayQueryGetIntersectionObjectToWorldEXT($0, false))")
    __glsl_extension(GL_EXT_ray_query)
    __glsl_version(460)
    float3x4 CandidateObjectToWorld3x4();

    __target_intrinsic(glsl, "rayQueryGetIntersectionObjectToWorldEXT($0, false)")
    __glsl_extension(GL_EXT_ray_query)
    __glsl_version(460)
    float4x3 CandidateObjectToWorld4x3();

    __target_intrinsic(glsl, "transpose(rayQueryGetIntersectionWorldToObjectEXT($0, false))")
    __glsl_extension(GL_EXT_ray_query)
    __glsl_version(460)
    float3x4 CandidateWorldToObject3x4();

    __target_intrinsic(glsl, "rayQueryGetIntersectionWorldToObjectEXT($0, false)")
    __glsl_extension(GL_EXT_ray_query)
    __glsl_version(460)
    float4x3 CandidateWorldToObject4x3();

    __target_intrinsic(glsl, "rayQueryGetIntersectionInstanceIdEXT($0, false)")
    __glsl_extension(GL_EXT_ray_query)
    __glsl_version(460)
    uint CandidateInstanceIndex();

    __target_intrinsic(glsl, "rayQueryGetIntersectionInstanceCustomIndexEXT($0, false)")
    __glsl_extension(GL_EXT_ray_query)
    __glsl_version(460)
    uint CandidateInstanceID();

    __target_intrinsic(glsl, "rayQueryGetIntersectionGeometryIndexEXT($0, false)")
    __glsl_extension(GL_EXT_ray_query)
    __glsl_version(460)
    uint CandidateGeometryIndex();

    __target_intrinsic(glsl, "rayQueryGetIntersectionPrimitiveIndexEXT($0, false)")
    __glsl_extension(GL_EXT_ray_query)
    __glsl_version(460)
    uint CandidatePrimitiveIndex();

    __target_intrinsic(glsl, "rayQueryGetIntersectionInstanceShaderBindingTableRecordOffsetEXT($0, false)")
    __glsl_extension(GL_EXT_ray_query)
    __glsl_version(460)
    uint CandidateInstanceContributionToHitGroupIndex();

    // Access properties of the ray being traced
    // in the object space of a candidate hit.

    __target_intrinsic(glsl, "rayQueryGetIntersectionObjectRayOriginEXT($0, false)")
    __glsl_extension(GL_EXT_ray_query)
    __glsl_version(460)
    float3 CandidateObjectRayOrigin();

    __target_intrinsic(glsl, "rayQueryGetIntersectionObjectRayDirectionEXT($0, false)")
    __glsl_extension(GL_EXT_ray_query)
    __glsl_version(460)
    float3 CandidateObjectRayDirection();

    // Access properties of a candidate procedural primitive hit.

    __target_intrinsic(glsl, "rayQueryGetIntersectionCandidateAABBOpaqueEXT($0, false)")
    __glsl_extension(GL_EXT_ray_query)
    __glsl_version(460)
    bool CandidateProceduralPrimitiveNonOpaque();

    // Access properties of a candidate non-opaque triangle hit.

    __target_intrinsic(glsl, "rayQueryGetIntersectionFrontFaceEXT($0, false)")
    __glsl_extension(GL_EXT_ray_query)
    __glsl_version(460)
    bool CandidateTriangleFrontFace();

    __target_intrinsic(glsl, "rayQueryGetIntersectionBarycentricsEXT($0, false)")
    __glsl_extension(GL_EXT_ray_query)
    __glsl_version(460)
    float2 CandidateTriangleBarycentrics();

    __target_intrinsic(glsl, "rayQueryGetIntersectionTEXT($0, false)")
    __glsl_extension(GL_EXT_ray_query)
    __glsl_version(460)
    float CandidateTriangleRayT();

    // Commit the current non-opaque triangle hit.
    __target_intrinsic(glsl, rayQueryConfirmIntersectionEXT)
    __glsl_extension(GL_EXT_ray_query)
    __glsl_version(460)
    void CommitNonOpaqueTriangleHit();

    // Commit the current procedural primitive hit, with hit time `t`.
    __target_intrinsic(glsl, rayQueryGenerateIntersectionEXT)
    __glsl_extension(GL_EXT_ray_query)
    __glsl_version(460)
    void CommitProceduralPrimitiveHit(float t);

    // Get the status of the committed (closest) hit, if any.
    __target_intrinsic(glsl, "rayQueryGetIntersectionTypeEXT($0, true)")
    __glsl_extension(GL_EXT_ray_query)
    __glsl_version(460)
    COMMITTED_STATUS CommittedStatus();

    // Access properties of the committed hit.
    //
    __target_intrinsic(glsl, "transpose(rayQueryGetIntersectionObjectToWorldEXT($0, true))")
    __glsl_extension(GL_EXT_ray_query)
    __glsl_version(460)
    float3x4 CommittedObjectToWorld3x4();

    __target_intrinsic(glsl, "rayQueryGetIntersectionObjectToWorldEXT($0, true)")
    __glsl_extension(GL_EXT_ray_query)
    __glsl_version(460)
    float4x3 CommittedObjectToWorld4x3();

    __target_intrinsic(glsl, "transpose(rayQueryGetIntersectionWorldToObjectEXT($0, true))")
    __glsl_extension(GL_EXT_ray_query)
    __glsl_version(460)
    float3x4 CommittedWorldToObject3x4();

    __target_intrinsic(glsl, "rayQueryGetIntersectionWorldToObjectEXT($0, true)")
    __glsl_extension(GL_EXT_ray_query)
    __glsl_version(460)
    float4x3 CommittedWorldToObject4x3();

    __target_intrinsic(glsl, "rayQueryGetIntersectionTEXT($0, true)")
    __glsl_extension(GL_EXT_ray_query)
    __glsl_version(460)
    float CommittedRayT();

    __target_intrinsic(glsl, "rayQueryGetIntersectionInstanceIdEXT($0, true)")
    __glsl_extension(GL_EXT_ray_query)
    __glsl_version(460)
    uint CommittedInstanceIndex();

    __target_intrinsic(glsl, "rayQueryGetIntersectionInstanceCustomIndexEXT($0, true)")
    __glsl_extension(GL_EXT_ray_query)
    __glsl_version(460)
    uint CommittedInstanceID();

    __target_intrinsic(glsl, "rayQueryGetIntersectionGeometryIndexEXT($0, true)")
    __glsl_extension(GL_EXT_ray_query)
    __glsl_version(460)
    uint CommittedGeometryIndex();

    __target_intrinsic(glsl, "rayQueryGetIntersectionPrimitiveIndexEXT($0, true)")
    __glsl_extension(GL_EXT_ray_query)
    __glsl_version(460)
    uint CommittedPrimitiveIndex();

    __target_intrinsic(glsl, "rayQueryGetIntersectionInstanceShaderBindingTableRecordOffsetEXT($0, true)")
    __glsl_extension(GL_EXT_ray_query)
    __glsl_version(460)
    uint CommittedInstanceContributionToHitGroupIndex();

    // Access properties of the ray being traced
    // in the object space of a committed hit.

    __target_intrinsic(glsl, "rayQueryGetIntersectionObjectRayOriginEXT($0, true)")
    __glsl_extension(GL_EXT_ray_query)
    __glsl_version(460)
    float3 CommittedObjectRayOrigin();

    __target_intrinsic(glsl, "rayQueryGetIntersectionObjectRayDirectionEXT($0, true)")
    __glsl_extension(GL_EXT_ray_query)
    __glsl_version(460)
    float3 CommittedObjectRayDirection();

    // Access properties of a committed triangle hit.

    __target_intrinsic(glsl, "rayQueryGetIntersectionFrontFaceEXT($0, true)")
    __glsl_extension(GL_EXT_ray_query)
    __glsl_version(460)
    bool CommittedTriangleFrontFace();

    __target_intrinsic(glsl, "rayQueryGetIntersectionBarycentricsEXT($0, true)")
    __glsl_extension(GL_EXT_ray_query)
    __glsl_version(460)
    float2 CommittedTriangleBarycentrics();

    // Access properties of the ray being traced.

    __target_intrinsic(glsl, rayQueryGetRayFlagsEXT)
    __glsl_extension(GL_EXT_ray_query)
    __glsl_version(460)
    uint RayFlags();

    __target_intrinsic(glsl, rayQueryGetWorldRayOriginEXT)
    __glsl_extension(GL_EXT_ray_query)
    __glsl_version(460)
    float3 WorldRayOrigin();

    __target_intrinsic(glsl, rayQueryGetWorldRayDirectionEXT)
    __glsl_extension(GL_EXT_ray_query)
    __glsl_version(460)
    float3 WorldRayDirection();

    __target_intrinsic(glsl, rayQueryGetRayTMinEXT)
    __glsl_extension(GL_EXT_ray_query)
    __glsl_version(460)
    float RayTMin();
}

//
// Vulkan/SPIR-V specific features
//

struct VkSubpassInput<T>
{
    T SubpassLoad();
}

struct VkSubpassInputMS<T>
{
    T SubpassLoad(int sampleIndex);
}
