// Test TensorView's default (56-byte) and wide (96-byte) layouts, both aligned to eight bytes.

//TEST:REFLECTION(filecheck=CHECK):-stage compute -entry computeMain -target cuda -no-codegen

struct MyParams
{
    TensorView<float> tensor;
    int count;
    TensorView<float, uint64_t> wideTensor;
    int wideCount;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uniform MyParams params)
{
}

// CHECK: "name": "params",
// CHECK: "name": "tensor",
// CHECK: "binding": {"kind": "uniform", "offset": 0, "size": 56
// CHECK: "name": "count",
// CHECK: "binding": {"kind": "uniform", "offset": 56, "size": 4
// CHECK: "name": "wideTensor",
// CHECK: "binding": {"kind": "uniform", "offset": 64, "size": 96
// CHECK: "name": "wideCount",
// CHECK: "binding": {"kind": "uniform", "offset": 160, "size": 4
