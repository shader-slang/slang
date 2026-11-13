//TEST:REFLECTION(filecheck=CHECK):-stage compute -entry computeMain -target cuda -no-codegen

struct Params
{
    DiffTensorView narrow;
    int narrowCount;
    DiffTensorView<float, uint64_t> wide;
    int wideCount;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uniform Params params)
{
}

// CHECK: "name": "params",
// CHECK: "name": "narrow",
// CHECK: "binding": {"kind": "uniform", "offset": 0, "size": 112
// CHECK: "name": "narrowCount",
// CHECK: "binding": {"kind": "uniform", "offset": 112, "size": 4
// CHECK: "name": "wide",
// CHECK: "binding": {"kind": "uniform", "offset": 120, "size": 192
// CHECK: "name": "wideCount",
// CHECK: "binding": {"kind": "uniform", "offset": 312, "size": 4
