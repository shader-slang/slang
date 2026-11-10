// precompiled-spirv-buffer-element-specialization.slang

// Buffer-element-type lowering specializes `readS` for the storage-typed `buf[id.x]` argument
// while this module is precompiled, and keeps the original for the logical `local` argument. The
// specialization loses its linkage through `removeLinkageDecorations`, so it must also lose
// `DownstreamModuleExport`, which `collectMetadata` and the SPIR-V emitter resolve through the
// export name. If it keeps that decoration, precompiling the module crashes.

//TEST:COMPILE: tests/library/precompiled-spirv-buffer-element-specialization.slang -o tests/library/precompiled-spirv-buffer-element-specialization.slang-module -target spirv -embed-downstream-ir -incomplete-library

module "precompiled-spirv-buffer-element-specialization";

public struct S
{
    bool b;
    float3x3 m;
    float x;
}

public RWStructuredBuffer<S> buf;
public RWStructuredBuffer<float> outv;

public float readS(S s)
{
    return s.b ? s.m[0][0] : s.x;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void libMain(uint3 id: SV_DispatchThreadID)
{
    S local = buf[1];
    local.x += 1;
    outv[id.x] = readS(buf[id.x]) + readS(local);
}
