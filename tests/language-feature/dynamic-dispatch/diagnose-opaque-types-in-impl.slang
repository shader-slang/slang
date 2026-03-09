// Opaque GPU resource types (textures, samplers, ray queries, stream
// outputs) cannot be packed into the fixed-size AnyValue byte buffer
// used for dynamic dispatch. On non-bindless targets, opaque types have
// no byte-level representation, so implementations containing them are
// rejected at compile time.
//
// The existing test in tests/diagnostics/resource-type-in-dynamic-dispatch.slang
// covers Texture2D. This test extends coverage to other opaque resource types.

//TEST:SIMPLE(filecheck=CHECK): -target spirv -conformance "ImplSampler:IFoo=0" -conformance "ImplRWTexture:IFoo=1" -conformance "ImplTexture1D:IFoo=2" -conformance "ImplTexture3D:IFoo=3" -conformance "ImplTextureCube:IFoo=4" -conformance "ImplRayQuery:IFoo=5" -conformance "ImplTriangleStream:IFoo=6" -conformance "ImplMeshOutput:IFoo=7" -conformance "GoodImpl:IFoo=8"
//TEST:SIMPLE(filecheck=CHECK): -target hlsl -stage compute -entry computeMain -conformance "ImplSampler:IFoo=0" -conformance "ImplRWTexture:IFoo=1" -conformance "ImplTexture1D:IFoo=2" -conformance "ImplTexture3D:IFoo=3" -conformance "ImplTextureCube:IFoo=4" -conformance "ImplRayQuery:IFoo=5" -conformance "ImplTriangleStream:IFoo=6" -conformance "ImplMeshOutput:IFoo=7" -conformance "GoodImpl:IFoo=8"

struct MyVertex
{
    float4 pos : SV_Position;
};

interface IFoo
{
    float get();
}

// CHECK-DAG: type 'ImplSampler' contains fields that cannot be packed into ordinary bytes for dynamic dispatch
export struct ImplSampler : IFoo
{
    SamplerState s;
    float get() { return 1.0; }
}

// CHECK-DAG: type 'ImplRWTexture' contains fields that cannot be packed into ordinary bytes for dynamic dispatch
export struct ImplRWTexture : IFoo
{
    RWTexture2D<float> t;
    float get() { return 1.0; }
}

// CHECK-DAG: type 'ImplTexture1D' contains fields that cannot be packed into ordinary bytes for dynamic dispatch
export struct ImplTexture1D : IFoo
{
    Texture1D<float> t;
    float get() { return 1.0; }
}

// CHECK-DAG: type 'ImplTexture3D' contains fields that cannot be packed into ordinary bytes for dynamic dispatch
export struct ImplTexture3D : IFoo
{
    Texture3D<float> t;
    float get() { return 1.0; }
}

// CHECK-DAG: type 'ImplTextureCube' contains fields that cannot be packed into ordinary bytes for dynamic dispatch
export struct ImplTextureCube : IFoo
{
    TextureCube<float> t;
    float get() { return 1.0; }
}

// CHECK-DAG: type 'ImplRayQuery' contains fields that cannot be packed into ordinary bytes for dynamic dispatch
export struct ImplRayQuery : IFoo
{
    RayQuery<RAY_FLAG_NONE> rq;
    float get() { return 1.0; }
}

// CHECK-DAG: type 'ImplTriangleStream' contains fields that cannot be packed into ordinary bytes for dynamic dispatch
export struct ImplTriangleStream : IFoo
{
    TriangleStream<MyVertex> stream;
    float get() { return 1.0; }
}

// CHECK-DAG: type 'ImplMeshOutput' contains fields that cannot be packed into ordinary bytes for dynamic dispatch
export struct ImplMeshOutput : IFoo
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
