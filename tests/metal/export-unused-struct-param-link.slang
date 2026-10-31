//TEST:COMPILE: tests/metal/export-unused-struct-param-module.slang -o tests/metal/export-unused-struct-param-module.slang-module
//TEST:SIMPLE(filecheck=CHECK): -r tests/metal/export-unused-struct-param-module.slang-module -target metal -entry vmain -stage vertex
//TEST:SIMPLE(filecheck=FRAG): -r tests/metal/export-unused-struct-param-module.slang-module -target metal -entry fmain -stage fragment

// An export from a precompiled module is linked in even when the entry point being compiled never
// calls it, so its reference parameters must still be given the `thread` address space on Metal.
// `helperInout` is declared `extern` here and defined in the module; only `fmain` calls it, as in
// link-time specialization where each stage uses a different subset of a shared module.

// CHECK-DAG: float unusedExport_{{[0-9]+}}(const Payload_{{[0-9]+}} thread* {{[A-Za-z0-9_]+}})
// CHECK-DAG: void helperInout_{{[0-9]+}}(float thread* {{[A-Za-z0-9_]+}})

// FRAG: void helperInout_{{[0-9]+}}(float thread* {{[A-Za-z0-9_]+}})
// FRAG: helperInout_{{[0-9]+}}(&{{[A-Za-z0-9_]+}})

extern void helperInout(inout float x);

[shader("vertex")]
float4 vmain(float3 p: POSITION) : SV_Position
{
    return float4(p, 1);
}

[shader("fragment")]
float4 fmain(float4 c: COLOR) : SV_Target
{
    float v = c.x;
    helperInout(v);
    return float4(v, 0, 0, 1);
}
