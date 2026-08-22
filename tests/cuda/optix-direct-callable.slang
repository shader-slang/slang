// Verify that OptiX direct-callable entry points are retained in PTX emitted by NVRTC.
//
// NVRTC and NVCC both delete device functions that aren't referenced in
// the translation unit. Because OptiX callables aren't called by name, this
// means that the compiler will delete them if Slang doesn't specify the
// `--relocatable-device-code=true` flag.

//TEST:SIMPLE(filecheck=CHECK-PTX): -target ptx -entry callableMain -stage callable

struct Payload
{
    int value;
};

[shader("callable")]
void callableMain(inout Payload payload)
{
    payload.value = 42;
}

// CHECK-PTX: __direct_callable__callableMain
