// Check that functions with [CudaDeviceExport] are not made "static" in OptiX shaders.

//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry callableMain -stage callable

[shader("callable")]
void callableMain() {}

[CudaDeviceExport]
int someExportedHelper(int x) { return x + 1; }

// CHECK-NOT: static __device__ {{.*}}someExportedHelper
