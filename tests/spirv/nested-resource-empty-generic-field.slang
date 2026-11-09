//TEST:SIMPLE(filecheck=CHECK): -target spirv-assembly -entry computeMain -stage compute

// A complete model can contain more than one nested resource-owning component. Resource
// legalization splits the resources out of the model, while the zero-sized generic activation
// leaves no ordinary storage in `Network<float>`. The physical model must omit that zero-sized
// field: OpTypeStruct does not permit OpTypeVoid as a member type.

public struct Activation<T>
{
}

public struct Network<T>
{
    StructuredBuffer<T> parameters;
    Activation<T> activation;
}

public struct Model
{
    uint2 imageResolution;
    StructuredBuffer<float> hashGrid;
    Network<float> network;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(Model model, RWStructuredBuffer<float> output)
{
    output[0] = model.hashGrid[model.imageResolution.x];
}

// CHECK: OpMemberName [[MODEL:%[A-Za-z0-9_]+]] 0 "imageResolution"
// CHECK-NOT: OpMemberName [[MODEL]] 1
// CHECK: [[MODEL]] = OpTypeStruct %v2uint{{$}}
