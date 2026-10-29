//TEST:COMPILE: tests/metal/export-unused-struct-param-module.slang -o tests/metal/export-unused-struct-param-module.slang-module
//TEST:SIMPLE(filecheck=CHECK): -r tests/metal/export-unused-struct-param-module.slang-module -target metal -entry vmain -stage vertex

// An export from a precompiled module is linked in even when the entry point never calls it, so
// its struct parameter must still be given the `thread` address space on Metal.

// CHECK: float unusedExport_{{[0-9]+}}(const Payload_{{[0-9]+}} thread* {{[A-Za-z0-9_]+}})

[shader("vertex")]
float4 vmain(float3 p: POSITION) : SV_Position
{
    return float4(p, 1);
}
