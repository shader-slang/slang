// Regression test for multiple-definition errors when OptiX links separately compiled entry-point
// modules. OptiX requires `--relocatable-device-code=true`, which gives CUDA device functions
// external visibility by default. Verify that generated and user-authored helpers remain local so
// their repeated definitions do not collide across modules.
//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry callableMain -stage callable

// Exercise the synthesized initializer involved in the original duplicate-symbol failure.
struct RayPayload
{
    int x;
    int y;
};

// Exercise an ordinary helper that is emitted into the entry point's CUDA module.
RayPayload makePayload(int x, int y)
{
    RayPayload result = {x, y};
    return result;
}

// Keep both helper definitions reachable from an OptiX entry point.
[shader("callable")]
void callableMain(inout RayPayload payload)
{
    payload = makePayload(1, 2);
}

// Verify that both potentially repeated definitions have module-local CUDA linkage.
// CHECK: static __device__ RayPayload_{{[0-9]+}} RayPayload_x24init
// CHECK: static __device__ RayPayload_{{[0-9]+}} makePayload
