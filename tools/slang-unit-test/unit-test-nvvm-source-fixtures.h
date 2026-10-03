#pragma once

// Immutable source fixtures shared by NVVM emitter and integration tests.
// Fake state and callbacks remain isolated in each support-header translation unit.
namespace
{

static const char kDirectNVVMEmptyComputeSource[] =
    "[shader(\"compute\")] [numthreads(1, 1, 1)] void computeMain() {}";

static const char kDirectNVVMWriteScalarSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int value)
{
    *destination = value;
}
)";

static const char kDirectNVVMFloat16ValueSource[] = R"(
half2 chooseHalf2(half2 left, half2 right, bool chooseLeft)
{
    half2 selected;
    if (chooseLeft)
        selected = left;
    else
        selected = right;
    return selected;
}

half2 adjustHalf2(half2 value)
{
    return -(value + half2(1.0h, 2.0h));
}

[CUDAKernel]
void computeMain(
    uniform Ptr<float, Access::ReadWrite, AddressSpace::Device> destination,
    uniform float left,
    uniform float right,
    uniform int integerValue)
{
    half first = half(left);
    half second = half(integerValue);
    half2 pair = half2(first, second);
    half2 converted = half2(float2(right, float(integerValue + 1)));
    half2 result = adjustHalf2(chooseHalf2(pair, converted, left > right));
    bool2 compared = result < pair;
    float2 widened = float2(result);
    int2 integers = int2(result);
    half selectedLane = result[integerValue & 1];
    *destination =
        widened.x + float(integers.y) + float(selectedLane) + (compared.x ? 1.0 : 0.0);
}
)";

static const char kDirectNVVMOpaqueHalfConversionSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<float, Access::ReadWrite, AddressSpace::Device> destination,
    uniform float input)
{
    half narrowed = f32tof16_(input);
    *destination = f16tof32(narrowed);
}
)";

static const char kDirectNVVMUnsupportedOpaqueHalfConversionSignatureSource[] = R"(
half malformedFloatToHalf(float input, int extra)
{
    __target_switch
    {
    case cuda: __intrinsic_asm "__float2half";
    default: return half(input + float(extra));
    }
}

[CUDAKernel]
void computeMain(
    uniform Ptr<float, Access::ReadWrite, AddressSpace::Device> destination,
    uniform float input)
{
    half narrowed = malformedFloatToHalf(input, 0);
    *destination = f16tof32(narrowed);
}
)";

static const char kDirectNVVMUnsupportedSurfaceSignatureSource[] = R"SLANG(
RWTexture2D<half> surface;

half malformedSurfaceLoad(RWTexture2D<half> resource, int2 coordinate, int extra)
{
    __target_switch
    {
    case cuda:
        __intrinsic_asm
            "surf2Dread$C<$T0>($0, ($1).x * $E, ($1).y, SLANG_CUDA_BOUNDARY_MODE)";
    default:
        return half(extra);
    }
}

[CUDAKernel]
void computeMain(
    uniform Ptr<float, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int x,
    uniform int y)
{
    *destination = float(malformedSurfaceLoad(surface, int2(x, y), 0));
}
)SLANG";

static const char kDirectNVVMLocalVectorSwizzleSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<float, Access::ReadWrite, AddressSpace::Device> destination,
    uniform float input)
{
    half4 value = half4(
        half(input),
        half(input + 1.0),
        half(input + 2.0),
        half(input + 3.0));
    value.xyz = -value.zwx;
    *destination = float(value.x + value.y + value.z + value.w);
}
)";

static const char kDirectNVVMStatefulAggregateHelperSource[] = R"(
struct Counter
{
    __init(int initialValue)
    {
        value = initialValue;
    }

    [mutating] int next()
    {
        int result = value;
        value++;
        return result;
    }

    int value;
};

[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int initialValue)
{
    Counter counter = Counter(initialValue);
    *destination = counter.next() + counter.next();
}
)";

static const char kDirectNVVMThreadLocalGlobalContextSource[] = R"(
static int accumulator = 7;

int accumulate(int value)
{
    accumulator += value;
    return accumulator;
}

[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int value)
{
    *destination = accumulate(value);
}
)";

static const char kDirectNVVMScalarTruthinessSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int integerValue,
    uniform float floatingPointValue)
{
    uint unsignedValue = uint(integerValue);
    half halfValue = half(floatingPointValue);
    bool boolValue = integerValue != 0;
    destination[0] =
        (all(integerValue) ? 1 : 0) +
        (any(unsignedValue) ? 2 : 0) +
        (all(floatingPointValue) ? 4 : 0) +
        (any(halfValue) ? 8 : 0) +
        (all(boolValue) ? 16 : 0);
}
)";

static const char kDirectNVVMUnsupportedScalarTruthinessSignatureSource[] = R"SLANG(
bool malformedTruthiness(int value, int extra)
{
    __target_switch
    {
    case cuda:
        __intrinsic_asm "bool($0)";
    default:
        return extra != 0;
    }
}

[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int value)
{
    *destination = malformedTruthiness(value, 0) ? 1 : 0;
}
)SLANG";

static const char kDirectNVVMUnsupportedMinMaxSignatureSource[] = R"SLANG(
int malformedMinimum(int left, int right, int extra)
{
    __target_switch
    {
    case cuda:
        __intrinsic_asm "$P_min($0, $1)";
    default:
        return min(left, right) + extra;
    }
}

[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int value)
{
    *destination = malformedMinimum(value, 0, 1);
}
)SLANG";

static const char kDirectNVVMUnsupportedIntegerBitSignatureSource[] = R"SLANG(
uint malformedCountBits(uint value, uint extra)
{
    __target_switch
    {
    case cuda:
        __intrinsic_asm "$P_countbits($0)";
    default:
        return countbits(value) + extra;
    }
}

[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform uint value)
{
    *destination = int(malformedCountBits(value, 1));
}
)SLANG";

static const char kDirectNVVMUnsupportedVectorIntegerBitSource[] = R"SLANG(
uint2 malformedReverseBits(uint2 value)
{
    __target_switch
    {
    case cuda:
        __intrinsic_asm "$P_reversebits($0)";
    default:
        return reversebits(value);
    }
}

[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform uint value)
{
    *destination = int(malformedReverseBits(uint2(value, value + 1)).x);
}
)SLANG";

static const char kDirectNVVMUnsupportedVectorScalarMathSource[] = R"SLANG(
float2 malformedTangent(float2 value)
{
    __target_switch
    {
    case cuda:
        __intrinsic_asm "$P_tan($0)";
    default:
        return tan(value);
    }
}

[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform float value)
{
    *destination = int(malformedTangent(float2(value, value + 1.0)).x);
}
)SLANG";

static const char kDirectNVVMCopyableValueHelperSource[] = R"(
struct Payload
{
    int bias;
    float4 lanes;
};

Payload addOffset(Payload value, inout int offset)
{
    value.bias += offset;
    offset += 1;
    return value;
}

float readValue(Payload value)
{
    return float(value.bias) + value.lanes.x;
}

[CUDAKernel]
void computeMain(
    uniform Ptr<float, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int input)
{
    Payload value;
    value.bias = input;
    value.lanes = float4(1.0, 2.0, 3.0, 4.0);
    int offset = 2;
    value = addOffset(value, offset);
    *destination = readValue(value) + float(offset);
}
)";

static const char kDirectNVVMRecursiveCopyableValueHelperSource[] = R"(
struct PayloadWithArray
{
    int values[2];
    float scale;
};

float sumPayload(PayloadWithArray value)
{
    return float(value.values[0] + value.values[1]) * value.scale;
}

[CUDAKernel]
void computeMain(
    uniform Ptr<float, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int input)
{
    PayloadWithArray value;
    value.values[0] = input;
    value.values[1] = 3;
    value.scale = 0.5;
    *destination = sumPayload(value);
}
)";

static const char kDirectNVVMResourceStructHelperSource[] = R"(
struct ResourceParam
{
    Texture2D texture;
    SamplerState sampler;
    float base;
};

Texture2D texture;
SamplerState sampler;
RWStructuredBuffer<float> destination;

float4 sampleResource(ResourceParam value)
{
    return value.texture.SampleLevel(value.sampler, float2(0.0), 0.0) + value.base;
}

ResourceParam preserveResource(ResourceParam value)
{
    return value;
}

[CUDAKernel]
void computeMain()
{
    ResourceParam value;
    value.texture = texture;
    value.sampler = sampler;
    value.base = -0.5;
    value = preserveResource(value);
    destination[0] = sampleResource(value).x;
}
)";

static const char kDirectNVVMAggregateValueArraySource[] = R"(
struct Payload
{
    int2 lanes;
};

void setFirstLane(inout Payload value, int lane)
{
    value.lanes.x = lane;
}

[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int index)
{
    Payload first = { int2(1, 2) };
    setFirstLane(first, 3);
    Payload second = { int2(7, 11) };
    Payload values[2] = { first, second };
    *destination = values[index].lanes.x;
}
)";

static const char kDirectNVVMComposableAggregateAddressSource[] = R"(
struct Payload
{
    int2 lanes;
    int value;
};

void initialize(out Payload value, int input)
{
    value.lanes = int2(input, input + 1);
    value.value = input + 2;
}

int readValue(__constref Payload value)
{
    return value.lanes.x + value.value;
}

[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int index)
{
    Payload values[2];
    initialize(values[index & 1], 3);
    values[index & 1].value += 1;
    *destination = readValue(values[index & 1]);
}
)";

static const char kDirectNVVMResourceArrayStorageSource[] = R"(
RWStructuredBuffer<int> sources[2];
RWStructuredBuffer<int> destination;

[numthreads(1, 1, 1)]
void computeMain()
{
    destination[0] = sources[1][0];
}
)";

static const char kDirectNVVMLocalArrayHelperSource[] = R"(
void initializeArray(out float3 values[4])
{
    values[0] = float3(1.0, 1.0, 1.0);
    values[1] = float3(2.0, 2.0, 2.0);
    values[2] = float3(3.0, 3.0, 3.0);
    values[3] = float3(4.0, 4.0, 4.0);
}

void updateArray(inout float3 values[4])
{
    values[0] = float3(5.0, 5.0, 5.0);
}

[CUDAKernel]
void computeMain(uniform Ptr<float, Access::ReadWrite, AddressSpace::Device> destination)
{
    float3 values[4];
    initializeArray(values);
    updateArray(values);
    *destination = values[0].x;
}
)";

static const char kDirectNVVMCopyableStructuredBufferAggregateSource[] = R"(
struct Thing
{
    uint pos;
    float radius;
    half4 color;
};

[CUDAKernel]
void computeMain(RWStructuredBuffer<Thing> destination, uniform uint index)
{
    Thing value;
    value.pos = index;
    value.radius = float(index);
    value.color = half4(1.0h, 2.0h, 3.0h, 4.0h);
    destination[index] = value;
}
)";

static const char kDirectNVVMCopyableStructArraySource[] = R"(
struct Payload
{
    int value;
    float weight;
};

[CUDAKernel]
void computeMain(
    StructuredBuffer<Payload> source,
    RWStructuredBuffer<int> destination,
    uniform uint index)
{
    Payload loaded = source.Load(0);
    Payload values[2];
    values[0] = loaded;
    values[1] = loaded;
    Payload selected = values[index & 1];
    destination[0] = loaded.value + selected.value;
}
)";

static const char kDirectNVVMMutableStructuredBufferAggregateFieldSource[] = R"(
struct Payload
{
    int4 first;
    int4 second;
};

[CUDAKernel]
void computeMain(RWStructuredBuffer<Payload> values, uniform uint index)
{
    values[index].second.y = values[0].first.x;
}
)";

static const char kDirectNVVMIncompatibleStructuredBufferAggregateLayoutSource[] = R"(
struct MisalignedThing
{
    half leading;
    half4 payload;
};

[CUDAKernel]
void computeMain(RWStructuredBuffer<MisalignedThing> destination)
{
    MisalignedThing value;
    value.leading = 1.0h;
    value.payload = half4(2.0h, 3.0h, 4.0h, 5.0h);
    destination[0] = value;
}
)";

static const char kDirectNVVMDynamicLocalVectorStoreSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<float, Access::ReadWrite, AddressSpace::Device> destination,
    uniform float input,
    uniform int index)
{
    half4 value = half4(1.0h, 2.0h, 3.0h, 4.0h);
    value[index & 3] = half(input);
    *destination = float(value.x + value.y + value.z + value.w);
}
)";

static const char kDirectNVVMFloatMatrixValueSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<float, Access::ReadWrite, AddressSpace::Device> destination,
    uniform float a,
    uniform float b)
{
    float2x2 ma = float2x2(a, b, b, a);
    float2x2 mb = ma + 1.0;
    float2x2 ms = ma + mb;
    float2x2 selected;
    if (a > b)
        selected = ms;
    else
        selected = mb;
    *destination = selected[1][1];
}
)";

static const char kDirectNVVMMatrixMemorySource[] = R"(
ConstantBuffer<float4x4> matrixBuffer;
RWStructuredBuffer<float> outputBuffer;

[CudaDeviceExport]
float __slang_nvvm_internal_0(float value)
{
    return value;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    float4x4 input = matrixBuffer;
    float4x4 squared = mul(input, input);
    float4 transformed = mul(float4(1.0, 2.0, 3.0, 1.0), input);
    outputBuffer[0] = __slang_nvvm_internal_0(squared[0][0] + transformed.x);
}
)";

static const char kDirectNVVMStructuredMatrixMemorySource[] = R"(
RWStructuredBuffer<float4x4> matrixBuffer;
RWStructuredBuffer<int> outputBuffer;

[numthreads(4, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    int value = int(tid.x);
    outputBuffer[tid.x] = asint(matrixBuffer[0][(value + 1) & 3][(value + 3) & 3]);
}
)";

static const char kDirectNVVMReadOnlyStructuredMatrixMemorySource[] = R"(
StructuredBuffer<float2x2> matrixBuffer;
RWStructuredBuffer<float> outputBuffer;

[CUDAKernel]
void computeMain()
{
    outputBuffer[0] = matrixBuffer[0][0][0];
}
)";

static const char kDirectNVVMUnsupportedStructuredMatrixWriteSource[] = R"(
RWStructuredBuffer<float4x4> matrixBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    matrixBuffer[0][tid.x & 3][tid.y & 3] = float(tid.x);
}
)";

static const char kDirectNVVMVectorOperationFamilySource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination)
{
    int2 shift = int2(7, -3) >> 1;
    int2 broadcastSum = int2(7, -3) + 2;
    int2 reverseDifference = 20 - int2(1, 3);
    int8_t2 narrow = int8_t2(-6, 7);
    int8_t2 quotient = narrow / int8_t2(2, 2);
    int8_t2 remainder = narrow % int8_t2(4, 4);
    bool2 negative = narrow < int8_t(0);
    bool predicate = destination[8] != 0;
    bool2 logic = (!negative && predicate) || negative;
    float3 sum = float3(1.5, 2.5, 3.5) + 0.5;
    float3 floatRemainder = float3(7.5, -7.5, 8.5) % float3(2.0, 2.0, 3.0);
    bool3 floatLess = sum < floatRemainder;
    bool2 explicitBoolean = bool2(predicate, negative.x);
    bool2 equalBoolean = explicitBoolean == logic;
    destination[0] = shift.y;
    destination[1] = int(quotient.x);
    destination[2] = int(remainder.y);
    destination[3] = negative.x ? 1 : 0;
    destination[4] = int(sum.z * 2.0);
    destination[5] = int(floatRemainder.y * 2.0);
    destination[6] = broadcastSum.x + reverseDifference.y;
    destination[7] = logic.y ? 1 : 0;
    destination[8] = floatLess.x ? 1 : 0;
    destination[9] = equalBoolean.y ? 1 : 0;
}
)";

static const char kDirectNVVMFloat64VectorAlgebraSource[] = R"(
RWStructuredBuffer<double> destination;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int index = int(dispatchThreadID.x);
    double left = double(index) + 1.0;
    double right = double(index) + 2.0;
    double2 values = double2(left, right);
    double2 sum = values + left;
    double2 difference = right - values;
    double2 product = sum * difference;
    double2 quotient = product / double2(right + 1.0, left + 1.0);
    int2 indices = int2(index, index + 1);
    double2 converted = double2(indices);
    destination[index] = quotient.x + converted.y;
}
)";

static const char kDirectNVVMTypedSelectSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int value)
{
    int2 values = int2(value, value + 1);
    bool2 condition = values > 0;
    bool2 whenTrue = values < 4;
    bool2 whenFalse = values == 0;
    bool2 selected = condition ? whenTrue : whenFalse;
    *destination = all(selected) ? 1 : 0;
}
)";

static const char kDirectNVVMFlattenedVectorConstructionSource[] = R"(
[noinline]
half2 makePair(float left, float right)
{
    return half2(half(left), half(right));
}

[noinline]
half selectLane(half4 value, int index)
{
    return value[index & 3];
}

[CUDAKernel]
void computeMain(
    uniform Ptr<float, Access::ReadWrite, AddressSpace::Device> destination,
    uniform float first,
    uniform float second,
    uniform float third,
    uniform float fourth,
    uniform int index)
{
    half2 pair = makePair(first, second);
    half4 combined = half4(pair, half(third), half(fourth));
    *destination = float(selectLane(combined, index));
}
)";

static const char kDirectNVVMWaveLaneIndexSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<uint, Access::ReadWrite, AddressSpace::Device> destination)
{
    uint laneIndex = WaveGetLaneIndex();
    destination[laneIndex] = laneIndex;
}
)";

static const char kDirectNVVMCUDAExecutionSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<uint, Access::ReadWrite, AddressSpace::Device> destination)
{
    uint3 threadIndex = cudaThreadIdx();
    uint3 blockIndex = cudaBlockIdx();
    uint3 blockDimensions = cudaBlockDim();
    uint3 gridDimensions = cudaGridDim();
    destination[0] = threadIndex.x;
    destination[1] = threadIndex.y;
    destination[2] = threadIndex.z;
    destination[3] = blockIndex.x;
    destination[4] = blockIndex.y;
    destination[5] = blockIndex.z;
    destination[6] = blockDimensions.x;
    destination[7] = blockDimensions.y;
    destination[8] = blockDimensions.z;
    destination[9] = gridDimensions.x;
    destination[10] = gridDimensions.y;
    destination[11] = gridDimensions.z;
    GroupMemoryBarrierWithGroupSync();
}
)";

static const char kDirectNVVMIntegerVectorSwizzleSource[] = R"(
[shader("compute")]
[numthreads(2, 2, 1)]
void computeMain(
    int2 dispatchThreadID : SV_DispatchThreadID,
    StructuredBuffer<uint> source,
    RWStructuredBuffer<uint> destination)
{
    destination[uint(dispatchThreadID.x)] = source[uint(dispatchThreadID.y)];
}
)";

static const char kDirectNVVMDynamicVectorIndexSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<uint, Access::ReadWrite, AddressSpace::Device> destination,
    uniform uint index)
{
    uint3 threadIndex = cudaThreadIdx();
    destination[0] = threadIndex[index];
}
)";

static const char kDirectNVVMCUDATypeLayoutSource[] = R"(
struct Empty {};
typedef int Unsized[];
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination)
{
    destination[0] = __alignOf<uint8_t>();
    destination[1] = __alignOf<vector<half, 3> >();
    destination[2] = __alignOf<vector<half, 4> >();
    destination[3] = __alignOf<vector<double, 2> >();
    destination[4] = __sizeOf<vector<half, 3> >();
    destination[5] = __sizeOf<vector<double, 4> >();
    destination[6] = __alignOf<Unsized>();
    destination[7] = sizeof(Empty, __CUDADataLayout);
    destination[8] = __sizeOf<Unsized>();
}
)";

static const char kDirectNVVMCUDAAggregateLayoutSource[] = R"(
struct PadLadenStruct
{
    double a;
    uint8_t b;
};

struct StructWithArray : IDefaultInitializable
{
    PadLadenStruct a[1];
    uint8_t b;
    matrix<half, 3, 3> c;
    uint8_t d;
};

struct OffsetPair<T>
{
    T first;
    T second;
};

[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination)
{
    StructWithArray value;
    destination[0] = __sizeOf(value);
    destination[1] = __offsetOf(value, value.a);
    destination[2] = __offsetOf(value, value.b);
    destination[3] = __offsetOf(value, value.c);
    destination[4] = __offsetOf(value, value.d);
    destination[5] = __sizeOf<int>();
    destination[6] = __alignOf<StructWithArray>();
    destination[7] = __alignOf(value);
    destination[8] = __sizeOf<StructWithArray>();
    OffsetPair<int> pair = {0, 0};
    OffsetPair<double> wide = {0.0, 0.0};
    destination[9] = __offsetOf(pair, pair.first);
    destination[10] = __offsetOf(pair, pair.second);
    destination[11] = __offsetOf(wide, wide.second);
    OffsetPair<int> other = {0, 0};
    __offsetOf(pair, other.second); // An unused readNone query remains removable.
}
)";

static const char kDirectNVVMNonCanonicalCUDAOffsetSource[] = R"(
struct OffsetStruct : IDefaultInitializable
{
    int value;
};

[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination)
{
    OffsetStruct left;
    OffsetStruct right;
    left.value = 1;
    right.value = 2;
    destination[0] = __offsetOf(left, right.value);
}
)";

static const char kDirectNVVMCUDAExecutionRuntimeSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> counter,
    uniform Ptr<uint, Access::ReadWrite, AddressSpace::Device> destination)
{
    uint3 threadIndex = cudaThreadIdx();
    uint3 blockIndex = cudaBlockIdx();
    uint3 blockDimensions = cudaBlockDim();
    uint3 gridDimensions = cudaGridDim();
    int slot;
    InterlockedAdd(*counter, 1, slot);
    int outputBase = slot * 12;
    GroupMemoryBarrierWithGroupSync();
    destination[outputBase + 0] = threadIndex.x;
    destination[outputBase + 1] = threadIndex.y;
    destination[outputBase + 2] = threadIndex.z;
    destination[outputBase + 3] = blockIndex.x;
    destination[outputBase + 4] = blockIndex.y;
    destination[outputBase + 5] = blockIndex.z;
    destination[outputBase + 6] = blockDimensions.x;
    destination[outputBase + 7] = blockDimensions.y;
    destination[outputBase + 8] = blockDimensions.z;
    destination[outputBase + 9] = gridDimensions.x;
    destination[outputBase + 10] = gridDimensions.y;
    destination[outputBase + 11] = gridDimensions.z;
}
)";

static const char kDirectNVVMConventionalComputeSource[] = R"(
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int index = int(dispatchThreadID.x);
    outputBuffer[index] = 42;
}
)";

static const char kDirectNVVMConventionalScalarParameterBlockSource[] = R"(
uniform uint frame;

struct Block
{
    uint dummy;
};

ParameterBlock<Block> block;
RWStructuredBuffer<uint> outputBuffer;

struct TestGlobalParams
{
    uint frame;
    Block* block;
};

[numthreads(1, 1, 1)]
void computeMain()
{
    TestGlobalParams gp = {};
    outputBuffer[0] = __offsetOf(gp, gp.frame);
    outputBuffer[1] = __offsetOf(gp, gp.block);
    outputBuffer[2] = __sizeOf<TestGlobalParams>();
    outputBuffer[3] = __alignOf<Block*>();
    outputBuffer[4] = frame;
    outputBuffer[5] = block.dummy;
}
)";

static const char kDirectNVVMConventionalScalarConstantBufferSource[] = R"(
struct Params
{
    uint value;
    float scale;
};

ConstantBuffer<Params> params;
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = params.value;
}
)";

static const char kDirectNVVMLoadedParameterGroupValueSource[] = R"(
struct Params
{
    uint value;
};

ConstantBuffer<Params> params;
RWStructuredBuffer<uint> outputBuffer;

uint readValue(Params value)
{
    return value.value;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = readValue(params);
}
)";

static const char kDirectNVVMLoadedCompactParameterGroupValueSource[] = R"(
struct Params
{
    float3 value;
};

ConstantBuffer<Params> params;
RWStructuredBuffer<float> outputBuffer;

float readValue(Params value)
{
    return value.value.x;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = readValue(params);
}
)";

static const char kDirectNVVMCompactParameterGroupVectorSource[] = R"(
cbuffer VectorParams
{
    float3 first;
    float3 second;
};

RWStructuredBuffer<float> outputBuffer;

float sumLanes(float3 value)
{
    return value.x + value.y + value.z;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = sumLanes(first) + second.z;
}
)";

static const char kDirectNVVMFloat64ValueFamilySource[] = R"(
RWStructuredBuffer<uint64_t> outputBuffer;

double transformDouble(double x, int64_t y)
{
    double value = ((-x + double(y)) * 3.0 - 1.0) / 2.0;
    double remainder = value % 5.0;
    double selected = select(remainder > x, remainder, x);
    int64_t integral = int64_t(selected);
    float narrowed = float(selected);
    return double(narrowed) + double(integral);
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    double source = double(dispatchThreadID.x) + 1.0;
    double transformed = transformDouble(source, 4);
    outputBuffer[0] = bit_cast<uint64_t>(transformed);
    outputBuffer[1] = bit_cast<uint64_t>(bit_cast<int64_t>(3.0));
}
)";

static const char kDirectNVVMConventionalSamplerStorageSource[] = R"(
SamplerComparisonState comparisonSampler;
SamplerComparisonState comparisonSamplers[];
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = 1.0f;
}
)";

static const char kDirectNVVMMultidimensionalWaveSource[] = R"(
uniform RWStructuredBuffer<float> outputBuffer;

[numthreads(8, 8, 1)]
void computeMain(uint lane : SV_GroupIndex)
{
    uint i = lane * 2;
    outputBuffer[i] = WaveIsFirstLane() ? 1.0 : 0.0;
    outputBuffer[i + 1] = float(WaveGetLaneIndex());
}
)";

static const char kDirectNVVMSharedMemorySource[] = R"(
groupshared int sharedValues[64];

[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> counter,
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination)
{
    int ticket;
    InterlockedAdd(*counter, 1, ticket);
    sharedValues[ticket] = ticket * 3 + 1;
    GroupMemoryBarrierWithGroupSync();
    destination[ticket] = sharedValues[63 - ticket];
}
)";

static const char kDirectNVVMUnsignedSharedArrayIndexSource[] = R"(
groupshared uint sharedValues[4];

[CUDAKernel]
void computeMain(
    uniform Ptr<uint, Access::ReadWrite, AddressSpace::Device> destination,
    uniform uint writeIndex,
    uniform uint readIndex)
{
    sharedValues[writeIndex] = writeIndex + 1;
    GroupMemoryBarrierWithGroupSync();
    destination[writeIndex] = sharedValues[readIndex];
}
)";

static const char kDirectNVVMSharedFloatArraySource[] = R"(
groupshared float sharedValues[64];

[CUDAKernel]
void computeMain(
    uniform Ptr<float, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int index)
{
    sharedValues[index] = 1.0;
    GroupMemoryBarrierWithGroupSync();
    destination[index] = sharedValues[index];
}
)";

static const char kDirectNVVMWaveLaneCountSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<uint, Access::ReadWrite, AddressSpace::Device> destination)
{
    uint laneIndex = WaveGetLaneIndex();
    uint laneCount = WaveGetLaneCount();
    destination[laneIndex] = laneCount;
}
)";

static const char kDirectNVVMWaveReadLaneAtUIntSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<uint, Access::ReadWrite, AddressSpace::Device> destination,
    uniform uint mask,
    uniform int sourceLane)
{
    uint laneIndex = WaveGetLaneIndex();
    destination[laneIndex] = WaveMaskReadLaneAt(mask, laneIndex, sourceLane);
}
)";

static const char kDirectNVVMWaveReadLaneAtIntSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform Ptr<int, Access::Read, AddressSpace::Device> source,
    uniform uint mask,
    uniform int sourceLane)
{
    uint laneIndex = WaveGetLaneIndex();
    destination[laneIndex] = WaveMaskReadLaneAt(mask, source[laneIndex], sourceLane);
}
)";

static const char kDirectNVVMWaveReadLaneAtFloatSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<float, Access::ReadWrite, AddressSpace::Device> destination,
    uniform Ptr<float, Access::Read, AddressSpace::Device> source,
    uniform uint mask,
    uniform int sourceLane)
{
    uint laneIndex = WaveGetLaneIndex();
    destination[laneIndex] = WaveMaskReadLaneAt(mask, source[laneIndex], sourceLane);
}
)";

static const char kDirectNVVMWaveActiveMaskSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<uint, Access::ReadWrite, AddressSpace::Device> destination)
{
    uint laneIndex = WaveGetLaneIndex();
    destination[laneIndex] = WaveGetActiveMask();
}
)";

static const char kDirectNVVMWaveReadLaneFirstUIntSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<uint, Access::ReadWrite, AddressSpace::Device> destination)
{
    uint laneIndex = WaveGetLaneIndex();
    destination[laneIndex] = WaveReadLaneFirst(laneIndex);
}
)";

static const char kDirectNVVMWaveReadLaneFirstIntSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform Ptr<int, Access::Read, AddressSpace::Device> source)
{
    uint laneIndex = WaveGetLaneIndex();
    destination[laneIndex] = WaveReadLaneFirst(source[laneIndex]);
}
)";

static const char kDirectNVVMWaveReadLaneFirstFloatSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<float, Access::ReadWrite, AddressSpace::Device> destination,
    uniform Ptr<float, Access::Read, AddressSpace::Device> source)
{
    uint laneIndex = WaveGetLaneIndex();
    destination[laneIndex] = WaveReadLaneFirst(source[laneIndex]);
}
)";

static const char kDirectNVVMWaveIsFirstLaneSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination)
{
    uint laneIndex = WaveGetLaneIndex();
    destination[laneIndex] = WaveIsFirstLane() ? 1 : 0;
}
)";

static const char kDirectNVVMWaveActiveAnyTrueSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform Ptr<int, Access::Read, AddressSpace::Device> source)
{
    uint laneIndex = WaveGetLaneIndex();
    destination[laneIndex] = WaveActiveAnyTrue(source[laneIndex] != 0) ? 1 : 0;
}
)";

static const char kDirectNVVMWaveActiveAllTrueSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform Ptr<int, Access::Read, AddressSpace::Device> source)
{
    uint laneIndex = WaveGetLaneIndex();
    destination[laneIndex] = WaveActiveAllTrue(source[laneIndex] != 0) ? 1 : 0;
}
)";

static const char kDirectNVVMWaveActiveAllEqualIntSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform Ptr<int, Access::Read, AddressSpace::Device> source)
{
    uint laneIndex = WaveGetLaneIndex();
    destination[laneIndex] = WaveActiveAllEqual(source[laneIndex]) ? 1 : 0;
}
)";

static const char kDirectNVVMWaveActiveAllEqualUIntSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform Ptr<uint, Access::Read, AddressSpace::Device> source)
{
    uint laneIndex = WaveGetLaneIndex();
    destination[laneIndex] = WaveActiveAllEqual(source[laneIndex]) ? 1 : 0;
}
)";

static const char kDirectNVVMWaveActiveAllEqualFloatSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform Ptr<float, Access::Read, AddressSpace::Device> source)
{
    uint laneIndex = WaveGetLaneIndex();
    destination[laneIndex] = WaveActiveAllEqual(source[laneIndex]) ? 1 : 0;
}
)";

static const char kDirectNVVMWaveMaskMatchSwitchSource[] = R"(
RWStructuredBuffer<uint> destination;

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    switch (dispatchThreadID.x)
    {
    case 0:
    case 1:
        destination[dispatchThreadID.x] = WaveGetActiveMask();
        break;
    default:
        destination[dispatchThreadID.x] = WaveGetActiveMask();
        break;
    }
}
)";

static const char kDirectNVVMUnmaskedWaveReadLaneAtUIntSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<uint, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int sourceLane)
{
    uint laneIndex = WaveGetLaneIndex();
    destination[laneIndex] = WaveReadLaneAt(laneIndex, sourceLane);
}
)";

static const char kDirectNVVMUnmaskedWaveReadLaneAtIntSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform Ptr<int, Access::Read, AddressSpace::Device> source,
    uniform int sourceLane)
{
    uint laneIndex = WaveGetLaneIndex();
    destination[laneIndex] = WaveReadLaneAt(source[laneIndex], sourceLane);
}
)";

static const char kDirectNVVMUnmaskedWaveReadLaneAtFloatSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<float, Access::ReadWrite, AddressSpace::Device> destination,
    uniform Ptr<float, Access::Read, AddressSpace::Device> source,
    uniform int sourceLane)
{
    uint laneIndex = WaveGetLaneIndex();
    destination[laneIndex] = WaveReadLaneAt(source[laneIndex], sourceLane);
}
)";

static const char kDirectNVVMCompoundWaveOperationsSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform uint mask,
    uniform int sourceLane)
{
    uint lane = WaveGetLaneIndex();
    float2 shuffled = WaveMaskReadLaneAt(mask, float2(lane, lane + 1), sourceLane);
    bool allEqual = WaveMaskAllEqual(mask, int2(sourceLane, sourceLane + 1));
    uint count = WaveMaskCountBits(mask, lane < 2);
    destination[lane] = int(shuffled.x + shuffled.y) + (allEqual ? 16 : 0) + int(count);
}
)";

static const char kDirectNVVMMaskedWaveScalarOperationsSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform uint mask)
{
    uint lane = WaveGetLaneIndex();
    uint4 partition = uint4(mask, 0, 0, 0);
    int value = int(lane) + 1;
    int reduction = WaveMultiSum(value, partition);
    int prefix = WaveMultiPrefixSum(value, partition);
    destination[lane] = reduction + prefix;
}
)";

static const char kDirectNVVMAggregateWaveOperationsSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform uint mask,
    uniform int sourceLane)
{
    uint lane = WaveGetLaneIndex();
    int2 vectorValue = int2(int(lane) + 1, int(lane) + 2);
    int2 vectorSum = WaveMaskSum(mask, vectorValue);
    float2 vectorPrefixMin = WaveMaskPrefixMin(mask, float2(lane + 1, lane + 2));

    matrix<int, 2, 2> matrixValue = matrix<int, 2, 2>(
        int(lane) + 1,
        int(lane) + 2,
        int(lane) + 3,
        int(lane) + 4);
    matrix<int, 2, 2> matrixSum = WaveMaskSum(mask, matrixValue);
    matrix<int, 2, 2> matrixShuffle =
        WaveMaskReadLaneAt(mask, matrixValue, sourceLane);

    matrix<int, 2, 2> implicitShuffle = WaveReadLaneAt(matrixValue, int(lane));
    uint convergedMask = WaveGetConvergedMask();
    uint4 convergedMulti = WaveGetConvergedMulti();
    destination[lane] = vectorSum.x + vectorSum.y + int(vectorPrefixMin.x) +
        matrixSum[0][0] + matrixShuffle[1][1] + implicitShuffle[0][0] +
        int(convergedMask + convergedMulti.x);
}
)";

static const char kDirectNVVMFloat64ImplicitAggregateShuffleSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int sourceLane)
{
    double lane = double(WaveGetLaneIndex());
    double2x2 value = double2x2(lane, lane + 1.0l, lane + 2.0l, lane + 3.0l);
    double2x2 shuffled = WaveReadLaneAt(value, sourceLane);
    *destination = int(shuffled[0][0] + shuffled[0][1] + shuffled[1][0] + shuffled[1][1]);
}
)";

static const char kDirectNVVMUnsupportedMaskedWaveScalarSignatureSource[] = R"SLANG(
int malformedMaskedWaveSum(int value, uint mask)
{
    __target_switch
    {
    case cuda:
        __intrinsic_asm "_waveSum($1.x, $0)";
    default:
        return value;
    }
}

[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform uint mask)
{
    *destination = malformedMaskedWaveSum(1, mask);
}
)SLANG";

static const char kDirectNVVMUnsupportedCompoundWaveSignatureSource[] = R"SLANG(
float2 malformedWaveShuffle(int mask, float2 value, int lane)
{
    __target_switch
    {
    case cuda:
        __intrinsic_asm "_waveShuffleMultiple($0, $1, $2)";
    default:
        return value;
    }
}

[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int mask)
{
    *destination = int(malformedWaveShuffle(mask, float2(1.0, 2.0), 0).x);
}
)SLANG";

static const char kDirectNVVMUnsupportedAggregateWaveSignatureSource[] = R"SLANG(
matrix<int, 2, 2> malformedAggregateWaveShuffle(
    int mask,
    matrix<int, 2, 2> value,
    int lane)
{
    __target_switch
    {
    case cuda:
        __intrinsic_asm "_waveShuffleMultiple($0, $1, $2)";
    default:
        return value;
    }
}

[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int mask)
{
    matrix<int, 2, 2> value = matrix<int, 2, 2>(1, 2, 3, 4);
    *destination = malformedAggregateWaveShuffle(mask, value, 0)[0][0];
}
)SLANG";

static const char kDirectNVVMFloat32CopySource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<float, Access::ReadWrite, AddressSpace::Device> destination,
    uniform Ptr<float, Access::Read, AddressSpace::Device> source)
{
    *destination = *source;
}
)";

static const char kDirectNVVMFloat32ConstantSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<float, Access::ReadWrite, AddressSpace::Device> destination)
{
    *destination = 1.5f;
}
)";

static const char kDirectNVVMFloat32PhiSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<float, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int condition,
    uniform float left,
    uniform float right)
{
    float selected;
    if (condition != 0)
        selected = left;
    else
        selected = right;
    *destination = selected;
}
)";

static const char kDirectNVVMFloat32FunctionSource[] = R"(
float addFloat32(float left, float right)
{
    return left + right;
}

[CUDAKernel]
void computeMain(
    uniform Ptr<float, Access::ReadWrite, AddressSpace::Device> destination,
    uniform float left,
    uniform float right)
{
    *destination = addFloat32(left, right);
}
)";

static const char kDirectNVVMUnsupportedHalfAddSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<half, Access::ReadWrite, AddressSpace::Device> destination,
    uniform half left,
    uniform half right)
{
    *destination = left + right;
}
)";

static const char kDirectNVVMUnsupportedDoubleAddSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<double, Access::ReadWrite, AddressSpace::Device> destination,
    uniform double left,
    uniform double right)
{
    *destination = left + right;
}
)";

static const char kDirectNVVMCopyScalarSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform Ptr<int, Access::Read, AddressSpace::Device> source)
{
    *destination = *source;
}
)";

static const char kDirectNVVMChooseScalarSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int x,
    uniform int y)
{
    if (x < y)
        *destination = x + y;
    else
        *destination = x - y;
}

)";

static const char kDirectNVVMUnsignedIntegerEqualSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform uint left,
    uniform uint right)
{
    *destination = left == right ? 1 : 0;
}
)";

static const char kDirectNVVMWideIntegerEqualSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int64_t left,
    uniform int64_t right)
{
    *destination = left == right ? 1 : 0;
}
)";

static const char kDirectNVVMPointerEqualSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform Ptr<int, Access::Read, AddressSpace::Device> left,
    uniform Ptr<int, Access::Read, AddressSpace::Device> right)
{
    *destination = left == right ? 1 : 0;
}
)";

static const char kDirectNVVMUnsignedIntegerNotEqualSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform uint left,
    uniform uint right)
{
    *destination = left != right ? 1 : 0;
}
)";

static const char kDirectNVVMWideIntegerNotEqualSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int64_t left,
    uniform int64_t right)
{
    *destination = left != right ? 1 : 0;
}
)";

static const char kDirectNVVMPointerNotEqualSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform Ptr<int, Access::Read, AddressSpace::Device> left,
    uniform Ptr<int, Access::Read, AddressSpace::Device> right)
{
    *destination = left != right ? 1 : 0;
}
)";

static const char kDirectNVVMUnsignedIntegerGreaterThanSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform uint left,
    uniform uint right)
{
    *destination = left > right ? 1 : 0;
}
)";

static const char kDirectNVVMWideIntegerGreaterThanSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int64_t left,
    uniform int64_t right)
{
    *destination = left > right ? 1 : 0;
}
)";

static const char kDirectNVVMPointerGreaterThanSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform Ptr<int, Access::Read, AddressSpace::Device> left,
    uniform Ptr<int, Access::Read, AddressSpace::Device> right)
{
    *destination = left > right ? 1 : 0;
}
)";

static const char kDirectNVVMUnsignedIntegerLessEqualSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform uint left,
    uniform uint right)
{
    *destination = left <= right ? 1 : 0;
}
)";

static const char kDirectNVVMWideIntegerLessEqualSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int64_t left,
    uniform int64_t right)
{
    *destination = left <= right ? 1 : 0;
}
)";

static const char kDirectNVVMPointerLessEqualSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform Ptr<int, Access::Read, AddressSpace::Device> left,
    uniform Ptr<int, Access::Read, AddressSpace::Device> right)
{
    *destination = left <= right ? 1 : 0;
}
)";

static const char kDirectNVVMUnsignedIntegerGreaterEqualSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform uint left,
    uniform uint right)
{
    *destination = left >= right ? 1 : 0;
}
)";

static const char kDirectNVVMWideIntegerGreaterEqualSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int64_t left,
    uniform int64_t right)
{
    *destination = left >= right ? 1 : 0;
}
)";

static const char kDirectNVVMPointerGreaterEqualSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform Ptr<int, Access::Read, AddressSpace::Device> left,
    uniform Ptr<int, Access::Read, AddressSpace::Device> right)
{
    *destination = left >= right ? 1 : 0;
}
)";

static const char kDirectNVVMSelectedKernelSource[] = R"(
[CUDAKernel]
void unselectedKernel()
{
    GroupMemoryBarrierWithGroupSync();
}

[CUDAKernel]
void computeMain()
{}
)";

static const char kDirectNVVMConventionalParameterizedComputeSource[] = R"(
[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uniform int value)
{}
)";

static const char kDirectNVVMChosenUndefinedAndDebugMarkerSource[] = R"(
RWStructuredBuffer<float> outputBuffer;

[ForceInline]
float chooseScalarNonVar(int seed)
{
    for (int i = 0; i < 2; ++i)
    {
        if (i == 0)
            continue;
        return float(seed + i);
    }
    return float(seed);
}

[numthreads(1, 1, 1)]
void computeMain(uint tid : SV_DispatchThreadID)
{
    if (tid == 0)
        outputBuffer[0] = chooseScalarNonVar(4);
}
)";

static const char kDirectNVVMStableStringHashSource[] = R"(
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint tid : SV_DispatchThreadID)
{
    if (tid == 0)
        outputBuffer[0] = getStringHash("Hello World!");
}
)";

static const char kDirectNVVMIntegerConstantSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int value)
{
    *destination = value + 1;
}
)";

static const char kDirectNVVMMixedNumericSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int8_t, Access::ReadWrite, AddressSpace::Device> output8,
    uniform Ptr<uint16_t, Access::ReadWrite, AddressSpace::Device> output16,
    uniform Ptr<int64_t, Access::ReadWrite, AddressSpace::Device> output64,
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> output32,
    uniform Ptr<float, Access::ReadWrite, AddressSpace::Device> outputFloat,
    uniform Ptr<int2, Access::ReadWrite, AddressSpace::Device> outputVector,
    uniform Ptr<int2, Access::Read, AddressSpace::Device> leftVector,
    uniform Ptr<int2, Access::Read, AddressSpace::Device> rightVector,
    uniform int8_t a,
    uniform uint8_t b,
    uniform int16_t c,
    uniform uint16_t d,
    uniform int64_t e,
    uniform uint64_t f,
    uniform float g)
{
    int index = int(cudaThreadIdx().x);
    output8[index] = ~(a + int8_t(b));
    output16[index] = (uint16_t(c) + d) ^ uint16_t(0x55aa);
    output64[index] = (int64_t(f) + e) * int64_t(3);
    int converted = int(g) + int(b);
    if (a < int8_t(b))
        converted += 1000;
    if (d > uint16_t(c))
        converted += 2000;
    output32[index] = converted;
    outputFloat[index] = float(c) + g;
    outputVector[index] = leftVector[index] + rightVector[index];
}
)";

static const char kDirectNVVMMergePhiSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int x,
    uniform int y)
{
    int selected;
    if (x < y)
        selected = x;
    else
        selected = y;
    *destination = selected;
}
)";

static const char kDirectNVVMFiniteLoopSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int limit)
{
    int sum = 0;
    for (int i = 0; i < limit; ++i)
        sum += i;
    *destination = sum;
}
)";

static const char kDirectNVVMUnsignedMultiplySource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform uint x,
    uniform uint y)
{
    *destination = int(x * y);
}
)";

static const char kDirectNVVMWideIntegerMultiplySource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int64_t x,
    uniform int64_t y)
{
    *destination = int(x * y);
}
)";

static const char kDirectNVVMFloatingMultiplySource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform float x,
    uniform float y)
{
    *destination = int(x * y);
}
)";

static const char kDirectNVVMExactLibdeviceUnarySource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform float x)
{
    double y = double(x);
    *destination = int(sin(x) + cos(x) + trunc(x) + sin(y) + cos(y));
}
)";

static const char kDirectNVVMScalarMathOperationsSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform float x,
    uniform int integerValue)
{
    double y = double(x);
    destination[0] = abs(integerValue);
    destination[1] = int(abs(half(x)));
    destination[2] = int(tan(y));
    destination[3] = int(pow(y, 2.0));
    destination[4] = isnan(y) ? 1 : 0;
    destination[5] = sign(y);
}
)";

static const char kDirectNVVMScalarIntrinsicRecipeSource[] = R"SLANG(
half halfFromSignedBits(int16_t value)
{
    return asfloat16(value);
}

half halfFromUnsignedBits(uint16_t value)
{
    return asfloat16(value);
}

uint16_t halfToBits(half value)
{
    return asuint16(value);
}

float floatFromPackedHalf(uint value)
{
    return f16tof32(value);
}

uint packedHalfFromFloat(float value)
{
    return f32tof16(value);
}

double doubleFromWords(uint low, uint high)
{
    return asdouble(low, high);
}

void doubleToWords(double value, out uint low, out uint high)
{
    asuint(value, low, high);
}

bool finiteHalf(half value)
{
    return isfinite(value);
}

bool finiteFloat(float value)
{
    return isfinite(value);
}

bool finiteDouble(double value)
{
    return isfinite(value);
}

bool infiniteHalf(half value)
{
    return isinf(value);
}

bool infiniteFloat(float value)
{
    return isinf(value);
}

bool infiniteDouble(double value)
{
    return isinf(value);
}

bool nanHalf(half value)
{
    return isnan(value);
}

half minimumHalf(half left, half right)
{
    return min(left, right);
}

half maximumHalf(half left, half right)
{
    return max(left, right);
}

int signHalf(half value)
{
    return sign(value);
}

half hyperbolicSineHalf(half value)
{
    return sinh(value);
}

half hyperbolicCosineHalf(half value)
{
    return cosh(value);
}

half hyperbolicTangentHalf(half value)
{
    return tanh(value);
}

half fusedMultiplyAddHalf(half left, half right, half addend)
{
    return fma(left, right, addend);
}

void sineCosineFloat(float value, out float sineValue, out float cosineValue)
{
    sincos(value, sineValue, cosineValue);
}

void sineCosineDouble(double value, out double sineValue, out double cosineValue)
{
    sincos(value, sineValue, cosineValue);
}

float frexpFloat(float value, out int exponent)
{
    return frexp(value, exponent);
}

double frexpDouble(double value, out int exponent)
{
    return frexp(value, exponent);
}

half frexpHalf(half value, out int exponent)
{
    return frexp(value, exponent);
}

half modfHalf(half value, out half integral)
{
    return modf(value, integral);
}

[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform float value,
    uniform int seed)
{
    half signedHalf = halfFromSignedBits(int16_t(seed));
    half unsignedHalf = halfFromUnsignedBits(uint16_t(seed));
    uint16_t halfBits = halfToBits(unsignedHalf);
    float unpacked = floatFromPackedHalf(uint(halfBits));
    uint packed = packedHalfFromFloat(value);
    double assembled = doubleFromWords(packed, uint(seed));
    uint low;
    uint high;
    doubleToWords(assembled, low, high);
    float sineFloat;
    float cosineFloat;
    double sineDouble;
    double cosineDouble;
    sineCosineFloat(value, sineFloat, cosineFloat);
    sineCosineDouble(assembled, sineDouble, cosineDouble);
    int floatExponent;
    int doubleExponent;
    float floatFraction = frexpFloat(value, floatExponent);
    double doubleFraction = frexpDouble(assembled, doubleExponent);
    int halfExponent;
    half halfFraction = frexpHalf(signedHalf, halfExponent);
    half integralHalf;
    half fractionalHalf = modfHalf(unsignedHalf, integralHalf);
    float integralFloat;
    double integralDouble;
    float fractionalFloat = modf(value, integralFloat);
    double fractionalDouble = modf(assembled, integralDouble);
    half halfMath =
        minimumHalf(signedHalf, unsignedHalf) + maximumHalf(signedHalf, unsignedHalf) +
        hyperbolicSineHalf(signedHalf) + hyperbolicCosineHalf(unsignedHalf) +
        hyperbolicTangentHalf(signedHalf) +
        fusedMultiplyAddHalf(signedHalf, unsignedHalf, half(1));
    int classifications =
        (finiteHalf(signedHalf) ? 1 : 0) +
        (finiteFloat(value) ? 2 : 0) +
        (finiteDouble(assembled) ? 4 : 0) +
        (infiniteHalf(unsignedHalf) ? 8 : 0) +
        (infiniteFloat(value) ? 16 : 0) +
        (infiniteDouble(assembled) ? 32 : 0) +
        (nanHalf(signedHalf) ? 64 : 0);
    destination[0] =
        int(halfBits) + int(unpacked) + int(packed + low + high) +
        int(sineFloat + cosineFloat + sineDouble + cosineDouble) +
        int(floatFraction + doubleFraction) + floatExponent + doubleExponent + classifications +
        signHalf(signedHalf) + halfExponent + int(halfFraction + fractionalHalf + integralHalf) +
        int(halfMath) + int(fractionalFloat + integralFloat + fractionalDouble + integralDouble);
}
)SLANG";

static const char kDirectNVVMUnsupportedScalarIntrinsicRecipeSignatureSource[] = R"SLANG(
void malformedDoubleToWords(double value, out uint low, out int high)
{
    __target_switch
    {
    case cuda: __intrinsic_asm "$P_asuint($0, $1, $2)";
    default:
        low = 0;
        high = 0;
        return;
    }
}

[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform float value)
{
    uint low;
    int high;
    malformedDoubleToWords(double(value), low, high);
    *destination = int(low) + high;
}
)SLANG";

static const char kDirectNVVMMinMaxSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform float x)
{
    double y = double(x);
    int integerMinimum = min(int(x), 2);
    uint integerMaximum = max(uint(x), 3);
    float floatMinimum = min(x, 1.0);
    float floatMaximum = max(x, 2.0);
    double doubleMinimum = min(y, 3.0);
    double doubleMaximum = max(y, 4.0);
    *destination =
        integerMinimum + int(integerMaximum) +
        int(floatMinimum + floatMaximum + doubleMinimum + doubleMaximum);
}
)";

static const char kDirectNVVMIntegerBitOperationsSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int signedValue,
    uniform uint unsignedValue)
{
    destination[0] = int(countbits(signedValue));
    destination[1] = int(reversebits(unsignedValue));
    destination[2] = int(firstbithigh(signedValue));
    destination[3] = int(firstbitlow(unsignedValue));
}
)";

static const char kDirectNVVMIntegerTruthinessBitfieldSource[] = R"(
[noinline]
uint firstLane(uint2 value)
{
    return value.x;
}

[noinline]
int secondLane(int2 value)
{
    return value.y;
}

[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform uint value)
{
    uint inserted = bitfieldInsert(value, 15u, 4u, 4u);
    int extracted = bitfieldExtract(int(value), 3u, 4u);
    uint2 vectorBase = uint2(value, value + 1u);
    uint2 vectorInserted = bitfieldInsert(vectorBase, uint2(3u, 5u), 2u, 3u);
    int2 vectorExtracted = bitfieldExtract(int2(value, value + 1u), 1u, 5u);
    bool hasValue = bool(value);
    destination[0] = int(inserted + firstLane(vectorInserted)) + extracted +
                     secondLane(vectorExtracted) + (hasValue ? 1 : 0);
}
)";

static const char kDirectNVVMFloatingTruthinessSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform float floatValue)
{
    half halfValue = half(floatValue);
    double doubleValue = double(floatValue);
    destination[0] = bool(halfValue) ? 1 : 0;
    destination[1] = bool(floatValue) ? 1 : 0;
    destination[2] = bool(doubleValue) ? 1 : 0;
}
)";

static const char kDirectNVVMLogicalNotSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform bool x)
{
    *destination = !x ? 1 : 0;
}
)";

static const char kDirectNVVMUnsignedIntegerBitNotSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform uint x)
{
    *destination = int(~x);
}
)";

static const char kDirectNVVMWideIntegerBitNotSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int64_t x)
{
    *destination = int(~x);
}
)";

static const char kDirectNVVMUnsignedIntegerNegateSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform uint x)
{
    *destination = int(-x);
}
)";

static const char kDirectNVVMWideIntegerNegateSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int64_t x)
{
    *destination = int(-x);
}
)";

static const char kDirectNVVMFloatingNegateSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform float x)
{
    *destination = int(-x);
}
)";

static const char kDirectNVVMRelaxedGlobalI32AtomicAddSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination)
{
    InterlockedAdd(*destination, 1);
}
)";

static const char kDirectNVVMRelaxedGlobalI32AtomicAddOldValueSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> oldValueDestination)
{
    int oldValue;
    InterlockedAdd(*destination, 1, oldValue);
    *oldValueDestination = oldValue;
}
)";

static const char kDirectNVVMUnsignedAtomicAddSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<uint, Access::ReadWrite, AddressSpace::Device> destination)
{
    InterlockedAdd(*destination, 1u);
}
)";

static const char kDirectNVVMAtomicReductionSource[] = R"(
RWStructuredBuffer<uint> integerTarget;
RWStructuredBuffer<float> floatTarget;
RWStructuredBuffer<double> doubleTarget;
RWStructuredBuffer<half2> half2Target;
RWStructuredBuffer<int> output;

[numthreads(1, 1, 1)]
void computeMain()
{
    __atomic_reduce_xor(integerTarget[0], 7u);
    __atomic_reduce_add(floatTarget[0], 0.5f);
    __atomic_reduce_add(doubleTarget[0], 0.25);
    __atomic_reduce_add(half2Target[0], half2(0.125h, 0.25h));
    output[0] = int(integerTarget[0]);
}
)";

static const char kDirectNVVMUnsupportedLocalAtomicReductionSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<uint, Access::ReadWrite, AddressSpace::Device> output)
{
    uint localValue = 0;
    __atomic_reduce_add(localValue, 1u);
    *output = localValue;
}
)";

static const char kDirectNVVMSharedHelperPointerSource[] = R"(
groupshared uint4 sharedValues[2];

[noinline]
void writeShared(Ptr<uint4, Access::ReadWrite, AddressSpace::GroupShared> values)
{
    values[0] = uint4(1u, 2u, 3u, 4u);
    values[1] = uint4(5u, 6u, 7u, 8u);
}

[CUDAKernel]
void computeMain(
    uniform Ptr<uint, Access::ReadWrite, AddressSpace::Device> output)
{
    writeShared(__getAddress(sharedValues[0]));
    *output = sharedValues[1].w;
}
)";

static const char kDirectNVVMWideAtomicAddSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int64_t, Access::ReadWrite, AddressSpace::Device> destination)
{
    InterlockedAdd(*destination, int64_t(1));
}
)";

static const char kDirectNVVMFloatingAtomicAddSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<float, Access::ReadWrite, AddressSpace::Device> destination)
{
    InterlockedAdd(*destination, 1.0f);
}
)";

static const char kDirectNVVMCommonSharedAtomicAlgebraSource[] = R"(
groupshared Atomic<int> atomicValue;

[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> output)
{
    atomicValue.store(0);
    output[0] = atomicValue.load();
    output[1] = atomicValue.exchange(1);
    output[2] = atomicValue.compareExchange(1, 2);
    output[3] = atomicValue.add(3);
    output[4] = atomicValue.sub(4);
    output[5] = atomicValue.max(5);
    output[6] = atomicValue.min(6);
    output[7] = atomicValue.and(7);
    output[8] = atomicValue.or(8);
    output[9] = atomicValue.xor(9);
    output[10] = atomicValue.increment();
    output[11] = atomicValue.decrement();
}
)";

static const char kDirectNVVMAcquireGlobalI32AtomicAddSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination)
{
    __atomic_add(*destination, 1, MemoryOrder::Acquire);
}
)";

static const char kDirectNVVMGroupSharedI32AtomicAddSource[] = R"(
groupshared int atomicCounter;

[CUDAKernel]
void computeMain()
{
    InterlockedAdd(atomicCounter, 1);
}
)";

static const char kDirectNVVMIntegerLeftShiftSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int x,
    uniform int amount)
{
    *destination = x << amount;
}
)";

static const char kDirectNVVMIntegerRightShiftSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int x,
    uniform int amount)
{
    *destination = x >> amount;
}
)";

static const char kDirectNVVMIntegerDivideSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int x,
    uniform int y)
{
    *destination = x / y;
}
)";

static const char kDirectNVVMIntegerRemainderSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int x,
    uniform int y)
{
    *destination = x % y;
}
)";

static const char kDirectNVVMUnsignedIntegerBitAndSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform uint x,
    uniform uint y)
{
    *destination = int(x & y);
}
)";

static const char kDirectNVVMWideIntegerBitAndSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int64_t x,
    uniform int64_t y)
{
    *destination = int(x & y);
}
)";

static const char kDirectNVVMUnsignedIntegerBitOrSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform uint x,
    uniform uint y)
{
    *destination = int(x | y);
}
)";

static const char kDirectNVVMWideIntegerBitOrSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int64_t x,
    uniform int64_t y)
{
    *destination = int(x | y);
}
)";

static const char kDirectNVVMUnsignedIntegerBitXorSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform uint x,
    uniform uint y)
{
    *destination = int(x ^ y);
}
)";

static const char kDirectNVVMWideIntegerBitXorSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int64_t x,
    uniform int64_t y)
{
    *destination = int(x ^ y);
}
)";

static const char kDirectNVVMPointerOffsetSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform Ptr<int, Access::Read, AddressSpace::Device> source,
    uniform int index)
{
    *(destination + index) = *(source + index);
}
)";

static const char kDirectNVVMUnsignedPointerOffsetSource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform Ptr<int, Access::Read, AddressSpace::Device> source)
{
    *(destination + uint(1)) = *(source + uint(1));
}
)";

static const char kDirectNVVMFixedDeviceArraySource[] = R"(
typealias RWIntArray4 = Ptr<int[4], Access::ReadWrite, AddressSpace::Device>;
typealias RIntArray4 = Ptr<int[4], Access::Read, AddressSpace::Device>;

[CUDAKernel]
void computeMain(
    uniform RWIntArray4 destination,
    uniform RIntArray4 source,
    uniform int index)
{
    (*destination)[index] = (*source)[index];
}
)";

static const char kDirectNVVMRawRWStructuredBufferI32StoreSource[] = R"(
[CUDAKernel]
void computeMain(RWStructuredBuffer<int> destination, uniform int index)
{
    destination[index] = 42;
}
)";

static const char kDirectNVVMRawRWStructuredBufferU32StoreSource[] = R"(
[CUDAKernel]
void computeMain(RWStructuredBuffer<uint> destination, uniform int index)
{
    destination[index] = 42;
}
)";

static const char kDirectNVVMRawRWStructuredBufferF32StoreSource[] = R"(
[CUDAKernel]
void computeMain(RWStructuredBuffer<float> destination, uniform int index)
{
    destination[index] = 42.0;
}
)";

static const char kDirectNVVMSelectedNumericStructuredBufferSource[] = R"(
[noinline]
half preserveHalf(half value)
{
    return value;
}

[CUDAKernel]
void computeMain(
    RWStructuredBuffer<half> halfValues,
    RWStructuredBuffer<double> doubleValues,
    RWStructuredBuffer<half2> halfVectors,
    RWStructuredBuffer<double2> doubleVectors,
    uniform int selector)
{
    bool flag = selector != 0;
    halfValues[0] = preserveHalf(half(flag));
    doubleValues[0] = double(flag);
    halfVectors[0] = half2(3.0, 4.0);
    doubleVectors[0] = double2(5.0, 6.0);
}
)";

static const char kDirectNVVMRawBufferDataPointerSource[] = R"(
[CUDAKernel]
void computeMain(
    RWStructuredBuffer<int> structuredSource,
    RWByteAddressBuffer byteSource,
    RWStructuredBuffer<int> destination,
    uniform uint index)
{
    let structuredPointer = __getStructuredBufferPtr(structuredSource);
    let bytePointer = __getByteAddressBufferPtr(byteSource);
    destination[index] = (*structuredPointer)[index] + int((*bytePointer)[index]);
}
)";

static const char kDirectNVVMReadOnlyByteAddressDataPointerSource[] = R"(
[CUDAKernel]
void computeMain(
    ByteAddressBuffer source,
    RWStructuredBuffer<uint> destination,
    uniform uint index)
{
    let sourcePointer = __getByteAddressBufferPtr(source);
    destination[index] = (*sourcePointer)[index];
}
)";

static const char kDirectNVVMReadOnlyByteAddressStoreSource[] = R"(
[CUDAKernel]
void computeMain(ByteAddressBuffer source, uniform uint index)
{
    let sourcePointer = __getByteAddressBufferPtr(source);
    (*sourcePointer)[index] = 42;
}
)";

static const char kDirectNVVMCoreByteAddressAccessSource[] = R"(
[CUDAKernel]
void computeMain(
    ByteAddressBuffer source,
    RWByteAddressBuffer destination,
    uniform Ptr<uint, Access::ReadWrite, AddressSpace::Device> output,
    uniform uint offset)
{
    uint4 sourceValues = source.Load4Aligned(offset, 16);
    uint destinationValue = destination.Load(offset + 4);
    destination.Store(offset, sourceValues.x + destinationValue);
    output[0] = sourceValues.y;
}
)";

static const char kDirectNVVMFloatVectorByteAddressAccessSource[] = R"(
[CUDAKernel]
void computeMain(
    RWByteAddressBuffer source,
    uniform Ptr<float, Access::ReadWrite, AddressSpace::Device> destination)
{
    float4 wide = source.LoadAligned<float4>(0, 16);
    float4 scalarized = source.LoadAligned<float4>(16, 4);
    source.StoreAligned(32, scalarized);
    source.Store<float4>(48, wide, 4);
    int signedValue = source.Load<int>(64);
    source.Store<int>(68, signedValue);
    destination[0] = wide.x;
}
)";

static const char kDirectNVVMWideIntegerByteAddressAccessSource[] = R"(
[CUDAKernel]
void computeMain(
    ByteAddressBuffer readOnlySource,
    RWByteAddressBuffer readWriteSource,
    uniform Ptr<uint64_t, Access::ReadWrite, AddressSpace::Device> destination)
{
    int64_t signedValue = readOnlySource.LoadAligned<int64_t>(0, 8);
    uint64_t unsignedValue = readWriteSource.Load<uint64_t>(8);
    readWriteSource.Store<int64_t>(16, signedValue, 8);
    readWriteSource.Store<uint64_t>(24, unsignedValue);
    destination[0] = unsignedValue;
}
)";

static const char kDirectNVVMNumericArrayByteAddressAccessSource[] = R"(
struct Block
{
    float4 values[2];
};

[CUDAKernel]
void computeMain(ByteAddressBuffer source, RWByteAddressBuffer destination)
{
    destination.Store(0, source.LoadAligned<Block>(0));
}
)";

static const char kDirectNVVMUnsupportedNestedArrayByteAddressAccessSource[] = R"(
struct NestedBlock
{
    float4 values[2][2];
};

[CUDAKernel]
void computeMain(RWByteAddressBuffer source)
{
    source.Store(0, source.LoadAligned<NestedBlock>(0));
}
)";

static const char kDirectNVVMAggregateAndReadOnlyResourceSource[] = R"(
struct Padding
{
    uint64_t big;
    uint16_t little;
};

[CUDAKernel]
void computeMain(
    uniform Padding padding,
    RWStructuredBuffer<float> destination,
    StructuredBuffer<float> source,
    uniform uint index)
{
    destination[index] = source[index] + float(padding.big) + float(padding.little);
}
)";

static const char kDirectNVVMVectorStructuredBufferSource[] = R"(
[CUDAKernel]
void computeMain(
    StructuredBuffer<int4> source,
    RWStructuredBuffer<float4> destination,
    RWStructuredBuffer<int> output)
{
    int4 loaded = source[0];
    destination[0].wzyx = float4(1.0, 2.0, 3.0, 4.0);
    destination[0][0] = 5.0;
    float4 stored = destination[0];
    output[0] = loaded.x + int(stored.w);
}
)";

static const char kDirectNVVMRawRWStructuredBufferU32AtomicAddSource[] = R"(
[CUDAKernel]
void computeMain(RWStructuredBuffer<uint> destination, uniform uint index)
{
    InterlockedAdd(destination[index], 1u);
}
)";

static const char kDirectNVVMMixedWidthByteAddressAtomicSource[] = R"(
RWByteAddressBuffer buffer;

void doMax(RWByteAddressBuffer uav)
{
    uav.InterlockedMaxU64(0, 5);
}

[CUDAKernel]
void computeMain()
{
    doMax(buffer);
    uint previous;
    buffer.InterlockedAdd(16, 3, previous);
}
)";

static const char kDirectNVVMRawBufferHelperSource[] = R"(
uint preserveStructured(StructuredBuffer<int> source, uint value)
{
    return value;
}

uint preserveByte(ByteAddressBuffer source, uint value)
{
    return value;
}

void writeStructured(RWStructuredBuffer<uint> destination, uint index, uint value)
{
    destination[index] = value;
}

void incrementByte(RWByteAddressBuffer destination, uint offset)
{
    uint originalValue;
    destination.InterlockedAdd(offset, 1u, originalValue);
}

[CUDAKernel]
void computeMain(
    StructuredBuffer<int> structuredSource,
    ByteAddressBuffer byteSource,
    RWStructuredBuffer<uint> destination,
    RWByteAddressBuffer counter,
    uniform uint index)
{
    uint value = preserveStructured(structuredSource, index) + preserveByte(byteSource, index);
    writeStructured(destination, index, value);
    incrementByte(counter, 0);
}
)";

static const char kDirectNVVMResourceResultHelperSource[] = R"(
RWStructuredBuffer<float> destination;
Texture2D texture;
SamplerState sampler;

[noinline]
RWStructuredBuffer<float> preserveDestination(RWStructuredBuffer<float> value)
{
    return value;
}

[noinline]
Texture2D preserveTexture(Texture2D value)
{
    return value;
}

[CUDAKernel]
void computeMain()
{
    preserveDestination(destination)[0] =
        preserveTexture(texture).SampleLevel(sampler, float2(0.0), 0.0).x;
}
)";

static const char kDirectNVVMTexture2DGatherSource[] = R"(
Texture2D<float4> texture;
SamplerState sampler;
RWStructuredBuffer<float4> destination;

[CUDAKernel]
void computeMain()
{
    float2 coordinate = float2(0.5, 0.5);
    destination[0] = texture.GatherRed(sampler, coordinate);
    destination[1] = texture.GatherGreen(sampler, coordinate);
    destination[2] = texture.GatherBlue(sampler, coordinate);
    destination[3] = texture.GatherAlpha(sampler, coordinate, int2(1, 1));
}
)";

static const char kDirectNVVMTexture2DImplicitSampleSource[] = R"(
Texture2D<float4> texture;
SamplerState sampler;
RWStructuredBuffer<float4> destination;

[CUDAKernel]
void computeMain()
{
    destination[0] = texture.Sample(sampler, float2(0.5, 0.5));
}
)";

static const char kDirectNVVMUnsignedFixedArrayIndexSource[] = R"(
typealias RWIntArray4 = Ptr<int[4], Access::ReadWrite, AddressSpace::Device>;
typealias RIntArray4 = Ptr<int[4], Access::Read, AddressSpace::Device>;

[CUDAKernel]
void computeMain(uniform RWIntArray4 destination, uniform RIntArray4 source)
{
    (*destination)[uint(1)] = (*source)[uint(1)];
}
)";

static const char kDirectNVVMUnsupportedFloatArraySource[] = R"(
typealias RWFloatArray4 = Ptr<float[4], Access::ReadWrite, AddressSpace::Device>;
typealias RFloatArray4 = Ptr<float[4], Access::Read, AddressSpace::Device>;

[CUDAKernel]
void computeMain(
    uniform RWFloatArray4 destination,
    uniform RFloatArray4 source,
    uniform int index)
{
    (*destination)[index] = (*source)[index];
}
)";

static const char kDirectNVVMUnsupportedNestedArraySource[] = R"(
typealias RWNestedArray = Ptr<int[2][2], Access::ReadWrite, AddressSpace::Device>;
typealias RNestedArray = Ptr<int[2][2], Access::Read, AddressSpace::Device>;

[CUDAKernel]
void computeMain(
    uniform RWNestedArray destination,
    uniform RNestedArray source,
    uniform int index)
{
    (*destination)[index][0] = (*source)[index][0];
}
)";

static const char kDirectNVVMUnsupportedNestedLocalArraySource[] = R"(
[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int x)
{
    int values[2][2];
    values[0][0] = x;
    *destination = values[0][0];
}
)";

static const char kDirectNVVMUnsupportedStructPointerSource[] = R"(
struct Pair
{
    int x;
    int y;
};

[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform Ptr<Pair, Access::Read, AddressSpace::Device> source)
{
    *destination = (*source).x;
}
)";

static const char kDirectNVVMUnsupportedArrayPointerHelperSource[] = R"(
typealias RIntArray4 = Ptr<int[4], Access::Read, AddressSpace::Device>;

int readArrayElement(RIntArray4 source, int index)
{
    return (*source)[index];
}

[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform RIntArray4 source,
    uniform int index)
{
    *destination = readArrayElement(source, index);
}
)";

static const char kDirectNVVMUnsupportedPointerHelperParameterSource[] = R"(
int readValue(Ptr<int, Access::Read, AddressSpace::Device> source)
{
    return *source;
}

[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform Ptr<int, Access::Read, AddressSpace::Device> source)
{
    *destination = readValue(source);
}
)";

static const char kDirectNVVMUnsupportedPointerHelperResultSource[] = R"(
Ptr<int, Access::Read, AddressSpace::Device> identity(
    Ptr<int, Access::Read, AddressSpace::Device> source)
{
    return source;
}

[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform Ptr<int, Access::Read, AddressSpace::Device> source)
{
    *destination = *identity(source);
}
)";

static const char kDirectNVVMScalarFunctionSource[] = R"(
int increment(int value)
{
    return value + 1;
}

int incrementTwice(int value)
{
    return increment(increment(value));
}

[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int value)
{
    *destination = increment(value) + incrementTwice(value);
}
)";

static const char kDirectNVVMVectorFunctionSource[] = R"(
RWStructuredBuffer<int> outputBuffer;

[noinline]
int4 chooseInt4(bool condition, int4 left, int4 right)
{
    return condition ? left : right;
}

[noinline]
float3 identityFloat3(float3 value)
{
    return value;
}

[noinline]
bool2 identityBool2(bool2 value)
{
    return value;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    int4 selected = chooseInt4(true, int4(1, 2, 3, 4), int4(5, 6, 7, 8));
    float3 floats = identityFloat3(float3(9.0, 10.0, 11.0));
    bool2 booleans = identityBool2(int2(1, 2) == int2(1, 3));
    outputBuffer[0] = selected.x + int(floats.y) + (booleans.x ? 1 : 0);
}
)";

static const char kDirectNVVMUnsupportedVectorFunctionSources[][512] = {
    R"(
RWStructuredBuffer<int> outputBuffer;
vector<int, 5> identity(vector<int, 5> value) { return value; }
[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain() { outputBuffer[0] = identity(vector<int, 5>(1)).x; }
)",
};

static const char kDirectNVVMFunctionContractSource[] = R"(
RWStructuredBuffer<int> outputBuffer;

[noinline]
int helperFunc(int value)
{
    return value + 1;
}

int plainHelper(int value)
{
    return value * 2;
}

[CudaDeviceExport]
[noinline]
int exportedFunc(int value)
{
    return value + 3;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = helperFunc(42) + plainHelper(7) + exportedFunc(1);
}
)";

static const char kDirectNVVMPrunesUnreachableHelperSource[] = R"(
int unusedMultiply(int x, int y)
{
    return x * y;
}

int increment(int value)
{
    return value + 1;
}

[CUDAKernel]
void computeMain(
    uniform Ptr<int, Access::ReadWrite, AddressSpace::Device> destination,
    uniform int value)
{
    *destination = increment(value);
}
)";

} // namespace
