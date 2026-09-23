// The front end admits each file- or namespace-scope `static` resource below because its value
// remains one IR value throughout resource-global legalization. The emitted program must use the
// corresponding shader parameter directly and contain no declaration whose name starts with
// `cached`.

//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry main -stage compute -O0 -validate-ir

//CHECK-NOT: cached
//CHECK-DAG: StructuredBuffer<float4 > [[STRUCTURED:inputStructured_[0-9]+]]
//CHECK-DAG: RWStructuredBuffer<float4 > [[RW_STRUCTURED:inputRWStructured_[0-9]+]]
//CHECK-DAG: RasterizerOrderedStructuredBuffer<float4 > [[RO_STRUCTURED:inputROStructured_[0-9]+]]
//CHECK-DAG: RWByteAddressBuffer [[RW_BYTES:inputRWBytes_[0-9]+]]
//CHECK-DAG: RasterizerOrderedByteAddressBuffer [[RO_BYTES:inputROBytes_[0-9]+]]
//CHECK-LABEL: void main()
//CHECK: [[STRUCTURED]].Load(int(0)) + [[RW_STRUCTURED]][int(0)] + [[RO_STRUCTURED]][int(0)]
//CHECK: [[RW_BYTES]].Load4(0U)
//CHECK: [[RO_BYTES]].Load4(0U)
//CHECK-NOT: cached

StructuredBuffer<float4> inputStructured;
RWStructuredBuffer<float4> inputRWStructured;
RasterizerOrderedStructuredBuffer<float4> inputROStructured;
RWByteAddressBuffer inputRWBytes;
RasterizerOrderedByteAddressBuffer inputROBytes;
RWStructuredBuffer<float4> output;

static StructuredBuffer<float4> cachedStructured = inputStructured;
static RWStructuredBuffer<float4> cachedRWStructured = inputRWStructured;
static RasterizerOrderedStructuredBuffer<float4> cachedROStructured = inputROStructured;
static RWByteAddressBuffer cachedRWBytes = inputRWBytes;
static RasterizerOrderedByteAddressBuffer cachedROBytes = inputROBytes;

[numthreads(1, 1, 1)]
void main()
{
    output[0] = cachedStructured[0] + cachedRWStructured[0] + cachedROStructured[0] +
                asfloat(cachedRWBytes.Load4(0)) + asfloat(cachedROBytes.Load4(0));
}
