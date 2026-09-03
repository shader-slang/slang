// Verify that CUDA functions with internal linkage are emitted as static device functions for both
// ordinary CUDA and OptiX entry points.

// Compile both plain compute and ray tracing entry points.
//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry rayGen -stage raygeneration

// Constructing this struct causes Slang to synthesize an initializer, which should be internal.
struct HelperResult
{
    uint value;
};

// A forward-declared definition should have matching internal linkage on its declaration and body.
uint internalDeclared(uint value);

// Slang public visibility should not imply external linkage in emitted CUDA.
public uint publicDefined(uint value)
{
    return value + 1;
}

// A source-level export should retain external linkage for downstream linking.
export uint downstreamExported(uint value)
{
    return value + 3;
}

// A declaration without a definition may be satisfied by another CUDA module and remains external.
__extern_cpp uint downstreamExtern(uint value);

// An emitted C++ name alone does not export a definition, so this function remains internal.
__extern_cpp uint namedButNotExported(uint value)
{
    return value + 4;
}

// Non-trivial global initialization produces another synthesized function that should be internal.
static uint internalGlobal = publicDefined(1);

uint internalDeclared(uint value)
{
    return publicDefined(value) + 1;
}

HelperResult internalDefined(uint value)
{
    HelperResult result = {internalDeclared(value)};
    return result;
}

// [CudaDeviceExport] explicitly gives a definition external CUDA linkage.
[CudaDeviceExport]
uint exportedDefined(uint value)
{
    return value + 2;
}

// [CudaDeviceExport] also preserves external linkage on a declaration-only function.
[CudaDeviceExport]
uint exportedDeclared(uint value);

RWStructuredBuffer<uint> outputBuffer;

// Exercise the linkage cases through ordinary CUDA source emission.
[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = internalDefined(internalGlobal).value + downstreamExported(1) +
                      downstreamExtern(1) + namedButNotExported(1);
}

// Make the relevant linkage cases reachable from an OptiX entry point as well.
[shader("raygeneration")]
void rayGen()
{
    HelperResult payload = internalDefined(internalGlobal);
    payload.value += downstreamExtern(1) + namedButNotExported(1);
    CallShader(0, payload);
}

// Slang visibility does not imply linkage between generated CUDA modules. Ordinary functions and
// synthesized functions have module-local CUDA linkage, even when they are public to Slang.
// CHECK-DAG: {{^}}static __device__ uint publicDefined
// CHECK-DAG: {{^}}static __device__ uint internalDeclared
// CHECK-DAG: {{^}}static __device__ HelperResult_{{[0-9]+}} HelperResult_x24init
// CHECK-DAG: {{^}}static __device__ HelperResult_{{[0-9]+}} internalDefined
// CHECK-DAG: {{^}}static __device__ uint namedButNotExported
// CHECK-DAG: {{^}}static __device__ uint _S{{[0-9]+}}()

// Exported definitions are deliberately not all reachable from each entry point. Their export
// decorations must both keep them in the emitted CUDA and give them external linkage for downstream
// device linking. Bodyless external declarations also remain externally visible.
// CHECK-DAG: {{^}}__device__ uint downstreamExported
// CHECK-DAG: {{^}}__device__ uint downstreamExtern
// CHECK-DAG: {{^}}__device__ uint exportedDefined
// CHECK-DAG: {{^}}__device__ uint exportedDeclared
