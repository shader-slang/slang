// Buffer and other opaque resource types cannot be packed into the
// fixed-size AnyValue byte buffer used for dynamic dispatch on
// non-bindless targets.
//
// This test covers typed buffers, byte-address buffers, structured
// buffer variants, acceleration structures, and subpass inputs --
// complementing diagnose-opaque-types-in-impl.slang which covers
// textures, samplers, ray queries, and stream outputs.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -conformance "ImplBuffer:IFoo=0" -conformance "ImplRWBuffer:IFoo=1" -conformance "ImplByteAddressBuffer:IFoo=2" -conformance "ImplRWByteAddressBuffer:IFoo=3" -conformance "ImplStructuredBuffer:IFoo=4" -conformance "ImplRWStructuredBuffer:IFoo=5" -conformance "ImplAppendStructuredBuffer:IFoo=6" -conformance "ImplConsumeStructuredBuffer:IFoo=7" -conformance "ImplRTAS:IFoo=8" -conformance "ImplSubpassInput:IFoo=9" -conformance "GoodImpl:IFoo=10"
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -stage compute -entry computeMain -conformance "ImplBuffer:IFoo=0" -conformance "ImplRWBuffer:IFoo=1" -conformance "ImplByteAddressBuffer:IFoo=2" -conformance "ImplRWByteAddressBuffer:IFoo=3" -conformance "ImplStructuredBuffer:IFoo=4" -conformance "ImplRWStructuredBuffer:IFoo=5" -conformance "ImplAppendStructuredBuffer:IFoo=6" -conformance "ImplConsumeStructuredBuffer:IFoo=7" -conformance "ImplRTAS:IFoo=8" -conformance "ImplSubpassInput:IFoo=9" -conformance "GoodImpl:IFoo=10"

interface IFoo
{
    float get();
}

export struct ImplBuffer : IFoo
//CHECK:      ^^^^^^^^^^ type cannot be packed for dynamic dispatch
//CHECK:      ^^^^^^^^^^ type 'ImplBuffer' contains fields that cannot be packed into ordinary bytes for dynamic dispatch.
{
    Buffer<float> b;
    float get() { return 1.0; }
}

export struct ImplRWBuffer : IFoo
//CHECK:      ^^^^^^^^^^^^ type cannot be packed for dynamic dispatch
//CHECK:      ^^^^^^^^^^^^ type 'ImplRWBuffer' contains fields that cannot be packed into ordinary bytes for dynamic dispatch.
{
    RWBuffer<float> b;
    float get() { return 1.0; }
}

export struct ImplByteAddressBuffer : IFoo
//CHECK:      ^^^^^^^^^^^^^^^^^^^^^ type cannot be packed for dynamic dispatch
//CHECK:      ^^^^^^^^^^^^^^^^^^^^^ type 'ImplByteAddressBuffer' contains fields that cannot be packed into ordinary bytes for dynamic dispatch.
{
    ByteAddressBuffer b;
    float get() { return 1.0; }
}

export struct ImplRWByteAddressBuffer : IFoo
//CHECK:      ^^^^^^^^^^^^^^^^^^^^^^^ type cannot be packed for dynamic dispatch
//CHECK:      ^^^^^^^^^^^^^^^^^^^^^^^ type 'ImplRWByteAddressBuffer' contains fields that cannot be packed into ordinary bytes for dynamic dispatch.
{
    RWByteAddressBuffer b;
    float get() { return 1.0; }
}

export struct ImplStructuredBuffer : IFoo
//CHECK:      ^^^^^^^^^^^^^^^^^^^^ type cannot be packed for dynamic dispatch
//CHECK:      ^^^^^^^^^^^^^^^^^^^^ type 'ImplStructuredBuffer' contains fields that cannot be packed into ordinary bytes for dynamic dispatch.
{
    StructuredBuffer<float> b;
    float get() { return 1.0; }
}

export struct ImplRWStructuredBuffer : IFoo
//CHECK:      ^^^^^^^^^^^^^^^^^^^^^^ type cannot be packed for dynamic dispatch
//CHECK:      ^^^^^^^^^^^^^^^^^^^^^^ type 'ImplRWStructuredBuffer' contains fields that cannot be packed into ordinary bytes for dynamic dispatch.
{
    RWStructuredBuffer<float> b;
    float get() { return 1.0; }
}

export struct ImplAppendStructuredBuffer : IFoo
//CHECK:      ^^^^^^^^^^^^^^^^^^^^^^^^^^ type cannot be packed for dynamic dispatch
//CHECK:      ^^^^^^^^^^^^^^^^^^^^^^^^^^ type 'ImplAppendStructuredBuffer' contains fields that cannot be packed into ordinary bytes for dynamic dispatch.
{
    AppendStructuredBuffer<float> b;
    float get() { return 1.0; }
}

export struct ImplConsumeStructuredBuffer : IFoo
//CHECK:      ^^^^^^^^^^^^^^^^^^^^^^^^^^^ type cannot be packed for dynamic dispatch
//CHECK:      ^^^^^^^^^^^^^^^^^^^^^^^^^^^ type 'ImplConsumeStructuredBuffer' contains fields that cannot be packed into ordinary bytes for dynamic dispatch.
{
    ConsumeStructuredBuffer<float> b;
    float get() { return 1.0; }
}

export struct ImplRTAS : IFoo
//CHECK:      ^^^^^^^^ type cannot be packed for dynamic dispatch
//CHECK:      ^^^^^^^^ type 'ImplRTAS' contains fields that cannot be packed into ordinary bytes for dynamic dispatch.
{
    RaytracingAccelerationStructure accel;
    float get() { return 1.0; }
}

export struct ImplSubpassInput : IFoo
//CHECK:      ^^^^^^^^^^^^^^^^ type cannot be packed for dynamic dispatch
//CHECK:      ^^^^^^^^^^^^^^^^ type 'ImplSubpassInput' contains fields that cannot be packed into ordinary bytes for dynamic dispatch.
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
