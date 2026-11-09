//TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main -experimental-feature
//TEST:SIMPLE(filecheck=CHECK): -target hgbc -entry main -experimental-feature -obfuscate

public struct Box<T> { T value; }

public extension<T : ITexelElement> RWTexture2D<T>
{
    public struct Marker { public T value; }
}

[shader("compute")]
[numthreads(1, 1, 1)]
void probe<T>() {}

// Keep the type symbolic until specialization, after frontend name-hint stripping.
void forward<T>() { dispatch(probe<T>, uint3(1, 1, 1))(); }

[host]
void main()
{
    forward<StructuredBuffer<Box<float>>>();
    forward<StructuredBuffer<Box<float>, ScalarDataLayout>>();
    forward<RWTexture2D<uint>>();
    forward<RWTexture2D<float4>>();
    forward<Texture2D<float4, 0, 3>>();
    forward<matrix<float, 2, 3, MatrixLayoutMode(1)>>();
    forward<Array<Box<float>, 3>>();
    forward<Box<Tuple<uint, float>>>();
    forward<Box<Tuple<Tuple<uint, bool>, float>>>();
    forward<Box<Tuple<>>>();
    forward<DescriptorHandle<StructuredBuffer<Box<float>>>>();
    forward<RWTexture2D<uint>.Marker>();
    forward<Atomic<uint>>();
    forward<Atomic<int>>();
    forward<RWStructuredBuffer<Atomic<uint>>>();
}

// CHECK-DAG: "StructuredBuffer<Box<float>, DefaultDataLayout>"
// CHECK-DAG: "StructuredBuffer<Box<float>, ScalarDataLayout>"
// CHECK-DAG: "_Texture<uint, __Shape2D, 0, 0, 0, 1, 0, 0, 0>"
// CHECK-DAG: "_Texture<vector<float, 4>, __Shape2D, 0, 0, 0, 1, 0, 0, 0>"
// CHECK-DAG: "_Texture<vector<float, 4>, __Shape2D, 0, 0, 0, 0, 0, 0, 3>"
// CHECK-DAG: "matrix<float, 2, 3, MatrixLayoutMode(1)>"
// CHECK-DAG: "Array<Box<float>, 3>"
// CHECK-DAG: "Box<Tuple<uint, float>>"
// CHECK-DAG: "Box<Tuple<Tuple<uint, bool>, float>>"
// CHECK-DAG: "Box<Tuple<>>"
// CHECK-DAG: "DescriptorHandle<StructuredBuffer<Box<float>, DefaultDataLayout>>"
// CHECK-DAG: "_Texture<uint, __Shape2D, 0, 0, 0, 1, 0, 0, 0>.Marker"
// CHECK-DAG: "Atomic<uint>"
// CHECK-DAG: "Atomic<int>"
// CHECK-DAG: "RWStructuredBuffer<Atomic<uint>, DefaultDataLayout>"
