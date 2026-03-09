// Opaque GPU resource types (textures, samplers, ray queries, stream
// outputs) cannot be packed into the fixed-size AnyValue byte buffer
// used for dynamic dispatch. On non-bindless targets, opaque types have
// no byte-level representation, so implementations containing them are
// rejected at compile time.
//
// The existing test in tests/diagnostics/resource-type-in-dynamic-dispatch.slang
// covers Texture2D. This test extends coverage to other opaque resource types.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -conformance "ImplSampler:IFoo=0" -conformance "ImplRWTexture:IFoo=1" -conformance "ImplTexture1D:IFoo=2" -conformance "ImplTexture3D:IFoo=3" -conformance "ImplTextureCube:IFoo=4" -conformance "ImplRayQuery:IFoo=5" -conformance "ImplTriangleStream:IFoo=6" -conformance "ImplMeshOutput:IFoo=7" -conformance "GoodImpl:IFoo=8"
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -stage compute -entry computeMain -conformance "ImplSampler:IFoo=0" -conformance "ImplRWTexture:IFoo=1" -conformance "ImplTexture1D:IFoo=2" -conformance "ImplTexture3D:IFoo=3" -conformance "ImplTextureCube:IFoo=4" -conformance "ImplRayQuery:IFoo=5" -conformance "ImplTriangleStream:IFoo=6" -conformance "ImplMeshOutput:IFoo=7" -conformance "GoodImpl:IFoo=8"

struct MyVertex
{
    float4 pos : SV_Position;
};

interface IFoo
{
    float get();
}

export struct ImplSampler : IFoo
//CHECK:      ^^^^^^^^^^^ type cannot be packed for dynamic dispatch
//CHECK:      ^^^^^^^^^^^ type 'ImplSampler' contains fields that cannot be packed into ordinary bytes for dynamic dispatch.
{
    SamplerState s;
    float get() { return 1.0; }
}

export struct ImplRWTexture : IFoo
//CHECK:      ^^^^^^^^^^^^^ type cannot be packed for dynamic dispatch
//CHECK:      ^^^^^^^^^^^^^ type 'ImplRWTexture' contains fields that cannot be packed into ordinary bytes for dynamic dispatch.
{
    RWTexture2D<float> t;
    float get() { return 1.0; }
}

export struct ImplTexture1D : IFoo
//CHECK:      ^^^^^^^^^^^^^ type cannot be packed for dynamic dispatch
//CHECK:      ^^^^^^^^^^^^^ type 'ImplTexture1D' contains fields that cannot be packed into ordinary bytes for dynamic dispatch.
{
    Texture1D<float> t;
    float get() { return 1.0; }
}

export struct ImplTexture3D : IFoo
//CHECK:      ^^^^^^^^^^^^^ type cannot be packed for dynamic dispatch
//CHECK:      ^^^^^^^^^^^^^ type 'ImplTexture3D' contains fields that cannot be packed into ordinary bytes for dynamic dispatch.
{
    Texture3D<float> t;
    float get() { return 1.0; }
}

export struct ImplTextureCube : IFoo
//CHECK:      ^^^^^^^^^^^^^^^ type cannot be packed for dynamic dispatch
//CHECK:      ^^^^^^^^^^^^^^^ type 'ImplTextureCube' contains fields that cannot be packed into ordinary bytes for dynamic dispatch.
{
    TextureCube<float> t;
    float get() { return 1.0; }
}

export struct ImplRayQuery : IFoo
//CHECK:      ^^^^^^^^^^^^ type cannot be packed for dynamic dispatch
//CHECK:      ^^^^^^^^^^^^ type 'ImplRayQuery' contains fields that cannot be packed into ordinary bytes for dynamic dispatch.
{
    RayQuery<RAY_FLAG_NONE> rq;
    float get() { return 1.0; }
}

export struct ImplTriangleStream : IFoo
//CHECK:      ^^^^^^^^^^^^^^^^^^ type cannot be packed for dynamic dispatch
//CHECK:      ^^^^^^^^^^^^^^^^^^ type 'ImplTriangleStream' contains fields that cannot be packed into ordinary bytes for dynamic dispatch.
{
    TriangleStream<MyVertex> stream;
    float get() { return 1.0; }
}

export struct ImplMeshOutput : IFoo
//CHECK:      ^^^^^^^^^^^^^^ type cannot be packed for dynamic dispatch
//CHECK:      ^^^^^^^^^^^^^^ type 'ImplMeshOutput' contains fields that cannot be packed into ordinary bytes for dynamic dispatch.
{
    OutputVertices<MyVertex, 3> verts;
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
    IFoo f = createDynamicObject<IFoo>(8, 0);
    outputBuffer[0] = f.get();
}
