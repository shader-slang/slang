// Buffer and other opaque resource types cannot be packed into the
// fixed-size AnyValue byte buffer used for dynamic dispatch on
// non-bindless targets.
//
// This test covers typed buffers, byte-address buffers, structured
// buffer variants, acceleration structures, and subpass inputs --
// complementing diagnose-opaque-types-in-impl.slang which covers
// textures, samplers, ray queries, and stream outputs.

//TEST:SIMPLE(filecheck=CHECK): -target spirv -conformance "ImplBuffer:IFoo=0" -conformance "ImplRWBuffer:IFoo=1" -conformance "ImplByteAddressBuffer:IFoo=2" -conformance "ImplRWByteAddressBuffer:IFoo=3" -conformance "ImplStructuredBuffer:IFoo=4" -conformance "ImplRWStructuredBuffer:IFoo=5" -conformance "ImplAppendStructuredBuffer:IFoo=6" -conformance "ImplConsumeStructuredBuffer:IFoo=7" -conformance "ImplRTAS:IFoo=8" -conformance "ImplSubpassInput:IFoo=9" -conformance "GoodImpl:IFoo=10"
//TEST:SIMPLE(filecheck=CHECK): -target hlsl -stage compute -entry computeMain -conformance "ImplBuffer:IFoo=0" -conformance "ImplRWBuffer:IFoo=1" -conformance "ImplByteAddressBuffer:IFoo=2" -conformance "ImplRWByteAddressBuffer:IFoo=3" -conformance "ImplStructuredBuffer:IFoo=4" -conformance "ImplRWStructuredBuffer:IFoo=5" -conformance "ImplAppendStructuredBuffer:IFoo=6" -conformance "ImplConsumeStructuredBuffer:IFoo=7" -conformance "ImplRTAS:IFoo=8" -conformance "ImplSubpassInput:IFoo=9" -conformance "GoodImpl:IFoo=10"

interface IFoo
{
    float get();
}

// CHECK-DAG: type 'ImplBuffer' contains fields that cannot be packed into ordinary bytes for dynamic dispatch
export struct ImplBuffer : IFoo
{
    Buffer<float> b;
    float get() { return 1.0; }
}

// CHECK-DAG: type 'ImplRWBuffer' contains fields that cannot be packed into ordinary bytes for dynamic dispatch
export struct ImplRWBuffer : IFoo
{
    RWBuffer<float> b;
    float get() { return 1.0; }
}

// CHECK-DAG: type 'ImplByteAddressBuffer' contains fields that cannot be packed into ordinary bytes for dynamic dispatch
export struct ImplByteAddressBuffer : IFoo
{
    ByteAddressBuffer b;
    float get() { return 1.0; }
}

// CHECK-DAG: type 'ImplRWByteAddressBuffer' contains fields that cannot be packed into ordinary bytes for dynamic dispatch
export struct ImplRWByteAddressBuffer : IFoo
{
    RWByteAddressBuffer b;
    float get() { return 1.0; }
}

// CHECK-DAG: type 'ImplStructuredBuffer' contains fields that cannot be packed into ordinary bytes for dynamic dispatch
export struct ImplStructuredBuffer : IFoo
{
    StructuredBuffer<float> b;
    float get() { return 1.0; }
}

// CHECK-DAG: type 'ImplRWStructuredBuffer' contains fields that cannot be packed into ordinary bytes for dynamic dispatch
export struct ImplRWStructuredBuffer : IFoo
{
    RWStructuredBuffer<float> b;
    float get() { return 1.0; }
}

// CHECK-DAG: type 'ImplAppendStructuredBuffer' contains fields that cannot be packed into ordinary bytes for dynamic dispatch
export struct ImplAppendStructuredBuffer : IFoo
{
    AppendStructuredBuffer<float> b;
    float get() { return 1.0; }
}

// CHECK-DAG: type 'ImplConsumeStructuredBuffer' contains fields that cannot be packed into ordinary bytes for dynamic dispatch
export struct ImplConsumeStructuredBuffer : IFoo
{
    ConsumeStructuredBuffer<float> b;
    float get() { return 1.0; }
}

// CHECK-DAG: type 'ImplRTAS' contains fields that cannot be packed into ordinary bytes for dynamic dispatch
export struct ImplRTAS : IFoo
{
    RaytracingAccelerationStructure accel;
    float get() { return 1.0; }
}

// CHECK-DAG: type 'ImplSubpassInput' contains fields that cannot be packed into ordinary bytes for dynamic dispatch
export struct ImplSubpassInput : IFoo
{
    SubpassInput si;
    float get() { return 1.0; }
}

export struct GoodImpl : IFoo
{
    float v;
    float get() { return v; }
}

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    IFoo f = createDynamicObject<IFoo>(10, 0);
    outputBuffer[0] = f.get();
}
