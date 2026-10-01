//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry chsMain -stage closesthit

// Regression test for shader-slang/slang#12628: a shader-record global that reaches its reads
// indirectly still reads the OptiX SBT record, and none of those reads is `__ldg`'d (#10188).
//
// Specialization turns some indirect uses into direct reads. Other uses keep the buffer as a value,
// whose loads `lowerImmutableBufferLoadForCUDA` keeps off `__ldg` because the value has the
// record's type. That check is keyed by type, so `gOther`, an ordinary buffer of the same type, also
// loses `__ldg`, while `gParams`, of another type, keeps it. CUDA emits helpers before the kernel.

//CHECK-NOT: __ldg

// `expand`: one SBT read per unrolled element.
//CHECK: sumPlusId_0(
//CHECK-NOT: __ldg
//CHECK: optixGetSbtDataPointer())->id_0
//CHECK-NOT: __ldg
//CHECK: optixGetSbtDataPointer())->id_0
//CHECK-NOT: __ldg

// Interface method called from a generic.
//CHECK: SbtColor_get_0()
//CHECK-NOT: __ldg
//CHECK: optixGetSbtDataPointer())->color_0
//CHECK-NOT: __ldg

// A buffer of another type keeps `__ldg`.
//CHECK: readScale_0()
//CHECK: __ldg(&globalParams_0->gParams_0->scale_0)
//CHECK-NOT: __ldg

// Struct field: the read through `holder_0` has no `__ldg`.
//CHECK: readHolder_0(
//CHECK-NOT: __ldg

// A returned record handle still points into the SBT when its caller reads `id`.
//CHECK: getRecord_0()
//CHECK-NOT: __ldg
//CHECK: return ((SbtData_0*)optixGetSbtDataPointer());
//CHECK-NOT: __ldg

// `select` operand, read through a `ConstantBuffer` parameter.
//CHECK: readId_0(
//CHECK-NOT: __ldg

// `gOther` is in `GlobalParams` but shares the SBT handle type, so it also loses `__ldg`.
//CHECK: readOtherDirect_0()
//CHECK-NOT: __ldg
//CHECK: return globalParams_0->gOther_0->id_0;
//CHECK-NOT: __ldg

// `ConstantBuffer` argument: `readFrom_0()` loses its parameter.
//CHECK: readFrom_0()
//CHECK-NOT: __ldg
//CHECK: optixGetSbtDataPointer())->color_0
//CHECK-NOT: __ldg

// The kernel stores the record in `holder` and passes it to the `select`.
//CHECK: __closesthit__chsMain
//CHECK-NOT: __ldg
//CHECK: buffer_0 = ((SbtData_0*)optixGetSbtDataPointer());
//CHECK-NOT: __ldg
//CHECK: = ((SbtData_0*)optixGetSbtDataPointer());
//CHECK-NOT: __ldg

struct SbtData
{
    float4 color;
    uint id;
};

layout(shaderRecordEXT) ConstantBuffer<SbtData> gSbtRecordData;
ConstantBuffer<SbtData> gOther;

struct Params
{
    float scale;
};
ConstantBuffer<Params> gParams;

RWStructuredBuffer<uint> gOut;

float4 readFrom(ConstantBuffer<SbtData> record)
{
    return record.color;
}

interface IColorSource
{
    float4 get();
}

struct SbtColor : IColorSource
{
    float4 get() { return gSbtRecordData.color; }
}

float4 readVia<T : IColorSource>(T source)
{
    return source.get();
}

void accumulate(inout uint dest, uint value)
{
    dest += value;
}

uint sumPlusId<each T>(expand each T args)
    where T == uint
{
    uint result = 0;
    expand accumulate(result, (each args) + gSbtRecordData.id);
    return result;
}

struct Holder
{
    ConstantBuffer<SbtData> buffer;
}

uint readHolder(Holder holder)
{
    return holder.buffer.id;
}

ConstantBuffer<SbtData> getRecord()
{
    return gSbtRecordData;
}

uint readId(ConstantBuffer<SbtData> record)
{
    return record.id;
}

float readScale()
{
    return gParams.scale;
}

uint readOtherDirect()
{
    return gOther.id;
}

[shader("closesthit")]
void chsMain(inout float4 payload, in BuiltInTriangleIntersectionAttributes attr)
{
    SbtColor source;
    payload = (readFrom(gSbtRecordData) + readVia(source)) * readScale();

    Holder holder;
    holder.buffer = gSbtRecordData;
    gOut[0] = sumPlusId(1u, 2u) + readHolder(holder) + getRecord().id +
              readId(payload.x > 0 ? gSbtRecordData : gOther) + readOtherDirect();
}
