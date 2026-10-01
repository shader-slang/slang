//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry chsMain -stage closesthit

// Regression test for shader-slang/slang#12628: a shader-record global that reaches its reads
// indirectly still reads the OptiX SBT record, and none of those reads is `__ldg`'d (#10188).
//
// Specialization turns some indirect uses into direct reads: the buffer passed to a helper as a
// `ConstantBuffer` argument (`readFrom_0()` loses its parameter), read through an interface
// method called from a generic, and read inside an `expand`. Other uses keep the buffer as a
// value: stored in a struct field, returned from a function, or picked by a `select`.
//
// A genuine constant buffer of another type keeps its `__ldg` (`readScale_0`), while `gOther`,
// which has the record's type, is read like the record.

//CHECK-NOT: __ldg
//CHECK: sumPlusId_0(
//CHECK-NOT: __ldg
//CHECK: optixGetSbtDataPointer())->id_0
//CHECK-NOT: __ldg
//CHECK: optixGetSbtDataPointer())->id_0
//CHECK-NOT: __ldg
//CHECK: SbtColor_get_0()
//CHECK-NOT: __ldg
//CHECK: optixGetSbtDataPointer())->color_0
//CHECK-NOT: __ldg
//CHECK: readScale_0()
//CHECK: __ldg(&globalParams_0->gParams_0->scale_0)
//CHECK-NOT: __ldg
//CHECK: readHolder_0(
//CHECK-NOT: __ldg
//CHECK: getRecord_0()
//CHECK-NOT: __ldg
//CHECK: return ((SbtData_0*)optixGetSbtDataPointer());
//CHECK-NOT: __ldg
//CHECK: readId_0(
//CHECK-NOT: __ldg
//CHECK: readFrom_0()
//CHECK-NOT: __ldg
//CHECK: optixGetSbtDataPointer())->color_0
//CHECK-NOT: __ldg
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

[shader("closesthit")]
void chsMain(inout float4 payload, in BuiltInTriangleIntersectionAttributes attr)
{
    SbtColor source;
    payload = (readFrom(gSbtRecordData) + readVia(source)) * readScale();

    Holder holder;
    holder.buffer = gSbtRecordData;
    gOut[0] = sumPlusId(1u, 2u) + readHolder(holder) + getRecord().id +
              readId(payload.x > 0 ? gSbtRecordData : gOther);
}
